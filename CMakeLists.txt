cmake_minimum_required(VERSION 3.24)

project(AsmCube LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE True)

set(SOURCES
    src/cereal_overrides.h
    src/magic_enum_overrides.h
    src/lexer.cpp
    src/lexer.h
    src/parser.cpp
    src/parser.h
    src/main.cpp
)

SET(EXTERNALS
    src/externals/cereal/include
    src/externals/argparse/include
    src/externals/magic_enum/include
)

add_executable(AsmCube ${SOURCES})

target_include_directories(AsmCube PRIVATE
    ${EXTERNALS}
    src
)