
#pragma once

#include <libretro.h>
#include "Common/WindowSystemInfo.h"
#include "VideoBackends/Null/Render.h"
#include "VideoBackends/Software/SWRenderer.h"
#include "VideoBackends/Software/SWTexture.h"
#include "VideoCommon/VideoConfig.h"
#ifdef _WIN32
#include "VideoBackends/D3D/D3DBase.h"
#include "VideoBackends/D3D/D3DState.h"
#include "VideoBackends/D3D/D3DUtil.h"
#include "VideoBackends/D3D/DXShader.h"
#include "VideoBackends/D3D/DXTexture.h"
#include "VideoBackends/D3D/FramebufferManager.h"
#include "VideoBackends/D3D/PixelShaderCache.h"
#include "VideoBackends/D3D/Render.h"
#include "VideoBackends/D3D/TextureCache.h"
#endif

namespace Libretro
{
namespace Video
{
void Init(void);
extern retro_video_refresh_t video_cb;
extern struct retro_hw_render_callback hw_render;
extern WindowSystemInfo wsi;

}  // namespace Video
}  // namespace Libretro
