/**
* Copyright (C) 2019-2021 Xilinx, Inc
*
* Licensed under the Apache License, Version 2.0 (the "License"). You may
* not use this file except in compliance with the License. A copy of the
* License is located at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
* WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
* License for the specific language governing permissions and limitations
* under the License.
*/

//------------------------------------------------------------------------------
//
// kernel:  fir
//
// Purpose: Demonstrate Vector Add Kernel
//

#define BUFFER_SIZE 256
#define DATA_SIZE 4096
// TRIPCOUNT identifier
const unsigned int c_len = DATA_SIZE / BUFFER_SIZE;
const unsigned int c_size = BUFFER_SIZE;

/*
    FIR Kernel Implementation
    Arguments:
        in1   (input)     --> Input Vector
        coef  (input)     --> Input Vector
        out_r (output)    --> Output Vector
        size  (input)     --> Size of Vector in Integer
        taps  (input)     --> FIR taps in Integer
*/

extern "C" {
void krnl_fir( const unsigned short* in1,  // Read-Only Vector 1
               const unsigned short* coef, // Read-Only Vector 2
               unsigned  short* out_r,      // Output Result
               int size,                 // Size in integer
               int taps                  // FIR taps in integer
               ) {

      short hw_coef[64];

      for(int j = 0;j < taps;j++){
         hw_coef[j] = coef[j];
      }

      for (int i = 0; i < size-taps; i++) {
        unsigned short fir = 0;
        for (int k = 0; k < taps; ++k) {
                fir += in1[i + k] * hw_coef[taps - 1 - k];
        }
        out_r[i] = fir;
      }
}
}
