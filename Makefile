
CC=/home/ruinmmal/termo/gcc-4.1.2-glibc-2.5-nptl-3/arm-none-linux-gnueabi/bin/arm-none-linux-gnueabi-g++

OUT = ./arm_obj

EXECUTABLE = Ritex

INC_PATH = -I/home/ruinmmal/termo/sqlite-3.6.22
LIB_PATH = -L/home/ruinmmal/termo/sqlite-3.6.22/.libs

#for debug. unstripped. file is ab out 1.5 Mb!
#CC_FLAGS = -O0 -g3 -Wall $(INC_PATH)

#for Release
CC_FLAGS = -O3 -Wall $(INC_PATH)

LD_FLAGS = -lpthread -lsqlite3 $(LIB_PATH)

CPP_FILES := $(wildcard src/*.cpp)
OBJ_FILES = $(patsubst src/%.cpp,$(OUT)/%.o,$(CPP_FILES))

all::dirs $(EXECUTABLE) Makefile

$(EXECUTABLE): $(OBJ_FILES) 
	$(CC) $(LD_FLAGS) -o $@ $^

$(OUT)/%.o: src/%.cpp
	$(CC) $(CC_FLAGS) -c -o $@ $^

.PHONY: dirs
dirs:
	mkdir -p $(OUT)
clean::
	rm -rf $(OUT)/*
	rm -rf $(EXECUTABLE)
