// Copyright 2009 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "AudioCommon/AudioCommon.h"
#include "AudioCommon/Mixer.h"
#include "AudioCommon/NullSoundStream.h"
#include "Common/Common.h"
#include "Common/FileUtil.h"
#include "Common/Logging/Log.h"
#include "Core/ConfigManager.h"

// This shouldn't be a global, at least not here.
std::unique_ptr<SoundStream> g_sound_stream;

static bool s_sound_stream_running = false;

namespace AudioCommon
{
static const int AUDIO_VOLUME_MIN = 0;
static const int AUDIO_VOLUME_MAX = 100;

void InitSoundStream()
{
  std::string backend = SConfig::GetInstance().sBackend;
  if (backend == BACKEND_NULLSOUND)
    g_sound_stream = std::make_unique<NullSound>();

  if (!g_sound_stream || !g_sound_stream->Init())
  {
    WARN_LOG(AUDIO, "Could not initialize backend %s, using %s instead.", backend.c_str(),
             BACKEND_NULLSOUND);
    g_sound_stream = std::make_unique<NullSound>();
  }

  UpdateSoundStream();
  SetSoundStreamRunning(true);
}

void ShutdownSoundStream()
{
  INFO_LOG(AUDIO, "Shutting down sound stream");

  SetSoundStreamRunning(false);
  g_sound_stream.reset();

  INFO_LOG(AUDIO, "Done shutting down sound stream");
}

std::string GetDefaultSoundBackend()
{
  std::string backend = BACKEND_NULLSOUND;
  return backend;
}

std::vector<std::string> GetSoundBackends()
{
  std::vector<std::string> backends;

  backends.push_back(BACKEND_NULLSOUND);

  return backends;
}

void UpdateSoundStream()
{
  if (g_sound_stream)
  {
    int volume = SConfig::GetInstance().m_IsMuted ? 0 : SConfig::GetInstance().m_Volume;
    g_sound_stream->SetVolume(volume);
  }
}

void SetSoundStreamRunning(bool running)
{
  if (!g_sound_stream)
    return;

  if (s_sound_stream_running == running)
    return;
  s_sound_stream_running = running;

  if (g_sound_stream->SetRunning(running))
    return;
  if (running)
    ERROR_LOG(AUDIO, "Error starting stream.");
  else
    ERROR_LOG(AUDIO, "Error stopping stream.");
}

void SendAIBuffer(const short* samples, unsigned int num_samples)
{
  if (!g_sound_stream)
    return;

  Mixer* pMixer = g_sound_stream->GetMixer();

  if (pMixer && samples)
  {
    pMixer->PushSamples(samples, num_samples);
  }

  g_sound_stream->Update();
}

}  // namespace AudioCommon
