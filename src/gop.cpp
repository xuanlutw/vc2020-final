#include <stdio.h>
#include <stdlib.h>
#include "gop.h"

Gop::Gop (Stream* vs) {
    this->drop_frame_flag = vs->read(1);
    this->hour            = vs->read(5);
    this->min             = vs->read(6);
    /* marker bit */        vs->read(1);
    this->sec             = vs->read(6);
    this->pic             = vs->read(6);
    this->closed_gop      = vs->read(1);
    this->broken_link     = vs->read(1);
}

void Gop::print () {
    printf("=================================================\n");
    printf("Drop Frame Flag : %d\n", this->drop_frame_flag);
    printf("Time            : %d:%d:%d-%d\n", \
            this->hour, this->min, this->sec, this->pic);
    printf("Closed gop      : %d\n", this->closed_gop);
    printf("Broken link     : %d\n", this->broken_link);
    printf("=================================================\n");
}
//=====================================================
