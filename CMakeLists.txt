cmake_minimum_required(VERSION 3.28)
project(file_system_simulator C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(file_system_simulator
        src/cabeceras.h
        src/simul_ext_esqueleto.c
)
