#include "platform/renderer.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

namespace platform {

    static bool createRenderTarget(Renderer& r) {
        ID3D11Texture2D* backBuffer = nullptr;
        HRESULT hr = r.swapChain->GetBuffer(
            0, __uuidof(ID3D11Texture2D),
            reinterpret_cast<void**>(&backBuffer));
        if (FAILED(hr) || backBuffer == nullptr) {
            return false;
        }
        hr = r.device->CreateRenderTargetView(
            backBuffer, nullptr, &r.renderTargetView);
        backBuffer->Release();
        return SUCCEEDED(hr);
    }

    static void destroyRenderTarget(Renderer& r) {
        if (r.renderTargetView != nullptr) {
            r.renderTargetView->Release();
            r.renderTargetView = nullptr;
        }
    }

    bool initRenderer(Renderer& r, HWND hwnd, std::string& errorMessage) {
        r.device           = nullptr;
        r.context          = nullptr;
        r.swapChain        = nullptr;
        r.renderTargetView = nullptr;
        r.clearColor[0] = 0.08f;
        r.clearColor[1] = 0.09f;
        r.clearColor[2] = 0.11f;
        r.clearColor[3] = 1.0f;

        DXGI_SWAP_CHAIN_DESC desc{};
        desc.BufferCount                        = 2;
        desc.BufferDesc.Width                   = 0;
        desc.BufferDesc.Height                  = 0;
        desc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.BufferDesc.RefreshRate.Numerator   = 60;
        desc.BufferDesc.RefreshRate.Denominator = 1;
        desc.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        desc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.OutputWindow                       = hwnd;
        desc.SampleDesc.Count                   = 1;
        desc.SampleDesc.Quality                 = 0;
        desc.Windowed                           = TRUE;
        desc.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;

        UINT flags = 0;
    #ifdef _DEBUG
        flags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif
        D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_0
        };
        D3D_FEATURE_LEVEL actualLevel;
        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
            flags,
            featureLevels, 2,
            D3D11_SDK_VERSION,
            &desc,
            &r.swapChain,
            &r.device,
            &actualLevel,
            &r.context);
        if (hr == DXGI_ERROR_UNSUPPORTED) {
            hr = D3D11CreateDeviceAndSwapChain(
                nullptr, D3D_DRIVER_TYPE_WARP, nullptr,
                flags,
                featureLevels, 2,
                D3D11_SDK_VERSION,
                &desc,
                &r.swapChain,
                &r.device,
                &actualLevel,
                &r.context);
        }
        if (FAILED(hr)) {
            std::ostringstream ss;
            ss << "D3D11CreateDeviceAndSwapChain failed (hr=0x"
               << std::hex << hr << ")";
            errorMessage = ss.str();
            return false;
        }
        if (!createRenderTarget(r)) {
            errorMessage = "failed to create initial render target view";
            return false;
        }
        return true;
    }

    void shutdownRenderer(Renderer& r) {
        destroyRenderTarget(r);
        if (r.swapChain != nullptr) {
            r.swapChain->Release();
            r.swapChain = nullptr;
        }
        if (r.context != nullptr) {
            r.context->Release();
            r.context = nullptr;
        }
        if (r.device != nullptr) {
            r.device->Release();
            r.device = nullptr;
        }
    }

    void handleResize(Renderer& r, unsigned width, unsigned height) {
        if (r.swapChain == nullptr || width == 0 || height == 0) {
            return;
        }
        destroyRenderTarget(r);
        r.swapChain->ResizeBuffers(
            0, width, height,
            DXGI_FORMAT_UNKNOWN,
            DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
        createRenderTarget(r);
    }

    void beginFrame(Renderer& r) {
        if (r.context == nullptr || r.renderTargetView == nullptr) {
            return;
        }
        r.context->OMSetRenderTargets(1, &r.renderTargetView, nullptr);
        r.context->ClearRenderTargetView(r.renderTargetView, r.clearColor);
    }

    void presentFrame(Renderer& r, bool vsync) {
        if (r.swapChain == nullptr) {
            return;
        }
        r.swapChain->Present(vsync ? 1 : 0, 0);
    }

    bool initImGui(Renderer& r, HWND hwnd) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        if (!ImGui_ImplWin32_Init(hwnd)) {
            return false;
        }
        if (!ImGui_ImplDX11_Init(r.device, r.context)) {
            ImGui_ImplWin32_Shutdown();
            return false;
        }
        return true;
    }

    void shutdownImGui() {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    void imGuiNewFrame() {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    void imGuiRender() {
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

}
