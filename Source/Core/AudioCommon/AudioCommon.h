// Copyright 2009 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "AudioCommon/SoundStream.h"

class Mixer;

extern std::unique_ptr<SoundStream> g_sound_stream;

namespace AudioCommon
{
void InitSoundStream();
void ShutdownSoundStream();
std::string GetDefaultSoundBackend();
std::vector<std::string> GetSoundBackends();
void UpdateSoundStream();
void SetSoundStreamRunning(bool running);
void SendAIBuffer(const short* samples, unsigned int num_samples);
}
