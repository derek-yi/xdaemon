
##########################################################
# tools
##########################################################
ifeq ($(ARCH), arm)
  export CROSS_COMPILE = arm-linux-gnueabihf-
  export kernel_dir = /home/derek/share/nr_rru_g3/out_fhk_zc706_uImage
endif

CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
MV = mv -f
RM = rm -rf
LN = ln -sf

##########################################################
# path and flags
##########################################################
TOP_PATH = $(shell pwd)
TARGET = daemon_app

#SRC_PATH = common driver service
SRC_PATH = common \
           driver/fpga driver/misc driver/talise \
           service/devm service/hwmon service/init service/msg service/upcfg

CFLAGS = -std=gnu99
CFLAGS += -Wall -Wno-unused -Wno-format
#CFLAGS += -rdynamic -g
CFLAGS += -g

ifeq ($(BD_TYPE), rhub)
  CFLAGS += -DBOARD_RHUB_G1
else
  CFLAGS += -DBOARD_RRU_G3
endif

ifeq ($(DAEMON_RELEASE), 1)
CFLAGS += -DDAEMON_RELEASE
endif

LINK_FLAGS += -lrt -lpthread -lzlog

##########################################################
# kernel driver
##########################################################
MOD_PATH = $(TOP_PATH)/module
MOD_LIB_PATH = $(MOD_PATH)/lib

modules = daemon_k
MODULES_PATH = $(foreach m, $(modules), $(MOD_PATH)/$(m))

##########################################################
# src and include
##########################################################
#DIRS = $(shell find $(SRC_PATH) -maxdepth 3 -type d)
DIRS = $(SRC_PATH)

SRCS_CPP += $(foreach dir, $(DIRS), $(wildcard $(dir)/*.cpp))
SRCS_C += $(foreach dir, $(DIRS), $(wildcard $(dir)/*.c))
#SRCS_CC += $(foreach dir, $(DIRS), $(wildcard $(dir)/*.cc))

INC_PATH += -I$(TOP_PATH)/include
INC_PATH += $(foreach dir, $(DIRS), -I$(dir))

OBJS_CPP = $(patsubst %.cpp, %.o, $(SRCS_CPP))
OBJS_C = $(patsubst %.c, %.o, $(SRCS_C))

##########################################################
# libs
##########################################################
LIB_PATH += -L$(TOP_PATH)/lib
LIB_PATH += -L$(MOD_LIB_PATH)

LIBS += 

##########################################################
# building
##########################################################
all:$(TARGET)

$(TARGET) : $(OBJS_CPP) $(OBJS_C)
	@ $(CC) $^ -o $@ $(LIB_PATH) $(LIBS) $(LINK_FLAGS)
	@ echo Create $(TARGET) ok...

$(OBJS_CPP):%.o : %.cpp
	$(CXX) $(CFLAGS) $< -c -o $@ $(INC_PATH)

$(OBJS_C):%.o : %.c
	$(CC) $(CFLAGS) $< -c -o $@ $(INC_PATH)

modules:
	@ for i in $(MODULES_PATH); \
	do \
	make -C $$i; \
	done

.PHONY : clean
clean:
	@ $(RM) $(TARGET) $(OBJS_CPP) $(OBJS_C)
	@ for i in $(MODULES_PATH); \
	do \
	make clean -C $$i; \
	done
	@echo clean all...

	