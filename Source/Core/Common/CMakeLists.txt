add_library(common
  CDUtils.cpp
  ColorUtil.cpp
  CommonFuncs.cpp
  Config/Config.cpp
  Config/ConfigInfo.cpp
  Config/Layer.cpp
  Crypto/AES.cpp
  Crypto/bn.cpp
  Crypto/ec.cpp
  Debug/MemoryPatches.cpp
  Debug/Watches.cpp
  ENetUtil.cpp
  File.cpp
  FileSearch.cpp
  FileUtil.cpp
  FloatUtils.cpp
  GekkoDisassembler.cpp
  Hash.cpp
  HttpRequest.cpp
  Image.cpp
  IniFile.cpp
  JitRegister.cpp
  Logging/LogManager.cpp
  MathUtil.cpp
  MD5.cpp
  MemArena.cpp
  MemoryUtil.cpp
  MsgHandler.cpp
  NandPaths.cpp
  Network.cpp
  PcapFile.cpp
  PerformanceCounter.cpp
  Profiler.cpp
  QoSSession.cpp
  Random.cpp
  SDCardUtil.cpp
  SFMLHelper.cpp
  SettingsHandler.cpp
  StringUtil.cpp
  SymbolDB.cpp
  Thread.cpp
  Timer.cpp
  TraversalClient.cpp
  UPnP.cpp
  Version.cpp
)

target_link_libraries(common
PUBLIC
  ${CMAKE_THREAD_LIBS_INIT}
  enet
  ${MBEDTLS_LIBRARIES}

PRIVATE
  ${CURL_LIBRARIES}
  ${ICONV_LIBRARIES}
  png
  ${VTUNE_LIBRARIES}
)

if (APPLE)
  target_link_libraries(common
  PRIVATE
    ${APPKIT_LIBRARY}
    ${COREFOUNDATION_LIBRARY}
    ${IOK_LIBRARY}
  )
endif()

if(WIN32)
  target_sources(common PRIVATE
    Logging/ConsoleListenerWin.cpp
  )
else()
  target_sources(common PRIVATE
    Logging/ConsoleListenerNix.cpp
  )
endif()

if(_M_ARM_64)
  target_sources(common PRIVATE
    Arm64Emitter.cpp
    ArmCPUDetect.cpp
    GenericFPURoundMode.cpp
  )
else()
  if(_M_X86) #X86
    target_sources(common PRIVATE
      x64ABI.cpp
      x64Emitter.cpp
      x64FPURoundMode.cpp
      x64CPUDetect.cpp
    )
  else() # Generic
    target_sources(common PRIVATE
      GenericFPURoundMode.cpp
      GenericCPUDetect.cpp
    )
  endif()
endif()

if(MSVC)
  target_sources(common PRIVATE
    LdrWatcher.cpp
    CompatPatches.cpp
  )
endif()

# OpenGL Interface
target_sources(common PRIVATE
  GL/GLUtil.cpp
  GL/GLExtensions/GLExtensions.cpp
  GL/GLContext.cpp
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(common PUBLIC dl rt)
endif()

if(USE_UPNP)
  target_link_libraries(common PRIVATE Miniupnpc::miniupnpc)
endif()

if(OPROFILE_FOUND)
  target_link_libraries(common PRIVATE OProfile::OProfile)
endif()

if(UNIX AND NOT LIBRETRO)
  # Posix networking code needs to be fixed for Windows
  add_executable(traversal_server TraversalServer.cpp)
  target_link_libraries(traversal_server PRIVATE common)
  if(SYSTEMD_FOUND)
    target_link_libraries(traversal_server PRIVATE ${SYSTEMD_LIBRARIES})
  endif()
elseif(WIN32)
  if(MSVC)
    target_link_libraries(common PRIVATE "-INCLUDE:enableCompatPatches")
    target_link_libraries(common PRIVATE "-NODEFAULTLIB:libcmt")
  endif()
  target_link_libraries(common PRIVATE comctl32 comdlg32 winmm shlwapi)
endif()
