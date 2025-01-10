cmake_minimum_required(VERSION 3.26)
project(src C)

set(CMAKE_C_STANDARD 11)

add_executable(src gen.c
        gen.h
        main.c


        TNOF.h
        TNOF.c)
