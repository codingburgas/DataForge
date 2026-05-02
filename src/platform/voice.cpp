#include "platform/voice.h"

#include <sapi.h>

namespace platform {

    namespace {

        bool             s_initialized = false;
        bool             s_comInited   = false;
        ISpRecognizer*   s_recognizer  = nullptr;
        ISpRecoContext*  s_context     = nullptr;
        ISpRecoGrammar*  s_grammar     = nullptr;
        char*            s_targetBuf   = nullptr;
        int              s_targetSize  = 0;
        std::string      s_lastError;

        void releaseEventParam(SPEVENT& ev) {
            if (ev.lParam == 0) {
                return;
            }
            switch (ev.elParamType) {
                case SPET_LPARAM_IS_OBJECT:
                    reinterpret_cast<IUnknown*>(ev.lParam)->Release();
                    break;
                case SPET_LPARAM_IS_POINTER:
                case SPET_LPARAM_IS_STRING:
                    ::CoTaskMemFree(reinterpret_cast<void*>(ev.lParam));
                    break;
                default:
                    break;
            }
            ev.lParam = 0;
        }

        void appendUtf8(char* buf, int size, const std::string& utf8) {
            int len = static_cast<int>(std::strlen(buf));
            int avail = size - len - 1;
            if (avail <= 0) {
                return;
            }
            bool needSpace = len > 0 && buf[len - 1] != ' ' && buf[len - 1] != '\n';
            int sp = needSpace ? 1 : 0;
            int copy = static_cast<int>(utf8.size());
            if (copy + sp > avail) {
                copy = avail - sp;
            }
            if (copy <= 0) {
                return;
            }
            if (sp) {
                buf[len++] = ' ';
            }
            std::memcpy(buf + len, utf8.data(), copy);
            buf[len + copy] = '\0';
        }

        std::string wideToUtf8(LPCWSTR text) {
            if (text == nullptr) {
                return std::string();
            }
            int needed = ::WideCharToMultiByte(CP_UTF8, 0, text, -1,
                                               nullptr, 0, nullptr, nullptr);
            if (needed <= 1) {
                return std::string();
            }
            std::string out(needed - 1, '\0');
            ::WideCharToMultiByte(CP_UTF8, 0, text, -1,
                                  out.data(), needed, nullptr, nullptr);
            return out;
        }

    }

    bool initVoice(std::string& errorMessage) {
        if (s_initialized) {
            return true;
        }

        HRESULT hr = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        if (hr == S_OK || hr == S_FALSE) {
            s_comInited = true;
        } else if (hr != RPC_E_CHANGED_MODE) {
            errorMessage = "CoInitializeEx failed";
            s_lastError  = errorMessage;
            return false;
        }

        hr = ::CoCreateInstance(__uuidof(SpSharedRecognizer),
                                nullptr, CLSCTX_ALL,
                                __uuidof(ISpRecognizer),
                                reinterpret_cast<void**>(&s_recognizer));
        if (FAILED(hr) || s_recognizer == nullptr) {
            errorMessage = "Speech recognizer unavailable";
            s_lastError  = errorMessage;
            shutdownVoice();
            return false;
        }

        hr = s_recognizer->CreateRecoContext(&s_context);
        if (FAILED(hr) || s_context == nullptr) {
            errorMessage = "CreateRecoContext failed";
            s_lastError  = errorMessage;
            shutdownVoice();
            return false;
        }

        ULONGLONG interest = SPFEI(SPEI_RECOGNITION);
        s_context->SetInterest(interest, interest);

        hr = s_context->CreateGrammar(1, &s_grammar);
        if (FAILED(hr) || s_grammar == nullptr) {
            errorMessage = "CreateGrammar failed";
            s_lastError  = errorMessage;
            shutdownVoice();
            return false;
        }

        hr = s_grammar->LoadDictation(nullptr, SPLO_STATIC);
        if (FAILED(hr)) {
            errorMessage = "LoadDictation failed";
            s_lastError  = errorMessage;
            shutdownVoice();
            return false;
        }

        s_grammar->SetDictationState(SPRS_INACTIVE);
        s_initialized = true;
        s_lastError.clear();
        return true;
    }

    void shutdownVoice() {
        if (s_grammar) {
            s_grammar->SetDictationState(SPRS_INACTIVE);
            s_grammar->Release();
            s_grammar = nullptr;
        }
        if (s_context) {
            s_context->Release();
            s_context = nullptr;
        }
        if (s_recognizer) {
            s_recognizer->Release();
            s_recognizer = nullptr;
        }
        s_targetBuf  = nullptr;
        s_targetSize = 0;
        if (s_comInited) {
            ::CoUninitialize();
            s_comInited = false;
        }
        s_initialized = false;
    }

    bool isVoiceAvailable() {
        return s_initialized;
    }

    void setVoiceTarget(char* buffer, int bufferSize) {
        s_targetBuf  = buffer;
        s_targetSize = bufferSize;
        if (s_grammar) {
            SPEVENT ev{};
            ULONG fetched = 0;
            while (s_context && s_context->GetEvents(1, &ev, &fetched) == S_OK && fetched > 0) {
                releaseEventParam(ev);
                fetched = 0;
            }
            s_grammar->SetDictationState(SPRS_ACTIVE);
        }
    }

    void clearVoiceTarget() {
        s_targetBuf  = nullptr;
        s_targetSize = 0;
        if (s_grammar) {
            s_grammar->SetDictationState(SPRS_INACTIVE);
        }
    }

    bool voiceTargetIs(const char* buffer) {
        return s_targetBuf != nullptr && s_targetBuf == buffer;
    }

    bool isVoiceListening() {
        return s_targetBuf != nullptr;
    }

    void pollVoiceEvents() {
        if (!s_initialized || s_context == nullptr || s_targetBuf == nullptr) {
            return;
        }
        SPEVENT ev{};
        ULONG fetched = 0;
        while (s_context->GetEvents(1, &ev, &fetched) == S_OK && fetched > 0) {
            if (ev.eEventId == SPEI_RECOGNITION && ev.lParam != 0) {
                ISpRecoResult* result = reinterpret_cast<ISpRecoResult*>(ev.lParam);
                LPWSTR text = nullptr;
                HRESULT hr = result->GetText(SP_GETWHOLEPHRASE,
                                             SP_GETWHOLEPHRASE,
                                             TRUE, &text, nullptr);
                if (SUCCEEDED(hr) && text != nullptr) {
                    std::string utf8 = wideToUtf8(text);
                    if (!utf8.empty() && s_targetBuf != nullptr) {
                        appendUtf8(s_targetBuf, s_targetSize, utf8);
                    }
                    ::CoTaskMemFree(text);
                }
            }
            releaseEventParam(ev);
            fetched = 0;
        }
    }

    const std::string& voiceLastError() {
        return s_lastError;
    }

}
