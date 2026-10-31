cmake_minimum_required(VERSION 3.21)
project(hid_radio LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick Widgets)
find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBUSB REQUIRED IMPORTED_TARGET libusb-1.0)

add_executable(hid_radio main.cpp)

target_include_directories(${PROJECT_NAME}
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

qt_add_qml_module(
  hid_radio
  URI
  Radio
  VERSION
  1.0
  SOURCES
    src/Usb/HidDevice.cpp
    include/Usb/HidDevice.h
    include/Usb/Report.h
    src/Settings/AppSettings.cpp
    include/Settings/AppSettings.h
    src/Tray/TrayController.cpp
    include/Tray/TrayController.h
  QML_FILES
    qml/Main.qml
    qml/SettingsPage.qml
    qml/SettingsGroup.qml
    qml/SettingRow.qml
    qml/SettingSwitch.qml
    qml/SettingChoice.qml
    qml/SettingSlider.qml
    qml/SettingInfo.qml
    qml/SettingButton.qml
    qml/GeneralPage.qml
    qml/AudioPage.qml
    qml/HardwarePage.qml
    qml/skins/SkinBase.qml
    qml/skins/RoadRadioSkin.qml
    qml/skins/NokiaSkin.qml
    qml/skins/MinimalSkin.qml
  RESOURCES
    qml/skins/icons/power_round_off.png
    qml/skins/icons/power_round_on.png
    qml/skins/icons/arrow_btn_l.png
    qml/skins/icons/arrow_btn_r.png
  )

target_link_libraries(hid_radio PRIVATE Qt6::Quick Qt6::Widgets PkgConfig::LIBUSB)
