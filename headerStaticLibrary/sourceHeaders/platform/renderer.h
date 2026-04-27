#pragma once

#include "libraries.h"

namespace platform {

    struct Renderer {
        ID3D11Device*           device;
        ID3D11DeviceContext*    context;
        IDXGISwapChain*         swapChain;
        ID3D11RenderTargetView* renderTargetView;
        float                   clearColor[4];
    };

    bool initRenderer(Renderer& r, HWND hwnd, std::string& errorMessage);
    void shutdownRenderer(Renderer& r);
    void handleResize(Renderer& r, unsigned width, unsigned height);
    void beginFrame(Renderer& r);
    void presentFrame(Renderer& r, bool vsync);

    bool initImGui(Renderer& r, HWND hwnd);
    void shutdownImGui();
    void imGuiNewFrame();
    void imGuiRender();

}
