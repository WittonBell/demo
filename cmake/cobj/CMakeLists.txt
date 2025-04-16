cmake_minimum_required(VERSION 3.25.0)
project(cobj VERSION 0.1.0)

set(CMAKE_ASM_NASM_SOURCE_FILE_EXTENSIONS asm)
ENABLE_LANGUAGE(ASM_NASM)

if(CMAKE_C_COMPILER_ID MATCHES "Clang")
set(C_OPTIONS "-fms-extensions	-Wno-microsoft-anon-tag")
elseif(CMAKE_C_COMPILER_ID MATCHES "MSVC")
set(C_OPTIONS "/source-charset:utf-8 /execution-charset:utf-8 /EHsc")
set(CMAKE_ASM_NASM_FLAGS -DWIN32)
endif()

if(MINGW)
set(CMAKE_ASM_NASM_FLAGS -DWIN32)
endif()

set(CMAKE_C_FLAGS ${C_OPTIONS})
set(CMAKE_CXX_FLAGS ${C_OPTIONS})

add_executable(${PROJECT_NAME} shape.c shape.cc main.c nasm.asm)
