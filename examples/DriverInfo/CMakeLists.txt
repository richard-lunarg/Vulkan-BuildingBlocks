# example usage:
#
# cmake --build . --config Release  

# Cheat sheet
if(ANDROID)
endif()

if(APPLE)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
endif()


cmake_minimum_required(VERSION 3.15.0)
project(Donut LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Source code in this folder
file(GLOB FILES_HEADER ./*.h ../../include/*.h)
file(GLOB FILES_SOURCE ./*.cpp ../../src/*.cpp)

add_executable(DriverInfo ${FILES_HEADER} ${FILES_SOURCE})

set_target_properties(DriverInfo PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(DriverInfo PRIVATE "$ENV{VULKAN_SDK}/include" "$ENV{VULKAN_SDK}/include/SDL2" "$ENV{VULKAN_SDK}/include/SDL2" "./" "../../include")
target_link_directories(DriverInfo PRIVATE $ENV{VULKAN_SDK}/lib)
target_compile_definitions(DriverInfo PRIVATE VK_NO_PROTOTYPES)


if(APPLE)
	target_link_libraries(DriverInfo "-framework Cocoa -framework QuartzCore -framework Metal -framework GameController")
	target_link_libraries(DriverInfo "-framework ForceFeedback -framework CoreHaptics -framework InputMethodKit -framework CoreGraphics")
	target_link_libraries(DriverInfo "-framework AppKit -framework Carbon -framework CoreVideo -framework CoreAudio")
	target_link_libraries(DriverInfo "-framework AVFAudio -framework AudioToolbox -framework IOKit -framework CoreAudioKit")
	target_link_libraries(DriverInfo "-framework CoreFoundation")

endif()

if(APPLE)
	target_link_libraries(DriverInfo "$ENV{VULKAN_SDK}/lib/libvulkan.1.dylib")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	target_link_libraries(DriverInfo "$ENV{VULKAN_SDK}/lib/libvulkan.so")
endif()


