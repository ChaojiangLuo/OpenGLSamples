file(GLOB ANDROID_NATIVE_SOURCE *.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11  -Werror -D VK_USE_PLATFORM_ANDROID_KHR")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

add_library(${ANDROID_NATIVE_NAME} STATIC ${ANDROID_NATIVE_SOURCE})

add_library(native_activity_glue STATIC
            ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c )

target_include_directories( ${ANDROID_NATIVE_NAME} PRIVATE
                            ${ANDROID_NDK}/sources/android/native_app_glue
                            ${CMAKE_CURRENT_SOURCE_DIR}/../../include )

target_link_libraries( ${ANDROID_NATIVE_NAME}
                            android
                            log
                            native_activity_glue )
