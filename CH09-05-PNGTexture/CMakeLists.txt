cmake_minimum_required(VERSION 3.4.1)

# 1.build androidnative
set(ANDROID_NATIVE_NAME androidnative)

get_filename_component(GLMINC_PREFIX
        "${CMAKE_SOURCE_DIR}/../libs/androidnative"
        ABSOLUTE)

add_subdirectory(${CMAKE_SOURCE_DIR}/../libs/androidnative ${CMAKE_BINARY_DIR}/androidnative)

# 2.build glesutils
set(ANDROID_GLESUTILS_NAME glesutils)

get_filename_component(GLMINC_PREFIX
        "${CMAKE_SOURCE_DIR}/../libs/glesutils"
        ABSOLUTE)

add_subdirectory(${CMAKE_SOURCE_DIR}/../libs/glesutils ${CMAKE_BINARY_DIR}/glesutils)

# 3.build zlib
get_filename_component(GLMINC_PREFIX
        "${CMAKE_SOURCE_DIR}/zlib/zlib-1.2.11"
        ABSOLUTE)

add_subdirectory(${CMAKE_SOURCE_DIR}/zlib/zlib-1.2.11 ${CMAKE_BINARY_DIR}/zlib)

# 4.build libpng
add_definitions(-DPNG_STATIC=ON)
add_definitions(-DPNG_SHARED=OFF)
add_definitions(-DPNG_TESTS=OFF)

set(PNG_STATIC ON)
set(PNG_SHARED OFF)
set(PNG_TESTS OFF)

get_filename_component(GLMINC_PREFIX
        "${CMAKE_SOURCE_DIR}/png/libpng-1.6.37"
        ABSOLUTE)

add_subdirectory(${CMAKE_SOURCE_DIR}/png/libpng-1.6.37 ${CMAKE_BINARY_DIR}/png)

# 5.build opengles_simples
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11  -Werror -D VK_USE_PLATFORM_ANDROID_KHR")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

add_library( opengles_simples
        SHARED
        ${CMAKE_SOURCE_DIR}/src/main/cpp/utils.cpp
        ${CMAKE_SOURCE_DIR}/src/main/cpp/PngHandler.cpp
        ${CMAKE_SOURCE_DIR}/src/main/cpp/TextureWrap.cpp
        )

# 6.include header
target_include_directories( opengles_simples PRIVATE
        ${ANDROID_NDK}/sources/android/native_app_glue
        ${CMAKE_SOURCE_DIR}/../include
        ${CMAKE_SOURCE_DIR}/zlib/zlib-1.2.11
        ${CMAKE_SOURCE_DIR}/png/libpng-1.6.37
        )
# 7.link static libs
target_link_libraries( opengles_simples
        EGL
        GLESv3
        ${ANDROID_NATIVE_NAME}
        ${ANDROID_GLESUTILS_NAME}
        zlibstatic
        png_static
        )