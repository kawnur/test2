cmake_minimum_required(VERSION 3.5)

project(bridge_utility LANGUAGES C)

set(PROJECT_SOURCES
        main.c
        utility.c
        utility.h
        README
)

add_executable(bridge_utility ${PROJECT_SOURCES})
