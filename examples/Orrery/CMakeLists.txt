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


cmake_minimum_required(VERSION 3.28.1)
project(Orrery LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)


# Source code in this folder
file(GLOB FILES_HEADER ./*.h)
file(GLOB FILES_HEADER ../../include/*.h)

#file(GLOB FILES_SOURCE main.cpp $ENV{VULKAN_SDK}/include/volk/volk.c ../../*.cpp ./*.c)
file(GLOB FILES_SOURCE main.cpp ../../src/*.cpp ./*.c)


add_executable(Orrery MACOSX_BUNDLE ${FILES_HEADER} ${FILES_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/LunarGIcon.icns)

if(APPLE)
	set_target_properties(Orrery PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/macOS/Info.plist)
endif()

set_target_properties(Orrery PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(Orrery PRIVATE "$ENV{VULKAN_SDK}/include" "$ENV{VULKAN_SDK}/include/SDL2" "$ENV{VULKAN_SDK}/include/SDL2" "./" "../../include")
target_link_directories(Orrery PRIVATE $ENV{VULKAN_SDK}/lib)
target_compile_definitions(Orrery PRIVATE VK_NO_PROTOTYPES)

if(APPLE)
	set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/LunarGIcons.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
	target_compile_definitions(Orrery PRIVATE VK_USE_PLATFORM_MACOS_MVK)

	install(TARGETS Orrery RUNTIME DESTINATION MacOS BUNDLE DESTINATION MacOS) 
	target_link_libraries(Orrery "-framework Cocoa -framework QuartzCore -framework Metal -framework GameController")
	target_link_libraries(Orrery "-framework ForceFeedback -framework CoreHaptics -framework InputMethodKit -framework CoreGraphics")
	target_link_libraries(Orrery "-framework AppKit -framework Carbon -framework CoreVideo -framework CoreAudio")
	target_link_libraries(Orrery "-framework AVFAudio -framework AudioToolbox -framework IOKit -framework CoreAudioKit")
	target_link_libraries(Orrery "-framework CoreFoundation")

endif()

if(APPLE)
	target_link_libraries(Orrery "$ENV{VULKAN_SDK}/lib/libvulkan.1.dylib")
	target_link_libraries(Orrery "$ENV{VULKAN_SDK}/lib/libSDL2main.a")
	target_link_libraries(Orrery "$ENV{VULKAN_SDK}/lib/libSDL2.a")
	target_link_libraries(Orrery "iconv")
else()
	target_link_libraries(Orrery "$ENV{VULKAN_SDK}/lib/libvulkan.so")
	target_link_libraries(Orrery "libSDL2main.a")
	target_link_libraries(Orrery "libSDL2.a")
	target_link_libraries(Orrery "libwayland-client.so")
endif()


if(APPLE)
add_custom_command(TARGET Orrery POST_BUILD
    COMMAND cp
    COMMENT "Copying Info.plist to Orrery..."
    "${CMAKE_CURRENT_SOURCE_DIR}/macOS/Info.plist"
    COMMENT "Copying binary to Orrery..."
    "${CMAKE_CURRENT_BINARY_DIR}/Orrery.app/Contents/")


set_target_properties(Orrery PROPERTIES SKIP_BUILD_RPATH FALSE)
set_target_properties(Orrery PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
set_target_properties(Orrery PROPERTIES INSTALL_RPATH "")
set_target_properties(Orrery PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

