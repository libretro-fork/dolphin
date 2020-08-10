#pragma once

#include "Common/GL/GLContext.h"

namespace Libretro
{
namespace Video
{

class RetroGLContext : public ::GLContext
{
protected:
  virtual bool Initialize(void* display_handle, void* window_handle,
      bool stereo, bool core) override;
};

}  // namespace Video
}  // namespace Libretro
