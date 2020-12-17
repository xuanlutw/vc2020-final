#pragma once
#include "utils.h"
#include "stream.h"
#include "picture.h"
#include "slice.h"

class MB {
    public:
        MB (Stream* vs, Slice* slice);
        void read_mode (Stream* vs);
        void read_mvs (Stream* vs, u8 s);
        void read_mv (Stream* vs, u8 r, u8 s);
        void update_mv ();      // Update MV and PMV
        void reset_mv ();
        void pred ();
        u8   pred_pixel (u8 s, u8 cc, u16 x, u16 y);
        void decode (Stream* vs);

        Picture* pic;
        Slice* slice;

        // Mode
        u8 mb_quant;
        u8 mb_motion_f;
        u8 mb_motion_b;
        u8 mb_pattern;
        u8 mb_intra;
        u8 stwcf;   // This two always zero
        u8 p_stwc;  // in non scalable mode 
        u8 motion_type;
        u8 mv_count;
        u8 dmv;
        u8 decode_dct_type;
        u8 dct_type;
        u8 mv_field_select[2][2];
        i8 mv_code[2][2][2];
        i8 mv_residual[2][2][0];
        u8 dmvector[2];

        u8 q_scale_code;

        u8 pattern_code;

        // Motion Vectors
        i16 mv[2][2][2];    // Motion vector
        i16 int_mv[2][2][2];
        u8  half_f[2][2][2];
};

