cmake_minimum_required(VERSION 3.29)
project(Endless_Runner)

set(CMAKE_CXX_STANDARD 20)

set(SDL2_DEV_DIR "E:/newest_SDL/SDL2-2.32.8/x86_64-w64-mingw32")

add_subdirectory(box2d-main)

add_executable(Endless_Runner main.cpp
        Game.cpp
        Game.h
        Object.cpp
        Object.h
        Audio_Manager.cpp
        Audio_Manager.h
)

target_include_directories(Endless_Runner PRIVATE "${SDL2_DEV_DIR}/include")

target_link_libraries(Endless_Runner PRIVATE
        box2d
        "mingw32"
        "${SDL2_DEV_DIR}/lib/libSDL2main.a"
        "${SDL2_DEV_DIR}/lib/libSDL2.a"
        "${SDL2_DEV_DIR}/lib/libSDL2_image.a"
        "${SDL2_DEV_DIR}/lib/libSDL2_ttf.a"
        "${SDL2_DEV_DIR}/lib/libSDL2_mixer.a"
        mingw32
        winmm
        setupapi
        imm32
        version
        rpcrt4
)