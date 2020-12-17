#include <cstdlib>
#include <cstdio>
#include "huffman.h"

H_data::H_data () {
    this->type  = 0;
    this->run   = 0;
    this->level = 0;
}

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

void H_node::conv_terminal () {
    check (!this->pos && !this->neg, "Can't convert to a terminal node!");
    this->pos = this;
    this->neg = (H_node*)(new H_data());
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
            this->neg->triverse(prefix);
        else
            printf("%04x NEG-HAIYAA\n", prefix);
        prefix++;
        if (this->pos)
            this->pos->triverse(prefix);
        else
            printf("%04x POS-HAIYAA\n", prefix);
    }
}

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
                node_now = node_now->neg;
                break;
            case '1':
                if (!(node_now->pos))
                    node_now->pos = new H_node();
                node_now = node_now->pos;
                break;
            case '|':
                node_now->conv_terminal();
                // Type
                ++str; // char |
                node_now->terminal_data()->type = *str - '0';
                ++str; // char type
                // Run, may be negative
                ++str; // char |
                sign = 1;
                for (; *str != '|'; ++str) 
                    if (*str == '-')
                        sign = -1;
                    else if (*str != ' ') {
                        node_now->terminal_data()->run *= 10;
                        node_now->terminal_data()->run += (*str) - '0';
                    }
                node_now->terminal_data()->run *= sign;
                // Level
                ++str; // char |
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

i16 Huffman::get (Stream* vs) {
    H_node* node_now = this->root;
    while (!node_now->is_terminal()) {
        if (vs->read(1)) {
            //printf("1");
            check(node_now = node_now->pos, "Wrong Huffman code");
            if (this->is_B14 && this->is_DC && node_now == this->root->pos)
                check(node_now = node_now->pos, "Wrong Huffman code");
        }
        else {
            //printf("0");
            check(node_now = node_now->neg, "Wrong Huffman code");
        }
    }
    //printf("\n");
    this->type  = node_now->terminal_data()->type;
    this->run   = node_now->terminal_data()->run;
    this->level = node_now->terminal_data()->level;
    this->is_DC = false;
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
    return this->run;
}

void Huffman::set_is_DC () {
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
