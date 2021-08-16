cmake_minimum_required(VERSION 3.0.0)

project(UAC2Test VERSION 0.1 LANGUAGES CXX)

#find_path(LIBUSB_INCLUDE_DIR
#  NAMES libusb.h
#  PATH_SUFFIXES "include" "libusb" "libusb-1.0")
#find_library(LIBUSB_LIBRARY
#  NAMES usb
#  PATH_SUFFIXES "lib" "lib32" "lib64" "x86_64-linux-gnu")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(PkgConfig)
pkg_check_modules(libusb-1.0 REQUIRED libusb-1.0)


set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(${PROJECT_NAME} 
    "main.cpp"
    Utils.cpp
    UsbDevice.cpp
    UacDevice.cpp
    )
target_include_directories(${PROJECT_NAME} PUBLIC ${libusb-1.0_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${libusb-1.0_LIBRARIES} Threads::Threads)
target_link_directories(${PROJECT_NAME} PUBLIC libusb)
