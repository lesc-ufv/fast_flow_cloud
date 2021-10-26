extern "C" {
void krnl_mult_add(const unsigned int *in0, // Read-Only Vector 1
          unsigned int *out_r,     // Output Result
          int size                 // Size in integer
	) {
#pragma HLS INTERFACE s_axilite bundle=mult_add_io port=in0
#pragma HLS INTERFACE s_axilite bundle=mult_add_io port=out_r
#pragma HLS INTERFACE s_axilite bundle=mult_add_io port=return

		for (int k = 0; k < size; ++k) {
		    int a = in0[k] * in0[k] + 3;
		    for(int p = 0; p < 15; p++){
		        a = a * a + 3;
		    }
		    out_r[k] = a;
		}
	}
}
