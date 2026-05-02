#pragma once

#include "libraries.h"

namespace platform {

    bool initVoice(std::string& errorMessage);
    void shutdownVoice();

    bool isVoiceAvailable();

    void setVoiceTarget(char* buffer, int bufferSize);
    void clearVoiceTarget();
    bool voiceTargetIs(const char* buffer);
    bool isVoiceListening();

    void pollVoiceEvents();

    const std::string& voiceLastError();

}
