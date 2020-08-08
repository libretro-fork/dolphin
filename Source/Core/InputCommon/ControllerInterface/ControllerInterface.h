// Copyright 2010 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include "InputCommon/ControllerInterface/Device.h"

//
// ControllerInterface
//
// Some crazy shit I made to control different device inputs and outputs
// from lots of different sources, hopefully more easily.
//
class ControllerInterface : public ciface::Core::DeviceContainer
{
public:
  ControllerInterface() : m_is_init(false) {}
  void Initialize();
  void ChangeWindow(void* hwnd);
  void RefreshDevices();
  void Shutdown();
  void AddDevice(std::shared_ptr<ciface::Core::Device> device);
  void RemoveDevice(std::function<bool(const ciface::Core::Device*)> callback);
  bool IsInit() const { return m_is_init; }
  void UpdateInput();

  void RegisterDevicesChangedCallback(std::function<void(void)> callback);
  void InvokeDevicesChangedCallbacks() const;

private:
  std::vector<std::function<void()>> m_devices_changed_callbacks;
  mutable std::mutex m_callbacks_mutex;
  bool m_is_init;
  std::atomic<bool> m_is_populating_devices{false};
};

extern ControllerInterface g_controller_interface;
