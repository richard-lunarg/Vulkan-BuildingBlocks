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


cmake_minimum_required(VERSION 3.10.2)
project(Donut LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Source code in this folder
file(GLOB FILES_HEADER ./*.h)
file(GLOB FILES_HEADER ../../*.H)

#file(GLOB FILES_SOURCE main.cpp $ENV{VULKAN_SDK}/include/volk/volk.c ../../*.cpp ./*.c)
file(GLOB FILES_SOURCE main.cpp ../../*.cpp ./*.c)


add_executable(Donut MACOSX_BUNDLE ${FILES_HEADER} ${FILES_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/LunarGIcon.icns)

if(APPLE)
	set_target_properties(Donut PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/macOS/Info.plist)
endif()

set_target_properties(Donut PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(Donut PRIVATE "$ENV{VULKAN_SDK}/include" "$ENV{VULKAN_SDK}/include/SDL2" "./" "../../")
target_link_directories(Donut PRIVATE $ENV{VULKAN_SDK}/lib)
target_compile_definitions(Donut PRIVATE VK_NO_PROTOTYPES)

if(APPLE)
	set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/LunarGIcons.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
	target_compile_definitions(Donut PRIVATE VK_USE_PLATFORM_MACOS_MVK)

	install(TARGETS Donut RUNTIME DESTINATION MacOS BUNDLE DESTINATION MacOS) 
endif()

if(APPLE)
	target_link_libraries(Donut "-framework Cocoa -framework QuartzCore -framework Metal -framework GameController")
	target_link_libraries(Donut "-framework ForceFeedback -framework CoreHaptics -framework InputMethodKit -framework CoreGraphics")
	target_link_libraries(Donut "-framework AppKit -framework Carbon -framework CoreVideo -framework CoreAudio")
	target_link_libraries(Donut "-framework AVFAudio -framework AudioToolbox -framework IOKit -framework CoreAudioKit")
	target_link_libraries(Donut "-framework CoreFoundation")

endif()

if(APPLE)
	target_link_libraries(Donut "$ENV{VULKAN_SDK}/lib/libvulkan.1.dylib")
	target_link_libraries(Donut "$ENV{VULKAN_SDK}/lib/libSDL2main.a")
	target_link_libraries(Donut "$ENV{VULKAN_SDK}/lib/libSDL2.a")
	target_link_libraries(Donut "iconv")
else()
	target_link_libraries(Donut "$ENV{VULKAN_SDK}/lib/libvulkan.so")
	target_link_libraries(Donut "libSDL2main.a")
	target_link_libraries(Donut "libSDL2.a")
	target_link_libraries(Donut "libwayland-client.so")
endif()


if(APPLE)
add_custom_command(TARGET Donut POST_BUILD
    COMMAND cp
    "${CMAKE_CURRENT_SOURCE_DIR}/macOS/Info.plist"
    "${CMAKE_CURRENT_BINARY_DIR}/Donut.app/Contents/"
    COMMENT "Copying Info.plist to Donut.app...")


set_target_properties(Donut PROPERTIES SKIP_BUILD_RPATH FALSE)
set_target_properties(Donut PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
set_target_properties(Donut PROPERTIES INSTALL_RPATH "")
set_target_properties(Donut PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

