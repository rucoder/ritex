
CC_PATH=$(HOME)/termo/gcc-4.1.2-glibc-2.5-nptl-3/arm-none-linux-gnueabi/bin

CC=$(CC_PATH)/arm-none-linux-gnueabi-g++
AS=$(CC_PATH)/arm-none-linux-gnueabi-as
STRIP=$(CC_PATH)/arm-none-linux-gnueabi-strip

VERSION=`git describe`

OUT = ./arm_obj

EXECUTABLE = Ritex

INC_PATH = -I$(HOME)/termo/sqlite-3.6.22
LIB_PATH = -L$(HOME)/termo/sqlite-3.6.22/.libs

#for debug. unstripped. file is ab out 1.5 Mb!
#CC_FLAGS = -O0 -g3 -Wall -rdynamic -fno-omit-frame-pointer -mapcs-frame $(INC_PATH) -D_ADAPTER_VERSION_=\"$(VERSION)\"

#for Release
CC_FLAGS = -O3 -g3 -Wall -rdynamic -fno-omit-frame-pointer -mapcs-frame $(INC_PATH) -D_ADAPTER_VERSION_=\"$(VERSION)\"

LD_FLAGS = -rdynamic -fno-omit-frame-pointer -mapcs-frame -lpthread -lsqlite3 -lrt $(LIB_PATH) -ldl

CPP_FILES := $(wildcard src/*.cpp)
AS_FILES := $(wildcard src/*.s)

OBJ_FILES=$(patsubst src/%.cpp,$(OUT)/%.o,$(CPP_FILES))
AS_OBJ_FILES=$(patsubst src/%.s,$(OUT)/%.o,$(AS_FILES))

OBJ_FILES+=$(AS_OBJ_FILES)

all::dirs $(EXECUTABLE) Makefile

$(EXECUTABLE): $(OBJ_FILES) 
	@echo Linking $@ version $(VERSION)
	@$(CC) $(LD_FLAGS) -o $@ $^
#	@$(STRIP) $@

$(OUT)/%.o: src/%.cpp
	@echo Compiling $^
	@$(CC) $(CC_FLAGS) -c -o $@ $^

$(OUT)/%.o: src/%.s
	@echo Assembling $^
	@$(AS) $(AS_FLAGS) -o $@ $^

.PHONY: dirs
dirs:
	mkdir -p $(OUT)
clean::
	rm -rf $(OUT)/*
	rm -rf $(EXECUTABLE)
