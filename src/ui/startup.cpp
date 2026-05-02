#include "libraries.h"
#include "ui/startup.h"
#include "data/types.h"
#include "logic/persistence.h"
#include "logic/tasks.h"
#include "ui/ui_state.h"
#include "ui/app_shell.h"
#include "ui/theme.h"
#include "ui/toast.h"
#include "platform/window.h"
#include "platform/renderer.h"
#include "platform/voice.h"

namespace {

    const char* DEFAULT_SAMPLE_PATH = "Resources/sample_tasks.dftasks";

    void tryAutoloadSample(data::TaskStore& store, ui::UiState& uiState) {
        std::ifstream probe(DEFAULT_SAMPLE_PATH, std::ios::binary);
        if (!probe) {
            return;
        }
        probe.close();
        std::string err;
        if (logic::loadStore(store, DEFAULT_SAMPLE_PATH, err)) {
            ui::pushToast(uiState, "Loaded sample tasks.");
        } else {
            ui::pushToast(uiState, "Sample load failed: " + err);
        }
    }

    void handleFileFlags(data::TaskStore& store,
                         ui::UiState& uiState,
                         HWND owner) {
        if (uiState.triggeredNew) {
            uiState.triggeredNew = false;
            if (store.dirty) {
                ui::pushToast(uiState, "Save before starting a new store.");
            } else {
                store = logic::createEmptyStore();
                uiState.selectedTaskId = -1;
                ui::pushToast(uiState, "New store.");
            }
        }
        if (uiState.triggeredOpen) {
            uiState.triggeredOpen = false;
            std::string path;
            if (platform::openFileDialog(owner, path)) {
                std::string err;
                if (logic::loadStore(store, path, err)) {
                    uiState.selectedTaskId  = -1;
                    uiState.hasUndoSnapshot = false;
                    ui::pushToast(uiState, "Opened: " + path);
                } else {
                    ui::pushToast(uiState, "Open failed: " + err);
                }
            }
        }
        if (uiState.triggeredOpenPath) {
            uiState.triggeredOpenPath = false;
            std::string path = uiState.pendingFilePath;
            uiState.pendingFilePath.clear();
            if (!path.empty()) {
                if (store.dirty) {
                    ui::pushToast(uiState, "Save current store before loading another file.");
                } else {
                    std::string err;
                    if (logic::loadStore(store, path, err)) {
                        uiState.selectedTaskId  = -1;
                        uiState.hasUndoSnapshot = false;
                        ui::pushToast(uiState, "Opened: " + path);
                    } else {
                        ui::pushToast(uiState, "Open failed: " + err);
                    }
                }
            }
        }
        if (uiState.triggeredSave) {
            uiState.triggeredSave = false;
            if (store.filePath.empty()) {
                uiState.triggeredSaveAs = true;
            } else {
                std::string err;
                if (logic::saveStore(store, store.filePath, err)) {
                    ui::pushToast(uiState, "Saved.");
                } else {
                    ui::pushToast(uiState, "Save failed: " + err);
                }
            }
        }
        if (uiState.triggeredSaveAs) {
            uiState.triggeredSaveAs = false;
            std::string path = store.filePath;
            if (platform::saveFileDialog(owner, path)) {
                std::string err;
                if (logic::saveStore(store, path, err)) {
                    ui::pushToast(uiState, "Saved: " + path);
                } else {
                    ui::pushToast(uiState, "Save failed: " + err);
                }
            }
        }
    }

    void handleQuitRequest(data::TaskStore& store, ui::UiState& uiState) {
        if (platform::consumeCloseRequest()) {
            uiState.requestedQuit = true;
        }
        if (uiState.requestedQuit) {
            if (store.dirty && !uiState.showDirtyExitConfirm) {
                uiState.showDirtyExitConfirm = true;
                uiState.requestedQuit        = false;
            } else if (!store.dirty) {
                platform::postQuit();
            }
        }
    }

}

namespace ui {

    int run() {

        platform::PlatformWindow window{};
        std::string err;
        if (!platform::createMainWindow(window,
                                        "DataForge - Task Manager",
                                        1280, 800,
                                        err)) {
            ::MessageBoxA(nullptr, err.c_str(),
                          "DataForge", MB_ICONERROR | MB_OK);
            return 1;
        }

        platform::Renderer renderer{};
        if (!platform::initRenderer(renderer, window.hwnd, err)) {
            ::MessageBoxA(window.hwnd, err.c_str(),
                          "DataForge", MB_ICONERROR | MB_OK);
            platform::destroyMainWindow(window);
            return 2;
        }

        ::ShowWindow(window.hwnd, SW_SHOWDEFAULT);
        ::UpdateWindow(window.hwnd);

        if (!platform::initImGui(renderer, window.hwnd)) {
            ::MessageBoxA(window.hwnd, "Failed to initialise ImGui.",
                          "DataForge", MB_ICONERROR | MB_OK);
            platform::shutdownRenderer(renderer);
            platform::destroyMainWindow(window);
            return 3;
        }

        ui::loadFonts();
        ui::applyTheme();

        data::TaskStore store = logic::createEmptyStore();
        ui::UiState uiState{};
        ui::initUiState(uiState);

        tryAutoloadSample(store, uiState);

        std::string voiceErr;
        if (!platform::initVoice(voiceErr)) {
            ui::pushToast(uiState, "Voice input disabled: " + voiceErr);
        }

        bool quit = false;
        while (!quit) {
            if (!platform::pumpMessages(quit)) {
                break;
            }

            unsigned rw = 0;
            unsigned rh = 0;
            if (platform::consumePendingResize(rw, rh)) {
                platform::handleResize(renderer, rw, rh);
            }

            platform::pollVoiceEvents();

            handleFileFlags(store, uiState, window.hwnd);
            handleQuitRequest(store, uiState);

            platform::imGuiNewFrame();
            ui::renderApp(store, uiState);

            renderer.clearColor[0] = ui::ColBgBase.x;
            renderer.clearColor[1] = ui::ColBgBase.y;
            renderer.clearColor[2] = ui::ColBgBase.z;
            renderer.clearColor[3] = ui::ColBgBase.w;

            platform::beginFrame(renderer);
            platform::imGuiRender();
            platform::presentFrame(renderer, true);
        }

        platform::shutdownVoice();
        platform::shutdownImGui();
        platform::shutdownRenderer(renderer);
        platform::destroyMainWindow(window);
        return 0;
    }

}
