#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <thread>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include "ws2811/ws2811.h"

#define HOST "0.0.0.0"
#define PORT 65432
#define LED_SEGMENTS 20

struct Strip {
    int LED_COUNT;
    int LED_FREQ_HZ;
    int LED_DMA;
    bool LED_INVERT;
    int LED_BRIGHTNESS;
    int LED_CHANNEL;
    bool RGB;      // TRUE == RGB, FALSE == BGR
    bool REVERSE;  // Mirror order
};

Strip strip_dense = {287, 800000, 10, false, 255, 0, false, true};
Strip strip_old   = {185, 800000, 10, false, 255, 0, false, false};
std::vector<Strip> strips = {strip_dense, strip_old};

ws2811_t ledstring = {
    .freq = WS2811_TARGET_FREQ,
    .dmanum = 10,
    .channel = {
        [0] = {
            .gpionum = 12,
            .invert = 0,
            .count = 0,  // filled later
            .strip_type = WS2811_STRIP_RGB,
            .brightness = 255,
        },
        [1] = {0}
    }
};


void LEDS(const std::vector<int> &data, const Strip &st, int strip_index) {
    int j = st.REVERSE ? (data.size() - 3) : 0;
    int leds_per_segment = st.LED_COUNT / LED_SEGMENTS;

    // Offset for second strip
    int skip_leds = 0;
    for (int i = 0; i < strip_index; i++)
        skip_leds += strips[i].LED_COUNT;

    for (int k = 0; k < LED_SEGMENTS; k++) {
        int r = data[j], g = data[j+1], b = data[j+2];

        // Dark cutoff & boost
        r = (r > 160) ? r + 20 : r - 20;
        g = (g > 160) ? g + 20 : g - 20;
        b = (b > 160) ? b + 20 : b - 20;

        r = std::max(r, 0); g = std::max(g, 0); b = std::max(b, 0);

        for (int l = 0; l < leds_per_segment; l++) {
            int index = skip_leds + k * leds_per_segment + l;
            uint32_t color;
            if (st.RGB)
                color = ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
            else
                color = ((uint32_t)g << 16) | ((uint32_t)b << 8) | r;

            ledstring.channel[0].leds[index] = color;
        }

        j += (st.REVERSE ? -3 : 3);
    }
}


int main() {
    int total_leds = 0;
    for (auto &st : strips) total_leds += st.LED_COUNT;
    ledstring.channel[0].count = total_leds;

    if (ws2811_init(&ledstring) != WS2811_SUCCESS) {
        std::cerr << "WS2811 init failed\n";
        return -1;
    }

    // Setup UDP socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return -1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(sock);
        return -1;
    }

    char buffer[2048];
    while (true) {
        sockaddr_in sender{};
        socklen_t sender_len = sizeof(sender);
        int n;
        std::string latest;
        // Drain socket buffer, keep only newest packet (in case we're sending too much data)
        while ( (n = recvfrom(sock, buffer, sizeof(buffer)-1, MSG_DONTWAIT,
                (struct sockaddr*)&sender, &sender_len)) > 0 ) {
            buffer[n] = '\0';
            latest.assign(buffer, n);
        }

        if (latest.empty()) {continue;}

        buffer[n] = '\0';
        std::string received(buffer);

        // Split by ';'
        std::vector<std::string> parts;
        std::stringstream ss(received);
        std::string item;
        while (std::getline(ss, item, ';')) {
            if (!item.empty()) parts.push_back(item);
        }
        std::reverse(parts.begin(), parts.end());

        std::vector<int> ledifo;
        for (auto &p : parts) {
            std::stringstream sp(p);
            std::string val;
            while (std::getline(sp, val, ',')) {
                if (!val.empty()) ledifo.push_back(std::stoi(val));
            }
        }

        if (ledifo.size() == LED_SEGMENTS * 3) {
            for (size_t i = 0; i < strips.size(); i++) {
                LEDS(ledifo, strips[i], i);
            }
            ws2811_render(&ledstring);
        }
    }

    ws2811_fini(&ledstring);
    close(sock);
    return 0;
}
