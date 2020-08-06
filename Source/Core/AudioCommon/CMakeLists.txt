add_library(audiocommon
  AudioCommon.cpp
  DPL2Decoder.cpp
  Mixer.cpp
  NullSoundStream.cpp
)

target_link_libraries(audiocommon PRIVATE)
