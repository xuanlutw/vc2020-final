#pragma once
#include "utils.h"
#include "stream.h"

class H_data{
    public:
        H_data();
        i16 type;
        i16 run;
        i16 level;
};

class H_node {
    public:
        H_node();
        bool    is_terminal ();         // Return wheather it is terminal node
        H_data* terminal_data ();       // Return its terminal data
        void    conv_terminal ();       // Convert to terminal node
        void    triverse (u32 prefix);
        H_node* pos;
        H_node* neg;
};

class Huffman {
    public:
        Huffman (char* str, bool is_B14);
        i16  get (Stream* vc);  // Get one code word and return run
        void set_is_DC();       // Set next get is a DC get, if it is B14
        void triverse ();       // Triverse whole tree and print
        i16 type;
        i16 run;
        i16 level;
    private:
        H_node* root;
        bool    is_B14;
        bool    is_DC;
};

extern Huffman B1;
extern Huffman B2;
extern Huffman B3;
extern Huffman B4;
extern Huffman B9;
extern Huffman B10;
extern Huffman B11;
extern Huffman B12;
extern Huffman B13;
extern Huffman B14;
extern Huffman B15;

