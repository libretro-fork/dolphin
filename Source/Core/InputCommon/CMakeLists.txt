add_library(inputcommon
  InputConfig.cpp
  InputProfile.cpp
  ControllerEmu/ControllerEmu.cpp
  ControllerEmu/Control/Control.cpp
  ControllerEmu/Control/Input.cpp
  ControllerEmu/Control/Output.cpp
  ControllerEmu/ControlGroup/AnalogStick.cpp
  ControllerEmu/ControlGroup/Buttons.cpp
  ControllerEmu/ControlGroup/ControlGroup.cpp
  ControllerEmu/ControlGroup/Cursor.cpp
  ControllerEmu/ControlGroup/Extension.cpp
  ControllerEmu/ControlGroup/Force.cpp
  ControllerEmu/ControlGroup/MixedTriggers.cpp
  ControllerEmu/ControlGroup/ModifySettingsButton.cpp
  ControllerEmu/ControlGroup/Slider.cpp
  ControllerEmu/ControlGroup/Tilt.cpp
  ControllerEmu/ControlGroup/Triggers.cpp
  ControllerEmu/Setting/BooleanSetting.cpp
  ControllerEmu/Setting/NumericSetting.cpp
  ControllerInterface/ControllerInterface.cpp
  ControllerInterface/Device.cpp
  ControlReference/ControlReference.cpp
  ControlReference/ExpressionParser.cpp
)

target_link_libraries(inputcommon PUBLIC
  common
)

if(ANDROID)
  target_sources(inputcommon PRIVATE GCAdapter_Android.cpp)
else()
  target_sources(inputcommon PRIVATE GCAdapter.cpp)
  target_link_libraries(inputcommon PUBLIC ${LIBUSB_LIBRARIES})
endif()
