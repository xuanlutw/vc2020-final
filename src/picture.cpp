#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "picture.h"
#include "slice.h"

Picture::Picture (Stream* vs, Seq* seq, Picture* ref[2]) {
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
    this->pixel[0]  = new i16[this->horz_size * this->vert_size]();
    this->pixel[1]  = new i16[this->horz_size * this->vert_size / 4]();
    this->pixel[2]  = new i16[this->horz_size * this->vert_size / 4]();
    this->seq       = seq;
    switch(this->type) {
        case PIC_TYPE_P:
            this->ref[0] = ref[1];
            break;
        case PIC_TYPE_B:
            this->ref[0] = ref[0];
            this->ref[1] = ref[1];
    }

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

Picture::~Picture () {
    delete this->pixel[0];
    delete this->pixel[1];
    delete this->pixel[2];
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
    printf("Temp ref             : %d\n", this->temp_ref);
    printf("Type                 : %d\n", this->type);
    printf("VBV delay            : %d\n", this->vbv_delay);
    printf("Full pel forw v      : %d\n", this->full_pel_forw_v);
    printf("Forw f code          : %d\n", this->forw_f_code);
    printf("Full pel back v      : %d\n", this->full_pel_back_v);
    printf("Back f code          : %d\n", this->back_f_code);
    printf("Extension\n");
    printf("Fcode forw horz      : %d\n", this->f_code[0][0]);
    printf("Fcode forw vert      : %d\n", this->f_code[0][1]);
    printf("Fcode back horz      : %d\n", this->f_code[1][0]);
    printf("Fcode back vert      : %d\n", this->f_code[1][1]);
    printf("Intra dc prec        : %d\n", this->intra_dc_prec);
    printf("Pic structure        : %d\n", this->pic_structure);
    printf("Top field first      : %d\n", this->top_field_first);
    printf("Frame pred frame dct : %d\n", this->frame_pred_frame_dct);
    printf("Concealment mv       : %d\n", this->concealment_mv);
    printf("Q scale type         : %d\n", this->q_scale_type);
    printf("Intra vlc format     : %d\n", this->intra_vlc_format);
    printf("Alternate scan       : %d\n", this->alternate_scan);
    printf("Repeat first field   : %d\n", this->repeat_first_field);
    printf("Chroma 420 type      : %d\n", this->chroma_420_type);
    printf("Progressive frame    : %d\n", this->progressive_frame);
    printf("Composite dsp flag   : %d\n", this->composite_dsp_flag);
    printf("=================================================\n");
}

void Picture::dump (char* filename) {
    FILE* fp = fopen(filename, "w");
    fprintf(fp, "P3\n%d %d\n255\n", this->horz_size, this->vert_size);
    for (u16 i = 0; i < this->seq->vert_size; ++i) {
        for (u16 j = 0; j < this->seq->horz_size; ++j) {
            u32 idx_l = idx2(i, j, this->horz_size);
            u32 idx_c = idx2(i/2, j/2, this->horz_size/2);
            i16 Y  = this->pixel[0][idx_l];
            i16 C1 = this->pixel[1][idx_c] - 128;
            i16 C2 = this->pixel[2][idx_c] - 128;
            u8  R  = bandpass(1.0 * Y - 0.00093 * C1 + 1.401687 * C2);
            u8  G  = bandpass(1.0 * Y - 0.3437  * C1 - 0.71417  * C2);
            u8  B  = bandpass(1.0 * Y + 1.77216 * C1 + 0.00099  * C2);
            fprintf(fp, "%d %d %d ", R, G, B);
            //fprintf(fp, "%d %d %d ", bandpass(Y), 0, 0);
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

void Picture::write_YUV (FILE* fp) {
    for (u16 i = 0; i < this->seq->vert_size; ++i)
        for (u16 j = 0; j < this->seq->horz_size; ++j) {
            u8 data = this->pixel[0][idx2(i, j, this->horz_size)];
            fwrite(&data, sizeof(u8), 1, fp);
        }
    fflush(fp);
    for (u16 i = 0; i < this->seq->vert_size / 2; ++i)
        for (u16 j = 0; j < this->seq->horz_size / 2; ++j) {
            u8 data = this->pixel[1][idx2(i, j, this->horz_size/2)];
            fwrite(&data, sizeof(u8), 1, fp);
        }
    fflush(fp);
    for (u16 i = 0; i < this->seq->vert_size / 2; ++i)
        for (u16 j = 0; j < this->seq->horz_size / 2; ++j) {
            u8 data = this->pixel[2][idx2(i, j, this->horz_size/2)];
            fwrite(&data, sizeof(u8), 1, fp);
        }
    fflush(fp);
}

void Picture::decode (Stream* vs) {
    while (true) {
        vs->next_start_code();
        if (vs->now_start_code() > SCODE_MAX_SLICE || !vs->now_start_code()) {
            vs->keep_start_code();
            printf("Finish Decode Picture %d\n", this->temp_ref);
            break;
        }
        Slice slice(vs, this);
        slice.decode(vs);
    }
    for (u32 x = 0; x < this->horz_size * this->vert_size; ++x)
            saturate_u8(this->pixel[0][x]);
    for (u32 x = 0; x < this->horz_size * this->vert_size / 4; ++x) {
            saturate_u8(this->pixel[1][x]);
            saturate_u8(this->pixel[2][x]);
    }
}
