// Copyright 2010 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include <algorithm>

#include "Common/CommonTypes.h"
#include "Common/Logging/Log.h"
#include "Common/MsgHandler.h"
#include "Common/StringUtil.h"
#include "Core/Config/GraphicsSettings.h"
#include "Core/ConfigManager.h"
#include "VideoBackends/D3D/D3DBase.h"
#include "VideoBackends/D3D/D3DState.h"
#include "VideoBackends/D3D/D3DTexture.h"
#include "VideoCommon/VideoConfig.h"

namespace DX11
{
static HINSTANCE s_d3d_compiler_dll;
static int s_d3dcompiler_dll_ref;
pD3DCompile PD3DCompile = nullptr;

CREATEDXGIFACTORY PCreateDXGIFactory = nullptr;
static HINSTANCE s_dxgi_dll;
static int s_dxgi_dll_ref;

static D3D11CREATEDEVICE s_d3d11_create_device;
static HINSTANCE s_d3d_dll;
static int s_d3d_dll_ref;

namespace D3D
{
ID3D11Device* device = nullptr;
ID3D11Device1* device1 = nullptr;
ID3D11DeviceContext* context = nullptr;
D3D_FEATURE_LEVEL featlevel = D3D_FEATURE_LEVEL_10_0;

static IDXGIFactory2* s_dxgi_factory;

static std::vector<DXGI_SAMPLE_DESC> s_aa_modes;  // supported AA modes of the current adapter

static bool s_bgra_textures_supported;

constexpr UINT NUM_SUPPORTED_FEATURE_LEVELS = 3;
constexpr D3D_FEATURE_LEVEL supported_feature_levels[NUM_SUPPORTED_FEATURE_LEVELS] = {
    D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0};

HRESULT LoadDXGI()
{
  if (s_dxgi_dll_ref++ > 0)
    return S_OK;

  if (s_dxgi_dll)
    return S_OK;
  s_dxgi_dll = LoadLibraryA("dxgi.dll");
  if (!s_dxgi_dll)
  {
    MessageBoxA(nullptr, "Failed to load dxgi.dll", "Critical error", MB_OK | MB_ICONERROR);
    --s_dxgi_dll_ref;
    return E_FAIL;
  }

  // Even though we use IDXGIFactory2 we use CreateDXGIFactory1 to create it to maintain
  // compatibility with Windows 7
  PCreateDXGIFactory = (CREATEDXGIFACTORY)GetProcAddress(s_dxgi_dll, "CreateDXGIFactory1");
  if (PCreateDXGIFactory == nullptr)
    MessageBoxA(nullptr, "GetProcAddress failed for CreateDXGIFactory1!", "Critical error",
                MB_OK | MB_ICONERROR);

  return S_OK;
}

HRESULT LoadD3D()
{
  if (s_d3d_dll_ref++ > 0)
    return S_OK;

  if (s_d3d_dll)
    return S_OK;
  s_d3d_dll = LoadLibraryA("d3d11.dll");
  if (!s_d3d_dll)
  {
    MessageBoxA(nullptr, "Failed to load d3d11.dll", "Critical error", MB_OK | MB_ICONERROR);
    --s_d3d_dll_ref;
    return E_FAIL;
  }
  s_d3d11_create_device = (D3D11CREATEDEVICE)GetProcAddress(s_d3d_dll, "D3D11CreateDevice");
  if (s_d3d11_create_device == nullptr)
    MessageBoxA(nullptr, "GetProcAddress failed for D3D11CreateDevice!", "Critical error",
                MB_OK | MB_ICONERROR);

  return S_OK;
}

HRESULT LoadD3DCompiler()
{
  if (s_d3dcompiler_dll_ref++ > 0)
    return S_OK;
  if (s_d3d_compiler_dll)
    return S_OK;

  // The older version of the D3D compiler cannot compile our ubershaders without various
  // graphical issues. D3DCOMPILER_DLL_A should point to d3dcompiler_47.dll, so if this fails
  // to load, inform the user that they need to update their system.
  s_d3d_compiler_dll = LoadLibraryA(D3DCOMPILER_DLL_A);
  if (!s_d3d_compiler_dll)
  {
    PanicAlertT("Failed to load %s. If you are using Windows 7, try installing the "
                "KB4019990 update package.",
                D3DCOMPILER_DLL_A);
    return E_FAIL;
  }

  PD3DCompile = (pD3DCompile)GetProcAddress(s_d3d_compiler_dll, "D3DCompile");
  if (PD3DCompile == nullptr)
    MessageBoxA(nullptr, "GetProcAddress failed for D3DCompile!", "Critical error",
                MB_OK | MB_ICONERROR);

  return S_OK;
}

void UnloadDXGI()
{
  if (!s_dxgi_dll_ref)
    return;
  if (--s_dxgi_dll_ref != 0)
    return;

  if (s_dxgi_dll)
    FreeLibrary(s_dxgi_dll);
  s_dxgi_dll = nullptr;
  PCreateDXGIFactory = nullptr;
}

void UnloadD3D()
{
  if (!s_d3d_dll_ref)
    return;
  if (--s_d3d_dll_ref != 0)
    return;

  if (s_d3d_dll)
    FreeLibrary(s_d3d_dll);
  s_d3d_dll = nullptr;
  s_d3d11_create_device = nullptr;
}

void UnloadD3DCompiler()
{
  if (!s_d3dcompiler_dll_ref)
    return;
  if (--s_d3dcompiler_dll_ref != 0)
    return;

  if (s_d3d_compiler_dll)
    FreeLibrary(s_d3d_compiler_dll);
  s_d3d_compiler_dll = nullptr;
}

std::vector<DXGI_SAMPLE_DESC> EnumAAModes(IDXGIAdapter* adapter)
{
  std::vector<DXGI_SAMPLE_DESC> _aa_modes;

  // NOTE: D3D 10.0 doesn't support multisampled resources which are bound as depth buffers AND
  // shader resources.
  // Thus, we can't have MSAA with 10.0 level hardware.
  ID3D11Device* _device;
  ID3D11DeviceContext* _context;
  D3D_FEATURE_LEVEL feat_level;
  HRESULT hr = s_d3d11_create_device(adapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, 0,
                                     supported_feature_levels, NUM_SUPPORTED_FEATURE_LEVELS,
                                     D3D11_SDK_VERSION, &_device, &feat_level, &_context);
  if (FAILED(hr) || feat_level == D3D_FEATURE_LEVEL_10_0)
  {
    DXGI_SAMPLE_DESC desc;
    desc.Count = 1;
    desc.Quality = 0;
    _aa_modes.push_back(desc);
    SAFE_RELEASE(_context);
    SAFE_RELEASE(_device);
  }
  else
  {
    for (int samples = 0; samples < D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT; ++samples)
    {
      UINT quality_levels = 0;
      _device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, samples, &quality_levels);

      DXGI_SAMPLE_DESC desc;
      desc.Count = samples;
      desc.Quality = 0;

      if (quality_levels > 0)
        _aa_modes.push_back(desc);
    }
    _context->Release();
    _device->Release();
  }
  return _aa_modes;
}

D3D_FEATURE_LEVEL GetFeatureLevel(IDXGIAdapter* adapter)
{
  D3D_FEATURE_LEVEL feat_level = D3D_FEATURE_LEVEL_9_1;
  s_d3d11_create_device(adapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, 0, supported_feature_levels,
                        NUM_SUPPORTED_FEATURE_LEVELS, D3D11_SDK_VERSION, nullptr, &feat_level,
                        nullptr);
  return feat_level;
}

static bool SupportsS3TCTextures(ID3D11Device* dev)
{
  UINT bc1_support, bc2_support, bc3_support;
  if (FAILED(dev->CheckFormatSupport(DXGI_FORMAT_BC1_UNORM, &bc1_support)) ||
      FAILED(dev->CheckFormatSupport(DXGI_FORMAT_BC2_UNORM, &bc2_support)) ||
      FAILED(dev->CheckFormatSupport(DXGI_FORMAT_BC3_UNORM, &bc3_support)))
  {
    return false;
  }

  return ((bc1_support & bc2_support & bc3_support) & D3D11_FORMAT_SUPPORT_TEXTURE2D) != 0;
}

static bool SupportsBPTCTextures(ID3D11Device* dev)
{
  // Currently, we only care about BC7. This could be extended to BC6H in the future.
  UINT bc7_support;
  if (FAILED(dev->CheckFormatSupport(DXGI_FORMAT_BC7_UNORM, &bc7_support)))
    return false;

  return (bc7_support & D3D11_FORMAT_SUPPORT_TEXTURE2D) != 0;
}

HRESULT Create(HWND wnd)
{
  HRESULT hr = LoadDXGI();
  if (SUCCEEDED(hr))
    hr = LoadD3D();
  if (SUCCEEDED(hr))
    hr = LoadD3DCompiler();
  if (FAILED(hr))
  {
    UnloadDXGI();
    UnloadD3D();
    UnloadD3DCompiler();
    return hr;
  }

  hr = PCreateDXGIFactory(__uuidof(IDXGIFactory2), (void**)&s_dxgi_factory);
  if (FAILED(hr))
    MessageBox(wnd, _T("Failed to create IDXGIFactory object"), _T("Dolphin Direct3D 11 backend"),
               MB_OK | MB_ICONERROR);

  IDXGIAdapter* adapter;
  hr = s_dxgi_factory->EnumAdapters(g_ActiveConfig.iAdapter, &adapter);
  if (FAILED(hr))
  {
    // try using the first one
    hr = s_dxgi_factory->EnumAdapters(0, &adapter);
    if (FAILED(hr))
      MessageBox(wnd, _T("Failed to enumerate adapters"), _T("Dolphin Direct3D 11 backend"),
                 MB_OK | MB_ICONERROR);
  }

  // get supported AA modes
  s_aa_modes = EnumAAModes(adapter);

  if (std::find_if(s_aa_modes.begin(), s_aa_modes.end(), [](const DXGI_SAMPLE_DESC& desc) {
        return desc.Count == g_Config.iMultisamples;
      }) == s_aa_modes.end())
  {
    Config::SetCurrent(Config::GFX_MSAA, UINT32_C(1));
    UpdateActiveConfig();
  }

  // Creating debug devices can sometimes fail if the user doesn't have the correct
  // version of the DirectX SDK. If it does, simply fallback to a non-debug device.
  if (g_Config.bEnableValidationLayer)
  {
    hr = s_d3d11_create_device(adapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, D3D11_CREATE_DEVICE_DEBUG,
                               supported_feature_levels, NUM_SUPPORTED_FEATURE_LEVELS,
                               D3D11_SDK_VERSION, &device, &featlevel, &context);

  }

  if (!g_Config.bEnableValidationLayer || FAILED(hr))
  {
    hr = s_d3d11_create_device(adapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, 0,
                               supported_feature_levels, NUM_SUPPORTED_FEATURE_LEVELS,
                               D3D11_SDK_VERSION, &device, &featlevel, &context);
  }

  SAFE_RELEASE(adapter);

  if (FAILED(hr))
  {
    MessageBox(
        wnd,
        _T("Failed to initialize Direct3D.\nMake sure your video card supports at least D3D 10.0"),
        _T("Dolphin Direct3D 11 backend"), MB_OK | MB_ICONERROR);
    SAFE_RELEASE(device);
    SAFE_RELEASE(context);
    SAFE_RELEASE(s_dxgi_factory);
    return E_FAIL;
  }

  hr = device->QueryInterface<ID3D11Device1>(&device1);
  if (FAILED(hr))
  {
    WARN_LOG(VIDEO, "Missing Direct3D 11.1 support. Logical operations will not be supported.");
    g_Config.backend_info.bSupportsLogicOp = false;
  }

  // BGRA textures are easier to deal with in TextureCache, but might not be supported
  UINT format_support;
  device->CheckFormatSupport(DXGI_FORMAT_B8G8R8A8_UNORM, &format_support);
  s_bgra_textures_supported = (format_support & D3D11_FORMAT_SUPPORT_TEXTURE2D) != 0;
  g_Config.backend_info.bSupportsST3CTextures = SupportsS3TCTextures(device);
  g_Config.backend_info.bSupportsBPTCTextures = SupportsBPTCTextures(device);

  SetDebugObjectName(context, "device context");

  stateman = new StateManager;
  return S_OK;
}

void Close()
{
  // release all bound resources
  context->ClearState();
  SAFE_DELETE(stateman);
  context->Flush();  // immediately destroy device objects

  SAFE_RELEASE(context);
  SAFE_RELEASE(device1);
  device = nullptr;

  // unload DLLs
  UnloadD3D();
  UnloadDXGI();
}

const char* VertexShaderVersionString()
{
  if (featlevel == D3D_FEATURE_LEVEL_11_0)
    return "vs_5_0";
  else if (featlevel == D3D_FEATURE_LEVEL_10_1)
    return "vs_4_1";
  else /*if(featlevel == D3D_FEATURE_LEVEL_10_0)*/
    return "vs_4_0";
}

const char* GeometryShaderVersionString()
{
  if (featlevel == D3D_FEATURE_LEVEL_11_0)
    return "gs_5_0";
  else if (featlevel == D3D_FEATURE_LEVEL_10_1)
    return "gs_4_1";
  else /*if(featlevel == D3D_FEATURE_LEVEL_10_0)*/
    return "gs_4_0";
}

const char* PixelShaderVersionString()
{
  if (featlevel == D3D_FEATURE_LEVEL_11_0)
    return "ps_5_0";
  else if (featlevel == D3D_FEATURE_LEVEL_10_1)
    return "ps_4_1";
  else /*if(featlevel == D3D_FEATURE_LEVEL_10_0)*/
    return "ps_4_0";
}

const char* ComputeShaderVersionString()
{
  if (featlevel == D3D_FEATURE_LEVEL_11_0)
    return "cs_5_0";
  else if (featlevel == D3D_FEATURE_LEVEL_10_1)
    return "cs_4_1";
  else /*if(featlevel == D3D_FEATURE_LEVEL_10_0)*/
    return "cs_4_0";
}

bool BGRATexturesSupported()
{
  return s_bgra_textures_supported;
}

// Returns the maximum width/height of a texture. This value only depends upon the feature level in
// DX11
u32 GetMaxTextureSize(D3D_FEATURE_LEVEL feature_level)
{
  switch (feature_level)
  {
  case D3D_FEATURE_LEVEL_11_0:
    return D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;

  case D3D_FEATURE_LEVEL_10_1:
  case D3D_FEATURE_LEVEL_10_0:
    return D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION;

  case D3D_FEATURE_LEVEL_9_3:
    return 4096;

  case D3D_FEATURE_LEVEL_9_2:
  case D3D_FEATURE_LEVEL_9_1:
    return 2048;

  default:
    return 0;
  }
}

void Reset(HWND new_wnd)
{
}

void ResizeSwapChain()
{
}

void Present()
{
}

HRESULT SetFullscreenState(bool enable_fullscreen)
{
  return S_OK;
}

bool GetFullscreenState()
{
  return TRUE;
}

void SetDebugObjectName(ID3D11DeviceChild* resource, const char* name)
{
#if defined(_DEBUG) || defined(DEBUGFAST)
  if (resource)
    resource->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)(name ? strlen(name) : 0), name);
#endif
}

std::string GetDebugObjectName(ID3D11DeviceChild* resource)
{
  std::string name;
#if defined(_DEBUG) || defined(DEBUGFAST)
  if (resource)
  {
    UINT size = 0;
    resource->GetPrivateData(WKPDID_D3DDebugObjectName, &size, nullptr);  // get required size
    name.resize(size);
    resource->GetPrivateData(WKPDID_D3DDebugObjectName, &size, const_cast<char*>(name.data()));
  }
#endif
  return name;
}

}  // namespace D3D

}  // namespace DX11
