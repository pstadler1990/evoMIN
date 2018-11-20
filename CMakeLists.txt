cmake_minimum_required(VERSION 3.12)
project(mintest C)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c evomin.c tx_impl.c tx_impl.h testdata.h)


add_executable(mintest ${SOURCE_FILES})