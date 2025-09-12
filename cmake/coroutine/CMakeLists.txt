cmake_minimum_required(VERSION 3.25)

project(coc)

# 是否使用操作系统的API。
# 如果打开，Windows下使用纤程；Linux下使用ucontext；MacOS由于不再支持ucontext，所以不可用
option(USE_NATIVE_CO "Is Use OS coroutine API" OFF)

if(UNIX)
option(Unix32 "Is 32 bit OS" OFF)
endif()

set(CMAKE_ASM_NASM_SOURCE_FILE_EXTENSIONS asm)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
set(CMAKE_ASM_NASM_FLAGS "-g")
endif()

if(MSYS)
set(CMAKE_ASM_NASM_OBJECT_FORMAT win64)
elseif(Unix32)
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf32)
endif()

enable_language(ASM_NASM)

if(USE_NATIVE_CO)
add_compile_definitions(
  USE_NATIVE_CO
)
endif()

if(MSVC OR MINGW OR MSYS)

if(MSVC)
set(CMAKE_C_FLAGS "/utf-8")
endif()

set(CMAKE_ASM_NASM_FLAGS "-D_WIN32 ${CMAKE_ASM_NASM_FLAGS}")

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
set(NASM_SRC "src/swap_ctx_x64.asm")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
set(NASM_SRC "src/swap_ctx_x86.asm")
endif()

else()

if(LINUX)
set(CMAKE_ASM_NASM_FLAGS "-D__linux__ ${CMAKE_ASM_NASM_FLAGS}")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
set(NASM_SRC "src/swap_ctx_x64.asm")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
set(NASM_SRC "src/swap_ctx_x86.asm")
endif()

if(Unix32)
message(STATUS "Unix32")
set(CMAKE_C_FLAGS "-m32")
set(NASM_SRC "src/swap_ctx_x86.asm")
endif()

endif()

set(CMAKE_C_STANDARD 11)

include_directories(../inc)

add_library(${PROJECT_NAME} ${NASM_SRC} src/co.c src/ctx.c)

include_directories(inc)

add_executable(t "main.c")
target_link_libraries(t coc)
