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
    FILE* yuv_fp = NULL;
    int   pipe_fd[2];
    char  fps[32];
    char  width[32];
    char  height[32];
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
                sprintf(fps,    "%lf", seq->frame_rate);
                sprintf(width,  "%d",  seq->horz_size);
                sprintf(height, "%d",  seq->vert_size);
                execl("/bin/vlc", "vlc", \
                        "--demux", "rawvideo", \
                        "--rawvid-fps", fps, \
                        "--rawvid-width", width, \
                        "--rawvid-height", height, \
                        "--rawvid-chroma", "I420", "-", NULL);
                check(0, "Exec vlc fail!");
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
    Gop*     gop     = NULL;
    Picture* pic     = NULL;
    Picture* ref[2]  = {NULL, NULL};    // Keep two reference frame
    FILE*    yuv_fp  = NULL;

    while (true) {
        vs->next_start_code();
        //printf("Start Code: %x\n", vs->now_start_code());
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
                if (gop)
                    delete gop;
                gop = new Gop(vs);
                gop->print();
                break;
            case SCODE_PIC:
                pic = new Picture(vs, seq, ref);
                pic->print();
                pic->decode(vs);
                //pic->dump();
                switch (pic->type) {
                    case PIC_TYPE_I:
                    case PIC_TYPE_P:
                        if (ref[0])
                            delete ref[0];
                        if (ref[1])
                            ref[1]->write_YUV(yuv_fp);
                        ref[0] = ref[1];
                        ref[1] = pic;
                        break;
                    case PIC_TYPE_B:
                        pic->write_YUV(yuv_fp);
                        break;
                }
                break;
            case SCODE_END:
                printf("End of sequence\n");
                break;
            default:
                printf("Unknow start Code: %x\n", vs->now_start_code());
                break;
        }
    }
    fclose(yuv_fp);
}
