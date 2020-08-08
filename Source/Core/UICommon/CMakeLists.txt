add_library(uicommon
  Disassembler.cpp
  GameFile.cpp
  UICommon.cpp
  USBUtils.cpp
)

target_link_libraries(uicommon
PUBLIC
  common
  cpp-optparse

PRIVATE
  $<$<BOOL:APPLE>:${IOK_LIBRARY}>
)

if ((DEFINED CMAKE_ANDROID_ARCH_ABI AND CMAKE_ANDROID_ARCH_ABI MATCHES "x86|x86_64") OR
    (NOT DEFINED CMAKE_ANDROID_ARCH_ABI AND _M_X86))
  target_link_libraries(uicommon PRIVATE bdisasm)
endif()

if(LIBUSB_FOUND)
  target_link_libraries(uicommon PRIVATE ${LIBUSB_LIBRARIES})
endif()

if(ENABLE_LLVM)
  find_package(LLVM CONFIG QUIET)
  if(LLVM_FOUND AND TARGET LLVM)
    message(STATUS "LLVM found, enabling LLVM support in disassembler")
    target_compile_definitions(uicommon PRIVATE HAVE_LLVM)
    target_link_libraries(uicommon PRIVATE LLVM)
    target_include_directories(uicommon PRIVATE ${LLVM_INCLUDE_DIRS})
  endif()
endif()
