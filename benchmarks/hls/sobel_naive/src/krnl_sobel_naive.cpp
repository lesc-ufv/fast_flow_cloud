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

//------------------------------------------------------------------------------
//
// kernel:  sobel
//
// Purpose: Run sobel for CCPE journal
//

//#define BUFFER_SIZE 256
//#define DATA_SIZE 4096
//TRIPCOUNT identifier
//const unsigned int c_len = DATA_SIZE / BUFFER_SIZE;
//const unsigned int c_size = BUFFER_SIZE;

/*
    Vector Addition Kernel Implementation 
    Arguments:
        in0             (input) --> Input Vector0
        in1             (input) --> Input Vector1
        in2             (input) --> Input Vector2
        in3             (input) --> Input Vector3
        in4             (input) --> Input Vector4
        in5             (input) --> Input Vector5
        in6             (input) --> Input Vector6
        in7             (input) --> Input Vector7
        sobel_mtrx_v,   (input) --> Sobel Matrix
        sobel_mtrx_h,   (input) --> Sobel Matrix
        out             (output)--> Output Vector
        size            (input) --> Size of Vector in Integer
*/

extern "C" {
    void krnl_sobel_naive(
            const unsigned short *in0,          // Read-Only Vector 0
            const unsigned short *in1,          // Read-Only Vector 1
            const unsigned short *in2,          // Read-Only Vector 2
            const unsigned short *in3,          // Read-Only Vector 3
            const unsigned short *in4,          // Read-Only Vector 4
            const unsigned short *in5,          // Read-Only Vector 5
            const unsigned short *in6,          // Read-Only Vector 6
            const unsigned short *in7,          // Read-Only Vector 7
            const unsigned short *sobel_mtrx_v, // Read-Only Sobel Matrix
            const unsigned short *sobel_mtrx_h, // Read-Only Sobel Matrix
            unsigned short *out,                // Output Result
            long size                            // Size in integer
    ) {
        unsigned short hw_sobel_mtrx_v[8];
        unsigned short hw_sobel_mtrx_h[8];

        for(int i = 0 ; i < 8; i++){
            hw_sobel_mtrx_h[i] = sobel_mtrx_h[i];
            hw_sobel_mtrx_v[i] = sobel_mtrx_v[i];
        }

        unsigned short sum_v = 0, sum_h = 0;
        for (long i = 0; i < size; i++) {
            sum_h += in0[i] * hw_sobel_mtrx_h[0];
            sum_h += in1[i] * hw_sobel_mtrx_h[1];
            sum_h += in2[i] * hw_sobel_mtrx_h[2];
            sum_h += in3[i] * hw_sobel_mtrx_h[3];
            sum_h += in4[i] * hw_sobel_mtrx_h[4];
            sum_h += in5[i] * hw_sobel_mtrx_h[5];
            sum_h += in6[i] * hw_sobel_mtrx_h[6];
            sum_h += in7[i] * hw_sobel_mtrx_h[7];

            sum_v += in0[i] * hw_sobel_mtrx_v[0];
            sum_v += in1[i] * hw_sobel_mtrx_v[1];
            sum_v += in2[i] * hw_sobel_mtrx_v[2];
            sum_v += in3[i] * hw_sobel_mtrx_v[3];
            sum_v += in4[i] * hw_sobel_mtrx_v[4];
            sum_v += in5[i] * hw_sobel_mtrx_v[5];
            sum_v += in6[i] * hw_sobel_mtrx_v[6];
            sum_v += in7[i] * hw_sobel_mtrx_v[7];

            out[i] = (sum_h * sum_h) + (sum_v * sum_v);
        }
    }

}
