TARGET := hw_emu
DEVICE := $(AWS_PLATFORM)
HOST_ARCH = x86

LIB_READY=$(shell readlink -f ../lib/lib/)

CXXFLAGS += $(opencl_CXXFLAGS) -Wall -O3 -g -std=c++11 -fopenmp
LDFLAGS += $(opencl_LDFLAGS)

CXXFLAGS += -I$(shell readlink -f ./include) -I$(shell readlink -f ../lib/include/)
LDFLAGS += -L$(LIB_READY) -lrt -lstdc++ -lxready 

CXXFLAGS += -fmessage-length=0

HOST_SRCS =$(shell find ./src/ -iname *.cpp)

EMCONFIG_DIR=./emconfig_dir
EXECUTABLE=./main

.PHONY:first
first:host

include opencl.mk
include simconf.mk
include checks.mk

.PHONY: host
host: $(EXECUTABLE)

$(EXECUTABLE): check-xrt $(HOST_SRCS) 
	@$(CXX) $(CXXFLAGS) $(HOST_SRCS) -o '$@' $(LDFLAGS)
	
.PHONY: cpu
cpu: host
	@LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(LIB_READY) $(EXECUTABLE) "cpu" $(KERNEL_NAME) $(SIM_XCLBIN) cgra_arch.json

.PHONY: sim	
sim: host simconf emconfig
	@XCL_EMULATION_MODE=$(TARGET) LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(LIB_READY) $(EXECUTABLE) "sim" $(KERNEL_NAME) $(SIM_XCLBIN)  cgra_arch.json

.PHONY: cgra
cgra: host
	@LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(LIB_READY) $(EXECUTABLE) "cgra" $(KERNEL_NAME) $(HW_XCLBIN) cgra_arch.json

.PHONY:emconfig
emconfig: emconfig.json

emconfig.json:
	@mkdir -p $(EMCONFIG_DIR)
	@emconfigutil --platform $(DEVICE) --od $(EMCONFIG_DIR) > /dev/null
	@cp $(EMCONFIG_DIR)/emconfig.json .

.PHONY: clean	
clean: 
	@rm -rf $(EMCONFIG_DIR)
	@rm -rf emconfig.json
	@rm -rf $(EXECUTABLE)
	@rm -rf xsim.tcl
	@rm -rf xrt.ini
	@rm -rf *.csv
	@rm -rf *.link.*
	@rm -rf *.log
	@rm -rf *.jou
	@rm -rf .ipcache
	@rm -rf .Xil
	@rm -rf xsim.dir
	@rm -rf .hbs
	@rm -rf .run
