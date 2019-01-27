cmake_minimum_required(VERSION 2.8.4)
project(RayneUI)

include(../../CMake/LibraryTarget.cmake)
include(../../CMake/CopyResources.cmake)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 5)
set(VERSION_PATCH 0)
set(VERSION_ABI 1)
set(VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

set(SOURCES
        Sources/RNUIContext.cpp
        Sources/RNUIPath.cpp
        Sources/RNUIView.cpp
        Sources/RNUILabel.cpp
        Sources/RNUIWindow.cpp
        Sources/RNUIServer.cpp
        Sources/RNUIInit.cpp
        Sources/RNUIEdgeInsets.h
        Sources/RNUIImage.cpp
        Sources/RNUIFont.cpp
        Sources/RNUIImageView.cpp
        Sources/RNUIScrollView.cpp
        Sources/RNUIInternals.h)

set(RESOURCES
        Resources)

set(HEADERS
        Sources/RNUIContext.h
        Sources/RNUIPath.h
        Sources/RNUIView.h
        Sources/RNUILabel.h
        Sources/RNUIWindow.h
        Sources/RNUIServer.h
        Sources/RNUIConfig.h
        Sources/RNUIEdgeInsets.h
        Sources/RNUIImage.h
        Sources/RNUIFont.h
        Sources/RNUIImageView.h
        Sources/RNUIScrollView.h
        Sources/RNUI.h)

set(DEFINES
        "RN_BUILD_UI")

if(WIN32)
set(RAYNE_LIBRARIES Rayne)
elseif(APPLE)
#set(RAYNE_LIBRARIES Rayne ${CMAKE_CURRENT_SOURCE_DIR}/Vendor/libskia/macos/libskia.a ${CMAKE_CURRENT_SOURCE_DIR}/Vendor/libskia/macos/libskiashaper.a)
set(RAYNE_LIBRARIES Rayne ${CMAKE_CURRENT_SOURCE_DIR}/Vendor/libskia/macos/libskia.a)
elseif(ANDROID)
#set(RAYNE_LIBRARIES Rayne ${CMAKE_CURRENT_SOURCE_DIR}/Vendor/libskia/android/libskia.a ${CMAKE_CURRENT_SOURCE_DIR}/Vendor/libskia/android/libskiashaper.a)
set(RAYNE_LIBRARIES Rayne ${CMAKE_CURRENT_SOURCE_DIR}/Vendor/libskia/android/libskia.a)
endif()

set(INCLUDE_DIRECTORIES ${Rayne_BINARY_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/Vendor/libskia/ ${CMAKE_CURRENT_SOURCE_DIR}/Vendor/libskia/include/core ${CMAKE_CURRENT_SOURCE_DIR}/Vendor/libskia/include/gpu ${CMAKE_CURRENT_SOURCE_DIR}/Vendor/libskia/include/config)

rayne_add_library(RayneUI "${SOURCES}" "${HEADERS}" "${RAYNE_LIBRARIES}" "${VERSION_STRING}" "${VERSION_ABI}")
rayne_set_module_output_directory(RayneUI)
#rayne_copy_resources(RayneUI ${RESOURCES})

if(WIN32)
#target_link_libraries(RayneUI debug ${CMAKE_CURRENT_SOURCE_DIR}/Vendor/libskia/windows/debug/skia.lib debug ${CMAKE_CURRENT_SOURCE_DIR}/Vendor/libskia/windows/debug/skiashaper.lib optimized ${CMAKE_CURRENT_SOURCE_DIR}/Vendor/libskia/windows/release/skia.lib optimized ${CMAKE_CURRENT_SOURCE_DIR}/Vendor/libskia/windows/release/skiashaper.lib)
target_link_libraries(RayneUI debug ${CMAKE_CURRENT_SOURCE_DIR}/Vendor/libskia/windows/debug/skia.lib optimized ${CMAKE_CURRENT_SOURCE_DIR}/Vendor/libskia/windows/release/skia.lib)
endif()

target_include_directories(RayneUI SYSTEM PRIVATE "${INCLUDE_DIRECTORIES}")
target_compile_definitions(RayneUI PRIVATE "${DEFINES}")


rayne_install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../../Build/RayneUI/. DESTINATION lib/Rayne/RayneUI)