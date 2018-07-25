cmake_minimum_required(VERSION 3.4.1)

set(ANDROID_NATIVE_NAME androidnative)

get_filename_component(GLMINC_PREFIX
                       "${CMAKE_SOURCE_DIR}/../libs/androidnative"
                       ABSOLUTE)
add_subdirectory(${CMAKE_SOURCE_DIR}/../libs/androidnative ${CMAKE_BINARY_DIR}/androidnative)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11  -Werror -D VK_USE_PLATFORM_ANDROID_KHR")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

add_library( hello_triangle
             SHARED
             src/main/cpp/Hello_Triangle.cpp )

target_include_directories( hello_triangle PRIVATE
                            ${ANDROID_NDK}/sources/android/native_app_glue
                            ${CMAKE_CURRENT_SOURCE_DIR}/../include )

target_link_libraries( hello_triangle
                       EGL
                       GLESv3
                       ${ANDROID_NATIVE_NAME} )
