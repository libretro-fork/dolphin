
#pragma once

#include <libretro.h>
#include "Common/WindowSystemInfo.h"
#include "VideoBackends/Null/Render.h"
#include "VideoBackends/Software/SWRenderer.h"
#include "VideoBackends/Software/SWTexture.h"
#include "VideoCommon/VideoConfig.h"
#ifndef __APPLE__
#include "VideoBackends/Vulkan/VulkanLoader.h"
#endif

namespace Libretro
{
namespace Video
{
void Init(void);
extern retro_video_refresh_t video_cb;
extern struct retro_hw_render_callback hw_render;
extern WindowSystemInfo wsi;

#ifndef __APPLE__
namespace Vk
{
void Init(VkInstance instance, VkPhysicalDevice gpu, VkSurfaceKHR surface,
          PFN_vkGetInstanceProcAddr get_instance_proc_addr, const char** required_device_extensions,
          unsigned num_required_device_extensions, const char** required_device_layers,
          unsigned num_required_device_layers, const VkPhysicalDeviceFeatures* required_features);
void SetSurfaceSize(uint32_t width, uint32_t height);
void SetHWRenderInterface(retro_hw_render_interface* hw_render_interface);
void Shutdown();
void WaitForPresentation();
}  // namespace Vk
#endif

}  // namespace Video
}  // namespace Libretro
