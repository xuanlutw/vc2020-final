#include <cstdio>
#include <cstdlib>
#include <cmath>
#include "picture.h"
#include "slice.h"
#include "mb.h"
#include "block.h"

MB::MB (Stream* vs, Slice* slice) {
    this->pic   = slice->pic;
    this->slice = slice;

    // Read Address Increasing and Do MV Pred
    while (true) {
        B1.get(vs);
        for (u8 counter = 0; counter < B1.run - 1; ++counter) {
            ++this->slice->mb_col;
            this->slice->reset_pre_dc_coeff();
            switch (this->pic->type) {
                case PIC_TYPE_P:
                    this->slice->reset_pmv();
                    this->reset_mv();
                    this->pred();
                    break;
                case PIC_TYPE_B:
                    this->mb_motion_f = this->slice->pre_mb_motion_f;
                    this->mb_motion_b = this->slice->pre_mb_motion_b;
                    this->set_mv();
                    this->pred();
                    break;
            }
        }
        if (B1.run != MB_ESCAPE) {
            ++this->slice->mb_col;
            break;
        }
    }
    printf("pic_type = %d, slice = %d, mb_col = %d, mb_count = %d\n", \
            this->pic->type, this->slice->mb_row, this->slice->mb_col, \
            this->slice->mb_row * this->pic->horz_size / 16 + this->slice->mb_col);

    // Read MB Modes
    this->read_mode(vs);

    // Read MB Q scale code
    if (this->mb_quant)
        this->q_scale_code = vs->read(5);
    else
        this->q_scale_code = this->slice->q_scale_code;

    // Read MV
    if ((this->mb_motion_f) || 
        (this->mb_intra && this->pic->concealment_mv))
        this->read_mvs(vs, 0);
    if (this->mb_motion_b)
        this->read_mvs(vs, 1);
    if (this->mb_intra && this->pic->concealment_mv)
        vs->read(1);  //marker bit
    if (this->mb_intra && !this->pic->concealment_mv)
        this->slice->reset_pmv();
    if (this->pic->type == PIC_TYPE_P && !this->mb_intra && !this->mb_motion_f) {
        this->slice->reset_pmv();
        this->reset_mv();
    }
    if (!this->mb_intra)
        this->pred();

    // Read Pattern, only consider 420 mode
    if (this->mb_pattern)
        this->pattern_code = B9.get(vs);
    else if (mb_intra)
        this->pattern_code = 0xff;
    else 
        this->pattern_code = 0;
    //printf("Pattern: %x\n", this->pattern_code);
}

void MB::read_mode (Stream* vs) {
    u8 mode = 0;
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
    this->slice->pre_mb_motion_f = this->mb_motion_f;
    this->slice->pre_mb_motion_b = this->mb_motion_b;
    printf("MB Type: %d\n", mode);

    // Read MV info, suppose frame pred and non scalable
    if (this->mb_motion_f || this->mb_motion_b) {
        if (this->pic->frame_pred_frame_dct == 0)
            this->motion_type = vs->read(2);
        else    // Default, pp. 63
            this->motion_type = 2;
    }
    this->mv_count = (this->motion_type == 1) + 1;
    this->dmv      = (this->motion_type == 3);

    // Read DCT type
    this->decode_dct_type = (this->pic->pic_structure == PIC_STRUCT_F) && \
                            (this->pic->frame_pred_frame_dct == 0) && \
                            (this->mb_intra || this->mb_pattern);
    if (this->decode_dct_type)
        this->dct_type    = vs->read(1);
    else    // default value for frame pred, pp. 64
        this->dct_type    = 0;
}

void MB::read_mvs (Stream* vs, u8 s) {
    //printf("read_mvs: %d\n", s);
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
    //printf("read_mv:  %d %d\n", r, s);
    this->mv_code[r][s][0] = B10.get(vs);
    if ((this->pic->f_code[s][0] != 1) && (this->mv_code[r][s][0] != 0))
        this->mv_residual[r][s][0] = vs->read(this->pic->f_code[s][0] - 1);
    if (this->dmv == 1)
        this->dmvector[0] = B11.get(vs);

    this->mv_code[r][s][1] = B10.get(vs);
    if ((this->pic->f_code[s][1] != 1) && (this->mv_code[r][s][1] != 0))
        this->mv_residual[r][s][1] = vs->read(this->pic->f_code[s][1] - 1);
    if (this->dmv == 1)
        this->dmvector[1] = B11.get(vs);

    // Update
    for (u8 t = 0; t < 2; ++t) {
        u8  r_size = this->pic->f_code[s][t] - 1;
        u16 f      = 1 << r_size;
        i16 high   =  16 * f - 1;
        i16 low    = -16 * f;
        i16 range  =  32 * f;
        i16 delta;
        if ((f == 1) || (this->mv_code[r][s][t] == 0))
            delta = this->mv_code[r][s][t];
        else {
            delta = ((abs(this->mv_code[r][s][t]) - 1) * f) + \
                    this->mv_residual[r][s][t] + 1;
            if (this->mv_code[r][s][t] < 0)
                delta = -delta;
        }

        // Suppose frame pred
        this->mv[r][s][t] = this->slice->pmv[r][s][t] + delta;
        if (this->mv[r][s][t] < low)
            this->mv[r][s][t] += range;
        if (this->mv[r][s][t] > high)
            this->mv[r][s][t] -= range;

        // Again, suppose frame
        this->slice->pmv[r][s][t] = this->mv[r][s][t];
        this->comp_mv(r, s, t);
    }
}

void MB::comp_mv (u8 r, u8 s, u8 t) {
    for (u8 cc = 0; cc < 3; ++cc) {
        i16 reduce_mv = this->mv[r][s][t] / (1 + (cc > 0));
        this->int_mv[cc][r][s][t] = (reduce_mv - (reduce_mv < 0)) / 2;
        this->half_f[cc][r][s][t] = \
            (this->int_mv[cc][r][s][t] * 2 != reduce_mv);
    }
}

void MB::reset_mv () {
    for (u8 r = 0; r < 2; ++r)
        for (u8 s = 0; s < 2; ++s)
            for (u8 t = 0; t < 2; ++t) {
                this->mv[r][s][t] = 0;
                for (u8 cc = 0; cc < 3; ++cc)
                    this->int_mv[cc][r][s][t] = this->half_f[cc][r][s][t] = 0;
            }
}

void MB::set_mv () {
    for (u8 r = 0; r < 2; ++r)
        for (u8 s = 0; s < 2; ++s)
            for (u8 t = 0; t < 2; ++t) {
                this->mv[r][s][t] = this->slice->pmv[r][s][t];
                this->comp_mv(r, s, t);
            }
}

void MB::pred () {
    u8  ccs[6]   = {0, 0, 0, 0, 1, 2};
    u8  del_x[6] = {0, 0, 8, 8, 0, 0};
    u8  del_y[6] = {0, 8, 0, 8, 0, 0};
    for (u8 counter = 0; counter < 6; ++counter) {
        u8  cc        = ccs[counter];
        u16 x         = (this->slice->mb_row * 16 + del_x[counter]) >> (cc > 0);
        u16 y         = (this->slice->mb_col * 16 + del_y[counter]) >> (cc > 0);
        u16 horz_size = this->pic->horz_size >> (cc > 0);
        for (int i = 0; i < 8; ++i)
            for (int j = 0; j < 8; ++j)
                switch (this->pic->type) {
                    case PIC_TYPE_P:
                        this->pic->pixel[cc][idx2(x + i, y + j, horz_size)] = \
                            this->pred_pixel(0, cc, x + i, y + j);
                        break;
                    case PIC_TYPE_B:
                        if (this->mb_motion_f && this->mb_motion_b)
                            this->pic->pixel[cc][idx2(x + i, y + j, horz_size)] = \
                                (this->pred_pixel(0, cc, x + i, y + j) +
                                 this->pred_pixel(1, cc, x + i, y + j)) >> 1;
                        else if (this->mb_motion_f)
                            this->pic->pixel[cc][idx2(x + i, y + j, horz_size)] = \
                                (this->pred_pixel(0, cc, x + i, y + j));
                        else if (this->mb_motion_b)
                            this->pic->pixel[cc][idx2(x + i, y + j, horz_size)] = \
                                (this->pred_pixel(1, cc, x + i, y + j));
                        break;
                }
    }
}

u8 MB::pred_pixel (u8 s, u8 cc, u16 x, u16 y) {
    u16 horz_size = this->pic->horz_size >> (cc > 0);
    i16 int_mv_0  = this->int_mv[cc][0][s][0];
    i16 int_mv_1  = this->int_mv[cc][0][s][1];
    u32 idx_1     = idx2(int_mv_1 + x    , int_mv_0 + y    , horz_size);
    u32 idx_2     = idx2(int_mv_1 + x + 1, int_mv_0 + y    , horz_size);
    u32 idx_3     = idx2(int_mv_1 + x    , int_mv_0 + y + 1, horz_size);
    u32 idx_4     = idx2(int_mv_1 + x + 1, int_mv_0 + y + 1, horz_size);
    u16 data      = 0;

    if (!this->half_f[cc][0][s][0] && !this->half_f[cc][0][s][1])
        return (this->pic->ref[s]->pixel[cc][idx_1]);
    else if ( this->half_f[cc][0][s][0] && !this->half_f[cc][0][s][1]) {
        data = (this->pic->ref[s]->pixel[cc][idx_1] + \
                this->pic->ref[s]->pixel[cc][idx_3]);
        return (data > 0)? (data + 1) >> 1: -((-data + 1) >> 1);
    }
    else if (!this->half_f[cc][0][s][0] &&  this->half_f[cc][0][s][1]) {
        data = (this->pic->ref[s]->pixel[cc][idx_1] + \
                this->pic->ref[s]->pixel[cc][idx_2]);
        return (data > 0)? (data + 1) >> 1: -((-data + 1) >> 1);
    }
    else {
        data = (this->pic->ref[s]->pixel[cc][idx_1] + \
                this->pic->ref[s]->pixel[cc][idx_2] + \
                this->pic->ref[s]->pixel[cc][idx_3] + \
                this->pic->ref[s]->pixel[cc][idx_4]);
        return (data > 0)?  (data + 2) >> 2: -((-data + 2) >> 2);
    }
}

void MB::decode (Stream* vs) {
    Huffman* DC;
    Huffman* AC;
    u8* quant;
    u8  ccs[6]   = {0, 0, 0, 0, 1, 2};
    u8  del_x[6] = {0, 0, 8, 8, 0, 0};
    u8  del_y[6] = {0, 8, 0, 8, 0, 0};
    for (u8 counter = 0; counter < 6; ++counter) {
        u8  cc        = ccs[counter];
        u16 x         = (this->slice->mb_row * 16 + del_x[counter]) >> (cc > 0);
        u16 y         = (this->slice->mb_col * 16 + del_y[counter]) >> (cc > 0);
        u16 horz_size = this->pic->horz_size >> (cc > 0);
        if (!(this->pattern_code & (1 << (5 - counter))))
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
        for (int i = 0; i < 8; ++i)
            for (int j = 0; j < 8; ++j)
                this->pic->pixel[cc][idx2(x + i, y + j, horz_size)] = \
                    saturate(this->pic->pixel[cc][idx2(x + i, y + j, horz_size)] + \
                             block.data[idx(i, j)]);
        this->slice->pre_dc_coeff[cc] = block.dc_coeff;
    }
    if (!this->mb_intra)
        this->slice->reset_pre_dc_coeff();
}
