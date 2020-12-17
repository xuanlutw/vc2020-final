#pragma once
#include "utils.h"
#include "stream.h"
#include "picture.h"

class Slice {
    public:
        Slice (Stream* vs, Picture* pic);
        void decode (Stream* vs);
        void reset_pre_dc_coeff();
        void reset_pmv();
        Picture* pic;
        u16 mb_row;
        i16 mb_col;
        u8  q_scale_code;
        u8  intra_slice_f;
        u8  intra_slice;

        // Predictor val
        i16 pre_dc_coeff[3];
        i16 pmv[2][2][2];       // Motion vector predictors, 7.6.3
};

