# example usage:
# cmake ..
# cmake --build . --config Release  


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
cmake_minimum_required(VERSION 3.28.1)

project(Orrery LANGUAGES CXX)
add_compile_definitions(VBB_USE_SHADER_TOOLCHAIN)
add_compile_definitions(VK_NO_PROTOTYPES)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# This finds the Vulkan SDK
find_package(Vulkan)


# Source code in this folder and VBB up a few folders
file(GLOB FILES_HEADER ./*.h ../../include/*.h)
file(GLOB FILES_SOURCE ./*.cpp ../../src/*.cpp)

if(APPLE)
	add_executable(Orrery MACOSX_BUNDLE ${FILES_HEADER} ${FILES_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/LunarGIcon.icns)
	set_target_properties(Orrery PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/macOS/Info.plist)
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	add_executable(Orrery ${FILES_HEADER} ${FILES_SOURCE})
endif()

set_target_properties(Orrery PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(Orrery PRIVATE "$ENV{VULKAN_SDK}/include" "$ENV{VULKAN_SDK}/include/SDL2" "$ENV{VULKAN_SDK}/include/SDL2" "./" "../../include")
target_link_directories(Orrery PRIVATE $ENV{VULKAN_SDK}/lib)

if(APPLE)
	set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/LunarGIcons.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
	target_compile_definitions(Orrery PRIVATE VK_USE_PLATFORM_MACOS_MVK)

	install(TARGETS Orrery RUNTIME DESTINATION MacOS BUNDLE DESTINATION MacOS) 
	target_link_libraries(Orrery "-framework Cocoa -framework QuartzCore -framework Metal -framework GameController")
	target_link_libraries(Orrery "-framework ForceFeedback -framework CoreHaptics -framework InputMethodKit -framework CoreGraphics")
	target_link_libraries(Orrery "-framework AppKit -framework Carbon -framework CoreVideo -framework CoreAudio")
	target_link_libraries(Orrery "-framework AVFAudio -framework AudioToolbox -framework IOKit -framework CoreAudioKit")
	target_link_libraries(Orrery "-framework CoreFoundation")
	target_link_libraries(Orrery "$ENV{VULKAN_SDK}/lib/libvulkan.1.dylib")
	target_link_libraries(Orrery "$ENV{VULKAN_SDK}/lib/libSDL2main.a")
	target_link_libraries(Orrery "$ENV{VULKAN_SDK}/lib/libSDL2.a")
	target_link_libraries(Orrery "iconv")
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	target_link_libraries(Orrery "SDL2main.lib")
	target_link_libraries(Orrery "SDL2.lib")
	target_link_libraries(Orrery "shaderc_combined.lib")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	target_link_libraries(Orrery "$ENV{VULKAN_SDK}/lib/libvulkan.so")
	target_link_libraries(Orrery "libshaderc_combined.lib")
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

