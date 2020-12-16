#include <cstdio>
#include <cstdlib>
#include "picture.h"
#include "slice.h"
#include "mb.h"
#include "block.h"

MB::MB (Stream* vs, Slice* slice) {
    this->pic   = slice->pic;
    this->slice = slice;

    // Read Address Increasing
    while (B1.get(vs) == MB_ESCAPE) {
        this->slice->mb_col += MB_ESCAPE - 1;
        this->slice->reset_pre_dc_coeff();
        printf("MB ESCAPE HAIYAA\n");
    }
    this->slice->mb_col += B1.run;
    if (B1.run > 1)
        this->slice->reset_pre_dc_coeff();
    printf("pic_type = %d, slice = %d, mb_col = %d\n", \
            this->pic->type, this->slice->mb_row, this->slice->mb_col);

    // Read MB Modes
    this->read_mode(vs);

    // Read MB Q scale code
    if (this->mb_quant)
        q_scale_code = vs->read(5);
    else
        q_scale_code = this->slice->q_scale_code;

    // Read MV
    if ((this->mb_motion_f) || 
        (this->mb_intra && this->pic->concealment_mv))
        this->read_mvs(vs, 0);
    if (this->mb_motion_b)
        this->read_mvs(vs, 1);
    if (this->mb_intra && this->pic->concealment_mv)
        vs->read(1);  //marker bit

    // Read Pattern, only consider 420 mode
    if (this->mb_pattern)
        this->pattern_code = B9.get(vs);
    else
        this->pattern_code = 0xff;
    printf("Pattern: %x\n", this->pattern_code);

}

void MB::read_mode (Stream* vs) {
    u8 mode;
    switch (this->pic->type) {
        case PIC_TYPE_I:
            mode = B2.get(vs);
            break;
        case PIC_TYPE_P:
            mode = B3.get(vs);
            break;
        case PIC_TYPE_B:
            mode = B4.get(vs);
            break;
    }
    this->mb_quant    = (mode >> 6) & 1;
    this->mb_motion_f = (mode >> 5) & 1;
    this->mb_motion_b = (mode >> 4) & 1;
    this->mb_pattern  = (mode >> 3) & 1;
    this->mb_intra    = (mode >> 2) & 1;
    this->stwcf       = (mode >> 1) & 1; 
    this->p_stwc      = (mode >> 0) & 1; 
    printf("MB Type: %d\n", mode);

    // Read MV info, suppose frame pred and non scalable
    if (this->mb_motion_f || this->mb_motion_b) {
        if (this->pic->frame_pred_frame_dct == 0)
            this->motion_type = vs->read(2);
        else    // Default, pp. 63
            this->motion_type = 2;
    }
    this->mv_count = 1;
    this->dmv      = (this->motion_type == 3);

    // Read DCT type
    this->decode_dct_type = (this->pic->pic_structure == PIC_STRUCT_F) && \
                            (this->pic->frame_pred_frame_dct == 0) && \
                            (this->mb_intra || this->mb_pattern);
    if (this->decode_dct_type)
        this->dct_type    = vs->read(1); 
}

void MB::read_mvs (Stream* vs, u8 s) {
    printf("read_mvs: %d\n", s);
    if (this->mv_count == 1)    // Suppose Frame
        read_mv(vs, 0, s);
    else {
        this->mv_field_select[0][s] = vs->read(1);
        read_mv(vs, 0, s);
        this->mv_field_select[1][s] = vs->read(1);
        read_mv(vs, 1, s);
    }
}

void MB::read_mv (Stream* vs, u8 r, u8 s) {
    printf("read_mv:  %d %d\n", r, s);
    this->mv_code[r][s][0] = B10.get(vs);
    if ((this->pic->f_code[s][0] != 1) && (this->mv_code[r][s][0] != 0))
        this->mv_residular[r][s][0] = vs->read(this->pic->f_code[s][0] - 1);
    if (this->dmv == 1)
        this->dmvector[0] = B11.get(vs);

    this->mv_code[r][s][1] = B10.get(vs);
    if ((this->pic->f_code[s][0] != 1) && (this->mv_code[r][s][1] != 0))
        this->mv_residular[r][s][1] = vs->read(this->pic->f_code[s][1] - 1);
    if (this->dmv == 1)
        this->dmvector[1] = B11.get(vs);
}

void MB::decode (Stream* vs) {
    Huffman* DC;
    Huffman* AC;
    u8* quant;
    u8  ccs[6]   = {0, 0, 0, 0, 1, 2};
    u8  del_x[6] = {0, 0, 8, 8, 0, 0};
    u8  del_y[6] = {0, 8, 0, 8, 0, 0};
    for (u8 counter = 0; counter < 6; ++counter) {
        u8  cc = ccs[counter];
        u16 x  = this->slice->mb_row * 16 + del_x[counter]; 
        u16 y  = this->slice->mb_col * 16 + del_y[counter]; 
        if (!(this->pattern_code & (1 << counter))) 
            continue;
        if (this->mb_intra) {
            quant = this->pic->intra_q;
            DC = cc? &B13: &B12;
            AC = pic->intra_vlc_format? &B15: &B14;
        }
        else {
            // AC == NULL if inter frame
            quant = this->pic->inter_q;
            DC = &B14;
            AC = NULL;
        }
        Block block(vs, this->slice->pre_dc_coeff[cc], DC, AC, quant, \
                this->pic->intra_dc_prec, this->pic->q_scale_type, this->q_scale_code);
        this->slice->pre_dc_coeff[cc] = block.dc_coeff;
        for (int i = 0; i < 8; ++i)
            for (int j = 0; j < 8; ++j)
                if (!cc)
                    this->pic->pixel[cc][idx2(x + i, y + j, this->pic->horz_size)] = \
                        block.data[idx(i, j)];
                else
                    this->pic->pixel[cc][idx2(x+2*i  , y+2*j  , this->pic->horz_size)] = \
                    this->pic->pixel[cc][idx2(x+2*i+1, y+2*j  , this->pic->horz_size)] = \
                    this->pic->pixel[cc][idx2(x+2*i  , y+2*j+1, this->pic->horz_size)] = \
                    this->pic->pixel[cc][idx2(x+2*i+1, y+2*j+1, this->pic->horz_size)] = \
                        block.data[idx(i, j)];
    }
    if (!this->mb_intra)
        this->slice->reset_pre_dc_coeff();
}
