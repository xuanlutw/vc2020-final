#include <cstdio>
#include <cstdlib>
#include "stream.h"

Stream::Stream(char* file_name) {
    this->fp      = fopen(file_name, "rb");
    this->buf     = 0;
    this->buf_len = 0;
    this->counter = 0;
    this->keep    = false;
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
    ++counter;
}

void Stream::align () {
    this->read(this->buf_len % 8);
    check(this->buf_len % 8 == 0, "Stream align fail");
}

u32 Stream::read (u8 len) {
    check(len < 32, "Length more then 32!");

    while (this->buf_len < len)
        this->read_buf();

    u32 ret        = (this->buf >> (this->buf_len - len)) & ((1 << len) - 1);
    this->buf     &= ((1 << (this->buf_len - len)) - 1);
    this->buf_len -= len;
    return ret;
}

u8 Stream::read_u8 () {
    return this->read(8);
}

u16 Stream::read_u16 () {
    return this->read(16);
}

void Stream::put (u32 content, u8 len) {
    this->buf     += content << this->buf_len;
    this->buf_len += len;
    check(this->buf_len < 64, "Stream buffer overflow");
}

bool Stream::next_bits_eq (u32 val, u8 len) {
    check(len < 32, "Length more then 32!");
    while (this->buf_len < len)
        this->read_buf();

    return ((this->buf >> (this->buf_len - len)) ^ val) == 0;
}

u8 Stream::next_start_code () {
    if (this->keep) {
        this->keep = false;
        return this->start_code;
    }

    u8 status = 0;
    u8 tmp;

    this->align();
    while (status != 3) {
        switch (tmp = this->read_u8()) {
            case 0:
                status = (tmp == 0);
            case 1:
                status = (tmp == 0) << 1;
            case 2:
                status = (tmp == 0)? 2: (tmp == 1)? 3: 0;
        }
    }
    this->start_code   = this->read_u8();
    if (this->start_code == SCODE_EXT)
        this->ext_code = this->read(4);
    return this->start_code;
}

u8 Stream::now_start_code () {
    return this->start_code;
}

u8 Stream::now_ext_code () {
    return this->ext_code;
}

void Stream::keep_start_code () {
    this->keep = true;
}
