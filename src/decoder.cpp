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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

FILE* init_open_mode(Seq* seq, int argc, char* argv[]) {
    FILE* yuv_fp;
    int   pipe_fd[2];
    switch (argc) {
        case (2):
            pipe(pipe_fd);
            if (fork()) {
                close(pipe_fd[0]);
                check(yuv_fp = fdopen(pipe_fd[1], "w"), "Open pipe fail!");
            }
            else {
                close(STDIN_FILENO);
                close(STDOUT_FILENO);
                close(STDERR_FILENO);
                close(pipe_fd[1]);
                dup2(pipe_fd[0], STDIN_FILENO);
                char width[32];
                char height[32];
                sprintf(width,  "%d", seq->horz_size);
                sprintf(height, "%d", seq->vert_size);
                printf("HI\n");
                execl("/bin/vlc", "vlc", "--demux", "rawvideo", \
                        "--rawvid-fps",    "25",       \
                        "--rawvid-width",  width,   \
                        "--rawvid-height", height,  \
                        "--rawvid-chroma", "I420", "-", NULL);
                printf("HI\n");
            }
            break;
        case (3):
            check(yuv_fp = fopen(argv[2], "w"), "Open file %s fail!", argv[2]);
            break;
    }
    printf("%p\n", yuv_fp);
    return yuv_fp;
}

int main (int argc, char* argv[]) {
    //B10.triverse();
    check(argc == 2 || argc == 3, "Wrong argumants");

    Stream*  vs      = new Stream(argv[1]);
    Seq*     seq     = NULL;
    Gop*     gop;
    Picture* pic_now = NULL;
    Picture* ref_f   = NULL;
    Picture* ref_b   = NULL;
    FILE*    yuv_fp  = NULL;

    while (true) {
        vs->next_start_code();
        printf("Start Code: %x\n", vs->now_start_code());
        switch (vs->now_start_code()) {
            case SCODE_SEQ:
                seq = new Seq(vs);
                seq->print();
                if (!yuv_fp)
                    yuv_fp = init_open_mode(seq, argc, argv);
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
                            ref_f = NULL;
                            ref_b = pic_now;
                            break;
                        case PIC_TYPE_P:
                            if (ref_f) {
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
                pic_now = new Picture(vs, seq, ref_f, ref_b);
                pic_now->print();
                pic_now->decode(vs);
                //pic_now->dump();
                pic_now->write_YUV(yuv_fp);
                break;
            case SCODE_END:
                printf("End of sequence\n");
                break;
            default:
                printf("Unknow start Code: %x\n", vs->now_start_code());
        }
    }
    fclose(yuv_fp);
}
