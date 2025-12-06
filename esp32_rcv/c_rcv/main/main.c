#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/rmt_tx.h"
#include "esp_netif.h"

static const char *TAG = "LED_SERVER";

// ---------------- WiFi configuration ----------------
#define WIFI_SSID   "TP-Link_2.4"
#define WIFI_PASS   "001EAB040046"

#define STATIC_IP   "192.168.1.124"
#define GATEWAY_IP  "192.168.1.1"
#define NETMASK_IP  "255.255.255.0"
#define DNS_IP      "8.8.8.8"

// ---------------- LED config ----------------
#define LED_SEGMENTS 20
#define UDP_PORT     55555

#define STATUS_LED_PIN 2
#define STATUS_LED_ACTIVE_HIGH true

// Strip config (same three strips as Python)
typedef struct {
    int count;
    int pin;
    bool rgb;
    bool reverse;
    float brightness;
} Strip;

//Strip strips[] = {
//    {100, 12, true, false, 1.0},
//    {100, 12, true, false, 1.0},
//    {100, 12, true, false, 1.0}
//};

Strip strips[] = {
    {360, 12, false, true, 1.0}
};

#define NUM_STRIPS (sizeof(strips)/sizeof(Strip))

// Combined LED array
int total_leds = 0;
uint8_t *led_buffer;

// RMT LED transmitter object
rmt_channel_handle_t led_chan = NULL;
rmt_encoder_handle_t ws2812_encoder = NULL;

// ---------------- Status LED helpers ----------------
static inline void led_on() {
    gpio_set_level(STATUS_LED_PIN, STATUS_LED_ACTIVE_HIGH ? 1 : 0);
}
static inline void led_off() {
    gpio_set_level(STATUS_LED_PIN, STATUS_LED_ACTIVE_HIGH ? 0 : 1);
}
static inline void led_toggle() {
    gpio_set_level(STATUS_LED_PIN, !gpio_get_level(STATUS_LED_PIN));
}

// ---------------- WiFi connection ----------------
static void wifi_init() {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    esp_netif_t *sta = esp_netif_create_default_wifi_sta();

    // --- Static IP setup ---
    esp_netif_ip_info_t ip_info;
    inet_pton(AF_INET, STATIC_IP, &ip_info.ip);
    inet_pton(AF_INET, GATEWAY_IP, &ip_info.gw);
    inet_pton(AF_INET, NETMASK_IP, &ip_info.netmask);
    ESP_ERROR_CHECK(esp_netif_dhcpc_stop(sta));
    ESP_ERROR_CHECK(esp_netif_set_ip_info(sta, &ip_info));

    // DNS
    esp_netif_dns_info_t dns;
    inet_pton(AF_INET, DNS_IP, &dns.ip.u_addr.ip4);
    dns.ip.type = ESP_IPADDR_TYPE_V4;
    esp_netif_set_dns_info(sta, ESP_NETIF_DNS_MAIN, &dns);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    wifi_config_t sta_cfg = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        }
    };
    strcpy((char*)sta_cfg.sta.ssid, WIFI_SSID);
    strcpy((char*)sta_cfg.sta.password, WIFI_PASS);

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_cfg));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());

    ESP_LOGI(TAG, "Connecting to WiFi...");

    // Blink LED while connecting
    while (true) {
        wifi_ap_record_t info;
        if (esp_wifi_sta_get_ap_info(&info) == ESP_OK) break;
        led_toggle();
        vTaskDelay(pdMS_TO_TICKS(400));
    }

    led_on();
    ESP_LOGI(TAG, "WiFi connected!");
}

// ---------------- WS2812 LED Setup ----------------
static void led_init() {
    for (int i = 0; i < NUM_STRIPS; i++)
        total_leds += strips[i].count;
 
    // Use heap_caps_malloc for better memory management
    led_buffer = heap_caps_malloc(total_leds * 3, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    if (!led_buffer) {
        ESP_LOGE(TAG, "Failed to allocate LED buffer!");
        return;
    }
    memset(led_buffer, 0, total_leds * 3); // Initialize to 0
 
    // RMT Configuration (use all 8 memory blocks = 512 symbols)
    rmt_tx_channel_config_t tx_conf = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .gpio_num = strips[0].pin,
        .mem_block_symbols = 64, // Default block size
        .resolution_hz = 10 * 1000 * 1000, // 10 MHz
        .trans_queue_depth = 4,
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_conf, &led_chan));
    ESP_ERROR_CHECK(rmt_enable(led_chan));
 
    // Adjusted WS2812 timings (integer ticks for 10 MHz)
    rmt_bytes_encoder_config_t enc_cfg = {
        .bit0 = {.level0 = 1, .duration0 = 4, .level1 = 0, .duration1 = 8}, // 400ns, 800ns
        .bit1 = {.level0 = 1, .duration0 = 7, .level1 = 0, .duration1 = 6}, // 700ns, 600ns
        .flags = {.msb_first = 1}
    };
    ESP_ERROR_CHECK(rmt_new_bytes_encoder(&enc_cfg, &ws2812_encoder));
 
    ESP_LOGI(TAG, "LEDs initialized: %d total", total_leds);
}

static inline uint8_t clamp(int v) {
    return v < 0 ? 0 : (v > 255 ? 255 : v);
}

// ---------------- Apply segment → LED buffer ----------------
static void apply_leds(int *data, Strip *st, int strip_index) {
    int leds_per_segment = st->count / LED_SEGMENTS;
    int skip = 0;
    for (int i = 0; i < strip_index; i++)
        skip += strips[i].count;

    int j = st->reverse ? (LED_SEGMENTS - 1) * 3 : 0;
    int step = st->reverse ? -3 : 3;

    for (int seg = 0; seg < LED_SEGMENTS; seg++) {
        int r = data[j], g = data[j+1], b = data[j+2];

        // Color boost/cutoff
        r = clamp(r > 160 ? r + 20 : r - 20);
        g = clamp(g > 160 ? g + 20 : g - 20);
        b = clamp(b > 160 ? b + 20 : b - 20);

        for (int l = 0; l < leds_per_segment; l++) {
            int pos = skip + seg * leds_per_segment + l;
            if (pos >= total_leds) continue;
            //ESP_LOGI(TAG, "%d:%d:%d, %d, %d, %d", strip_index, seg, pos, r, g, b);
            int idx = pos * 3;
            if (st->rgb) {
                led_buffer[idx] = r;
                led_buffer[idx+1] = g;
                led_buffer[idx+2] = b;
            } else {
                led_buffer[idx] = g;
                led_buffer[idx+1] = r;
                led_buffer[idx+2] = b;
            }
        }
        j += step;
    }
}

// ---------------- Push buffer to WS2812 ----------------
static void write_leds() {
    rmt_transmit_config_t tconf = { .loop_count = 0 };
    ESP_ERROR_CHECK(rmt_transmit(
        led_chan,
        ws2812_encoder,
        led_buffer,
        total_leds * 3,
        &tconf
    ));
    rmt_tx_wait_all_done(led_chan, portMAX_DELAY);
}

// ---------------- UDP server task ----------------
void udp_server_task(void *arg)
{
    int sock = -1;
    struct sockaddr_in addr;

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(UDP_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        ESP_LOGE(TAG, "Socket bind failed: errno %d", errno);
        close(sock);
        vTaskDelete(NULL);
        return;
    }

    // Make non-blocking
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    ESP_LOGI(TAG, "UDP server listening on %d", UDP_PORT);

    // Allocate large buffers on heap (once)
    const size_t RECV_BUF_SZ = 2048;
    char *buf = (char*)heap_caps_malloc(RECV_BUF_SZ + 1, MALLOC_CAP_8BIT);
    if (!buf) {
        ESP_LOGE(TAG, "Failed to allocate recv buffer");
        close(sock);
        vTaskDelete(NULL);
        return;
    }

    int *values = (int*)heap_caps_malloc(sizeof(int) * LED_SEGMENTS * 3, MALLOC_CAP_8BIT);
    if (!values) {
        ESP_LOGE(TAG, "Failed to allocate values array");
        heap_caps_free(buf);
        close(sock);
        vTaskDelete(NULL);
        return;
    }

    // Parts pointer array (pointers into buf) - small, keep on stack
    char *parts[LED_SEGMENTS];
    int received_count = 0;

    while (1) {
        ssize_t len = recv(sock, buf, RECV_BUF_SZ, 0);
        if (len <= 0) {
            // no data, or error
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        //ESP_LOGI(TAG, "Applying leds %s", buf);

        // null-terminate safely
        if ((size_t)len >= RECV_BUF_SZ) len = RECV_BUF_SZ - 1;
        buf[len] = '\0';

        // split by ';' into parts[], up to LED_SEGMENTS parts
        int pc = 0;
        char *saveptr1 = NULL;
        char *token = strtok_r(buf, ";", &saveptr1);
        while (token && pc < LED_SEGMENTS) {
            // trim leading/trailing whitespace (optional)
            // store pointer
            parts[pc++] = token;
            token = strtok_r(NULL, ";", &saveptr1);
        }

        // reverse order and parse comma-separated ints into values[]
        int count = 0;
        for (int pi = pc - 1; pi >= 0 && count < LED_SEGMENTS * 3; pi--) {
            char *saveptr2 = NULL;
            char *valtok = strtok_r(parts[pi], ",", &saveptr2);
            while (valtok && count < LED_SEGMENTS * 3) {
                // skip whitespace
                while (*valtok == ' ' || *valtok == '\t') ++valtok;
                if (*valtok) {
                    values[count++] = atoi(valtok);
                }
                valtok = strtok_r(NULL, ",", &saveptr2);
            }
        }

        if (count != LED_SEGMENTS * 3) {
            // malformed packet — ignore
            continue;
        }

        // apply to strips (same apply_leds function you already have)
        for (int s = 0; s < NUM_STRIPS; s++) {
            apply_leds(values, &strips[s], s);
        }

        // write out to LEDs
        write_leds();
    }

    // unreachable in current loop, but cleanup if needed
    heap_caps_free(buf);
    heap_caps_free(values);
    close(sock);
    vTaskDelete(NULL);
}

void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());

    gpio_reset_pin(STATUS_LED_PIN);
    gpio_set_direction(STATUS_LED_PIN, GPIO_MODE_OUTPUT);
    led_off();

    wifi_init();
    led_init();

    xTaskCreate(udp_server_task, "udp_server", 4096, NULL, 5, NULL);
    
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));   // Keep main alive
    }
}