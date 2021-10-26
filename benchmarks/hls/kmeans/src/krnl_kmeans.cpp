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
// kernel:  kmeans
//
// Purpose: Run Kmeans for CCPE journal
//

#define MAX_DIM 8
#define MAX_CENTROIDS 8
#define CENTROIDS_MAX (MAX_DIM * MAX_CENTROIDS)

#define SHORT_MAX ((1<<16)-1)


//TRIPCOUNT identifier
//const unsigned int c_len = DATA_SIZE / BUFFER_SIZE;
//const unsigned int c_size = BUFFER_SIZE;

/*
    Vector Addition Kernel Implementation 
    Arguments:
        data_in   (input)       --> Input Vector1
        centroids   (input)     --> Input Centroids configuration
        out   (output)          --> Output Vector
        num_centroids (input)   --> Quantity of centroids
        int num_dim (input)     --> Quantity of dimensions
        size  (input)           --> Size of Vector in Integer
*/

extern "C" {
    void krnl_kmeans(
            const unsigned short *data_in,  // Read-Only Vector 1
            const unsigned short *centroids,// Read-Only Vector 2
            unsigned short *out,            // Output Result
            int num_centroids,              // Quantity of centroids
            int num_dim,                    // Quantity of dimensions
            long size                       // Size in integer
    ) {
#pragma HLS INTERFACE s_axilite bundle=kmeans_io port=centroids
#pragma HLS INTERFACE s_axilite bundle=kmeans_io port=out
#pragma HLS INTERFACE s_axilite bundle=kmeans_io port=data_in
#pragma HLS INTERFACE s_axilite bundle=kmeans_io port=return


        //Receiving and configuring the centroids values
        unsigned short hw_centroids[CENTROIDS_MAX];
#pragma HLS ARRAY_PARTITION variable=hw_centroids dim=1 complete
        for(long j = 0; j < num_centroids * num_dim; j++){
#pragma HLS PIPELINE
            hw_centroids[j] = centroids[j];
        }

        unsigned short min, min_id;
        for (long i = 0; i < size; i++) {
#pragma HLS DATAFLOW
            min = SHORT_MAX;
            min_id = 0;
            for (unsigned short c = 0; c < num_centroids; c++) {
            	unsigned long sum = 0;
                for (unsigned short d = 0; d < num_dim; d++) {
                	sum += data_in[d * num_dim + i] - hw_centroids[c * num_dim + d];
                }
                if (sum <= min) {
                    min = sum;
                    min_id = c;
                }
            }
            out[i] = min_id;
        }
    }
}
