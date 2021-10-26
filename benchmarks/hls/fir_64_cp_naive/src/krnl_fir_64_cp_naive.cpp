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
// kernel:  fir_64_cp
//
// Purpose: Demonstrate Vector Add Kernel
//

#define TAPS 64

/*
    FIR Kernel Implementation
    Arguments:
        in1   (input)   --> Input Vector
        coef  (input)   --> Input Vector
        out_r (output)  --> Output Vector
        size  (input)   --> Size of Vector in Integer
*/

extern "C" {
    void krnl_fir_64_cp_naive( const unsigned short* in1,     // Read-Only Vector 1
        const unsigned short* coef,                     // Read-Only coef Vector
        unsigned  short* out_r,                         // Output Result
        int size                                        // Size in integer
        ) {
//#pragma HLS TOP

        short hw_coef[TAPS];

        for(int i = 0; i < TAPS; i++){
            hw_coef[i] = coef[i];
        }

        for (int i = 0; i < size-TAPS; i++) {
            unsigned short fir = 0;
            for (int k = 0; k < TAPS; ++k) {
                    fir += in1[i + k] * hw_coef[TAPS - 1 - k];
            }
            out_r[i] = fir;
        }
    }
}
