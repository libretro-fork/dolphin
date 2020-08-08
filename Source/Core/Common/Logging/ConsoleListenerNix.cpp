// Copyright 2015 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include <cstdio>
#include <cstring>

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

#include "Common/Logging/ConsoleListener.h"
#include "Common/Logging/Log.h"

ConsoleListener::ConsoleListener()
{
}

ConsoleListener::~ConsoleListener()
{
  fflush(nullptr);
}

void ConsoleListener::Log(LogTypes::LOG_LEVELS level, const char* text)
{
  fprintf(stderr, "%s", text);
}
