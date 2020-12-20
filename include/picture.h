#pragma once
#include "utils.h"
#include "stream.h"
#include "seq.h"

class Picture {
    public:
        Picture (Stream* vs, Seq* seq, Picture* ref_f, Picture* ref_b);
        ~Picture ();
        void read_ext (Stream* vs);
        void print ();              // Print header info
        void dump (char* filename); // Dump pixel value in RGB ppm format
        void dump ();               // Dump with default name
        void write_YUV (FILE* fp);  // Write to to file in YUV format
        void decode (Stream* vs);

        // Essentail info
        i16* pixel[3];
        u16  horz_size;
        u16  vert_size;
        u8*  intra_q;
        u8*  inter_q;
        Seq* seq;
        Picture* ref[2];

        // Header
        u16 temp_ref;
        u8  type;
        u16 vbv_delay;
        u8  full_pel_forw_v; /* NOT */
        u8  forw_f_code;     /*  |  */
        u8  full_pel_back_v; /*  |  */
        u8  back_f_code;     /* USE */

        // Ext
        u8  f_code[2][2];
        u8  intra_dc_prec;
        u8  pic_structure;
        u8  top_field_first;
        u8  frame_pred_frame_dct;
        u8  concealment_mv;
        u8  q_scale_type;
        u8  intra_vlc_format;
        u8  alternate_scan;
        u8  repeat_first_field;
        u8  chroma_420_type;
        u8  progressive_frame;
        u8  composite_dsp_flag;
        u8  v_axis;
        u8  field_seq;
        u8  sub_carrier;
        u8  burst_amplitude;
        u8  sub_carrier_phase;
};
