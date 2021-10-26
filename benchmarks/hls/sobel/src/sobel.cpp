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
#include "sobel.h"

#define DATA_SIZE 512L



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
    
    // Compute the size of array in bytes
    size_t size_in_bytes = DATA_SIZE * sizeof(short);
    
    // Creates a vector of DATA_SIZE elements with an initial value of 10 and 32
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
    cl::Kernel krnl_vector_add(program,"krnl_sobel");
    
    // These commands will allocate memory on the Device. The cl::Buffer objects can
    // be used to reference the memory locations on the device. 

    short *ptr_in[8];
    short *ptr_sobel_mtrx_h;
    short *ptr_sobel_mtrx_v;
    short *ptr_result;
    for (int i = 0; i < 8; i++){
        posix_memalign((void **) &ptr_in[i], 4096, size_in_bytes);
    }
    posix_memalign((void **) &ptr_sobel_mtrx_h, 4096, (8 * sizeof(short)));
    posix_memalign((void **) &ptr_sobel_mtrx_v, 4096, (8 * sizeof(short)));
    posix_memalign((void **) &ptr_result, 4096, size_in_bytes);



    int narg=0;
    std::vector<cl::Memory> buffers;
    for (int i = 0; i < 8; i++){
        buffers.push_back(cl::Buffer(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, size_in_bytes, ptr_in[i]));
        krnl_vector_add.setArg(narg++,buffers[i]);
    }
    cl::Buffer buffer_sobel_mtrx_v(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, (8 * sizeof(short)), ptr_sobel_mtrx_v);
    cl::Buffer buffer_sobel_mtrx_h(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, (8 * sizeof(short)), ptr_sobel_mtrx_h);
    cl::Buffer buffer_result(context, CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, size_in_bytes, ptr_result);
    //set the kerneresultl Arguments
    krnl_vector_add.setArg(narg++,buffer_sobel_mtrx_v);
    krnl_vector_add.setArg(narg++,buffer_sobel_mtrx_h);
    krnl_vector_add.setArg(narg++,buffer_result);
    krnl_vector_add.setArg(narg++,DATA_SIZE);

    //setting input data
    short sobel_h[] = {-1, 0, 1, -2/*, 0*/, 2, -1, 0, 1};
    short sobel_v[] = {1, 2, 1, 0/*, 0*/, 0, -1, -2, -1};
    for(int i = 0 ; i < 8; i++){
	    for(int j = 0; j < DATA_SIZE; j++){
            ptr_in[i][j] = 1;
        }
        ptr_sobel_mtrx_h[i] = sobel_h[i];
        ptr_sobel_mtrx_v[i] = sobel_v[i];
    }


    // Data will be migrated to kernel space
    q.enqueueMigrateMemObjects(buffers,0/* 0 means from host*/);

    //Launch the Kernel
    q.enqueueTask(krnl_vector_add);

    // The result of the previous kernel execution will need to be retrieved in
    // order to view the results. This call will transfer the data from FPGA to
    // source_results vector
    q.enqueueMigrateMemObjects({buffer_result},CL_MIGRATE_MEM_OBJECT_HOST);

    q.finish();

    //Verify the result
    int match = 0;
    unsigned short sum_v = 0, sum_h = 0;
    for (int i = 0; i < DATA_SIZE; i++) {
        sum_h += ptr_in[0][i] * ptr_sobel_mtrx_h[0];
        sum_h += ptr_in[1][i] * ptr_sobel_mtrx_h[1];
        sum_h += ptr_in[2][i] * ptr_sobel_mtrx_h[2];
        sum_h += ptr_in[3][i] * ptr_sobel_mtrx_h[3];
        sum_h += ptr_in[4][i] * ptr_sobel_mtrx_h[4];
        sum_h += ptr_in[5][i] * ptr_sobel_mtrx_h[5];
        sum_h += ptr_in[6][i] * ptr_sobel_mtrx_h[6];
        sum_h += ptr_in[7][i] * ptr_sobel_mtrx_h[7];

        sum_v += ptr_in[0][i] * ptr_sobel_mtrx_v[0];
        sum_v += ptr_in[1][i] * ptr_sobel_mtrx_v[1];
        sum_v += ptr_in[2][i] * ptr_sobel_mtrx_v[2];
        sum_v += ptr_in[3][i] * ptr_sobel_mtrx_v[3];
        sum_v += ptr_in[4][i] * ptr_sobel_mtrx_v[4];
        sum_v += ptr_in[5][i] * ptr_sobel_mtrx_v[5];
        sum_v += ptr_in[6][i] * ptr_sobel_mtrx_v[6];
        sum_v += ptr_in[7][i] * ptr_sobel_mtrx_v[7];

        short host_result = (sum_h * sum_h) + (sum_v * sum_v);
        if (ptr_result[i] != host_result) {
            printf(error_message.c_str(), i, host_result, ptr_result[i]);
            match = 1;
            break;
        }
    }
    /*for (int i = 0; i < DATA_SIZE; i++) {
        //for(int j = 0; j < 8; j++){
        //    sum_h += ptr_in[j][i] * ptr_sobel_mtrx_h[i];
        //    sum_v += ptr_in[j][i] * ptr_sobel_mtrx_v[i];
        //}
        short host_result = (sum_h * sum_h) + (sum_v * sum_v);
        //printf(error_message.c_str(), i, host_result, ptr_result[i]);
        if (ptr_result[i] != host_result) {
            printf(error_message.c_str(), i, host_result, ptr_result[i]);
            match = 1;
            break;
        }
    }*/

    //for(int i = 0; i < 8; i++){
    //    q.enqueueUnmapMemObject(buffers[i] , ptr_in[i]);
    //}
    //q.enqueueUnmapMemObject(buffer_sobel_mtrx_h , ptr_sobel_mtrx_h);
    //q.enqueueUnmapMemObject(buffer_sobel_mtrx_v , ptr_sobel_mtrx_v);
    //q.enqueueUnmapMemObject(buffer_result , ptr_result);
    //q.finish();

    std::cout << "TEST " << (match ? "FAILED" : "PASSED") << std::endl; 
    return (match ? EXIT_FAILURE :  EXIT_SUCCESS);

}
