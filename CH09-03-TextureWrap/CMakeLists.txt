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

# 3.build opengles_simples
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11  -Werror -D VK_USE_PLATFORM_ANDROID_KHR")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

add_library( opengles_simples
             SHARED
             src/main/cpp/TextureWrap.cpp )

target_include_directories( opengles_simples PRIVATE
                            ${ANDROID_NDK}/sources/android/native_app_glue
                            ${CMAKE_CURRENT_SOURCE_DIR}/../include )

target_link_libraries( opengles_simples
                       EGL
                       GLESv3
                       ${ANDROID_NATIVE_NAME}
                       ${ANDROID_GLESUTILS_NAME} )
