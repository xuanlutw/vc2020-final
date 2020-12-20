#include <stdio.h>
#include <stdlib.h>
#include "seq.h"

Seq::Seq (Stream* vs) {
    check(vs->now_start_code() == SCODE_SEQ, "Not on sequence start header");
    this->horz_size       = vs->read(12);
    this->vert_size       = vs->read(12);
    this->aspect_ratio    = vs->read(4);
    this->frame_rate_code = vs->read(4);
    this->bit_rate        = vs->read(18);
    /* marker bit */        vs->read(1);
    this->vbv_buf_size    = vs->read(10);
    if (vs->read(1))
        for (int i = 0; i < 64; ++i)
            this->intra_q[i] = vs->read_u8();
    else
        for (int i = 0; i < 64; ++i)
            this->intra_q[i] = default_intra_q[i];
    if (vs->read(1))
        for (int i = 0; i < 64; ++i)
            this->inter_q[i] = vs->read_u8();
    else
        for (int i = 0; i < 64; ++i)
            this->inter_q[i] = default_inter_q[i];
    switch (this->frame_rate_code) {
        case 1:
            this->frame_rate = 24000. / 1001.;
            break;
        case 2:
            this->frame_rate = 24.;
            break;
        case 3:
            this->frame_rate = 25.;
            break;
        case 4:
            this->frame_rate = 30000. / 1001.;
            break;
        case 5:
            this->frame_rate = 30.;
            break;
        case 6:
            this->frame_rate = 50.;
            break;
        case 7:
            this->frame_rate = 60000. / 1001.;
            break;
        case 8:
            this->frame_rate = 60.;
            break;
    }

    // Read sequence ext
    while (vs->next_start_code() == SCODE_EXT) {
        switch (vs->now_ext_code()) {
            case ECODE_SEQ_EXT:
                this->read_ext(vs);
                break;
            case ECODE_SEQ_DSP:
                this->read_dsp(vs);
                break;
            default:
                check(0, "Wrong ext code after sequence header");
        }
    }
    vs->keep_start_code();
}

void Seq::read_ext (Stream* vs) {
    check(vs->now_start_code() == SCODE_EXT, "Not on sequence start header");
    check(vs->now_ext_code() == ECODE_SEQ_EXT, "Not on ext sequence ext code");
    this->profil_level_id  = vs->read(8);
    this->progressive_seq  = vs->read(1);
    this->chroma_format    = vs->read(2);
    this->horz_size       += vs->read(2) << 12;
    this->vert_size       += vs->read(2) << 12;
    this->bit_rate        += vs->read(12) << 18;
    /* marker bit */         vs->read(1);
    this->vbv_buf_size    += vs->read(8) << 8;
    this->low_delay        = vs->read(1);
    this->frame_rate_ext_n = vs->read(2);
    this->frame_rate_ext_d = vs->read(5);
    check(this->chroma_format == 1, "Not chroma format 420");
}

void Seq::read_dsp (Stream* vs) {
    check(vs->now_start_code() == SCODE_EXT, "Not on sequence start header");
    check(vs->now_ext_code() == ECODE_SEQ_DSP, "Not on ext sequence dsp code");
    this->video_format                 = vs->read(3);
    this->colour_discription           = vs->read(1);
    if (this->colour_discription) {
        this->colour_primaries         = vs->read(8);
        this->transfer_characteristics = vs->read(8);
        this->matrix_coefficients      = vs->read(8);
    }
    this->display_horz_size            = vs->read(14);
    /* marker bit */                     vs->read(1);
    this->display_vert_size            = vs->read(14);
}

void Seq::print () {
    printf("=================================================\n");
    printf("Horizontal size  : %d\n", this->horz_size);
    printf("Vertical size    : %d\n", this->vert_size);
    printf("Aspect ratio     : %d\n", this->aspect_ratio);
    printf("Frame rate code  : %d\n", this->frame_rate_code);
    printf("Frame rate       : %lf\n",this->frame_rate);
    printf("Bit rate         : %d\n", this->bit_rate);
    printf("VBV buf size     : %d\n", this->vbv_buf_size);
    printf("Intra quantizer  :");
    for (int i = 0; i < 8; ++i) {
        if (i)
            printf("                  ");
        for (int j = 0; j < 8; ++j)
            printf("%2d ", this->intra_q[idx(i, j)]);
        printf("\n");
    }
    printf("Inter quantizer  :");
    for (int i = 0; i < 8; ++i) {
        if (i)
            printf("                  ");
        for (int j = 0; j < 8; ++j)
            printf("%2d ", this->inter_q[idx(i, j)]);
        printf("\n");
    }

    printf("Extension\n");
    printf("Profile level id : %d\n", this->profil_level_id);
    printf("Progressive seq  : %d\n", this->progressive_seq);
    printf("Chroma format    : %d\n", this->chroma_format);
    printf("Low delay        : %d\n", this->low_delay);
    printf("Frame rate ext n : %d\n", this->frame_rate_ext_n);
    printf("Frame rate ext d : %d\n", this->frame_rate_ext_d);

    printf("Display Extension\n");
    printf("Video format     : %d\n", this->video_format);
    printf("Colour Discriptio: %d\n", this->colour_discription);
    printf("Colour Primaries : %d\n", this->colour_primaries);
    printf("Transfer Charact : %d\n", this->transfer_characteristics);
    printf("Matrix_coefficie : %d\n", this->matrix_coefficients);
    printf("Display horz size: %d\n", this->display_horz_size);
    printf("Display vert size: %d\n", this->display_vert_size);
    printf("=================================================\n");
}
