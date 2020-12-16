#pragma once
#include "utils.h"

class Stream {
    public:
        Stream (char* file_name);
        ~Stream ();
        u32  read (u8 len);             // Read at most 32 bits from buffer
        u8   read_u8 ();
        u16  read_u16 ();
        void put (u32 content, u8 len); // Put some bits back into buffer
        u8   next_start_code ();        // Go to next start code and return it       
        u8   now_start_code ();         // Return start code now
        u8   now_ext_code ();           // Return ext code now
        void keep_start_code ();        // Keep start code once
        bool next_bits_eq (u32 var, u8 len);    // Return whether next len bits 
                                                // equal to val
    private:
        FILE* fp;
        u32   buf;
        u8    buf_len;
        u32   counter;
        u8    start_code;
        u8    ext_code;
        bool  keep;
        void  read_buf ();
        void  align ();
};
