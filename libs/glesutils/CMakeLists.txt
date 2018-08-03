file(GLOB ANDROID_GLESUTILS_SOURCE *.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11  -Werror -D VK_USE_PLATFORM_ANDROID_KHR")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

add_library(${ANDROID_GLESUTILS_NAME} STATIC ${ANDROID_GLESUTILS_SOURCE})

target_include_directories( ${ANDROID_GLESUTILS_NAME} PRIVATE
                            ${ANDROID_NDK}/sources/android/native_app_glue
                            ${CMAKE_CURRENT_SOURCE_DIR}/../../include )

target_link_libraries( ${ANDROID_GLESUTILS_NAME}
                            android )
