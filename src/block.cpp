#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include "block.h"

Block::Block (Stream* vs, i16 pre_dc_coeff, Huffman* DC, Huffman* AC, \
        u8* quant, u8 intra_dc_prec, u8 q_scale_type, u8 q_scale_code) {
    for (int i = 0; i < 64; ++i)
        data[i] = 0;

    // Read coefficients
    if (AC) {   // Intra
        DC->get(vs);
        data[0] = (this->dc_coeff = pre_dc_coeff + DC->level);
        for (int i = 1; i <= 64; ++i) {
            i += AC->get(vs);
            if (AC->type == HUFFMAN_CODE_END)
                break;
            else
                data[i] = AC->level;
        }
    }
    else {      // Inter
        DC->set_is_DC();
        for (int i = 0; i <= 64; ++i) {
            i += DC->get(vs);
            if (DC->type == HUFFMAN_CODE_END)
                break;
            else
                data[i] = DC->level;
        }
    }
    //this->print();

    this->inverse_scan();
    //this->print();

    this->inverse_q(quant, AC? intra_dc_prec: 0, q_scale_type, q_scale_code);
    //this->print();

    this->inverse_DCT();
    //this->print();
}

void Block::inverse_scan () {
    i16 tmp[64];
    memcpy(tmp, this->data, sizeof(i16) * 64);
    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 8; ++j)
            this->data[idx(i, j)] = tmp[zz[idx(i, j)]];
}

void Block::inverse_q (u8* quant, \
                       u8 intra_dc_prec, u8 q_scale_type, u8 q_scale_code) {
    // intra_dc_prec = 0 indicate inter
    i64 sum = 0;

    // IQ
    for (int v = 0; v < 8; v++)
        for (int u = 0; u < 8; u++)
            if (!u && !v && intra_dc_prec ) { // Inter DC coeff
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
            else {  // Other coeff
                if (intra_dc_prec)
                    data[idx(v, u)] = \
                        (data[idx(v, u)] * quant[idx(v, u)] * \
                        q_scale[q_scale_type][q_scale_code] * 2) / 32;
                else
                    data[idx(v, u)] = \
                        ((data[idx(v, u)] * 2 + sign(data[idx(v, u)])) * \
                        quant[idx(v, u)] * q_scale[q_scale_type][q_scale_code]) / 32;
            }

    // Saturation
    for (int v = 0; v < 8; v++)
        for (int u = 0; u < 8; u++) {
            if (data[idx(v, u)] > 2047)
                data[idx(v, u)] = 2047;
            else if (data[idx(v, u)] < -2048 )
                data[idx(v, u)] = -2048;
            sum += data[idx(v, u)];
        }

    // Mismatch control
    if ((sum & 1) == 0) {
        if ((data[idx(7, 7)] & 1) != 0)
            data[idx(7, 7)] = data[idx(7, 7)] - 1;
        else
            data[idx(7, 7)] = data[idx(7, 7)] + 1;
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
    memcpy(this->data, tmp, sizeof(i16) * 64);
    return;
}

void Block::inverse_DCT () {
    this->inverse_DCT1();
    this->inverse_DCT1();
}

void Block::print () {
    for (int i = 0; i < 8; ++i) {
        printf("  ");
        for (int j = 0; j < 8; ++j)
            printf("%3d ", data[idx(i, j)]);
        printf("\n");
    }
    printf("\n");
}
