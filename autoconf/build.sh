#!/usr/bin/env bash
#
# build.sh - 递归收集 src/ 下所有源文件，全部编进 demo，产物落在 build/
#
set -e

PROJECT_NAME="demo"
PROJECT_VERSION="0.1"
BUG_REPORT="witton@163.com"

ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD="$ROOT/build"
cd "$ROOT"

# 默认编译器
CC_CMD="${CC:-gcc}"
CXX_CMD="${CXX:-g++}"

clean() {
	cd "$ROOT"
	#shopt -s extglob nullglob && rm -rf !(autogen.sh|src|build)
	rm -rf autom4te.cache build
	rm -f configure.ac Makefile.am configure configure~ aclocal.m4 Makefile.in config.log config.h.in config.h.in~ \
		compile depcomp install-sh missing ltmain.sh
}

# 允许命令行传入：./build.sh clang
case "${1:-}" in
	clang)
		CC_CMD=clang
		CXX_CMD=clang++
		;;
	gcc)
		CC_CMD=gcc
		CXX_CMD=g++
		;;
	clean)
		clean
		exit 0
		;;
	make)
		if [ -f "$BUILD"/Makefile ]; then
			cd "$BUILD"
			make -j8
			exit 0
		fi
		;;
	"")
		;;
	*)
		echo "用法: $0 [gcc|clang|make|clean]" >&2
		exit 1
		;;
esac

# 检查编译器是否可用
command -v "$CC_CMD" >/dev/null || { echo "错误: 找不到 $CC_CMD" >&2; exit 1; }
command -v "$CXX_CMD" >/dev/null || { echo "错误: 找不到 $CXX_CMD" >&2; exit 1; }

# ------------------------------------------------------------------
# 1. 前置检查
# ------------------------------------------------------------------
[ -f src/main.c ] || { echo "错误: 找不到 src/main.c" >&2; exit 1; }

# 递归收集所有 .c
SRC_LIST=$(find src -name '*.c' | sort | tr '\n' ' ')
[ -n "$SRC_LIST" ] || { echo "错误: src/ 下没有 .c 文件" >&2; exit 1; }

# 递归收集所有含头文件的目录，生成 -I 参数
INC_FLAGS=$(find src -type d -exec sh -c '[ -n "$(ls "$1"/*.h 2>/dev/null)" ] && echo -n "-I$1 "' _ {} \;)

# ------------------------------------------------------------------
# 2. 生成 configure.ac
# ------------------------------------------------------------------
cat > configure.ac <<EOF
AC_INIT([$PROJECT_NAME], [$PROJECT_VERSION], [$BUG_REPORT])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])
AC_PROG_CC
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_MKDIR_P
AM_INIT_AUTOMAKE([foreign subdir-objects])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
EOF

# ------------------------------------------------------------------
# 3. 生成 Makefile.am
# ------------------------------------------------------------------
cat > Makefile.am <<EOF
bin_PROGRAMS = $PROJECT_NAME
${PROJECT_NAME}_SOURCES  = $SRC_LIST
${PROJECT_NAME}_CPPFLAGS = $INC_FLAGS
AM_CFLAGS = -Wall -Wextra
EOF

# ------------------------------------------------------------------
# 4. 生成 configure / Makefile.in
# ------------------------------------------------------------------
autoreconf -fiv

# ------------------------------------------------------------------
# 4. 在 build/ 里做源码外构建
# ------------------------------------------------------------------
mkdir -p "$BUILD"
cd "$BUILD"
CC="$CC_CMD" CXX="$CXX_CMD" ../configure
make -j8

echo
echo "=========================================="
echo " 构建完成: $BUILD/$PROJECT_NAME"
echo " 运行:     $BUILD/$PROJECT_NAME"
echo "=========================================="
