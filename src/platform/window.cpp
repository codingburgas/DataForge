#include "platform/window.h"
#include "imgui.h"
#include <commdlg.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd,
                                                             UINT msg,
                                                             WPARAM wParam,
                                                             LPARAM lParam);

namespace platform {

    static unsigned s_pendingResizeWidth  = 0;
    static unsigned s_pendingResizeHeight = 0;
    static bool     s_hasPendingResize    = false;
    static bool     s_closeRequested      = false;

    static LRESULT CALLBACK wndProc(HWND hWnd, UINT msg,
                                    WPARAM wParam, LPARAM lParam) {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
            return true;
        }
        switch (msg) {
            case WM_SIZE: {
                if (wParam == SIZE_MINIMIZED) {
                    return 0;
                }
                s_pendingResizeWidth  = static_cast<unsigned>(LOWORD(lParam));
                s_pendingResizeHeight = static_cast<unsigned>(HIWORD(lParam));
                s_hasPendingResize    = true;
                return 0;
            }
            case WM_SYSCOMMAND: {
                if ((wParam & 0xfff0) == SC_KEYMENU) {
                    return 0;
                }
                break;
            }
            case WM_CLOSE: {
                s_closeRequested = true;
                return 0;
            }
            case WM_DESTROY: {
                ::PostQuitMessage(0);
                return 0;
            }
            default:
                break;
        }
        return ::DefWindowProcA(hWnd, msg, wParam, lParam);
    }

    bool createMainWindow(PlatformWindow& out,
                          const char* title,
                          int width,
                          int height,
                          std::string& errorMessage) {
        WNDCLASSEXA wc{};
        wc.cbSize        = sizeof(WNDCLASSEXA);
        wc.style         = CS_CLASSDC;
        wc.lpfnWndProc   = wndProc;
        wc.hInstance     = ::GetModuleHandleA(nullptr);
        wc.hIcon         = ::LoadIcon(nullptr, IDI_APPLICATION);
        wc.hCursor       = ::LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wc.lpszClassName = "DataForgeMainWindow";

        if (!::RegisterClassExA(&wc)) {
            DWORD err = ::GetLastError();
            if (err != ERROR_CLASS_ALREADY_EXISTS) {
                std::ostringstream ss;
                ss << "RegisterClassExA failed (error " << err << ")";
                errorMessage = ss.str();
                return false;
            }
        }

        HWND hwnd = ::CreateWindowA(
            wc.lpszClassName,
            title,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            width, height,
            nullptr, nullptr,
            wc.hInstance, nullptr);
        if (hwnd == nullptr) {
            DWORD err = ::GetLastError();
            std::ostringstream ss;
            ss << "CreateWindowA failed (error " << err << ")";
            errorMessage = ss.str();
            return false;
        }
        out.hwnd     = hwnd;
        out.wndClass = wc;
        return true;
    }

    void destroyMainWindow(PlatformWindow& w) {
        if (w.hwnd != nullptr) {
            ::DestroyWindow(w.hwnd);
            w.hwnd = nullptr;
        }
        ::UnregisterClassA(w.wndClass.lpszClassName, w.wndClass.hInstance);
    }

    bool pumpMessages(bool& outQuit) {
        MSG msg;
        while (::PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessageA(&msg);
            if (msg.message == WM_QUIT) {
                outQuit = true;
            }
        }
        return !outQuit;
    }

    bool consumePendingResize(unsigned& outWidth, unsigned& outHeight) {
        if (!s_hasPendingResize) {
            return false;
        }
        outWidth           = s_pendingResizeWidth;
        outHeight          = s_pendingResizeHeight;
        s_hasPendingResize = false;
        return true;
    }

    bool consumeCloseRequest() {
        if (!s_closeRequested) {
            return false;
        }
        s_closeRequested = false;
        return true;
    }

    void postQuit() {
        ::PostQuitMessage(0);
    }

    static bool runFileDialog(HWND owner, std::string& outPath, bool save) {
        char buffer[MAX_PATH];
        buffer[0] = '\0';
        if (!outPath.empty() && outPath.size() < MAX_PATH) {
            std::memcpy(buffer, outPath.data(), outPath.size());
            buffer[outPath.size()] = '\0';
        }
        OPENFILENAMEA ofn{};
        ofn.lStructSize  = sizeof(ofn);
        ofn.hwndOwner    = owner;
        ofn.lpstrFile    = buffer;
        ofn.nMaxFile     = MAX_PATH;
        ofn.lpstrFilter  = "DataForge task files (*.dftasks)\0*.dftasks\0All files (*.*)\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrDefExt  = "dftasks";
        if (save) {
            ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
            if (!::GetSaveFileNameA(&ofn)) {
                return false;
            }
        } else {
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
            if (!::GetOpenFileNameA(&ofn)) {
                return false;
            }
        }
        outPath = std::string(buffer);
        return true;
    }

    bool openFileDialog(HWND owner, std::string& outPath) {
        return runFileDialog(owner, outPath, false);
    }

    bool saveFileDialog(HWND owner, std::string& outPath) {
        return runFileDialog(owner, outPath, true);
    }

}
