// Copyright 2008 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <array>
#include <memory>
#include <string>
#include <utility>

#include "Common/CommonTypes.h"
#include "Common/WindowSystemInfo.h"

class GLContext
{
public:
  enum class Mode
  {
    Detect,
    OpenGL,
    OpenGLES
  };

  virtual ~GLContext();

  bool IsGLES() const { return m_opengl_mode == Mode::OpenGLES; }

  u32 GetBackBufferWidth() const { return m_backbuffer_width; }
  u32 GetBackBufferHeight() const { return m_backbuffer_height; }

  virtual std::unique_ptr<GLContext> CreateSharedContext();

  virtual bool MakeCurrent();
  virtual bool ClearCurrent();

  // Creates an instance of GLContext specific to the platform we are running on.
  // If successful, the context is made current on the calling thread.
  static std::unique_ptr<GLContext> Create(
        const WindowSystemInfo& wsi, bool stereo = false,
        bool core = true, bool prefer_egl = false,
        bool prefer_gles = false);

protected:
  virtual bool Initialize(void* display_handle, void* window_handle, bool stereo, bool core);

  Mode m_opengl_mode = Mode::Detect;

  // Window dimensions.
  u32 m_backbuffer_width = 0;
  u32 m_backbuffer_height = 0;
  bool m_is_shared = false;
};
