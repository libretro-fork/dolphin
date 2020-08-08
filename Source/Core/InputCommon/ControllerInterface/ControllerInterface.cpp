// Copyright 2010 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "InputCommon/ControllerInterface/ControllerInterface.h"

#include <algorithm>

#include "Common/Logging/Log.h"

ControllerInterface g_controller_interface;

//
// Init
//
// Detect devices and inputs outputs / will make refresh function later
//
void ControllerInterface::Initialize()
{
  if (m_is_init)
    return;

  m_is_populating_devices = true;

  m_is_init = true;
  RefreshDevices();
}

void ControllerInterface::ChangeWindow(void* hwnd)
{
  if (!m_is_init)
    return;

  RefreshDevices();
}

void ControllerInterface::RefreshDevices()
{
  if (!m_is_init)
    return;

#ifndef __LIBRETRO__
  {
    std::lock_guard<std::mutex> lk(m_devices_mutex);
    m_devices.clear();
  }
#endif

  m_is_populating_devices = true;

  m_is_populating_devices = false;
  InvokeDevicesChangedCallbacks();
}

//
// DeInit
//
// Remove all devices/ call library cleanup functions
//
void ControllerInterface::Shutdown()
{
  if (!m_is_init)
    return;

  {
    std::lock_guard<std::mutex> lk(m_devices_mutex);

    for (const auto& d : m_devices)
    {
      // Set outputs to ZERO before destroying device
      for (ciface::Core::Device::Output* o : d->Outputs())
        o->SetState(0);
    }

    m_devices.clear();
  }

  m_is_init = false;
}

void ControllerInterface::AddDevice(std::shared_ptr<ciface::Core::Device> device)
{
  {
    std::lock_guard<std::mutex> lk(m_devices_mutex);
    // Try to find an ID for this device
    int id = 0;
    while (true)
    {
      const auto it =
          std::find_if(m_devices.begin(), m_devices.end(), [&device, &id](const auto& d) {
            return d->GetSource() == device->GetSource() && d->GetName() == device->GetName() &&
                   d->GetId() == id;
          });
      if (it == m_devices.end())  // no device with the same name with this ID, so we can use it
        break;
      else
        id++;
    }
    device->SetId(id);

    NOTICE_LOG(SERIALINTERFACE, "Added device: %s", device->GetQualifiedName().c_str());
    m_devices.emplace_back(std::move(device));
  }

  if (!m_is_populating_devices)
    InvokeDevicesChangedCallbacks();
}

void ControllerInterface::RemoveDevice(std::function<bool(const ciface::Core::Device*)> callback)
{
  {
    std::lock_guard<std::mutex> lk(m_devices_mutex);
    auto it = std::remove_if(m_devices.begin(), m_devices.end(), [&callback](const auto& dev) {
      if (callback(dev.get()))
      {
        NOTICE_LOG(SERIALINTERFACE, "Removed device: %s", dev->GetQualifiedName().c_str());
        return true;
      }
      return false;
    });
    m_devices.erase(it, m_devices.end());
  }

  if (!m_is_populating_devices)
    InvokeDevicesChangedCallbacks();
}

//
// UpdateInput
//
// Update input for all devices
//
void ControllerInterface::UpdateInput()
{
  // Don't block the UI or CPU thread (to avoid a short but noticeable frame drop)
  if (m_devices_mutex.try_lock())
  {
    std::lock_guard<std::mutex> lk(m_devices_mutex, std::adopt_lock);
    for (const auto& d : m_devices)
      d->UpdateInput();
  }
}

//
// RegisterDevicesChangedCallback
//
// Register a callback to be called when a device is added or removed (as from the input backends'
// hotplug thread), or when devices are refreshed
//
void ControllerInterface::RegisterDevicesChangedCallback(std::function<void()> callback)
{
  std::lock_guard<std::mutex> lk(m_callbacks_mutex);
  m_devices_changed_callbacks.emplace_back(std::move(callback));
}

//
// InvokeDevicesChangedCallbacks
//
// Invoke all callbacks that were registered
//
void ControllerInterface::InvokeDevicesChangedCallbacks() const
{
  std::lock_guard<std::mutex> lk(m_callbacks_mutex);
  for (const auto& callback : m_devices_changed_callbacks)
    callback();
}
