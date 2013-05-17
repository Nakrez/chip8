cmake_minimum_required(VERSION 2.6)
project(chip8)

add_executable(chip8
               src/chip8.cc
               src/main.cc)

find_package(SDL REQUIRED)

target_link_libraries(chip8 ${SDL_LIBRARY})
