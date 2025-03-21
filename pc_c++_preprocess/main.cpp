#include <iostream>
#include <vlc/vlc.h>
#include <windows.h>

#define VIDEO_PATH "D:\\Tiedostot\\fina a way or make on cover.mp4"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int main() {    
    libvlc_instance_t* vlcInstance = libvlc_new(0, nullptr);
    if (!vlcInstance) {
        std::cerr << "Failed to initialize libVLC" << std::endl;
        return -1;
    }
    std::cout << "VLC Initialized!" << std::endl;
    
    libvlc_media_t* media = libvlc_media_new_path(vlcInstance, VIDEO_PATH);
    libvlc_media_player_t* mediaPlayer = libvlc_media_player_new_from_media(media);
    libvlc_media_release(media);
    libvlc_media_player_play(mediaPlayer);

    libvlc_release(vlcInstance);

    return 0;
}