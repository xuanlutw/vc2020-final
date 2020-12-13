#include <cstdint>
// Define data type
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;

char B12_str[] = "100         |3|0 |0|\
                  00          |3|1 |0|\
                  01          |3|2 |0|\
                  101         |3|3 |0|\
                  110         |3|4 |0|\
                  1110        |3|5 |0|\
                  1111 0      |3|6 |0|\
                  1111 10     |3|7 |0|\
                  1111 110    |3|8 |0|\
                  1111 1110   |3|9 |0|\
                  1111 1111 0 |3|10|0|\
                  1111 1111 1 |3|11|0|";

char B13_str[] = "00           |3|0 |0|\
                  01           |3|1 |0|\
                  10           |3|2 |0|\
                  110          |3|3 |0|\
                  1110         |3|4 |0|\
                  1111 0       |3|5 |0|\
                  1111 10      |3|6 |0|\
                  1111 110     |3|7 |0|\
                  1111 1110    |3|8 |0|\
                  1111 1111 0  |3|9 |0|\
                  1111 1111 10 |3|10|0|\
                  1111 1111 11 |3|11|0|";

char B14_str[] = "10                  |1|0 |0 |\
                  11                  |0|0 |1 |\
                  011                 |0|1 |1 |\
                  0100                |0|0 |2 |\
                  0101                |0|2 |1 |\
                  0010 1              |0|0 |3 |\
                  0011 1              |0|3 |1 |\
                  0011 0              |0|4 |1 |\
                  0001 10             |0|1 |2 |\
                  0001 11             |0|5 |1 |\
                  0001 01             |0|6 |1 |\
                  0001 00             |0|7 |1 |\
                  0000 110            |0|0 |4 |\
                  0000 100            |0|2 |2 |\
                  0000 111            |0|8 |1 |\
                  0000 101            |0|9 |1 |\
                  0000 01             |2|0 |0 |\
                  0010 0110           |0|0 |5 |\
                  0010 0001           |0|0 |6 |\
                  0010 0101           |0|1 |3 |\
                  0010 0100           |0|3 |2 |\
                  0010 0111           |0|10|1 |\
                  0010 0011           |0|11|1 |\
                  0010 0010           |0|12|1 |\
                  0010 0000           |0|13|1 |\
                  0000 0010 10        |0|0 |7 |\
                  0000 0011 00        |0|1 |4 |\
                  0000 0010 11        |0|2 |3 |\
                  0000 0011 11        |0|4 |2 |\
                  0000 0010 01        |0|5 |2 |\
                  0000 0011 10        |0|14|1 |\
                  0000 0011 01        |0|15|1 |\
                  0000 0010 00        |0|16|1 |\
                  0000 0001 1101      |0|0 |8 |\
                  0000 0001 1000      |0|0 |9 |\
                  0000 0001 0011      |0|0 |10|\
                  0000 0001 0000      |0|0 |11|\
                  0000 0001 1011      |0|1 |5 |\
                  0000 0001 0100      |0|2 |4 |\
                  0000 0001 1100      |0|3 |3 |\
                  0000 0001 0010      |0|4 |3 |\
                  0000 0001 1110      |0|6 |2 |\
                  0000 0001 0101      |0|7 |2 |\
                  0000 0001 0001      |0|8 |2 |\
                  0000 0001 1111      |0|17|1 |\
                  0000 0001 1010      |0|18|1 |\
                  0000 0001 1001      |0|19|1 |\
                  0000 0001 0111      |0|20|1 |\
                  0000 0001 0110      |0|21|1 |\
                  0000 0000 1101 0    |0|0 |12|\
                  0000 0000 1100 1    |0|0 |13|\
                  0000 0000 1100 0    |0|0 |14|\
                  0000 0000 1011 1    |0|0 |15|\
                  0000 0000 1011 0    |0|1 |6 |\
                  0000 0000 1010 1    |0|1 |7 |\
                  0000 0000 1010 0    |0|2 |5 |\
                  0000 0000 1001 1    |0|3 |4 |\
                  0000 0000 1001 0    |0|5 |3 |\
                  0000 0000 1000 1    |0|9 |2 |\
                  0000 0000 1000 0    |0|10|2 |\
                  0000 0000 1111 1    |0|22|1 |\
                  0000 0000 1111 0    |0|23|1 |\
                  0000 0000 1110 1    |0|24|1 |\
                  0000 0000 1110 0    |0|25|1 |\
                  0000 0000 1101 1    |0|26|1 |\
                  0000 0000 0111 11   |0|0 |16|\
                  0000 0000 0111 10   |0|0 |17|\
                  0000 0000 0111 01   |0|0 |18|\
                  0000 0000 0111 00   |0|0 |19|\
                  0000 0000 0110 11   |0|0 |20|\
                  0000 0000 0110 10   |0|0 |21|\
                  0000 0000 0110 01   |0|0 |22|\
                  0000 0000 0110 00   |0|0 |23|\
                  0000 0000 0101 11   |0|0 |24|\
                  0000 0000 0101 10   |0|0 |25|\
                  0000 0000 0101 01   |0|0 |26|\
                  0000 0000 0101 00   |0|0 |27|\
                  0000 0000 0100 11   |0|0 |28|\
                  0000 0000 0100 10   |0|0 |29|\
                  0000 0000 0100 01   |0|0 |30|\
                  0000 0000 0100 00   |0|0 |31|\
                  0000 0000 0011 000  |0|0 |32|\
                  0000 0000 0010 111  |0|0 |33|\
                  0000 0000 0010 110  |0|0 |34|\
                  0000 0000 0010 101  |0|0 |35|\
                  0000 0000 0010 100  |0|0 |36|\
                  0000 0000 0010 011  |0|0 |37|\
                  0000 0000 0010 010  |0|0 |38|\
                  0000 0000 0010 001  |0|0 |39|\
                  0000 0000 0010 000  |0|0 |40|\
                  0000 0000 0011 111  |0|1 |8 |\
                  0000 0000 0011 110  |0|1 |9 |\
                  0000 0000 0011 101  |0|1 |10|\
                  0000 0000 0011 100  |0|1 |11|\
                  0000 0000 0011 011  |0|1 |12|\
                  0000 0000 0011 010  |0|1 |13|\
                  0000 0000 0011 001  |0|1 |14|\
                  0000 0000 0001 0011 |0|1 |15|\
                  0000 0000 0001 0010 |0|1 |16|\
                  0000 0000 0001 0001 |0|1 |17|\
                  0000 0000 0001 0000 |0|1 |18|\
                  0000 0000 0001 0100 |0|6 |3 |\
                  0000 0000 0001 1010 |0|11|2 |\
                  0000 0000 0001 1001 |0|12|2 |\
                  0000 0000 0001 1000 |0|13|2 |\
                  0000 0000 0001 0111 |0|14|2 |\
                  0000 0000 0001 0110 |0|15|2 |\
                  0000 0000 0001 0101 |0|16|2 |\
                  0000 0000 0001 1111 |0|27|1 |\
                  0000 0000 0001 1110 |0|28|1 |\
                  0000 0000 0001 1101 |0|29|1 |\
                  0000 0000 0001 1100 |0|30|1 |\
                  0000 0000 0001 1011 |0|31|1 |";

char B15_str[] = "0110                |1|0 |0 |\
                  10                  |0|0 |1 |\
                  010                 |0|1 |1 |\
                  110                 |0|0 |2 |\
                  0010 1              |0|2 |1 |\
                  0111                |0|0 |3 |\
                  0011 1              |0|3 |1 |\
                  0001 10             |0|4 |1 |\
                  0011 0              |0|1 |2 |\
                  0001 11             |0|5 |1 |\
                  0000 110            |0|6 |1 |\
                  0000 100            |0|7 |1 |\
                  1110 0              |0|0 |4 |\
                  0000 111            |0|2 |2 |\
                  0000 101            |0|8 |1 |\
                  1111 000            |0|9 |1 |\
                  0000 01             |2|0 |0 |\
                  1110 1              |0|0 |5 |\
                  0001 01             |0|0 |6 |\
                  1111 001            |0|1 |3 |\
                  0010 0110           |0|3 |2 |\
                  1111 010            |0|10|1 |\
                  0010 0001           |0|11|1 |\
                  0010 0101           |0|12|1 |\
                  0010 0100           |0|13|1 |\
                  0001 00             |0|0 |7 |\
                  0010 0111           |0|1 |4 |\
                  1111 1100           |0|2 |3 |\
                  1111 1101           |0|4 |2 |\
                  0000 0010 0         |0|5 |2 |\
                  0000 0010 1         |0|14|1 |\
                  0000 0011 1         |0|15|1 |\
                  0000 0011 01        |0|16|1 |\
                  1111 011            |0|0 |8 |\
                  1111 100            |0|0 |9 |\
                  0010 0011           |0|0 |10|\
                  0010 0010           |0|0 |11|\
                  0010 0000           |0|1 |5 |\
                  0000 0011 00        |0|2 |4 |\
                  0000 0001 1100      |0|3 |3 |\
                  0000 0001 0010      |0|4 |3 |\
                  0000 0001 1110      |0|6 |2 |\
                  0000 0001 0101      |0|7 |2 |\
                  0000 0001 0001      |0|8 |2 |\
                  0000 0001 1111      |0|17|1 |\
                  0000 0001 1010      |0|18|1 |\
                  0000 0001 1001      |0|19|1 |\
                  0000 0001 0111      |0|20|1 |\
                  0000 0001 0110      |0|21|1 |\
                  1111 1010           |0|0 |12|\
                  1111 1011           |0|0 |13|\
                  1111 1110           |0|0 |14|\
                  1111 1111           |0|0 |15|\
                  0000 0000 1011 0    |0|1 |6 |\
                  0000 0000 1010 1    |0|1 |7 |\
                  0000 0000 1010 0    |0|2 |5 |\
                  0000 0000 1001 1    |0|3 |4 |\
                  0000 0000 1001 0    |0|5 |3 |\
                  0000 0000 1000 1    |0|9 |2 |\
                  0000 0000 1000 0    |0|10|2 |\
                  0000 0000 1111 1    |0|22|1 |\
                  0000 0000 1111 0    |0|23|1 |\
                  0000 0000 1110 1    |0|24|1 |\
                  0000 0000 1110 0    |0|25|1 |\
                  0000 0000 1101 1    |0|26|1 |\
                  0000 0000 0111 11   |0|0 |16|\
                  0000 0000 0111 10   |0|0 |17|\
                  0000 0000 0111 01   |0|0 |18|\
                  0000 0000 0111 00   |0|0 |19|\
                  0000 0000 0110 11   |0|0 |20|\
                  0000 0000 0110 10   |0|0 |21|\
                  0000 0000 0110 01   |0|0 |22|\
                  0000 0000 0110 00   |0|0 |23|\
                  0000 0000 0101 11   |0|0 |24|\
                  0000 0000 0101 10   |0|0 |25|\
                  0000 0000 0101 01   |0|0 |26|\
                  0000 0000 0101 00   |0|0 |27|\
                  0000 0000 0100 11   |0|0 |28|\
                  0000 0000 0100 10   |0|0 |29|\
                  0000 0000 0100 01   |0|0 |30|\
                  0000 0000 0100 00   |0|0 |31|\
                  0000 0000 0011 000  |0|0 |32|\
                  0000 0000 0010 111  |0|0 |33|\
                  0000 0000 0010 110  |0|0 |34|\
                  0000 0000 0010 101  |0|0 |35|\
                  0000 0000 0010 100  |0|0 |36|\
                  0000 0000 0010 011  |0|0 |37|\
                  0000 0000 0010 010  |0|0 |38|\
                  0000 0000 0010 001  |0|0 |39|\
                  0000 0000 0010 000  |0|0 |40|\
                  0000 0000 0011 111  |0|1 |8 |\
                  0000 0000 0011 110  |0|1 |9 |\
                  0000 0000 0011 101  |0|1 |10|\
                  0000 0000 0011 100  |0|1 |11|\
                  0000 0000 0011 011  |0|1 |12|\
                  0000 0000 0011 010  |0|1 |13|\
                  0000 0000 0011 001  |0|1 |14|\
                  0000 0000 0001 0011 |0|1 |15|\
                  0000 0000 0001 0010 |0|1 |16|\
                  0000 0000 0001 0001 |0|1 |17|\
                  0000 0000 0001 0000 |0|1 |18|\
                  0000 0000 0001 0100 |0|6 |3 |\
                  0000 0000 0001 1010 |0|11|2 |\
                  0000 0000 0001 1001 |0|12|2 |\
                  0000 0000 0001 1000 |0|13|2 |\
                  0000 0000 0001 0111 |0|14|2 |\
                  0000 0000 0001 0110 |0|15|2 |\
                  0000 0000 0001 0101 |0|16|2 |\
                  0000 0000 0001 1111 |0|27|1 |\
                  0000 0000 0001 1110 |0|28|1 |\
                  0000 0000 0001 1101 |0|29|1 |\
                  0000 0000 0001 1100 |0|30|1 |\
                  0000 0000 0001 1011 |0|31|1 |";

char B1_str[] = "1             |4|1 |0|\
                 011           |4|2 |0|\
                 010           |4|3 |0|\
                 0011          |4|4 |0|\
                 0010          |4|5 |0|\
                 0001 1        |4|6 |0|\
                 0001 0        |4|7 |0|\
                 0000 111      |4|8 |0|\
                 0000 110      |4|9 |0|\
                 0000 1011     |4|10|0|\
                 0000 1010     |4|11|0|\
                 0000 1001     |4|12|0|\
                 0000 1000     |4|13|0|\
                 0000 0111     |4|14|0|\
                 0000 0110     |4|15|0|\
                 0000 0101 11  |4|16|0|\
                 0000 0101 10  |4|17|0|\
                 0000 0101 01  |4|18|0|\
                 0000 0101 00  |4|19|0|\
                 0000 0100 11  |4|20|0|\
                 0000 0100 10  |4|21|0|\
                 0000 0100 011 |4|22|0|\
                 0000 0100 010 |4|23|0|\
                 0000 0100 001 |4|24|0|\
                 0000 0100 000 |4|25|0|\
                 0000 0011 111 |4|26|0|\
                 0000 0011 110 |4|27|0|\
                 0000 0011 101 |4|28|0|\
                 0000 0011 100 |4|29|0|\
                 0000 0011 011 |4|30|0|\
                 0000 0011 010 |4|31|0|\
                 0000 0011 001 |4|32|0|\
                 0000 0011 000 |4|33|0|\
                 0000 0001 000 |4|34|0|";

char B2_str[] = "1  |4|4 |0|\
                 01 |4|64|0|";

u8 default_intra_q[64] = { 8, 16, 19, 22, 26, 27, 29, 34, \
                          16, 16, 22, 24, 27, 29, 34, 37, \
                          19, 22, 26, 27, 29, 34, 34, 38, \
                          22, 22, 26, 27, 29, 34, 37, 40, \
                          22, 26, 27, 29, 32, 35, 40, 48, \
                          26, 27, 29, 32, 35, 40, 48, 58, \
                          26, 27, 29, 34, 38, 46, 56, 69, \
                          27, 29, 35, 38, 46, 56, 69, 83};

u8 default_inter_q[64] = {16, 16, 16, 16, 16, 16, 16, 16, \
                          16, 16, 16, 16, 16, 16, 16, 16, \
                          16, 16, 16, 16, 16, 16, 16, 16, \
                          16, 16, 16, 16, 16, 16, 16, 16, \
                          16, 16, 16, 16, 16, 16, 16, 16, \
                          16, 16, 16, 16, 16, 16, 16, 16, \
                          16, 16, 16, 16, 16, 16, 16, 16, \
                          16, 16, 16, 16, 16, 16, 16, 16};

#define SCODE_PIC 0x00
#define SCODE_USR 0xb2
#define SCODE_SEQ 0xb3
#define SCODE_EXT 0xb5
#define SCODE_END 0xb7
#define SCODE_GOP 0xb8

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
