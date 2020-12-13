#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include "default.h"

# define check(cond, msg, ...) do {                                             \
                                   if (!(cond)) {                               \
                                       fprintf(stderr, msg"\n", ##__VA_ARGS__); \
                                       exit(-1);                                \
                                   }                                            \
                               } while(0);

class Stream {
    public:
        Stream (char* file_name);
        ~Stream ();
        u32  read (u8 len);
        u8   read_u8 ();
        u16  read_u16 ();
        void put (u32 content, u8 len);
        u8   next_start_code ();
        u8   now_start_code ();
        u8   now_ext_code ();
    private:
        FILE* fp;
        u32   buf;
        u8    buf_len;
        u32   counter;
        u8    start_code;
        u8    ext_code;
        void  read_buf();
        void  align();
};

Stream::Stream(char* file_name) {
    this->fp      = fopen(file_name, "rb");
    this->buf     = 0;
    this->buf_len = 0;
    this->counter = 0;
    check(fp, "Open file %s fail!\n", file_name);
}

Stream::~Stream() {
    fclose(this->fp);
}

void Stream::read_buf () {
    u8 c;
    check(fread(&c, sizeof(u8), 1, this->fp), "Reach EOF");
    this->buf    <<= 8;
    this->buf     += c;
    this->buf_len += 8;
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
        u8 type;
        u8 run;
        u8 level;
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

#define HUFFMAN_CODE_NORMAL   0
#define HUFFMAN_CODE_END      1
#define HUFFMAN_CODE_ESCAPE   2
#define HUFFMAN_CODE_INTER_DC 3
#define HUFFMAN_CODE_OTHER    4
class Huffman {
    public:
        Huffman (char* str);
        void triverse ();
        void get (Stream* vc, bool fB14DC);
        u8  type;
        u8  run;
        i16 level;
    private:
        H_node* root;
};

Huffman::Huffman (char* str) {
    this->root = new H_node();
    H_node* node_now = this->root;
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
                for (; *str != '|'; ++str) 
                    if (*str != ' ') {
                        node_now->terminal_data()->run *= 10;
                        node_now->terminal_data()->run += (*str) - '0';
                    }
                ++str;
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

void Huffman::get (Stream* vs, bool fB14DC) {
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
            return;
        }
        tmp = vs->read(1);
        if (tmp) {
            check((H_node*)node_now->pos, "Wrong Huffman code");
            if (fB14DC && node_now == this->root) {
                check((H_node*)((H_node*)node_now->pos)->pos, "Wrong Huffman code");
                node_now = (H_node*)((H_node*)node_now->pos)->pos;
            }
            node_now = (H_node*)node_now->pos;
        }
        else {
            check((H_node*)node_now->neg, "Wrong Huffman code");
            node_now = (H_node*)node_now->neg;
        }
    }
}

void Huffman::triverse () {
    printf("Code\tType\tRun\tLevel\n");
    this->root->triverse(0);
}
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
    //check(this->progressive_seq, "Not a progressive sequence");
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

int main (int argc, char* argv[]) {
    check(argc == 2, "Wrong parameters");
    Stream* vs = new Stream(argv[1]);
    Seq_header* seq_header;

    u8 start_code;
    u8 pre_start_code;
    while (1) {
        start_code = vs->next_start_code();
        printf("Start Code: %x\n", start_code);
        switch (start_code) {
            case SCODE_SEQ:
                seq_header = new Seq_header(vs);
                pre_start_code = start_code;
                break;
            case SCODE_USR:
                /* DO NOTHING*/
                break;
            case SCODE_EXT:
                printf("  Ext Code: %d\n", vs->now_ext_code());
                if (pre_start_code == SCODE_SEQ)
                    switch (vs->now_ext_code()) {
                        case ECODE_SEQ_EXT:
                            seq_header->read_ext(vs);
                            break;
                        case ECODE_SEQ_DSP:
                            seq_header->read_dsp(vs);
                            seq_header->print();
                            break;
                        default:
                            check(0, "Wrong ext code after sequence header");
                    }
                break;
            case SCODE_END:
                printf("End of sequence\n");
                break;
            case SCODE_PIC:
            case SCODE_GOP:
                break;
        }
        if (start_code == 0x01)
            // TO FIREST SLICE
            break;
    }
    //printf("%x\n", vs.read_u16());
    //printf("%x\n", vs.read_u16());
    //printf("%x\n", vs.read_u8());
    //printf("%x\n", vs.read_u8());
    //printf("%x\n", vs.read(4));
    //printf("%x\n", vs.read(4));
    Huffman B12(B12_str);
    Huffman B13(B13_str);
    Huffman B14(B14_str);
    Huffman B15(B15_str);
    Huffman B1(B1_str);
    Huffman B2(B2_str);
    //B12.triverse();
    //B13.triverse();
    //B14.triverse();
    //B15.triverse();
    u8 quantiser_scale_code = vs->read(5);
    if (vs->read(1)) {
        vs->read(9);
        while (vs->read(1))
            vs->read(8);
    }
    u8 mb_row = start_code - 1;
    // End OF Slice Header
    B1.get(vs, false);
    printf("MB Address Increase: %d\n", B1.run);
    // B2 for I-picture
    B2.get(vs, false);
    u8 mb_quant    = (B2.run >> 6) & 1;
    u8 mb_motion_f = (B2.run >> 5) & 1;
    u8 mb_motion_b = (B2.run >> 4) & 1;
    u8 mb_pattern  = (B2.run >> 3) & 1;
    u8 mb_intra    = (B2.run >> 2) & 1;
    u8 stwcf       = (B2.run >> 1) & 1; // This two always zero
    u8 p_stwc      = (B2.run >> 0) & 1; // in non scalable mode
    printf("MB Type: %d\n", B2.run);
    if (mb_motion_f || mb_motion_b)
        u8 motion_type = vs->read(2);
        // NO motion in I frame
    // What is frame_pred_frame_dct = =
    u8 dct_type = vs->read(1);
    printf("%d\n", dct_type);
    // Read motion
    // SKIP
    vs->read(1); //marker bit
    // Start of BLOCK1
    // 4:2:0
    for (int i = 0; i < 4; ++i) {
        B12.get(vs, false);
        printf("%d\n", B12.level);
        while (1) {
            B14.get(vs, false);
            printf("%d %d %d\n", B14.type, B14.run, B14.level);
            if (B14.type == HUFFMAN_CODE_END)
                break;
        }
    }
    for (int i = 0; i < 2; ++i) {
        B13.get(vs, false);
        printf("%d\n", B12.level);
        while (1) {
            B14.get(vs, false);
            printf("%d %d %d\n", B14.type, B14.run, B14.level);
            if (B14.type == HUFFMAN_CODE_END)
                break;
    }
    }
}

