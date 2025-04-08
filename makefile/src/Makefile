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
LIBRARY_NAMES :=
#公共宏定义
PREPROCESSOR_MACROS :=

#工具集，Windows下最好是填写MinGW命令行工具的绝对路径，并且路径中需要使用/分隔
#因为Windows下的一些命令无相应的参数，会报错，
#比如mkdir, cp, rm, echo等等，如果安装了Git及命令行工具，建议直接使用git安装目录下usr/bin路径
CC := gcc
CXX := g++
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
ASFLAGS := -f win64
#ld链接参数
LDFLAGS := -Wl,-gc-sections
#公共编译参数，根据情况作修改
COMMONFLAGS := -g -ffunction-sections -finput-charset=UTF-8 -fexec-charset=UTF-8
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
endif

#需要编译的源文件的扩展名列表
SRC_EXTS := .c .cpp .cc .cxx .c++ .s .S .asm

#如果是在Linux下编译，需要打开此开关
#IS_LINUX_PROJECT := 1

#如果需要生成Map文件，需要打开此开关
#GENERATE_MAP_FILE := 1

#如果生成Bin文件，需要打开此开关
#GENERATE_BIN_FILE := 1

#如果生成十六进制文件，需要打开此开关
#GENERATE_IHEX_FILE := 1

#可修改区域结束，以下区域不建议修改，除非特别了解其含义

#======================华丽的分割线=============================

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

#遍历目录及子目录中的文件
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
ASFLAGS += $(addprefix -D,$(PREPROCESSOR_MACROS))

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
		ASFLAGS += -fPIC
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

define HandlePath
$(foreach x,$(foreach x,$(foreach x,$(subst //,/,$(1)),$(subst ../,*/,$(x))),$(subst ./,,$(x))),$(subst */,../,$(x)))
endef

AllSource := $(call HandlePath, $(all_source_files))
AllExcludeSource := $(call HandlePath, $(EXCLUDE_FILES))

Source := $(filter-out $(AllExcludeSource),$(AllSource))

CompileObjs := $(foreach x,$(SRC_EXTS),$(patsubst %$(x),%.o,$(filter %$(x),$(Source))))
all_objs := $(foreach x,$(CompileObjs),$(BINARYDIR)/$(x))

ASM_EXT := .s .S .asm
CompileCObjs := $(foreach x,$(filter-out $(ASM_EXT),$(SRC_EXTS)),$(patsubst %$(x),%.d,$(filter %$(x),$(Source))))
all_Cobjs := $(foreach x,$(CompileCObjs),$(BINARYDIR)/$(x))
all_Deps := $(subst ../,__/, $(foreach x,$(all_Cobjs),$(x)))

DEPS := $(all_Deps:.o=.d)

ifneq ($(PCH_H),)
AllStep += $(PCH_FILE)
endif

ifneq ($(PCH_X_H),)
AllStep += $(PCH_X_FILE)
endif

AllStep += $(CompileObjs)

WordNum := $(words $(AllStep))

ProgressInfo := $(foreach x,$(AllStep),$(eval Counter += A)$(addsuffix .$(words $(Counter)), $(basename $(x))))

define FindProgress
$(foreach x,$(ProgressInfo),$(if $(filter $(basename $(x)),$(1)),$(subst .,,$(suffix $(x))),))
endef

define ShowProgress
$(strip $(call FindProgress,$(basename $(1))))/$(WordNum)
endef

IS_GCC_ASM :=
ifneq ($(filter $(ASM),$(CXX)),)
IS_GCC_ASM = 1
else
ifneq ($(filter $(ASM),$(CC)),)
IS_GCC_ASM = 1
endif
endif

CCFLAGS = $(CFLAGS)
CC_DEPFLAGS = -MMD -MF $(@:.o=.d) -MT $@
CXX_DEPFLAGS = -MMD -MF $(@:.o=.d) -MT $@

ifndef V
define COMPILE
    @$(MKDIR) -p $(dir $@)
    @$(ECHO) [$(call ShowProgress,$<)] $(1) $<
    @$($(1)) $($(1)FLAGS) $($(1)_DEPFLAGS) -c -o $@ $<
endef
else
define COMPILE
    @$(MKDIR) -p $(dir $@)
    @$(ECHO) [$(call ShowProgress,$<)] \
    $($(1)) $($(1)FLAGS) $($(1)_DEPFLAGS) -c -o $@ $<
    @$($(1)) $($(1)FLAGS) $($(1)_DEPFLAGS) -c -o $@ $<
endef
endif

COMPILE_C = $(call COMPILE,CC)
COMPILE_CXX = $(call COMPILE,CXX)

ifeq ($(IS_GCC_ASM),1)
CompileA := $(ASM) $(CFLAGS) $(ASFLAGS) -c
else
CompileA := $(ASM) $(ASFLAGS)
endif

CompileC 	:= $(CC) $(PCH_FLAGS) $(CFLAGS) $(CC_DEPFLAGS)
CompileCXX	:= $(CXX) $(PCH_X_FLAGS) $(CXXFLAGS) $(CXX_DEPFLAGS)

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

distclean: clean

rebuild: clean all

$(BINARYDIR)/$(basename $(TARGETNAME)).bin: $(BINARYDIR)/$(TARGETNAME)
	@$(OBJCOPY) -O binary $< $(subst ../,__/, $@)

$(BINARYDIR)/$(basename $(TARGETNAME)).ihex: $(BINARYDIR)/$(TARGETNAME)
	@$(OBJCOPY) -O ihex $< $(subst ../,__/, $@)

ifeq ($(TARGETTYPE),APP)
ifneq ($(SHOW_CMD_DETAIL),)
$(BINARYDIR)/$(TARGETNAME): $(pch_objs) $(all_objs) $(EXTERNAL_LIBS)
	$(CC) -o $(subst ../,__/, $@) $(LDFLAGS) $(START_GROUP) $(subst ../,__/, $(all_objs)) $(LIBRARY_LDFLAGS) $(END_GROUP)
else
$(BINARYDIR)/$(TARGETNAME): $(pch_objs) $(all_objs) $(EXTERNAL_LIBS)
	@$(ECHO) Link App $(subst ../,__/, $@)
	@$(CC) -o $(subst ../,__/, $@) $(LDFLAGS) $(START_GROUP) $(subst ../,__/, $(all_objs)) $(LIBRARY_LDFLAGS) $(END_GROUP)
endif
endif

ifeq ($(TARGETTYPE),SHARED)
ifneq ($(SHOW_CMD_DETAIL),)
$(BINARYDIR)/$(TARGETNAME): $(pch_objs) $(all_objs) $(EXTERNAL_LIBS)
	$(CC) -shared -o $(subst ../,__/, $@) $(LDFLAGS) $(START_GROUP) $(subst ../,__/, $(all_objs)) $(LIBRARY_LDFLAGS) $(END_GROUP)
else
$(BINARYDIR)/$(TARGETNAME): $(pch_objs) $(all_objs) $(EXTERNAL_LIBS)
	@$(ECHO) Link Share lib $(subst ../,__/, $@)
	@$(CC) -shared -o $(subst ../,__/, $@) $(LDFLAGS) $(START_GROUP) $(subst ../,__/, $(all_objs)) $(LIBRARY_LDFLAGS) $(END_GROUP)
endif
endif

ifeq ($(TARGETTYPE),STATIC)
ifneq ($(SHOW_CMD_DETAIL),)
$(BINARYDIR)/$(TARGETNAME): $(pch_objs) $(all_objs)
	$(AR) -r $(subst ../,__/, $@) $(subst ../,__/, $^)
else
$(BINARYDIR)/$(TARGETNAME): $(pch_objs) $(all_objs)
	@$(ECHO) Link Static lib $(subst ../,__/, $@)
	@$(AR) -r $(subst ../,__/, $@) $(subst ../,__/, $^)
endif
endif

$(BINARYDIR):
	@$(MKDIR) $(BINARYDIR)

#Makefile的生成规则
#C文件的生成规则
$(BINARYDIR)/%.o : %.c $(PCH_FILE)
	$(COMPILE_C)

#C++的.cpp文件的生成规则
$(BINARYDIR)/%.o : %.cpp $(PCH_X_FILE)
	$(COMPILE_CXX)

#C++的.cc文件的生成规则
$(BINARYDIR)/%.o : %.cc $(PCH_X_FILE)
	$(COMPILE_CXX)

#C++的.cxx文件的生成规则
$(BINARYDIR)/%.o : %.cxx $(PCH_X_FILE)
	$(COMPILE_CXX)

#C++的.c++文件的生成规则
$(BINARYDIR)/%.o : %.c++ $(PCH_X_FILE)
	$(COMPILE_CXX)

#Asm的.S文件生成规则
$(BINARYDIR)/%.o : %.S
	$(call CompileSrc,$@,$<,$(CompileA))

#Asm的.s文件生成规则
$(BINARYDIR)/%.o : %.s
	$(call CompileSrc,$@,$<,$(CompileA))

#Asm的.asm文件生成规则
$(BINARYDIR)/%.o : %.asm
	$(call CompileSrc,$@,$<,$(CompileA))

ifneq ($(PCH_H),)
#C预编译头文件的生成规则
$(PCH_FILE) : $(PCH_H)
	@$(MKDIR) -p $(subst ../,__/, $(dir $@))
	@$(ECHO) [$(call ShowProgress,$@)] Precompiled C header $<
	@$(CC) $(CFLAGS) $> $^ -o $(subst ../,__/, $@)
endif

ifneq ($(PCH_X_H),)
#C++预编译头文件的生成规则
$(PCH_X_FILE) : $(PCH_X_H)
	@$(MKDIR) -p $(subst ../,__/, $(dir $@))
	@$(ECHO) [$(call ShowProgress,$@)] Precompiled C++ header $<
	@$(CXX) $(CXXFLAGS) $> $^ -o $(subst ../,__/, $@)
endif

-include $(DEPS)

#生成规则结束

help:
	@$(ECHO) "C/C++ general Makefile V1.4"
	@$(ECHO) "Copyright(C) 2017     E_Mail:witton@163.com by Witton Bell"
	@$(ECHO) "Usage:make [Target]"
	@$(ECHO) "Target:"
	@$(ECHO) "     all          compile and link to target directory(debug or release)"
	@$(ECHO) "     clean        remove target directory(debug or release)"
	@$(ECHO) "     rebuild      run target clean and all"
	@$(ECHO) "     help         show this help"
