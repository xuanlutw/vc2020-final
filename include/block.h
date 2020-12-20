#pragma once
#include "utils.h"
#include "stream.h"
#include "huffman.h"

// This class ONLY read and decode datas, 
// it doesn't decide decode mode and write data back
class Block {
    public:
        Block (Stream* vs, i16 pre_dc_coeff, Huffman* DC, Huffman* AC, \
               u8* quant, u8 intra_dc_prec, u8 q_scale_type, u8 q_scale_code);
        void inverse_scan ();
        void inverse_q (u8* quant, \
                        u8 intra_dc_prec, u8 q_scale_type, u8 q_scale_code);
        void inverse_DCT ();        // 2D IDCT
        void inverse_DCT1 ();       // 1D IDCT
        void inverse_DCT1_fast ();  // Implement B.G.Lee's IDCT algo.
        void print ();
        i16 data[64];
        i16 dc_coeff;
};

