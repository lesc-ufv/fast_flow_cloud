
SUDO:=sudo
PREFIX_PATH:=/usr/local/

TARGET := hw_emu
DEVICE := $(AWS_PLATFORM)
HOST_ARCH = x86

CXXFLAGS += $(opencl_CXXFLAGS) -Wall -O3 -std=c++11 -fopenmp
LDFLAGS += $(opencl_LDFLAGS)

CXXFLAGS += -I$(shell readlink -f ./include)
LDFLAGS += -lrt -lstdc++ 

CXXFLAGS += -fmessage-length=0 -fPIC

LIB_NAME=xready

SRC_FILES = $(shell find ./src -iname *.cpp)

O_FILES = $(patsubst %.cpp,build/%.o,$(SRC_FILES))

all: check-all lib/lib$(LIB_NAME).so

include opencl.mk
include simconf.mk
include checks.mk


lib/lib$(LIB_NAME).so: $(O_FILES)
	mkdir -p $(@D)
	$(CXX) $^ -shared -Wl,-soname,lib$(LIB_NAME).so -o $@

build/%.o: %.cpp
	mkdir -p $(@D)
	$(CXX) -c $(CXXFLAGS) $< -o $@
	
install: all
	
	@${SUDO} mkdir -p ${PREFIX_PATH}/lib
	@${SUDO} mkdir -p ${PREFIX_PATH}/include
	
	@${SUDO} cp lib/lib$(LIB_NAME).so ${PREFIX_PATH}/lib
	@${SUDO} cp -r ./include/json ${PREFIX_PATH}/include
	@${SUDO} cp -r ./include/xcl2 ${PREFIX_PATH}/include
	@${SUDO} cp -r ./include/xready ${PREFIX_PATH}/include
	@echo XReady Library installed with success. 
	@echo Include dir: ${PREFIX_PATH}/include/
	@echo Library dir: ${PREFIX_PATH}/lib/
	
	
uninstall:
	@${SUDO} rm -rf ${PREFIX_PATH}/lib/lib$(LIB_NAME).so
	@${SUDO} rm -rf ${PREFIX_PATH}/include/json
	@${SUDO} rm -rf ${PREFIX_PATH}/include/xcl2
	@${SUDO} rm -rf ${PREFIX_PATH}/include/xready
	@echo XReady Library uninstalled with success

clean:
	rm -rf build lib

CXXFLAGS += -MMD
-include $(shell find build -name "*.d" 2> /dev/null)

