
## $(ARCH) from arm_cc
ifeq ($(ARCH), arm)
  CROSS_COMPILE = arm-linux-gnueabihf-
  export kernel_dir = /home/yihongliang/nr_rru_g3/out_fhk_zc706_uImage
else
  CROSS_COMPILE = 
endif

CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
MV = mv -f
RM = rm -rf
LN = ln -sf

TARGET = daemon_app

TOP_PATH = $(shell pwd)
SRC_PATH = common driver service

CFLAGS = -Wall -c -std=gnu99 -g 
LINK_FLAGS += -lrt -lpthread

##########################################################
# modules
##########################################################
MOD_PATH = $(TOP_PATH)/module
MOD_LIB_PATH = $(MOD_PATH)/lib

modules = 
MODULES_PATH = $(foreach m, $(modules), $(MOD_PATH)/$(m))

##########################################################
# srcs
##########################################################
DIRS = $(shell find $(SRC_PATH) -maxdepth 3 -type d)

SRCS_CPP += $(foreach dir, $(DIRS), $(wildcard $(dir)/*.cpp))
SRCS_CC += $(foreach dir, $(DIRS), $(wildcard $(dir)/*.cc))
SRCS_C += $(foreach dir, $(DIRS), $(wildcard $(dir)/*.c))

##########################################################
# objs
##########################################################
OBJS_CPP = $(patsubst %.cpp, %.o, $(SRCS_CPP))
OBJS_C = $(patsubst %.c, %.o, $(SRCS_C))

##########################################################
# paths
##########################################################
INC_PATH += -I$(TOP_PATH)/include
INC_PATH += $(foreach dir, $(DIRS), -I$(dir))
INC_PATH += -I$(MOD_PATH)

LIB_PATH += -L$(TOP_PATH)/lib
LIB_PATH += -L$(MOD_LIB_PATH)

##########################################################
# libs
##########################################################
LIBS += 

##########################################################
# building
##########################################################
all:$(TARGET)

$(TARGET) : $(OBJS_CPP) $(OBJS_C)
	@ for i in $(MODULES_PATH); \
	do \
	make -C $$i; \
	done

	@ $(CC) $^ -o $@ $(LIB_PATH) $(LIBS) $(LINK_FLAGS)
	@ echo Create $(TARGET) ok...

$(OBJS_CPP):%.o : %.cpp
	$(CXX) $(CFLAGS) $< -o $@ $(INC_PATH)

$(OBJS_C):%.o : %.c
	$(CC) $(CFLAGS) $< -o $@ $(INC_PATH)

.PHONY : clean
clean:
	@ $(RM) $(TARGET) $(OBJS_CPP) $(OBJS_C)
	@ for i in $(MODULES_PATH); \
	do \
	make clean -C $$i; \
	done
	@echo clean all...

	