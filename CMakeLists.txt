# SPDX-FileCopyrightText: Copyright 2025 AsmCube Project
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.24)

project(AsmCube LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE True)

if(MSVC)
    add_compile_options(/Zc:preprocessor)
endif()

set(SOURCES
    src/cereal_overrides.h
    src/global_state.h
    src/logging.h
    src/magic_enum_overrides.h
    src/types.h
    src/lexer/lexer.cpp
    src/lexer/lexer.h
    src/parser/parser.cpp
    src/parser/parser.h
    src/interpreter/instructions.cpp
    src/interpreter/instructions.h
    src/interpreter/interpreter.cpp
    src/interpreter/interpreter.h
    src/interpreter/memory.h
    src/interpreter/registers.h
    src/interpreter/self_test.cpp
    src/interpreter/self_test.h
    src/interpreter/symbol_table.h
    src/interpreter/syscalls.cpp
    src/interpreter/syscalls.h
    src/testcases/loader.cpp
    src/testcases/loader.h
    src/testcases/testcase.h
    src/main.cpp
)

SET(EXTERNALS
    src/externals/cereal/include
    src/externals/argparse/include
    src/externals/magic_enum/include
)

add_subdirectory(src/externals/rapidyaml)

add_executable(AsmCube ${SOURCES})

target_include_directories(AsmCube PRIVATE
    ${EXTERNALS}
    src
)

target_link_libraries(AsmCube PRIVATE ryml)