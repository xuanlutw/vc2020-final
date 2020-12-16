#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cmath>
#include "utils.h"
#include "stream.h"
#include "huffman.h"
#include "seq.h"
#include "gop.h"

class Block {
    public:
        // Intra Block
        Block (Stream* cs, i16 pre_dc_coeff, Huffman* DC, Huffman* AC, u8* quant, \
               u8 intra_dc_prec, u8 q_scale_type, u8 q_scale_code);
        void print ();
        void inverse_scan ();
        // Set intra_dc_prec for inter
        void inverse_q (u8* quant, u8 intra_dc_prec, u8 q_scale_type, u8 q_scale_code);
        void inverse_DCT ();
        void inverse_DCT1 ();
        i16 data[64];
        i16 dc_coeff;
};

Block::Block (Stream* vs, i16 pre_dc_coeff, Huffman* DC, Huffman* AC, u8* quant, \
    u8 intra_dc_prec, u8 q_scale_type, u8 q_scale_code) {
    for (int i = 0; i < 64; ++i)
        data[i] = 0;

    if (AC) { // Intra
        // Read DC coeff
        DC->get(vs);
        //printf("DC: %d %d %d\n", DC->type, DC->run, DC->level);
        data[0] = pre_dc_coeff + DC->level;
        this->dc_coeff = data[0];

        // Read AC coeff
        for (int i = 1; i < 64; ++i) {
            AC->get(vs);
            //printf("%d %d %d\n", AC->type, AC->run, AC->level);
            if (AC->type == HUFFMAN_CODE_END)
                break;
            else {
                i += AC->run;
                data[i] = AC->level;
            }
        }
    }
    else {
        // Read ALL coeff
        DC->set_is_DC();
        for (int i = 0; i < 64; ++i) {
            DC->get(vs);
            if (DC->type == HUFFMAN_CODE_END)
                break;
            else {
                i += DC->run;
                data[i] = DC->level;
            }
        }
        intra_dc_prec = 0;
    }

    //printf("Block Start\n");
    //this->print();
    this->inverse_scan();
    //this->print();
    this->inverse_q(quant, intra_dc_prec, q_scale_type, q_scale_code);
    //this->print();
    this->inverse_DCT();
    //this->print();
    //exit(-1);
}

void Block::inverse_scan () {
    i16 tmp[64];
    for (int i = 0; i < 64; ++i)
        tmp[i] = this->data[i];
    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 8; ++j)
            this->data[idx(i, j)] = tmp[zz[idx(i, j)]];
}

void Block::inverse_q (u8* quant, u8 intra_dc_prec, u8 q_scale_type, u8 q_scale_code) {
    // intra_dc_prec = 0 indicate inter

    // IQ
    for (int v = 0; v < 8; v++) {
        for (int u = 0; u < 8; u++) {
            if ((u == 0) && (v == 0) && intra_dc_prec ) {
                switch (intra_dc_prec) {
                    case 8:
                        data[idx(v, u)] = 8 * data[idx(v, u)];
                        break;
                    case 9:
                        data[idx(v, u)] = 4 * data[idx(v, u)];
                        break;
                    case 10:
                        data[idx(v, u)] = 2 * data[idx(v, u)];
                        break;
                    case 11:
                        data[idx(v, u)] = 1 * data[idx(v, u)];
                        break;
                }
            }
            else {
                if (intra_dc_prec) {
                    data[idx(v, u)] = (data[idx(v, u)] * quant[idx(v, u)] * \
                            q_scale[q_scale_type][q_scale_code] * 2) / 32;
                }
                else {
                    data[idx(v, u)] = ((data[idx(v, u)] * 2 + sign(data[idx(v, u)])) * \
                            quant[idx(v, u)] * q_scale[q_scale_type][q_scale_code]) / 32;
                }
            }
        }
    }

    // Saturation
    i64 sum = 0;
    for (int v = 0; v < 8; v++) {
        for (int u = 0; u < 8; u++) {
            if (data[idx(v, u)] > 2047)
                data[idx(v, u)] = 2047;
            else if (data[idx(v, u)] < -2048 )
                data[idx(v, u)] = -2048;
            sum += data[idx(v, u)];
        }
    }

    // Mismatch control
    if ((sum & 1) == 0) {
        if ((data[idx(7, 7)] & 1) != 0) {
            data[idx(7, 7)] = data[idx(7, 7)] - 1;
        } else {
            data[idx(7, 7)] = data[idx(7, 7)] + 1;
        }
    }
}

void Block::inverse_DCT1 () {

    static double* cos_table = NULL;
    static double* coeff     = NULL;

    // Init cos table and only once
    if (!cos_table) {
        cos_table = (double*)malloc(sizeof(double) * M_COS);
        for (u8 i = 0; i < M_COS; i++)
            cos_table[i] = cos(i * M_PI / 16.0);
    }

    // Init coefficient and only once
    if (!coeff) {
        coeff = (double*)malloc(sizeof(double) * 8);
        coeff[0] = 1. / sqrt(2.);
        for (u8 i = 1; i < 8; i++)
            coeff[i] = 1.;
    }

    // Transform
    i16 tmp[64] = {0};
    for (int j = 0; j < 8; j++)
        for (int x = 0; x < 8; x++) {
            for (int y = 0; y < 8; y++)
                tmp[idx(j, x)] += coeff[y] * this->data[idx(x, y)] * cos_table[((j << 1) + 1) * y];
            tmp[idx(j, x)] /= 2.;
        }
    for (int x = 0; x < 8; x++)
        for (int y = 0; y < 8; y++)
            this->data[idx(x, y)] = tmp[idx(x, y)];
    return;
}

void Block::inverse_DCT () {
    // Inverse descrete cosine transform

    this->inverse_DCT1();
    this->inverse_DCT1();
}

void Block::print () {
    for (int i = 0; i < 8; ++i) {
        printf("||>>");
        for (int j = 0; j < 8; ++j)
            printf("%3d ", data[idx(i, j)]);
        printf("\n");
    }
    printf("\n");
}
//=====================================================

class Picture {
    public:
        Picture (Stream* vs, Seq* seq);
        void read_ext (Stream* vs);
        void print ();
        void read_slice (Stream* vs);
        void read_mb (Stream* vs);
        void dump (char* filename);
        void dump (); // dump with default name
        void decode (Stream* vs);
        i16* pixel[3];
        u16 horz_size;
        u16 vert_size;
        u8* intra_q;
        u8* inter_q;
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
        // Temporal Slice Data
        u16 mb_row;
        i16 mb_col;
        u8  q_scale_code;
        u8  intra_slice_flag;
        u8  intra_slice;
        // Temporal Macro Block Data
        u8  mb_mode;
        u8  mb_quant;
        u8  mb_motion_f;
        u8  mb_motion_b;
        u8  mb_pattern;
        u8  mb_intra; 
        u8  mb_stwcf;   // Always zero   
        u8  mb_p_stwc;  // in non scalable mode
        u8  mb_decode_dct_type;
        u8  mb_dct_type;
        u8  mb_q_scale_code;
        u8  mb_motion_type;
        u8  mb_pattern_code;
        u8  mb_mv_count;
        u8  mb_dmv;
        u8  mb_mv_field_select[2][2];
        i8  mb_mv_code[2][2][2];
        u8  mb_m_residular[2][2][2];
        u8  mb_dmvector[2];
        void read_mvs (Stream* vs, u8 s);
        void read_mv  (Stream* vs, u8 r, u8 s);
        // Block
        i16 pre_dc_coeff[3];
        void reset_pre_dc_coeff();
        void proc_block(Stream* vs, u8 cc, u16 x, u16 y);
};

Picture::Picture (Stream* vs, Seq* seq) {
    this->temp_ref            = vs->read(10);
    this->type                = vs->read(3);
    this->vbv_delay           = vs->read(16);
    if (this->type == PIC_TYPE_P || this->type == PIC_TYPE_B) {
        this->full_pel_forw_v = vs->read(1);
        this->forw_f_code     = vs->read(3);
    }
    if (this->type == PIC_TYPE_B) {
        this->full_pel_back_v = vs->read(1);
        this->back_f_code     = vs->read(3);
    }
    while (vs->read(1)) // Remove Extra Info
        vs->read_u8();
    check(this->type == PIC_TYPE_I || \
            this->type == PIC_TYPE_P || \
            this->type == PIC_TYPE_B, "Wrong picture type %d", this->type);

    // Init pixels
    this->horz_size = ((seq->horz_size + 15) / 16) * 16; 
    this->vert_size = ((seq->vert_size + 15) / 16) * 16;
    this->intra_q   = seq->intra_q;
    this->inter_q   = seq->inter_q;
    this->pixel[0] = new i16[this->horz_size * this->vert_size];
    this->pixel[1] = new i16[this->horz_size * this->vert_size];
    this->pixel[2] = new i16[this->horz_size * this->vert_size];
    // Default
    this->intra_dc_prec = 8;
    
    // Read picture ext
    while (vs->next_start_code() == SCODE_EXT) {
        switch (vs->now_ext_code()) {
            case ECODE_PIC_EXT:
                this->read_ext(vs);
                break;
            case ECODE_PIC_Q:   
            case ECODE_PIC_DSP:
            case ECODE_PIC_SSCA:
            case ECODE_PIC_TSCA:
                check(0, "HAIYAA, non implement picture ext, ext code: %d", \
                        vs->now_ext_code());
            default:
                check(0, "Wrong ext code after picture header");
        }
    }
    vs->keep_start_code();
}

void Picture::read_ext (Stream* vs) {
    this->f_code[0][0]          = vs->read(4);
    this->f_code[0][1]          = vs->read(4);
    this->f_code[1][0]          = vs->read(4);
    this->f_code[1][1]          = vs->read(4);
    this->intra_dc_prec         = vs->read(2) + 8;
    this->pic_structure         = vs->read(2);
    this->top_field_first       = vs->read(1);
    this->frame_pred_frame_dct  = vs->read(1);
    this->concealment_mv        = vs->read(1);
    this->q_scale_type          = vs->read(1);
    this->intra_vlc_format      = vs->read(1);
    this->alternate_scan        = vs->read(1);
    this->repeat_first_field    = vs->read(1);
    this->chroma_420_type       = vs->read(1);
    this->progressive_frame     = vs->read(1);
    this->composite_dsp_flag    = vs->read(1);
    if (this->composite_dsp_flag) {
        this->v_axis            = vs->read(1);
        this->field_seq         = vs->read(3);
        this->sub_carrier       = vs->read(1);
        this->burst_amplitude   = vs->read(7);
        this->sub_carrier_phase = vs->read(8);
    }
    check(this->pic_structure == PIC_STRUCT_F, "Not a frame picture");
}

void Picture::print () {
    printf("=================================================\n");
    printf("Temp ref        : %d\n", this->temp_ref);
    printf("Type            : %d\n", this->type);
    printf("VBV delay       : %d\n", this->vbv_delay);
    printf("Full pel forw v : %d\n", this->full_pel_forw_v);
    printf("Forw f code     : %d\n", this->forw_f_code);
    printf("Full pel back v : %d\n", this->full_pel_back_v);
    printf("Back f code     : %d\n", this->back_f_code);
    printf("Extension\n");
    printf("Fcode forw horz      : %d\n", this->f_code[0][0]        );
    printf("Fcode forw vert      : %d\n", this->f_code[0][1]        );
    printf("Fcode back horz      : %d\n", this->f_code[1][0]        );
    printf("Fcode back vert      : %d\n", this->f_code[1][1]        );
    printf("Intra dc prec        : %d\n", this->intra_dc_prec       );
    printf("Pic structure        : %d\n", this->pic_structure       );
    printf("Top field first      : %d\n", this->top_field_first     );
    printf("Frame pred frame dct : %d\n", this->frame_pred_frame_dct);
    printf("Concealment mv       : %d\n", this->concealment_mv      );
    printf("Q scale type         : %d\n", this->q_scale_type        );
    printf("Intra vlc format     : %d\n", this->intra_vlc_format    );
    printf("Alternate scan       : %d\n", this->alternate_scan      );
    printf("Repeat first field   : %d\n", this->repeat_first_field  );
    printf("Chroma 420 type      : %d\n", this->chroma_420_type     );
    printf("Progressive frame    : %d\n", this->progressive_frame   );
    printf("Composite dsp flag   : %d\n", this->composite_dsp_flag  );
    printf("=================================================\n");
}

void Picture::dump (char* filename) {
    FILE* fp = fopen(filename, "w");
    fprintf(fp, "P3\n%d %d\n255\n", this->horz_size, this->vert_size);
    for (u16 i = 0; i < this->vert_size; ++i) {
        for (u16 j = 0; j < this->horz_size; ++j) {
            u32 index = idx2(i, j, this->horz_size);
            i16 Y  = this->pixel[0][index];
            i16 C1 = this->pixel[1][index] - 128;
            i16 C2 = this->pixel[2][index] - 128;
            u8 R = bandpass(1.0 * Y - 0.00093 * C1 + 1.401687 * C2);
            u8 G = bandpass(1.0 * Y - 0.3437  * C1 - 0.71417  * C2);
            u8 B = bandpass(1.0 * Y + 1.77216 * C1 + 0.00099  * C2);
            fprintf(fp, "%d %d %d ", R, G, B);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

void Picture::dump () {
    char dump_name[128];
    sprintf(dump_name, "pic/%d.ppm", this->temp_ref);
    this->dump(dump_name);
}

void Picture::read_slice (Stream* vs) {
    check(vs->now_start_code() <= SCODE_MAX_SLICE, "Not on slice start header");
    this->mb_row           = vs->now_start_code() - 1;
    if (this->vert_size > 2800)
        mb_row            += (vs->read(3) << 7);
    this->mb_col           = -1;
    this->q_scale_code     = vs->read(5);
    this->intra_slice_flag = vs->read(1);
    if (this->intra_slice_flag) {
        this->intra_slice  = vs->read(1);
        /* reserved bits */  vs->read(7);
        while (vs->read(1)) // Remove Extra Info
            vs->read_u8();
    }
    this->reset_pre_dc_coeff();

    while (!vs->next_bits_eq(0, 23)) {
        this->read_mb(vs);
    }
}

void Picture::read_mb (Stream* vs) {
    // Read Address Increasing
    B1.get(vs);
    //printf("MB Address Increase: %d\n", B1.run);
    this-> mb_col += B1.run;
    if (B1.run > 1)
        this->reset_pre_dc_coeff();
    printf("pic_type = %d, slice = %d, mb_col = %d\n", this->type, this->mb_row, this->mb_col);
    if (B1.run == MB_ESCAPE) {
        // DO something
        printf("MB ESCAPE HAIYAA\n");
        return;
    }

    // Read MB Modes
    switch (this->type) {
        case PIC_TYPE_I:
            B2.get(vs);
            this->mb_mode = B2.run;
            break;
        case PIC_TYPE_P:
            B3.get(vs);
            this->mb_mode = B3.run;
            break;
        case PIC_TYPE_B:
            B4.get(vs);
            this->mb_mode = B4.run;
            break;
    }
    this->mb_quant    = (this->mb_mode >> 6) & 1;
    this->mb_motion_f = (this->mb_mode >> 5) & 1;
    this->mb_motion_b = (this->mb_mode >> 4) & 1;
    this->mb_pattern  = (this->mb_mode >> 3) & 1;
    this->mb_intra    = (this->mb_mode >> 2) & 1;
    this->mb_stwcf    = (this->mb_mode >> 1) & 1; // This two always zero
    this->mb_p_stwc   = (this->mb_mode >> 0) & 1; // in non scalable mode
    printf("MB Type: %d\n", this->mb_mode);
    if (!this->mb_intra)
        this->reset_pre_dc_coeff();

    // Read MV info, Only support frame pred now
    if (this->mb_motion_f || this->mb_motion_b) {
        if (this->frame_pred_frame_dct == 0)
            this->mb_motion_type = vs->read(2);
        else
            // Default, pp. 63
            this->mb_motion_type = 2;
    }
    this->mb_mv_count = 1; // Frame & Non scalable
    this->mb_dmv      = (this->mb_motion_type == 3);

    this->mb_decode_dct_type = (this->pic_structure == PIC_STRUCT_F) && \
                               (this->frame_pred_frame_dct == 0) && \
                               (this->mb_intra || this->mb_pattern);
    if (this->mb_decode_dct_type) // Read decode dct type
        this->mb_dct_type    = vs->read(1); 
    //printf("mb dct %d %d\n", this->mb_decode_dct_type, this->mb_dct_type);

    // Read MB Q scale code
    if (this->mb_quant)
        mb_q_scale_code = vs->read(5);

    // Read MV
    if ((this->mb_motion_f) || 
        (this->mb_intra && this->concealment_mv)) {
        this->read_mvs(vs, 0);
    }
    if (this->mb_motion_b) {
        this->read_mvs(vs, 1);
    }
    if (this->mb_intra && this->concealment_mv)
        vs->read(1);  //marker bit

    // Pattern, only consider 420
    if (this->mb_pattern) {
        B9.get(vs);
        this->mb_pattern_code = B9.run;
    }
    else
        this->mb_pattern_code = 0xff;
    printf("pattern: %x\n", mb_pattern_code);

    // DO BLOCKS, only consider 420, i.e. 4L 1Cb 1Cr
    u8 cc[6]    = {0, 0, 0, 0, 1, 2};
    u8 del_x[6] = {0, 0, 8, 8, 0, 0};
    u8 del_y[6] = {0, 8, 0, 8, 0, 0};
    for (u8 i = 0; i < 6; ++i)
        if (mb_pattern_code & (1 << i)) {
            //printf(" BLOCK %d \n", i);
            this->proc_block(vs, cc[i], this->mb_row * 16 + del_x[i],\
                                        this->mb_col * 16 + del_y[i]);
        }
}

void Picture::read_mvs (Stream* vs, u8 s) {
    printf("read_mvs: %d\n", s);
    if (this->mb_mv_count == 1) {
        // Suppose Frame
        read_mv(vs, 0, s);
    }
    else {
        this->mb_mv_field_select[0][s] = vs->read(1);
        read_mv(vs, 0, s);
        this->mb_mv_field_select[1][s] = vs->read(1);
        read_mv(vs, 1, s);
    }
}

void Picture::read_mv (Stream* vs, u8 r, u8 s) {
    printf("read_mv:  %d %d\n", r, s);
    B10.get(vs);
    this->mb_mv_code[r][s][0] = B10.run;
    if ((this->f_code[s][0] != 1) && (this->mb_mv_code[r][s][0] != 0))
        this->mb_m_residular[r][s][0] = vs->read(this->f_code[s][0] - 1);
    if (this->mb_dmv == 1) {
        B11.get(vs);
        this->mb_dmvector[0] = B11.run;
    }

    B10.get(vs);
    this->mb_mv_code[r][s][1] = B10.run;
    if ((this->f_code[s][0] != 1) && (this->mb_mv_code[r][s][1] != 0))
        this->mb_m_residular[r][s][1] = vs->read(this->f_code[s][1] - 1);
    if (this->mb_dmv == 1) {
        B11.get(vs);
        this->mb_dmvector[1] = B11.run;
    }
}

void Picture::reset_pre_dc_coeff() {
    this->pre_dc_coeff[0] = 1 << (this->intra_dc_prec - 1);
    this->pre_dc_coeff[1] = 1 << (this->intra_dc_prec - 1);
    this->pre_dc_coeff[2] = 1 << (this->intra_dc_prec - 1);
}

void Picture::proc_block(Stream* vs, u8 cc, u16 x, u16 y) {
    Huffman* DC;
    Huffman* AC;
    u8* quant;
    if (this->mb_intra) {
        quant = this->intra_q;
        if (intra_vlc_format) {
            if (cc == 0) {
                DC = &B12;
                AC = &B15;
            }
            else {
                DC = &B13;
                AC = &B15;
            }
        }
        else {
            if (cc == 0) {
                DC = &B12;
                AC = &B14;
            }
            else {
                DC = &B13;
                AC = &B14;
            }
        }
    }
    else {
        // AC == NULL if inter frame
        quant = this->intra_q;
        DC = &B14;
        AC = NULL;
    }

    Block block(vs, this->pre_dc_coeff[cc], DC, AC, quant, \
            this->intra_dc_prec, this->q_scale_type, this->q_scale_code);
    this->pre_dc_coeff[cc] = block.dc_coeff;
    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 8; ++j)
            if (!cc)
                this->pixel[cc][idx2(x + i, y + j, this->horz_size)] = block.data[idx(i, j)];
            else {
                this->pixel[cc][idx2(x + 2*i    , y+2 * j    , this->horz_size)] = block.data[idx(i, j)];
                this->pixel[cc][idx2(x + 2*i + 1, y+2 * j    , this->horz_size)] = block.data[idx(i, j)];
                this->pixel[cc][idx2(x + 2*i    , y+2 * j + 1, this->horz_size)] = block.data[idx(i, j)];
                this->pixel[cc][idx2(x + 2*i + 1, y+2 * j + 1, this->horz_size)] = block.data[idx(i, j)];
            }
}

void Picture::decode (Stream* vs) {
    while (1) {
        vs->next_start_code();
        if (vs->now_start_code() > SCODE_MAX_SLICE || !vs->now_start_code()) {
            vs->keep_start_code();
            this->dump();
            printf("Finish Decode Picture %d\n", this->temp_ref);
            break;
        }
        this->read_slice(vs);
    }
}
//=====================================================

int main (int argc, char* argv[]) {
    //B9.triverse();
    //B10.triverse();
    //B12.triverse();
    //B13.triverse();
    //B14.triverse();
    //B15.triverse();
    check(argc == 2, "Wrong parameters");
    Stream* vs = new Stream(argv[1]);
    Seq* seq;
    Gop* gop;
    Picture*    pic_now = NULL;
    Picture*    ref_f   = NULL;
    Picture*    ref_b   = NULL;

    while (true) {
        vs->next_start_code();
        printf("Start Code: %x\n", vs->now_start_code());
        switch (vs->now_start_code()) {
            case SCODE_SEQ:
                seq = new Seq(vs);
                seq->print();
                break;
            case SCODE_USR:
                /* DO NOTHING*/
                break;
            case SCODE_GOP:
                gop = new Gop(vs);
                gop->print();
                break;
            case SCODE_PIC:
                if (pic_now != NULL) {
                    switch (pic_now->type) {
                        case PIC_TYPE_I:
                            if (ref_f)
                                delete ref_f;
                            if (ref_b)
                                delete ref_b;
                            ref_f = pic_now;
                            ref_b = NULL;
                            break;
                        case PIC_TYPE_P:
                            if (ref_b) {
                                delete ref_f;
                                ref_f = ref_b;
                                ref_b = pic_now;
                            }
                            else
                                ref_b = pic_now;
                            break;
                        case PIC_TYPE_B:
                            break;
                    }
                }
                pic_now = new Picture(vs, seq);
                pic_now->print();
                pic_now->decode(vs);
                break;
            case SCODE_END:
                printf("End of sequence\n");
                break;
            default:
                printf("Unknow start Code: %x\n", vs->now_start_code());
        }
    }
}

