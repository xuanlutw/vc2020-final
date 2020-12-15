#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cmath>
#include "default.h"

# define check(cond, msg, ...) do {                                             \
                                   if (!(cond)) {                               \
                                       fprintf(stderr, msg"\n", ##__VA_ARGS__); \
                                       exit(-1);                                \
                                   }                                            \
                               } while(0);

u8 bandpass (double val) {
    // Bandpass filter

    if (val >= 255.0)
        return 255;
    else if (val <= 0.0)
        return 0;
    else
        return (u8) round(val);
}

class Stream {
    public:
        Stream (char* file_name);
        ~Stream ();
        u32  read (u8 len);
        u8   read_u8 ();
        u16  read_u16 ();
        void put (u32 content, u8 len);
        u8   next_start_code ();
        u8   next_bits_eq (u32, u8);
        u8   now_start_code ();
        u8   now_ext_code ();
    private:
        FILE* fp;
        u32   buf;
        u8    buf_len;
        u32   depes_buf;      // Remove stupid PES header
        u8    depes_buf_len;
        u32   counter;
        u8    start_code;
        u8    ext_code;
        void  read_buf ();
        void  align ();
};

Stream::Stream(char* file_name) {
    this->fp            = fopen(file_name, "rb");
    this->buf           = 0;
    this->buf_len       = 0;
    this->depes_buf     = 0;
    this->depes_buf_len = 0;
    this->counter       = 0;
    check(fp, "Open file %s fail!\n", file_name);
}

Stream::~Stream() {
    fclose(this->fp);
}

void Stream::read_buf () {
    u8 c;
    // To handle PES, not use now
    //while (1) {
        //if (this->depes_buf_len == 32) {
            //if (this->depes_buf == 0x000001e0) {
                //// FLUSH
                //this->depes_buf_len = 0;
                //this->depes_buf     = 0;
                //check(fread(&c, sizeof(u8), 1, this->fp), "Reach EOF");
                //printf("|%x\n", c);
                //check(fread(&c, sizeof(u8), 1, this->fp), "Reach EOF");
                //printf("|%x\n", c);
                //check(fread(&c, sizeof(u8), 1, this->fp), "Reach EOF");
                //printf("|%x\n", c);
            //}
            //else
                //break;
        //}
        //else {
            //check(fread(&c, sizeof(u8), 1, this->fp), "Reach EOF");
            //this->depes_buf    <<= 8;
            //this->depes_buf     += c;
            //this->depes_buf_len += 8;
        //}
    //}
    //c = (this->depes_buf >> 24) & 0xff;
    check(fread(&c, sizeof(u8), 1, this->fp), "Reach EOF");
    this->depes_buf_len -= 8;
    this->buf          <<= 8;
    this->buf           += c;
    this->buf_len       += 8;
    counter++;
}

void Stream::align () {
    this->read(this->buf_len % 8);
    check(this->buf_len % 8 == 0, "Stream align fail");
}

u32 Stream::read (u8 len) {
    // Read at most 32 bits from buffer

    check(len < 32, "Length more then 32!");
    this->start_code = 0xb0; // Forbiddien

    while (this->buf_len < len)
        this->read_buf();

    u32 ret = (this->buf >> (this->buf_len - len)) & ((1 << len) - 1);
    this->buf     &= ((1 << (this->buf_len - len)) - 1);
    this->buf_len -= len;
    return ret;
}

u8 Stream::read_u8 () {
    // Read u8 from buffer

    return this->read(8);
}

u16 Stream::read_u16 () {
    // Read u16 from buffer

    return this->read(16);
}

void Stream::put (u32 content, u8 len) {
    // Put some bits back into buffer

    this->buf     += content << this->buf_len;
    this->buf_len += len;
    check(this->buf_len < 64, "Stream buffer overflow");
}

u8 Stream::next_bits_eq (u32 val, u8 len) {
    // Return whether next 32 bits equal val

    check(len < 32, "Length more then 32!");
    while (this->buf_len < len)
        this->read_buf();

    return ((this->buf >> (this->buf_len - len)) ^ val) == 0;
}

u8 Stream::next_start_code () {
    u8 status = 0;
    u8 tmp;
    u8 tmp2;

    this->align();
    while (true) {
        tmp = this->read_u8();
        if (status == 3) {
            if (tmp == SCODE_EXT)
                tmp2 = this->read(4);
            this->start_code = tmp;
            this->ext_code   = tmp2;
            return tmp;
        }
        else if (tmp == 0)
            status += (status == 2)? 0: 1;
        else if (tmp == 1 && status == 2)
            status = 3;
        else 
            status = 0;
    }
}

u8 Stream::now_start_code () {
    return this->start_code;
}

u8 Stream::now_ext_code () {
    return this->ext_code;
}
//=======================================================

class H_data{
    public:
        i16 type;
        i16 run;
        i16 level;
        H_data();
};

H_data::H_data () {
    this->type  = 0;
    this->run   = 0;
    this->level = 0;
}

class H_node {
    public:
        void*   pos;
        void*   neg;
        H_node();
        bool    is_terminal ();
        H_data* terminal_data ();
        void    to_terminal ();
        void    triverse (u32 prefix);
};

H_node::H_node () {
    this->pos = NULL;
    this->neg = NULL;
}

bool H_node::is_terminal () {
    return (this == this->pos);
}

H_data* H_node::terminal_data () {
    check (this->is_terminal(), "Not a terminal node!");
    return (H_data*)this->neg;
}

void H_node::to_terminal () {
    check (this->pos == NULL, "Can't convert to a terminal node!");
    check (this->neg == NULL, "Can't convert to a terminal node!");
    this->pos = this;
    this->neg = new H_data();
}

void H_node::triverse (u32 prefix) {
    if (this->is_terminal())
        printf("%04x\t%d\t%d\t%d\n", prefix,    \
                this->terminal_data()->type,  \
                this->terminal_data()->run,   \
                this->terminal_data()->level);
    else {
        prefix <<= 1;
        if (this->neg)
            ((H_node*)this->neg)->triverse(prefix);
        else
            printf("%04x NEG-HAIYAA\n", prefix);
        prefix++;
        if (this->pos)
            ((H_node*)this->pos)->triverse(prefix);
        else
            printf("%04x POS-HAIYAA\n", prefix);
    }
}

class Huffman {
    public:
        Huffman (char* str, bool is_B14);
        void triverse ();
        void get (Stream* vc);
        void set_is_DC();
        u8  type;
        u8  run;
        i16 level;
    private:
        H_node* root;
        bool is_B14;
        bool is_DC;
};

Huffman::Huffman (char* str, bool is_B14) {
    this->is_B14 = is_B14;
    this->is_DC  = false;
    this->root = new H_node();
    H_node* node_now = this->root;
    i8 sign;
    for (; *str; ++str)
        switch (*str) {
            case ' ':
                break;
            case '0':
                if (!(node_now->neg))
                    node_now->neg = new H_node();
                node_now = (H_node*)node_now->neg;
                break;
            case '1':
                if (!(node_now->pos))
                    node_now->pos = new H_node();
                node_now = (H_node*)node_now->pos;
                break;
            case '|':
                node_now->to_terminal();
                ++str;
                node_now->terminal_data()->type = (*str) - '0';
                ++str;
                ++str;
                // Run, may be negative
                sign = 1;
                for (; *str != '|'; ++str) 
                    if (*str == '-')
                        sign = -1;
                    else if (*str != ' ') {
                        node_now->terminal_data()->run *= 10;
                        node_now->terminal_data()->run += (*str) - '0';
                    }
                node_now->terminal_data()->run *= sign;
                ++str;
                // Level
                for (; *str != '|'; ++str)
                    if (*str != ' ') {
                        node_now->terminal_data()->level *= 10;
                        node_now->terminal_data()->level += (*str) - '0';
                    }
                node_now = this->root;
                break;
            default:
                check(0, "Wrong Huffman construct string");
    }
}

void Huffman::get (Stream* vs) {
    H_node* node_now = this->root;
    u8 tmp;
    while (1) {
        if (node_now->is_terminal()) {
            this->type  = node_now->terminal_data()->type;
            this->run   = node_now->terminal_data()->run;
            this->level = node_now->terminal_data()->level;
            switch (this->type) {
                case HUFFMAN_CODE_NORMAL:
                    if (vs->read(1) == 1)
                        this->level = -this->level;
                    break;
                case HUFFMAN_CODE_ESCAPE:
                    this->run   = vs->read(6);
                    this->level = vs->read(12);
                    if (this->level & (1 << 11))
                        this->level |= 0xf000;
                    break;
                case HUFFMAN_CODE_INTER_DC:
                    if (this->run != 0) {
                        this->level = vs->read(this->run);
                        if (this->level < (1 << (this->run - 1)))
                            this->level = (this->level + 1) - (1 << this->run);
                    }
                    break;
            }
            this->is_DC  = false;
            return;
        }
        tmp = vs->read(1);
        if (tmp) {
            check((H_node*)node_now->pos, "Wrong Huffman code");
            if (this->is_B14 && this->is_DC && node_now == this->root) {
                check((H_node*)((H_node*)node_now->pos)->pos, "Wrong Huffman code");
                node_now = (H_node*)((H_node*)node_now->pos)->pos;
            }
            else {
                node_now = (H_node*)node_now->pos;
            }
        }
        else {
            check((H_node*)node_now->neg, "Wrong Huffman code");
            node_now = (H_node*)node_now->neg;
        }
    }
}

void Huffman::set_is_DC () {
    // Only work next get()
    this->is_DC = true;
}

void Huffman::triverse () {
    printf("Code\tType\tRun\tLevel\n");
    this->root->triverse(0);
}

Huffman B1(B1_str, false);
Huffman B2(B2_str, false);
Huffman B3(B3_str, false);
Huffman B4(B4_str, false);
Huffman B9(B9_str, false);
Huffman B10(B10_str, false);
Huffman B11(B11_str, false);
Huffman B12(B12_str, false);
Huffman B13(B13_str, false);
Huffman B14(B14_str, true);
Huffman B15(B15_str, false);
//=====================================================

class Seq_header {
    public:
        Seq_header (Stream* vs);
        void read_ext (Stream* vs);
        void read_dsp (Stream* vs);
        void print ();
        u16 horz_size;
        u16 vert_size;
        u16 aspect_ratio;
        u16 frame_rate_code;
        u32 bit_rate;
        u16 vbv_buf_size;
        u8  intra_q[64];
        u8  inter_q[64];
        // Ext
        u8  profil_level_id;
        u8  progressive_seq;
        u8  chroma_format;
        u8  low_delay;
        // Ext Display
        u8  video_format;
        u8  colour_discription;
        u8  colour_primaries;
        u8  transfer_characteristics;
        u8  matrix_coefficients;
        u8  display_horz_size;
        u8  display_vert_size;
};

Seq_header::Seq_header (Stream* vs) {
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
}

void Seq_header::read_ext (Stream* vs) {
    check(vs->now_start_code() == SCODE_EXT, "Not on sequence start header");
    check(vs->now_ext_code() == ECODE_SEQ_EXT, "Not on ext sequence ext code");
    this->profil_level_id = vs->read(8);
    this->progressive_seq = vs->read(1);
    this->chroma_format   = vs->read(2);
    this->horz_size      += vs->read(2) << 12;
    this->vert_size      += vs->read(2) << 12;
    this->bit_rate       += vs->read(12) << 18;
    /* marker bit */        vs->read(1);
    this->vbv_buf_size   += vs->read(8) << 8;
    this->low_delay       = vs->read(1);
    /* frame rate ext n*/   vs->read(2);
    /* frame rate ext d*/   vs->read(5);
    /* Both 0 in main profile*/
    check(this->chroma_format == 1, "Not chroma format 420");
}

void Seq_header::read_dsp (Stream* vs) {
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

void Seq_header::print () {
    printf("=================================================\n");
    printf("Horizontal size  : %d\n", this->horz_size);
    printf("Vertical size    : %d\n", this->vert_size);
    printf("Aspect ratio     : %d\n", this->aspect_ratio);
    printf("Frame rate code  : %d\n", this->frame_rate_code);
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
//=====================================================

class Gop_header {
    public:
        Gop_header (Stream* vs);
        void print ();
        u8 drop_frame_flag;
        u8 hour;
        u8 min;
        u8 sec;
        u8 pic;
        u8 closed_gop;
        u8 broken_link;
};

Gop_header::Gop_header (Stream* vs) {
    this->drop_frame_flag = vs->read(1);
    this->hour            = vs->read(5);
    this->min             = vs->read(6);
    /* marker bit */        vs->read(1);
    this->sec             = vs->read(6);
    this->pic             = vs->read(6);
    this->closed_gop      = vs->read(1);
    this->broken_link     = vs->read(1);
}

void Gop_header::print () {
    printf("=================================================\n");
    printf("Drop Frame Flag : %d\n", this->drop_frame_flag);
    printf("Time            : %d:%d:%d-%d\n", this->hour, this->min, this->sec, this->pic);
    printf("Closed gop      : %d\n", this->closed_gop);
    printf("Broken link     : %d\n", this->broken_link);
    printf("=================================================\n");
}
//=====================================================
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
        Picture (Stream* vs, Seq_header* seq);
        void read_ext (Stream* vs);
        void print ();
        void read_slice (Stream* vs);
        void read_mb (Stream* vs);
        void dump (char* filename);
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

Picture::Picture (Stream* vs, Seq_header* seq) {
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
            printf(" BLOCK %d \n", i);
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
//=====================================================

int main (int argc, char* argv[]) {
    B9.triverse();
    B10.triverse();
    //B12.triverse();
    //B13.triverse();
    //B14.triverse();
    //B15.triverse();
    check(argc == 2, "Wrong parameters");
    Stream* vs = new Stream(argv[1]);
    Seq_header* seq;
    Gop_header* gop_header;
    Picture*    pic_now = NULL;
    Picture*    ref_f   = NULL;
    Picture*    ref_b   = NULL;
    char        pic_dump_name[128];

    u8 start_code;
    u8 pre_start_code;
    while (1) {
        start_code = vs->next_start_code();
        printf("Start Code: %x\n", start_code);
        switch (start_code) {
            case SCODE_SEQ:
                seq = new Seq_header(vs);
                break;
            case SCODE_USR:
                /* DO NOTHING*/
                break;
            case SCODE_EXT:
                printf("  Ext Code: %d\n", vs->now_ext_code());
                switch (pre_start_code) {
                    case SCODE_SEQ:
                        switch (vs->now_ext_code()) {
                            case ECODE_SEQ_EXT:
                                seq->read_ext(vs);
                                seq->print();
                                break;
                            case ECODE_SEQ_DSP:
                                seq->read_dsp(vs);
                                break;
                            default:
                                check(0, "Wrong ext code after sequence header");
                        }
                        break;
                    case SCODE_PIC:
                        switch (vs->now_ext_code()) {
                            case ECODE_PIC_EXT:
                                pic_now->read_ext(vs);
                                pic_now->print();
                                break;
                            default:
                                check(0, "Wrong ext code after sequence header");
                        }
                        break;
                    default:
                        check(pre_start_code > 0xb8, "Wrong pre start header");
                }
                break;
            case SCODE_GOP:
                gop_header = new Gop_header(vs);
                gop_header->print();
                break;
            case SCODE_PIC:
                if (pic_now != NULL) {
                    printf("Finish Decode Picture %d\n", pic_now->temp_ref);
                    sprintf(pic_dump_name, "pic/%d.ppm", pic_now->temp_ref);
                    pic_now->dump(pic_dump_name);
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
                break;
            case SCODE_END:
                printf("End of sequence\n");
                break;
            default:
                if (start_code > SCODE_MAX_SLICE)
                    printf("Start Code: %x\n", start_code);
                if (start_code <= SCODE_MAX_SLICE)
                    pic_now->read_slice(vs);
                //if (start_code == 0xd)
                    //exit(-1);
        }
        if (start_code != SCODE_EXT && start_code != SCODE_USR) 
            pre_start_code = start_code;
        //if (start_code == 0x01)
            //// TO FIREST SLICE
            //break;
    }
}

