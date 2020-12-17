#pragma once
#include <cstdint>
// Define data type
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

#define HUFFMAN_CODE_NORMAL   0
#define HUFFMAN_CODE_END      1
#define HUFFMAN_CODE_ESCAPE   2
#define HUFFMAN_CODE_INTER_DC 3
#define HUFFMAN_CODE_OTHER    4

extern char B1_str[];
extern char B2_str[];
extern char B3_str[];
extern char B4_str[];
extern char B9_str[];
extern char B10_str[];
extern char B11_str[];
extern char B12_str[];
extern char B13_str[];
extern char B14_str[];
extern char B15_str[];
extern u8 default_intra_q[64];
extern u8 default_inter_q[64];
extern u8 zz[64];
extern u8 q_scale[2][32];

#define MB_ESCAPE 34

#define SCODE_PIC 0x00
#define SCODE_USR 0xb2
#define SCODE_SEQ 0xb3
#define SCODE_EXT 0xb5
#define SCODE_END 0xb7
#define SCODE_GOP 0xb8
#define SCODE_MAX_SLICE 0xaf

#define ECODE_SEQ_EXT  0x1
#define ECODE_SEQ_DSP  0x2
#define ECODE_SEQ_SCA  0x5 // Should NOT be used
#define ECODE_PIC_Q    0x3
#define ECODE_PIC_DSP  0x7
#define ECODE_PIC_EXT  0x8
#define ECODE_PIC_SSCA 0x9 // Should NOT be used
#define ECODE_PIC_TSCA 0xA // Should NOT be used

#define PIC_TYPE_I 0x01
#define PIC_TYPE_P 0x02
#define PIC_TYPE_B 0x03

#define PIC_STRUCT_T 0x01
#define PIC_STRUCT_B 0x02
#define PIC_STRUCT_F 0x03
#define idx(i, j) (8 * (i) + (j))
#define idx2(i, j, x) ((x) * (i) + (j))
#define sign(x) (((x) > 0) - ((x) < 0))
#define abs(x) (((x)>0)?(x):(-(x)))
#define saturate_u8(x) (x) = (x) > 255? 255: ((x) < 0? 0: (x))

#define M_COS 200

#define check(cond, msg, ...) do {                                             \
                                  if (!(cond)) {                               \
                                      fprintf(stderr, msg"\n", ##__VA_ARGS__); \
                                      exit(-1);                                \
                                  }                                            \
                              } while(0);

u8 bandpass (double val);   // Bandpass filter
