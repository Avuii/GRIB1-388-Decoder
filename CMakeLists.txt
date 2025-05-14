cmake_minimum_required(VERSION 3.30)
project(grib)

set(CMAKE_CXX_STANDARD 20)

set(SOURCES
        main.cpp
        methods.h
        classes.h
)

add_executable(grib ${SOURCES})