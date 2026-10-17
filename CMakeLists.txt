cmake_minimum_required(VERSION 3.21)
project(hid_radio LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick)
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
  QML_FILES
    qml/Main.qml
    qml/skins/SkinBase.qml
    qml/skins/NokiaSkin.qml
    qml/skins/MinimalSkin.qml
  )

target_link_libraries(hid_radio PRIVATE Qt6::Quick PkgConfig::LIBUSB)
