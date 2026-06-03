cmake_minimum_required(VERSION 3.16)
project(TI2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(TI2 WIN32
    main.cpp
    LfsrLogic.cpp
)

target_include_directories(TI2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32)
    target_link_libraries(TI2 PRIVATE comctl32 comdlg32 shlwapi)
endif()

if(MSVC)
    target_compile_options(TI2 PRIVATE /utf-8)
endif()
