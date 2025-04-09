################################################################################
#
#  C/C++通用Makefile
#
#  作者：Witton Bell         E_Mail:witton@163.com
#
#  版本V1.0 （2017/02/23）
#		初始版本，可以生成App，Share以及Static库等等
#  版本V1.1 （2017/02/28）
#		添加预预编译头的支持，将生成的中间文件放在debug或者release目录下
#  版本V1.2 （2017/03/12）
#		优化以及fixed:修改头文件后，不会编译
#  版本V1.3 （2017/03/13）
#		优化：将依赖文件放在.deps目录下，make clean时不会重复生成
#  版本V1.4 （2025/04/08）
#		在编译时生成依赖文件，不再单独生成，减少编译次数。使用变量V控制显示命令行。
#		生成的中间文件放在build下的debug或者release目录中
#
#  一个项目只需要在顶层目录配置一个Makefile，不需要各个目录单独配置，简单方便实用
#  原文链接：http://blog.csdn.net/witton/article/details/56670748
#
#  功能说明：
#  1.支持Windows以及Linux下的可执行文件(App)、动态库(Share)以及静态库(Static)的生成
#  2.支持生成Map文件，Bin文件以及十六进制文件
#  3.支持C/C++的预编译头，以加快编译速度，包括同时存在C与C++预编译头的情况
#  4.生成的中间以及目标代码会指定存放在debug或者release目录下
#  5.支持生成进度显示
#
################################################################################

# 可以修改的区域

# 目标文件的名字（必须指定）
TARGETNAME := app

#目标类型可以是APP, STATIC或者SHARED
TARGETTYPE := APP

#配置类型可以是Debug或者Release
CONFIG ?= Debug

# C 预编译头文件，支持带路径
PCH_H = prec.h
# C++ 预编译头文件，支持带路径
PCH_X_H = precxx.h

#源文件目录列表，可以填多个目录，如：src1 src2 ../src
#如果没填写则默认为当前目录
SRC_ROOT_DIRS := .

#排除的目录列表，需要带上相对路径
EXCLUDE_DIRS :=

#排除的文件列表，需要带上相对路径
EXCLUDE_FILES :=

#头文件所在目录列表
INCLUDE_DIRS :=
#库文件所在目录列表
LIBRARY_DIRS :=
#公共库文件名列表
LIBRARY_NAMES := event
#公共宏定义
PREPROCESSOR_MACROS :=

#工具集，Windows下最好是填写MinGW命令行工具的绝对路径，并且路径中需要使用/分隔
#因为Windows下的一些命令无相应的参数，会报错，
#比如mkdir, cp, rm, echo等等，如果安装了Git及命令行工具，建议直接使用git安装目录下usr/bin路径
CC := clang
CXX := clang++
LD := ld
ASM := nasm
AR := ar
OBJCOPY := objcopy
CP := cp
MKDIR = mkdir
RM = rm -rf
ECHO = echo
SHELL = /bin/sh

#各种编译链接参数
#汇编编译参数
ASMFLAGS := -f win64
#ld链接参数
LDFLAGS := -Wl,-gc-sections
#公共编译参数，根据情况作修改
COMMONFLAGS := -gdwarf-4 -ffunction-sections -finput-charset=UTF-8 -fexec-charset=UTF-8
#额外的库文件
EXTERNAL_LIBS :=

#根据配置类型填写不同的参数
ifeq ($(CONFIG), Debug)
	PREPROCESSOR_MACROS += DEBUG
	LIBRARY_NAMES +=
	ADDITIONAL_LINKER_INPUTS :=
	MACOS_FRAMEWORKS :=
	LINUX_PACKAGES :=
	CFLAGS := -O0 -Wall
	CXXFLAGS := -O0
	ASMFLAGS += -g
else
	PREPROCESSOR_MACROS +=
	LIBRARY_NAMES +=
	ADDITIONAL_LINKER_INPUTS :=
	MACOS_FRAMEWORKS :=
	LINUX_PACKAGES :=
	CFLAGS := -O3
	CXXFLAGS := -O3
endif

ifeq ($(CC), gcc)
COMMONFLAGS += -fwide-exec-charset=UTF-16LE
else ifeq ($(CC), clang)
else
$(error Unsupport Compiler)
endif

#如果需要生成Map文件，需要打开此开关
#GENERATE_MAP_FILE := 1

#如果生成Bin文件，需要打开此开关
#GENERATE_BIN_FILE := 1

#如果生成十六进制文件，需要打开此开关
#GENERATE_IHEX_FILE := 1

#可修改区域结束，以下区域不建议修改，除非特别了解其含义

#======================华丽的分割线=============================

#需要编译的源文件的扩展名列表
SRC_EXTS := .c .cpp .cc .cxx .c++ .s .S .asm

# 平台检测，Windows下的路径以分号分隔，其它的以冒号分隔
ifeq ($(findstring ;,$(PATH)), ;)
    UNAME := Windows
else
    UNAME := $(shell uname 2>/dev/null || echo Unknown)
    UNAME := $(patsubst CYGWIN%,Cygwin,$(UNAME))
    UNAME := $(patsubst MSYS%,MSYS,$(UNAME))
    UNAME := $(patsubst MINGW%,MINGW,$(UNAME))
endif

#根据平台定义相应的宏
ifneq ($(findstring $(UNAME), Windows, Cygwin, MSYS, MINGW),)
PREPROCESSOR_MACROS += WIN32 WIN64
else ifeq ($(UNAME), Linux)
PREPROCESSOR_MACROS += LINUX
IS_LINUX_PROJECT := 1
else ifeq ($(UNAME), Darwin)
PREPROCESSOR_MACROS += DARWIN OSX
else ifeq ($(UNAME), FreeBSD)
PREPROCESSOR_MACROS += FREEBSD
else
$(warning Platform:$(UNAME) not define `Platform Macro`)
endif

#小写函数
to_lowercase = $(subst A,a,$(subst B,b,$(subst C,c,$(subst D,d,$(subst E,e,\
	$(subst F,f,$(subst G,g,$(subst H,h,$(subst I,i,$(subst J,j,$(subst K,k,\
	$(subst L,l,$(subst M,m,$(subst N,n,$(subst O,o,$(subst P,p,$(subst Q,q,\
	$(subst R,r,$(subst S,s,$(subst T,t,$(subst U,u,$(subst V,v,$(subst W,w,\
	$(subst X,x,$(subst Y,y,$(subst Z,z,$1))))))))))))))))))))))))))

#设置输出目录
BINARYDIR := build/$(strip $(call to_lowercase,$(CONFIG)))

#如果没配置源文件目录，则默认为当前目录
ifeq ($(SRC_ROOT_DIRS),)
	SRC_ROOT_DIRS = .
endif

# 这里需要设置一下路径，以支持预编译头的查找
vpath %.h $(SRC_ROOT_DIRS) $(SRC_ROOT_DIRS)/src

#遍历目录及子目录中指定扩展名的文件
define walk
$(wildcard $(addprefix $(1)/*, $(SRC_EXTS))) $(foreach e, $(wildcard $(1)/*), $(call walk, $(e)))
endef

#匹配所有源文件
SOURCEFILES := $(foreach e, $(SRC_ROOT_DIRS), $(call walk, $(e)))
#匹配所有需要排除的源文件
EXCLUDE_SOURCE := $(foreach e, $(EXCLUDE_DIRS), $(call walk, $(e)))
EXCLUDE_FILES += $(EXCLUDE_SOURCE)
EXTERNAL_LIBS_COPIED := $(foreach lib, $(EXTERNAL_LIBS),$(BINARYDIR)/$(notdir $(lib)))

#设置C预编译相关变量
ifneq ($(PCH_H),)
PCH = $(PCH_H).gch
PCH_FILE := $(BINARYDIR)/$(PCH)
PCH_FLAGS := -Winvalid-pch -include $(PCH_FILE)
pch_objs += $(PCH_FILE)
endif

#设置C++预编译相关变量
ifneq ($(PCH_X_H),)
PCH_X = $(PCH_X_H).gch
PCH_X_FILE := $(BINARYDIR)/$(PCH_X)
PCH_X_FLAGS := -Winvalid-pch -include $(PCH_X_FILE)
pch_objs += $(PCH_X_FILE)
endif

START_GROUP := -Wl,--start-group
END_GROUP := -Wl,--end-group
INCLUDE_DIRS += .

#处理头文件目录
CFLAGS += $(addprefix -I,$(INCLUDE_DIRS))
CXXFLAGS += $(addprefix -I,$(INCLUDE_DIRS))

#处理宏定义
CFLAGS += $(addprefix -D,$(PREPROCESSOR_MACROS))
CXXFLAGS += $(addprefix -D,$(PREPROCESSOR_MACROS))
ASMFLAGS += $(addprefix -D,$(PREPROCESSOR_MACROS))

#处理框架
CFLAGS += $(addprefix -framework,$(MACOS_FRAMEWORKS))
CXXFLAGS += $(addprefix -framework,$(MACOS_FRAMEWORKS))
LDFLAGS += $(addprefix -framework,$(MACOS_FRAMEWORKS))

#处理库目录
LDFLAGS += $(addprefix -L,$(LIBRARY_DIRS))

CFLAGS += $(COMMONFLAGS)
CXXFLAGS += $(COMMONFLAGS)
LDFLAGS += $(COMMONFLAGS)

ifeq ($(GENERATE_MAP_FILE),1)
LDFLAGS += -Wl,-Map=$(BINARYDIR)/$(basename $(TARGETNAME)).map
endif

LIBRARY_LDFLAGS = $(addprefix -l,$(LIBRARY_NAMES))

ifeq ($(IS_LINUX_PROJECT),1)

ifeq ($(TARGETTYPE),SHARED)
TempName = $(addsuffix .so,$(basename $(TARGETNAME)))
TARGETNAME := $(TempName)
endif

	RPATH_PREFIX := -Wl,--rpath='$$ORIGIN/../
	LIBRARY_LDFLAGS += $(EXTERNAL_LIBS)
	LIBRARY_LDFLAGS += -Wl,--rpath='$$ORIGIN'
	LIBRARY_LDFLAGS += $(addsuffix ',$(addprefix $(RPATH_PREFIX),$(dir $(EXTERNAL_LIBS))))

	#如果是Linux下的共享库（Share）项目，则需要添加-fPIC参数，以实现位置无关代码
	ifeq ($(TARGETTYPE),SHARED)
		CFLAGS += -fPIC
		CXXFLAGS += -fPIC
		ASMFLAGS += -fPIC
		LIBRARY_LDFLAGS += -Wl,-soname,$(TARGETNAME)
	endif

	ifneq ($(LINUX_PACKAGES),)
		PACKAGE_CFLAGS := $(foreach pkg,$(LINUX_PACKAGES),$(shell pkg-config --cflags $(pkg)))
		PACKAGE_LDFLAGS := $(foreach pkg,$(LINUX_PACKAGES),$(shell pkg-config --libs $(pkg)))
		CFLAGS += $(PACKAGE_CFLAGS)
		CXXFLAGS += $(PACKAGE_CFLAGS)
		LIBRARY_LDFLAGS += $(PACKAGE_LDFLAGS)
	endif
else
	LIBRARY_LDFLAGS += $(EXTERNAL_LIBS)

ifeq ($(TARGETTYPE),APP)
TempName = $(addsuffix .exe,$(basename $(TARGETNAME)))
TARGETNAME := $(TempName)
endif

ifeq ($(TARGETTYPE),SHARED)
TempName = $(addsuffix .dll,$(basename $(TARGETNAME)))
TARGETNAME := $(TempName)
endif

endif

LIBRARY_LDFLAGS += $(ADDITIONAL_LINKER_INPUTS)

#静态库都是以.a为后缀
ifeq ($(TARGETTYPE),STATIC)
TempName = $(addsuffix .a,$(basename $(TARGETNAME)))
TARGETNAME := $(TempName)
endif

ifeq ($(STARTUPFILES),)
	all_source_files := $(SOURCEFILES)
else
	all_source_files := $(STARTUPFILES) $(filter-out $(STARTUPFILES),$(SOURCEFILES))
endif

# 处理路径，将//替换成/，去掉./
define HandlePath
$(foreach x,\
	$(foreach x,\
		$(foreach x,\
			$(subst //,/,$(1)),\
			$(x)),\
		$(subst ./,,$(x))),\
	$(x))
endef

AllSource := $(call HandlePath, $(all_source_files))
AllExcludeSource := $(call HandlePath, $(EXCLUDE_FILES))

Source := $(filter-out $(AllExcludeSource),$(AllSource))
#直接在文件名后面添加.o扩展名，以避免%.c与%.cc生成同名%.o
CompileObjs := $(foreach x,$(SRC_EXTS),$(addsuffix .o,$(filter %$(x),$(Source))))
all_objs := $(foreach x,$(CompileObjs),$(BINARYDIR)/$(x))

# 筛选C/C++源文件
C_EXT := .c .C .cc .cpp .cxx .c++
CompileCObjs := $(foreach x,$(filter $(C_EXT),$(SRC_EXTS)),$(patsubst %$(x),%.d,$(filter %$(x),$(Source))))
all_Cobjs := $(foreach x,$(CompileCObjs),$(BINARYDIR)/$(x))
all_Deps := $(foreach x,$(all_Cobjs),$(x))

DEPS := $(all_Deps:.o=.d)

ifneq ($(PCH_H),)
AllStep += $(call HandlePath, $(PCH_FILE))
endif

ifneq ($(PCH_X_H),)
AllStep += $(call HandlePath, $(PCH_X_FILE))
endif

AllStep += $(CompileObjs)

WordNum := $(words $(AllStep))

ProgressInfo := $(foreach x,$(AllStep),$(eval Counter += A)$(addsuffix .$(words $(Counter)), $(basename $(x))))

define FindProgress
$(foreach x,$(ProgressInfo),$(if $(filter $(basename $(x)),$(1)),$(subst .,,$(suffix $(x))),))
endef

define ShowProgress
$(strip $(call FindProgress,$(1)))/$(WordNum)
endef

# IS_GCC_ASM :=
# ifneq ($(filter $(ASM),$(CXX)),)
# IS_GCC_ASM = 1
# else
# ifneq ($(filter $(ASM),$(CC)),)
# IS_GCC_ASM = 1
# endif
# endif

CCFLAGS = $(CFLAGS)
CC_DEPFLAGS = -MMD -MF $(@:.o=.d) -MT $@
CC_PCH_FLAGS = $(PCH_FLAGS)
CC_C = -c

CXX_DEPFLAGS = -MMD -MF $(@:.o=.d) -MT $@
CXX_PCH_FLAGS = $(PCH_X_FLAGS)
CXX_C = -c

# NASM没有-c参数
ASM_C =

ifndef V
define COMPILE
    @$(MKDIR) -p $(dir $@)
    @$(ECHO) [$(call ShowProgress,$<)] $(1) $<
    @$($(1)) $($(1)FLAGS) $($(1)_DEPFLAGS) $($(1)_C) $< -o $@
endef
else
define COMPILE
    @$(MKDIR) -p $(dir $@)
    @$(ECHO) [$(call ShowProgress,$<)] \
    $($(1)) $($(1)FLAGS) $($(1)_DEPFLAGS) $($(1)_C) $< -o $@
    @$($(1)) $($(1)FLAGS) $($(1)_DEPFLAGS) $($(1)_C) $< -o $@
endef
endif

COMPILE_C = $(call COMPILE,CC)
COMPILE_CXX = $(call COMPILE,CXX)
COMPILE_ASM = $(call COMPILE,ASM)

# ifeq ($(IS_GCC_ASM),1)
# CompileASM := $(ASM) $(CFLAGS) $(ASMFLAGS) -c $< -o $@
# else
# CompileASM := $(ASM) $(ASMFLAGS) -c $< -o $@
# endif

PRIMARY_OUTPUTS :=

ifeq ($(GENERATE_BIN_FILE),1)
PRIMARY_OUTPUTS += $(BINARYDIR)/$(basename $(TARGETNAME)).bin
endif

ifeq ($(GENERATE_IHEX_FILE),1)
PRIMARY_OUTPUTS += $(BINARYDIR)/$(basename $(TARGETNAME)).ihex
endif

ifeq ($(PRIMARY_OUTPUTS),)
PRIMARY_OUTPUTS := $(BINARYDIR)/$(TARGETNAME)
endif

.PHONY: all clean rebuild help

.SUFFIXES:

all: $(PRIMARY_OUTPUTS)
	@$(ECHO) Built OK.

clean:
	@$(RM) $(BINARYDIR)

rebuild: clean all

$(BINARYDIR)/$(basename $(TARGETNAME)).bin: $(BINARYDIR)/$(TARGETNAME)
	@$(OBJCOPY) -O binary $< $@

$(BINARYDIR)/$(basename $(TARGETNAME)).ihex: $(BINARYDIR)/$(TARGETNAME)
	@$(OBJCOPY) -O ihex $< $@

ifeq ($(TARGETTYPE),APP)
ifneq ($(SHOW_CMD_DETAIL),)
$(BINARYDIR)/$(TARGETNAME): $(pch_objs) $(all_objs) $(EXTERNAL_LIBS)
	$(CC) -o $@ $(LDFLAGS) $(START_GROUP) $(all_objs) $(LIBRARY_LDFLAGS) $(END_GROUP)
else
$(BINARYDIR)/$(TARGETNAME): $(pch_objs) $(all_objs) $(EXTERNAL_LIBS)
	@$(ECHO) Link App $@
	@$(CC) -o $@ $(LDFLAGS) $(START_GROUP) $(all_objs) $(LIBRARY_LDFLAGS) $(END_GROUP)
endif
endif

ifeq ($(TARGETTYPE),SHARED)
ifneq ($(SHOW_CMD_DETAIL),)
$(BINARYDIR)/$(TARGETNAME): $(pch_objs) $(all_objs) $(EXTERNAL_LIBS)
	$(CC) -shared -o $@ $(LDFLAGS) $(START_GROUP) $(all_objs) $(LIBRARY_LDFLAGS) $(END_GROUP)
else
$(BINARYDIR)/$(TARGETNAME): $(pch_objs) $(all_objs) $(EXTERNAL_LIBS)
	@$(ECHO) Link Share lib $@
	@$(CC) -shared -o $@ $(LDFLAGS) $(START_GROUP) $(all_objs) $(LIBRARY_LDFLAGS) $(END_GROUP)
endif
endif

ifeq ($(TARGETTYPE),STATIC)
ifneq ($(SHOW_CMD_DETAIL),)
$(BINARYDIR)/$(TARGETNAME): $(pch_objs) $(all_objs)
	$(AR) -r $@ $^
else
$(BINARYDIR)/$(TARGETNAME): $(pch_objs) $(all_objs)
	@$(ECHO) Link Static lib $@
	@$(AR) -r $@ $^
endif
endif

$(BINARYDIR):
	@$(MKDIR) $(BINARYDIR)

#Makefile的生成规则
#C文件的生成规则
$(BINARYDIR)/%.c.o : %.c $(PCH_FILE)
	$(COMPILE_C)

#C++的.cpp文件的生成规则
$(BINARYDIR)/%.cpp.o : %.cpp $(PCH_X_FILE)
	$(COMPILE_CXX)

#C++的.cc文件的生成规则
$(BINARYDIR)/%.cc.o : %.cc $(PCH_X_FILE)
	$(COMPILE_CXX)

#C++的.cxx文件的生成规则
$(BINARYDIR)/%.cxx.o : %.cxx $(PCH_X_FILE)
	$(COMPILE_CXX)

#C++的.c++文件的生成规则
$(BINARYDIR)/%.c++.o : %.c++ $(PCH_X_FILE)
	$(COMPILE_CXX)

#Asm的.S文件生成规则
$(BINARYDIR)/%.S.o : %.S
	$(CompileASM)

#Asm的.s文件生成规则
$(BINARYDIR)/%.s.o : %.s
	$(CompileASM)

#Asm的.asm文件生成规则
$(BINARYDIR)/%.asm.o : %.asm
	$(COMPILE_ASM)

ifneq ($(PCH_H),)
#C预编译头文件的生成规则
$(PCH_FILE) : $(PCH_H)
	@$(MKDIR) -p $(dir $@)
	@$(ECHO) [$(call ShowProgress,$(basename $@))] Precompiled C header $<
	@$(CC) $(CFLAGS) -x c-header $^ -o $@
endif

ifneq ($(PCH_X_H),)
#C++预编译头文件的生成规则
$(PCH_X_FILE) : $(PCH_X_H)
	@$(MKDIR) -p $(dir $@)
	@$(ECHO) [$(call ShowProgress,$(basename $@))] Precompiled C++ header $<
	@$(CXX) $(CXXFLAGS) -x c++-header $^ -o $@
endif

-include $(DEPS)

#生成规则结束

help:
	@$(ECHO) "C/C++ general Makefile V1.4"
	@$(ECHO) "Copyright(C) 2017 ~ 2025 E_Mail:witton@163.com by Witton Bell"
	@$(ECHO) "Usage:make [Target]"
	@$(ECHO) "Target:"
	@$(ECHO) "     all          compile and link to target directory(debug or release)"
	@$(ECHO) "     clean        remove target directory(debug or release)"
	@$(ECHO) "     rebuild      run target clean and all"
	@$(ECHO) "     help         show this help"

#
# 自动化变量说明：
# $@
# 表示规则的目标文件名。如果目标是一个文档文件（Linux中，一般称.a文件为
# 文档文件，也称为静态库文件），那么它代表这个文档的文件名。在多目标模式
# 规则中，它代表的是哪个触发规则被执行的目标文件名。
#
# $%
# 当规则的目标文件是一个静态库文件时，代表静态库的一个成员名。例如，规则
# 的目标是“foo.a(bar.o)”，那么，“$%”的值就为“bar.o”，“$@”的值为“foo.a”。
# 如果目标不是静态库文件，其值为空。
#
# $<
# 规则的第一个依赖文件名。如果是一个目标文件使用隐含规则来重建，则它代表
# 由隐含规则加入的第一个依赖文件。
#
# $?
# 所有比目标文件更新的依赖文件列表，空格分割。如果目标是静态库文件名，代
# 表的是库成员（.o文件）。
#
# $^
# 规则的所有依赖文件列表，使用空格分隔。如果目标是静态库文件，它所代表的
# 只能是所有库成员（.o文件）名。一个文件可重复的出现在目标的依赖中，变量
# “$^”只记录它的一次引用情况。就是说变量“$^”会去掉重复的依赖文件。
#
# $+
# 类似“$^”，但是它保留了依赖文件中重复出现的文件。主要用在程序链接时库
# 的交叉引用场合。
#
# $*
# 在模式规则和静态模式规则中，代表“茎”。“茎”是目标模式中“%”所代表的
# 部分（当文件名中存在目录时，“茎”也包含目录（斜杠之前）部分，
# 例如：文件“dir/a.foo.b”，当目标的模式为“a.%.b”时，
# “$*”的值为“dir/a.foo”。“茎”对于构造相关文件名非常有用。
# 自动化变量“$*”需要两点说明：
# 1.对于一个明确指定的规则来说不存在“茎”，这种情况下“$*”的含义发
# 生改变。此时，如果目标文件名带有一个可识别的后缀，
# 那么“$*”表示文件中除后缀以外的部分。例如：“foo.c”
# 则“$*”的值为：“foo”，因为.c是一个可识别的文件后缀名。GUN make
# 对明确规则的这种奇怪的处理行为是为了和其它版本的make兼容。通
# 常，在除静态规则和模式规则以外，明确指定目标文件的规则中应该避免使用这个变量。
# 2.当明确指定文件名的规则中目标文件名包含不可识别的后缀时，此变量
# 为空。
# 自动化变量“$?”在显式规则中也是非常有用的，使用它规则可以指定只对更新
# 以后的依赖文件进行操作。例如，静态库文件“libN.a”，它由一些.o文件组成。这个规
# 则实现了只将更新后的.o文件加入到库中：
# lib: foo.o bar.o lose.o win.o
#  ar r lib $?
# 以上罗列的自动量变量中。其中有四个在规则中代表文件名（$@、$<、$%、$*）。
# 而其它三个的在规则中代表一个文件名列表。GUN make中，还可以通过这七个自动化
# 变量来获取一个完整文件名中的目录部分和具体文件名部分。在这些变量中加入“D”
# 或者“F”字符就形成了一系列变种的自动环变量。这些变量会出现在以前版本的make
# 中，在当前版本的make中，可以使用“dir”或者“notdir”函数来实现同样的功能。
#
# $(@D)
# 表示目标文件的目录部分（不包括斜杠）。如果“$@”是“dir/foo.o”，那么“$(@D)”
# 的值为“dir”。如果“$@”不存在斜杠，其值就是“.”（当前目录）。注意它和函
# 数“dir”的区别！
#
# $(@F)
# 目标文件的完整文件名中除目录以外的部分（实际文件名）。如果“$@”为
# “dir/foo.o”，那么“$(@F)”只就是“foo.o”。“$(@F)”等价于函数“$(notdir $@)”。
#
# $(*D)
# $(*F)
# 分别代表目标“茎”中的目录部分和文件名部分。
#
# $(%D)
# $(%F)
# 当以如“archive(member)”形式静态库为目标时，分别表示库文件成员
# “member”名中的目录部分和文件名部分。它仅对这种形式的规则目标有效。
#
# $(<D)
# $(<F)
# 分别表示规则中第一个依赖文件的目录部分和文件名部分。
#
# $(^D)
# $(^F)
# 分别表示所有依赖文件的目录部分和文件部分（不存在同一文件）。
#
# $(+D)
# $(+F)
# 分别表示所有依赖文件的目录部分和文件部分（可存在重复文件）。
#
# $(?D)
# $(?F)
# 分别表示被更新的依赖文件的目录部分和文件名部分。
