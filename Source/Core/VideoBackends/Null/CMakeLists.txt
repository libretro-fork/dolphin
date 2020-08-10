add_library(videonull
  NullBackend.cpp
  NullTexture.cpp
  Render.cpp
  VertexManager.cpp
)

target_include_directories(videonull
PRIVATE
${CMAKE_SOURCE_DIR}/Externals/Libretro/Include
)

target_link_libraries(videonull
PUBLIC
  common
  videocommon
)
