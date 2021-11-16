cmake_minimum_required(VERSION 3.20)
project(untitled C)

set(CMAKE_C_STANDARD 11)

add_library(untitled ex_1/library.c ex_1/pt.c ex_2/myshell.c)
