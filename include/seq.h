#pragma once
#include "utils.h"
#include "stream.h"

class Seq {
    public:
        Seq (Stream* vs);           // Read sequence header
        void read_ext (Stream* vs); // Read sequence extension header
        void read_dsp (Stream* vs); // Read sequence display extension header
        void print ();

        // Header
        u16 horz_size;
        u16 vert_size;
        u16 aspect_ratio;
        u16 frame_rate_code;
        u32 bit_rate;
        u16 vbv_buf_size;
        u8  intra_q[64];
        u8  inter_q[64];
        double frame_rate;

        // Ext
        u8  profil_level_id;
        u8  progressive_seq;
        u8  chroma_format;
        u8  low_delay;
        u8  frame_rate_ext_n;       /* Both 0 in main profile*/
        u8  frame_rate_ext_d;

        // Ext Display
        u8  video_format;
        u8  colour_discription;
        u8  colour_primaries;
        u8  transfer_characteristics;
        u8  matrix_coefficients;
        u8  display_horz_size;
        u8  display_vert_size;
};
