/**********
Copyright (c) 2020, Xilinx, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**********/
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include "kmeans_naive.h"

#define NUM_DIM       4
#define NUM_CENTROIDS 8
#define DNUM_POINTS    512
#define DATA_IN_SIZE  (DNUM_POINTS*NUM_DIM)
#define DAT_OUT_SIZE  DNUM_POINTS
#define CENTROID_SIZE (NUM_CENTROIDS*NUM_DIM)

static const long NUM_POINTS = DNUM_POINTS;

static const std::string error_message =
    "Error: Result mismatch:\n"
    "i = %d CPU result = %d Device result = %d\n";

int main(int argc, char* argv[]) {

    //TARGET_DEVICE macro needs to be passed from gcc command line
    if(argc != 2) {
		std::cout << "Usage: " << argv[0] <<" <xclbin>" << std::endl;
		return EXIT_FAILURE;
	}

    char* xclbinFilename = argv[1];
    //krnl_vector_add
    // Compute the size of array in bytes
    size_t data_in_size_in_bytes = DATA_IN_SIZE * sizeof(short);
    size_t data_out_size_in_bytes = DAT_OUT_SIZE * sizeof(short);
    size_t centroid_size_in_bytes = CENTROID_SIZE * sizeof(short);
    
    // Creates a vector of DATA_IN_SIZE elements with an initial value of 10 and 32
    // using customized allocator for getting buffer alignment to 4k boundary
    
    std::vector<cl::Device> devices;
    cl::Device device;
    std::vector<cl::Platform> platforms;
    bool found_device = false;

    //traversing all Platforms To find Xilinx Platform and targeted
    //Device in Xilinx Platform
    cl::Platform::get(&platforms);
    for(size_t i = 0; (i < platforms.size() ) & (found_device == false) ;i++){
        cl::Platform platform = platforms[i];
        std::string platformName = platform.getInfo<CL_PLATFORM_NAME>();
        if ( platformName == "Xilinx"){
            devices.clear();
            platform.getDevices(CL_DEVICE_TYPE_ACCELERATOR, &devices);
	    if (devices.size()){
		    device = devices[0];
		    found_device = true;
		    break;
	    }
        }
    }
    if (found_device == false){
       std::cout << "Error: Unable to find Target Device " 
           << device.getInfo<CL_DEVICE_NAME>() << std::endl;
       return EXIT_FAILURE; 
    }

    // Creating Context and Command Queue for selected device
    cl::Context context(device);
    cl::CommandQueue q(context, device, CL_QUEUE_PROFILING_ENABLE);

    // Load xclbin 
    std::cout << "Loading: '" << xclbinFilename << "'\n";
    std::ifstream bin_file(xclbinFilename, std::ifstream::binary);
    bin_file.seekg (0, bin_file.end);
    unsigned nb = bin_file.tellg();
    bin_file.seekg (0, bin_file.beg);
    char *buf = new char [nb];
    bin_file.read(buf, nb);
    
    // Creating Program from Binary File
    cl::Program::Binaries bins;
    bins.push_back({buf,nb});
    devices.resize(1);
    cl::Program program(context, devices, bins);
    
    // This call will get the kernel object from program. A kernel is an 
    // OpenCL function that is executed on the FPGA. 
    cl::Kernel krnl_kmeans(program,"krnl_kmeans_naive");
    
    // These commands will allocate memory on the Device. The cl::Buffer objects can
    // be used to reference the memory locations on the device. 
    cl::Buffer buffer_a(context, CL_MEM_READ_ONLY, data_in_size_in_bytes);
    cl::Buffer buffer_b(context, CL_MEM_READ_ONLY, centroid_size_in_bytes);
    cl::Buffer buffer_result(context, CL_MEM_WRITE_ONLY, data_out_size_in_bytes);
    
    //set the kernel Arguments
    int narg=0;
    krnl_kmeans.setArg(narg++, buffer_a);
    krnl_kmeans.setArg(narg++, buffer_b);
    krnl_kmeans.setArg(narg++, buffer_result);
    krnl_kmeans.setArg(narg++, NUM_CENTROIDS);
    krnl_kmeans.setArg(narg++, NUM_DIM);
    krnl_kmeans.setArg(narg++, NUM_POINTS);

    //We then need to map our OpenCL buffers to get the pointers
    short *ptr_a = (short *) q.enqueueMapBuffer (buffer_a , CL_TRUE , CL_MAP_WRITE , 0, data_in_size_in_bytes);
    short *ptr_b = (short *) q.enqueueMapBuffer (buffer_b , CL_TRUE , CL_MAP_WRITE , 0, centroid_size_in_bytes);
    short *ptr_result = (short *) q.enqueueMapBuffer (buffer_result , CL_TRUE , CL_MAP_READ , 0, data_out_size_in_bytes);

    //setting input data
    for(short i = 0 ; i< CENTROID_SIZE; i++){
	    ptr_b[i] = i;
    }

    for(long i = 0 ; i< DATA_IN_SIZE; i++){
	    ptr_a[i] = 1;
    }

    // Data will be migrated to kernel space
    q.enqueueMigrateMemObjects({buffer_a,buffer_b},0/* 0 means from host*/);

    //Launch the Kernel
    q.enqueueTask(krnl_kmeans);

    // The result of the previous kernel execution will need to be retrieved in
    // order to view the results. This call will transfer the data from FPGA to
    // source_results vector
    q.enqueueMigrateMemObjects({buffer_result},CL_MIGRATE_MEM_OBJECT_HOST);

    q.finish();

    //Verify the result
    int match = 0;
    for (long i = 0; i < NUM_POINTS; i++) {
        unsigned short min = ((1<<16)-1), min_id = 0;
        for (unsigned short c = 0; c < NUM_CENTROIDS; c++) {
            unsigned long sum = 0;
            for (unsigned short d = 0; d < NUM_DIM; d++) {
                sum += ptr_a[d * NUM_DIM + i] - ptr_b[c * NUM_DIM + d];
            }
            if (sum <= min) {
                min = sum;
                min_id = c;
            }
        }
        int host_result = min_id;
        if (ptr_result[i] != host_result) {
            printf(error_message.c_str(), i, host_result, ptr_result[i]);
            match = 1;
            break;
        }
    }

    q.enqueueUnmapMemObject(buffer_a , ptr_a);
    q.enqueueUnmapMemObject(buffer_b , ptr_b);
    q.enqueueUnmapMemObject(buffer_result , ptr_result);
    q.finish();

    std::cout << "TEST " << (match ? "FAILED" : "PASSED") << std::endl; 
    return (match ? EXIT_FAILURE :  EXIT_SUCCESS);
}
