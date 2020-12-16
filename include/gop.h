#pragma once
#include "utils.h"
#include "stream.h"

class Gop {
    public:
        Gop (Stream* vs);
        void print ();
        u8 drop_frame_flag;
        u8 hour;
        u8 min;
        u8 sec;
        u8 pic;
        u8 closed_gop;
        u8 broken_link;
};

