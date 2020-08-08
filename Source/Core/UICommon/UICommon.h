// Copyright 2014 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <string>

#include "Common/CommonTypes.h"

namespace UICommon
{
void Init();
void Shutdown();

void CreateDirectories();
void SetUserDirectory(const std::string& custom_path);

bool TriggerSTMPowerEvent();

}  // namespace UICommon
