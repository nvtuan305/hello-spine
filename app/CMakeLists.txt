cmake_minimum_required(VERSION 3.4.1)

# stb image library files
file(GLOB stb-image-lib
     "./src/main/cpp/src/libs/stb/stb_image.cpp")

# Spine library files
file(GLOB spine-lib
     "./src/main/cpp/src/libs/spine/*.c")

# Native Sticker library files
file(GLOB sticker-lib
     "./src/main/cpp/src/GLES2Utils.cpp"
     "./src/main/cpp/src/Sticker.cpp"
     "./src/main/cpp/src/StickerWrapper.cpp")

# Sticker library
add_library(sticker-lib SHARED
            ${stb-image-lib}
            ${spine-lib}
            ${sticker-lib})

# Dependency libraries for sticker-lib
target_link_libraries(sticker-lib
                      android
                      EGL
                      GLESv2
                      log)

# include directories
include_directories("./src/main/cpp/include"
                    "./src/main/cpp/include/libs")