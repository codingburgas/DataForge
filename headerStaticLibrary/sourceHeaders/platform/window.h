#pragma once

#include "libraries.h"

namespace platform {

    struct PlatformWindow {
        HWND       hwnd;
        WNDCLASSEXA wndClass;
        std::wstring title;
    };

    bool createMainWindow(PlatformWindow& out,
                          const char* title,
                          int width,
                          int height,
                          std::string& errorMessage);

    void destroyMainWindow(PlatformWindow& w);

    bool pumpMessages(bool& outQuit);

    bool consumePendingResize(unsigned& outWidth, unsigned& outHeight);
    bool consumeCloseRequest();
    void postQuit();

    bool openFileDialog(HWND owner, std::string& outPath);
    bool saveFileDialog(HWND owner, std::string& outPath);

}
