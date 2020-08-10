// Copyright 2014 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include <memory>

#include "Common/GL/GLContext.h"

#include "DolphinLibretro/RetroGLContext.h"

GLContext::~GLContext() = default;

bool GLContext::Initialize(void* display_handle, void* window_handle, bool stereo, bool core)
{
  return false;
}

bool GLContext::IsHeadless() const
{
  return true;
}

std::unique_ptr<GLContext> GLContext::CreateSharedContext()
{
  return nullptr;
}

bool GLContext::MakeCurrent()
{
  return false;
}

bool GLContext::ClearCurrent()
{
  return false;
}

void GLContext::Swap()
{
}

void* GLContext::GetFuncAddress(const std::string& name)
{
  return nullptr;
}

std::unique_ptr<GLContext> GLContext::Create(const WindowSystemInfo& wsi, bool stereo, bool core,
                                             bool prefer_egl, bool prefer_gles)
{
  std::unique_ptr<GLContext> context;
  if (wsi.type == WindowSystemType::Libretro)
    context = std::make_unique<Libretro::Video::RetroGLContext>();
  if (!context)
    return nullptr;

  // Option to prefer GLES on desktop platforms, useful for testing.
  if (prefer_gles)
    context->m_opengl_mode = Mode::OpenGLES;

  if (!context->Initialize(wsi.display_connection, wsi.render_surface, stereo, core))
    return nullptr;

  return context;
}
