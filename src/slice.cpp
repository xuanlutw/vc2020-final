#include <cstdio>
#include <cstdlib>
#include "picture.h"
#include "slice.h"
#include "mb.h"

Slice::Slice (Stream* vs, Picture* pic) {
    check(vs->now_start_code() <= SCODE_MAX_SLICE, "Not on slice start header");
    this->pic             = pic;
    this->mb_row          = vs->now_start_code() - 1;
    if (this->pic->vert_size > 2800)
       this->mb_row      += (vs->read(3) << 7);
    this->mb_col          = -1;
    this->q_scale_code    = vs->read(5);
    this->intra_slice_f   = vs->read(1);
    if (this->intra_slice_f) {
        this->intra_slice = vs->read(1);
        /* reserved bits */ vs->read(7);
        while (vs->read(1)) 
            vs->read_u8();
    }
}

void Slice::decode (Stream* vs) {
    this->reset_pre_dc_coeff();
    this->reset_pmv();
    while (!vs->next_bits_eq(0, 23)) {
        MB mb(vs, this);
        mb.decode(vs);
    }
}

void Slice::reset_pre_dc_coeff() {
    this->pre_dc_coeff[0] = 1 << (this->pic->intra_dc_prec - 1);
    this->pre_dc_coeff[1] = 1 << (this->pic->intra_dc_prec - 1);
    this->pre_dc_coeff[2] = 1 << (this->pic->intra_dc_prec - 1);
}

void Slice::reset_pmv() {
    for (u8 r = 0; r < 2; ++r)
        for (u8 s = 0; s < 2; ++s)
            for (u8 t = 0; t < 2; ++t)
                this->pmv[r][s][t] = 0;
}
