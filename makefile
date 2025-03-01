#生成makefile所在的目录的绝对路径
ARCH = $(shell uname -m)
#MAKEFILE_LIST是make工具定义的环境变量，最后一个值就是当前的makefile的启动路径（可能是相对路径）
TOP_DIR := $(patsubst %/, %, $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

#各个目录
INC_DIR := $(TOP_DIR)/include
SRC_DIR := $(TOP_DIR)/src
BIN_DIR := $(TOP_DIR)/release
BIN_DIR_DBG := $(TOP_DIR)/debug

#编译器、链接器
CXX := g++
LD := gcc

#编译选项
CXXFLAGS := -std=c++11 -Wall -m64 -O2 -fPIC -fmessage-length=0 -fvisibility=hidden
CXXFLAGS_DBG := -std=c++11 -Wall -m64 -O0 -g3 -fPIC -msse4.2 -fmessage-length=0 -fvisibility=hidden
ifeq ($(ARCH), x86_64)
	CXXFLAGS += -mfma
else ifeq ($(ARCH), aarch64)
	CXXFLAGS += -DRTE_ARCH_64 -DRTE_ARCH_ARM64_MEMCPY -DRTE_CACHE_LINE_SIZE=128
endif


#宏定义
MACROS := -D_LINUX
MACROS_DBG := -D_DEBUG -D_LINUX

#链接选项
LDFLAGS := -Wl,-rpath,./ -shared
LDOUTFLG := -o

#包含的头文件和库文件
INCS := -I$(INC_DIR)
LIBS :=

#后缀文件、源文件、中间目标文件和依赖文件
SUFFIX := .cpp
SRCS := $(notdir $(wildcard $(SRC_DIR)/*$(SUFFIX)))
OBJS := $(addprefix $(BIN_DIR)/, $(patsubst %$(SUFFIX), %.o, $(SRCS)))
DEPS := $(addprefix $(BIN_DIR)/, $(patsubst %$(SUFFIX), %.d, $(SRCS)))
OBJS_DBG := $(addprefix $(BIN_DIR_DBG)/, $(patsubst %$(SUFFIX), %.o, $(SRCS)))
DEPS_DBG := $(addprefix $(BIN_DIR_DBG)/, $(patsubst %$(SUFFIX), %.d, $(SRCS)))

#最终目标文件
TARGET := $(BIN_DIR)/librte_memcpy.so
TARGET_DBG := $(BIN_DIR_DBG)/librte_memcpy.so

#release最终目标
.PHONY : all
all : $(TARGET)

#debug最终目标
.PHONY : debug
debug : $(TARGET_DBG)

###################################release#########################################
#生成release最终目标
$(TARGET) : $(OBJS) | $(BIN_DIR)
	$(LD) $(LDFLAGS) $(LIBS) $(LDOUTFLG) $@ $^

#若没有BIN_DIR目录则自动生成
$(BIN_DIR) :
	mkdir -p $@

#生成release中间目标文件
$(BIN_DIR)/%.o : $(SRC_DIR)/%$(SUFFIX) $(BIN_DIR)/%.d | $(BIN_DIR)
	$(CXX) -MT $@ -MMD -MP -MF $(BIN_DIR)/$*.d -c $(CXXFLAGS) $(INCS) $(MACROS) -o $@ $<

#依赖文件会在生成中间文件的时候自动生成，这里只是为了防止报错
$(DEPS) :

#引入中间目标文件头文件依赖关系
include $(wildcard $(DEPS))

###################################debug#########################################
#生成debug最终目标
$(TARGET_DBG) : $(OBJS_DBG) | $(BIN_DIR_DBG)
	$(LD) $(LDFLAGS) $(LIBS) $(LDOUTFLG) $@ $^

#若没有debug目录则自动生成
$(BIN_DIR_DBG) :
	mkdir -p $@

#生成debug中间目标文件
$(BIN_DIR_DBG)/%.o : $(SRC_DIR)/%$(SUFFIX) $(BIN_DIR_DBG)/%.d | $(BIN_DIR_DBG)
	$(CXX) -MT $@ -MMD -MP -MF $(BIN_DIR_DBG)/$*.d -c $(CXXFLAGS_DBG) $(INCS) $(MACROS_DBG) -o $@ $<

#依赖文件会在生成中间文件的时候自动生成，这里只是为了防止报错
$(DEPS_DBG) :

#引入中间目标文件头文件依赖关系
include $(wildcard $(DEPS_DBG))

#删除makefile创建的目录
.PHONY : clean
clean :
	rm -rf $(BIN_DIR) $(BIN_DIR_DBG)
