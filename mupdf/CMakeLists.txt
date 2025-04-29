cmake_minimum_required(VERSION 3.25.0)

project(mupdf)

if(MINGW OR LINUX)
add_compile_options(
	-gdwarf-4
)
link_libraries(mupdf mupdf-third freetype openjp2 jbig jbig2dec jpeg harfbuzz gumbo m z pthread)
elseif(MSVC)
add_compile_options(
	/source-charset:utf-8
)
include_directories(H:/OpenSource/mupdf/include/)
link_directories(H:/OpenSource/mupdf/platform/win32/x64/Debug)
link_libraries(libmupdf libthirdparty)
endif()
set(CMAKE_C_STANDARD 23)

add_executable(print_text print_text.c)
add_executable(replace_builtin replace_builtin.c)
