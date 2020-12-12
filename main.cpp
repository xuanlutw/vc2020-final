#include <cstdio>
#include <cstdlib>
#include <cstdint>

# define check(cond, msg, ...) do {                                             \
                                   if (!(cond)) {                               \
                                       fprintf(stderr, msg"\n", ##__VA_ARGS__); \
                                       exit(-1);                                \
                                   }                                            \
                               } while(0);

// Define data type
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;

class Stream {
    public:
        Stream (char* file_name);
        ~Stream ();
        u32  read (u8 len);
        u8   read_u8 ();
        u16  read_u16 ();
        void put (u32 content, u8 len);
        u16  next_start_code ();
        u32   counter;
    private:
        FILE* fp;
        u32   buf;
        u8    buf_len;
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

u16 Stream::next_start_code () {
    u8 status = 0;
    u8 tmp;

    this->align();
    while (true) {
        tmp = this->read_u8();
        if (status == 3)
            return tmp;
        else if (tmp == 0)
            status += (status == 2)? 0: 1;
        else if (tmp == 1 && status == 2)
            status = 3;
        else 
            status = 0;
    }
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
class Huffman {
    public:
        Huffman (char* str);
        void triverse ();
        void get (Stream* vc, bool fB14DC);
    private:
        H_node* root;
        u8      type;
        u8      run;
        i16     level;
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
                ++str;
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

char B12_str[] = "100         |3|0 |0|\
                  00          |3|1 |0|\
                  01          |3|2 |0|\
                  101         |3|3 |0|\
                  110         |3|4 |0|\
                  1110        |3|5 |0|\
                  1111 0      |3|6 |0|\
                  1111 10     |3|7 |0|\
                  1111 110    |3|8 |0|\
                  1111 1110   |3|9 |0|\
                  1111 1111 0 |3|10|0|\
                  1111 1111 1 |3|11|0|";

char B13_str[] = "00           |3|0 |0|\
                  01           |3|1 |0|\
                  10           |3|2 |0|\
                  110          |3|3 |0|\
                  1110         |3|4 |0|\
                  1111 0       |3|5 |0|\
                  1111 10      |3|6 |0|\
                  1111 110     |3|7 |0|\
                  1111 1110    |3|8 |0|\
                  1111 1111 0  |3|9 |0|\
                  1111 1111 10 |3|10|0|\
                  1111 1111 11 |3|11|0|";

char B14_str[] = "10                  |1|0 |0 |\
                  11                  |0|0 |1 |\
                  011                 |0|1 |1 |\
                  0100                |0|0 |2 |\
                  0101                |0|2 |1 |\
                  0010 1              |0|0 |3 |\
                  0011 1              |0|3 |1 |\
                  0011 0              |0|4 |1 |\
                  0001 10             |0|1 |2 |\
                  0001 11             |0|5 |1 |\
                  0001 01             |0|6 |1 |\
                  0001 00             |0|7 |1 |\
                  0000 110            |0|0 |4 |\
                  0000 100            |0|2 |2 |\
                  0000 111            |0|8 |1 |\
                  0000 101            |0|9 |1 |\
                  0000 01             |2|0 |0 |\
                  0010 0110           |0|0 |5 |\
                  0010 0001           |0|0 |6 |\
                  0010 0101           |0|1 |3 |\
                  0010 0100           |0|3 |2 |\
                  0010 0111           |0|10|1 |\
                  0010 0011           |0|11|1 |\
                  0010 0010           |0|12|1 |\
                  0010 0000           |0|13|1 |\
                  0000 0010 10        |0|0 |7 |\
                  0000 0011 00        |0|1 |4 |\
                  0000 0010 11        |0|2 |3 |\
                  0000 0011 11        |0|4 |2 |\
                  0000 0010 01        |0|5 |2 |\
                  0000 0011 10        |0|14|1 |\
                  0000 0011 01        |0|15|1 |\
                  0000 0010 00        |0|16|1 |\
                  0000 0001 1101      |0|0 |8 |\
                  0000 0001 1000      |0|0 |9 |\
                  0000 0001 0011      |0|0 |10|\
                  0000 0001 0000      |0|0 |11|\
                  0000 0001 1011      |0|1 |5 |\
                  0000 0001 0100      |0|2 |4 |\
                  0000 0001 1100      |0|3 |3 |\
                  0000 0001 0010      |0|4 |3 |\
                  0000 0001 1110      |0|6 |2 |\
                  0000 0001 0101      |0|7 |2 |\
                  0000 0001 0001      |0|8 |2 |\
                  0000 0001 1111      |0|17|1 |\
                  0000 0001 1010      |0|18|1 |\
                  0000 0001 1001      |0|19|1 |\
                  0000 0001 0111      |0|20|1 |\
                  0000 0001 0110      |0|21|1 |\
                  0000 0000 1101 0    |0|0 |12|\
                  0000 0000 1100 1    |0|0 |13|\
                  0000 0000 1100 0    |0|0 |14|\
                  0000 0000 1011 1    |0|0 |15|\
                  0000 0000 1011 0    |0|1 |6 |\
                  0000 0000 1010 1    |0|1 |7 |\
                  0000 0000 1010 0    |0|2 |5 |\
                  0000 0000 1001 1    |0|3 |4 |\
                  0000 0000 1001 0    |0|5 |3 |\
                  0000 0000 1000 1    |0|9 |2 |\
                  0000 0000 1000 0    |0|10|2 |\
                  0000 0000 1111 1    |0|22|1 |\
                  0000 0000 1111 0    |0|23|1 |\
                  0000 0000 1110 1    |0|24|1 |\
                  0000 0000 1110 0    |0|25|1 |\
                  0000 0000 1101 1    |0|26|1 |\
                  0000 0000 0111 11   |0|0 |16|\
                  0000 0000 0111 10   |0|0 |17|\
                  0000 0000 0111 01   |0|0 |18|\
                  0000 0000 0111 00   |0|0 |19|\
                  0000 0000 0110 11   |0|0 |20|\
                  0000 0000 0110 10   |0|0 |21|\
                  0000 0000 0110 01   |0|0 |22|\
                  0000 0000 0110 00   |0|0 |23|\
                  0000 0000 0101 11   |0|0 |24|\
                  0000 0000 0101 10   |0|0 |25|\
                  0000 0000 0101 01   |0|0 |26|\
                  0000 0000 0101 00   |0|0 |27|\
                  0000 0000 0100 11   |0|0 |28|\
                  0000 0000 0100 10   |0|0 |29|\
                  0000 0000 0100 01   |0|0 |30|\
                  0000 0000 0100 00   |0|0 |31|\
                  0000 0000 0011 000  |0|0 |32|\
                  0000 0000 0010 111  |0|0 |33|\
                  0000 0000 0010 110  |0|0 |34|\
                  0000 0000 0010 101  |0|0 |35|\
                  0000 0000 0010 100  |0|0 |36|\
                  0000 0000 0010 011  |0|0 |37|\
                  0000 0000 0010 010  |0|0 |38|\
                  0000 0000 0010 001  |0|0 |39|\
                  0000 0000 0010 000  |0|0 |40|\
                  0000 0000 0011 111  |0|1 |8 |\
                  0000 0000 0011 110  |0|1 |9 |\
                  0000 0000 0011 101  |0|1 |10|\
                  0000 0000 0011 100  |0|1 |11|\
                  0000 0000 0011 011  |0|1 |12|\
                  0000 0000 0011 010  |0|1 |13|\
                  0000 0000 0011 001  |0|1 |14|\
                  0000 0000 0001 0011 |0|1 |15|\
                  0000 0000 0001 0010 |0|1 |16|\
                  0000 0000 0001 0001 |0|1 |17|\
                  0000 0000 0001 0000 |0|1 |18|\
                  0000 0000 0001 0100 |0|6 |3 |\
                  0000 0000 0001 1010 |0|11|2 |\
                  0000 0000 0001 1001 |0|12|2 |\
                  0000 0000 0001 1000 |0|13|2 |\
                  0000 0000 0001 0111 |0|14|2 |\
                  0000 0000 0001 0110 |0|15|2 |\
                  0000 0000 0001 0101 |0|16|2 |\
                  0000 0000 0001 1111 |0|27|1 |\
                  0000 0000 0001 1110 |0|28|1 |\
                  0000 0000 0001 1101 |0|29|1 |\
                  0000 0000 0001 1100 |0|30|1 |\
                  0000 0000 0001 1011 |0|31|1 |";

char B15_str[] = "0110                |1|0 |0 |\
                  10                  |0|0 |1 |\
                  010                 |0|1 |1 |\
                  110                 |0|0 |2 |\
                  0010 1              |0|2 |1 |\
                  0111                |0|0 |3 |\
                  0011 1              |0|3 |1 |\
                  0001 10             |0|4 |1 |\
                  0011 0              |0|1 |2 |\
                  0001 11             |0|5 |1 |\
                  0000 110            |0|6 |1 |\
                  0000 100            |0|7 |1 |\
                  1110 0              |0|0 |4 |\
                  0000 111            |0|2 |2 |\
                  0000 101            |0|8 |1 |\
                  1111 000            |0|9 |1 |\
                  0000 01             |2|0 |0 |\
                  1110 1              |0|0 |5 |\
                  0001 01             |0|0 |6 |\
                  1111 001            |0|1 |3 |\
                  0010 0110           |0|3 |2 |\
                  1111 010            |0|10|1 |\
                  0010 0001           |0|11|1 |\
                  0010 0101           |0|12|1 |\
                  0010 0100           |0|13|1 |\
                  0001 00             |0|0 |7 |\
                  0010 0111           |0|1 |4 |\
                  1111 1100           |0|2 |3 |\
                  1111 1101           |0|4 |2 |\
                  0000 0010 0         |0|5 |2 |\
                  0000 0010 1         |0|14|1 |\
                  0000 0011 1         |0|15|1 |\
                  0000 0011 01        |0|16|1 |\
                  1111 011            |0|0 |8 |\
                  1111 100            |0|0 |9 |\
                  0010 0011           |0|0 |10|\
                  0010 0010           |0|0 |11|\
                  0010 0000           |0|1 |5 |\
                  0000 0011 00        |0|2 |4 |\
                  0000 0001 1100      |0|3 |3 |\
                  0000 0001 0010      |0|4 |3 |\
                  0000 0001 1110      |0|6 |2 |\
                  0000 0001 0101      |0|7 |2 |\
                  0000 0001 0001      |0|8 |2 |\
                  0000 0001 1111      |0|17|1 |\
                  0000 0001 1010      |0|18|1 |\
                  0000 0001 1001      |0|19|1 |\
                  0000 0001 0111      |0|20|1 |\
                  0000 0001 0110      |0|21|1 |\
                  1111 1010           |0|0 |12|\
                  1111 1011           |0|0 |13|\
                  1111 1110           |0|0 |14|\
                  1111 1111           |0|0 |15|\
                  0000 0000 1011 0    |0|1 |6 |\
                  0000 0000 1010 1    |0|1 |7 |\
                  0000 0000 1010 0    |0|2 |5 |\
                  0000 0000 1001 1    |0|3 |4 |\
                  0000 0000 1001 0    |0|5 |3 |\
                  0000 0000 1000 1    |0|9 |2 |\
                  0000 0000 1000 0    |0|10|2 |\
                  0000 0000 1111 1    |0|22|1 |\
                  0000 0000 1111 0    |0|23|1 |\
                  0000 0000 1110 1    |0|24|1 |\
                  0000 0000 1110 0    |0|25|1 |\
                  0000 0000 1101 1    |0|26|1 |\
                  0000 0000 0111 11   |0|0 |16|\
                  0000 0000 0111 10   |0|0 |17|\
                  0000 0000 0111 01   |0|0 |18|\
                  0000 0000 0111 00   |0|0 |19|\
                  0000 0000 0110 11   |0|0 |20|\
                  0000 0000 0110 10   |0|0 |21|\
                  0000 0000 0110 01   |0|0 |22|\
                  0000 0000 0110 00   |0|0 |23|\
                  0000 0000 0101 11   |0|0 |24|\
                  0000 0000 0101 10   |0|0 |25|\
                  0000 0000 0101 01   |0|0 |26|\
                  0000 0000 0101 00   |0|0 |27|\
                  0000 0000 0100 11   |0|0 |28|\
                  0000 0000 0100 10   |0|0 |29|\
                  0000 0000 0100 01   |0|0 |30|\
                  0000 0000 0100 00   |0|0 |31|\
                  0000 0000 0011 000  |0|0 |32|\
                  0000 0000 0010 111  |0|0 |33|\
                  0000 0000 0010 110  |0|0 |34|\
                  0000 0000 0010 101  |0|0 |35|\
                  0000 0000 0010 100  |0|0 |36|\
                  0000 0000 0010 011  |0|0 |37|\
                  0000 0000 0010 010  |0|0 |38|\
                  0000 0000 0010 001  |0|0 |39|\
                  0000 0000 0010 000  |0|0 |40|\
                  0000 0000 0011 111  |0|1 |8 |\
                  0000 0000 0011 110  |0|1 |9 |\
                  0000 0000 0011 101  |0|1 |10|\
                  0000 0000 0011 100  |0|1 |11|\
                  0000 0000 0011 011  |0|1 |12|\
                  0000 0000 0011 010  |0|1 |13|\
                  0000 0000 0011 001  |0|1 |14|\
                  0000 0000 0001 0011 |0|1 |15|\
                  0000 0000 0001 0010 |0|1 |16|\
                  0000 0000 0001 0001 |0|1 |17|\
                  0000 0000 0001 0000 |0|1 |18|\
                  0000 0000 0001 0100 |0|6 |3 |\
                  0000 0000 0001 1010 |0|11|2 |\
                  0000 0000 0001 1001 |0|12|2 |\
                  0000 0000 0001 1000 |0|13|2 |\
                  0000 0000 0001 0111 |0|14|2 |\
                  0000 0000 0001 0110 |0|15|2 |\
                  0000 0000 0001 0101 |0|16|2 |\
                  0000 0000 0001 1111 |0|27|1 |\
                  0000 0000 0001 1110 |0|28|1 |\
                  0000 0000 0001 1101 |0|29|1 |\
                  0000 0000 0001 1100 |0|30|1 |\
                  0000 0000 0001 1011 |0|31|1 |";
//=====================================================

int main (int argc, char* argv[]) {
    check(argc == 2, "Wrong parameters");
    Stream vs(argv[1]);
    u8 start_code;
    while (1) {
        u8 start_code = vs.next_start_code();
        printf("%x\n", start_code);
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
    //B12.triverse();
    //B13.triverse();
    //B14.triverse();
    //B15.triverse();
    //u8 quantiser_scale_code = vs.read(5);
    //if (vs.read(1)) {
        //vs.read(9);
        //while (vs.read(1))
            //vs.read(8);
    //}
    //u8 mb_row = start_row - 1;
    // End OF Slice Header
}

