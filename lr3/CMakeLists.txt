cmake_minimum_required(VERSION 3.16)
project(TI3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(TI3 WIN32
    main.cpp
    MathUtils.cpp
    RsaLogic.cpp
)

target_include_directories(TI3 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32)
    target_link_libraries(TI3 PRIVATE comctl32 comdlg32 shlwapi)
endif()

if(MSVC)
    target_compile_options(TI3 PRIVATE /utf-8)
endif()
