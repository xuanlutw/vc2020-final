#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cmath>
#include "utils.h"
#include "stream.h"
#include "huffman.h"
#include "seq.h"
#include "gop.h"
#include "picture.h"
#include "slice.h"
#include "mb.h"
#include "block.h"

int main (int argc, char* argv[]) {
    //B9.triverse();
    check(argc == 2, "Wrong parameters");

    Stream*  vs      = new Stream(argv[1]);
    Seq*     seq;
    Gop*     gop;
    Picture* pic_now = NULL;
    Picture* ref_f   = NULL;
    Picture* ref_b   = NULL;

    while (true) {
        vs->next_start_code();
        printf("Start Code: %x\n", vs->now_start_code());
        switch (vs->now_start_code()) {
            case SCODE_SEQ:
                seq = new Seq(vs);
                seq->print();
                break;
            case SCODE_USR:
                /* DO NOTHING*/
                break;
            case SCODE_GOP:
                gop = new Gop(vs);
                gop->print();
                break;
            case SCODE_PIC:
                if (pic_now != NULL) {
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
                pic_now->print();
                pic_now->decode(vs);
                break;
            case SCODE_END:
                printf("End of sequence\n");
                break;
            default:
                printf("Unknow start Code: %x\n", vs->now_start_code());
        }
    }
}

