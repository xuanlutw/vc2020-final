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

#define MB_ESCAPE 34

char B2_str[] = "1  |4|4 |0|\
                 01 |4|64|0|";

char B3_str[] = "1       |4|40 |0|\
                 01      |4|8  |0|\
                 001     |4|32 |0|\
                 0001 1  |4|4  |0|\
                 0001 0  |4|104|0|\
                 0000 1  |4|72 |0|\
                 0000 01 |4|68 |0|";

char B4_str[] = "10      |4|48 |0|\
                 11      |4|56 |0|\
                 010     |4|16 |0|\
                 011     |4|24 |0|\
                 0010    |4|32 |0|\
                 0011    |4|40 |0|\
                 0001 1  |4|4  |0|\
                 0001 0  |4|120|0|\
                 0000 11 |4|104|0|\
                 0000 10 |4|88 |0|\
                 0000 01 |4|68 |0|";

char B9_str[] = "111         |4|60|0|\
                 1101        |4|4 |0|\
                 1100        |4|8 |0|\
                 1011        |4|16|0|\
                 1010        |4|32|0|\
                 1001 1      |4|12|0|\
                 1001 0      |4|48|0|\
                 1000 1      |4|20|0|\
                 1000 0      |4|40|0|\
                 0111 1      |4|28|0|\
                 0111 0      |4|44|0|\
                 0110 1      |4|52|0|\
                 0110 0      |4|56|0|\
                 0101 1      |4|1 |0|\
                 0101 0      |4|61|0|\
                 0100 1      |4|2 |0|\
                 0100 0      |4|62|0|\
                 0011 11     |4|24|0|\
                 0011 10     |4|36|0|\
                 0011 01     |4|3 |0|\
                 0011 00     |4|63|0|\
                 0010 111    |4|5 |0|\
                 0010 110    |4|9 |0|\
                 0010 101    |4|17|0|\
                 0010 100    |4|33|0|\
                 0010 011    |4|6 |0|\
                 0010 010    |4|10|0|\
                 0010 001    |4|18|0|\
                 0010 000    |4|34|0|\
                 0001 1111   |4|7 |0|\
                 0001 1110   |4|11|0|\
                 0001 1101   |4|19|0|\
                 0001 1100   |4|35|0|\
                 0001 1011   |4|13|0|\
                 0001 1010   |4|49|0|\
                 0001 1001   |4|21|0|\
                 0001 1000   |4|41|0|\
                 0001 0111   |4|14|0|\
                 0001 0110   |4|50|0|\
                 0001 0101   |4|22|0|\
                 0001 0100   |4|42|0|\
                 0001 0011   |4|15|0|\
                 0001 0010   |4|51|0|\
                 0001 0001   |4|23|0|\
                 0001 0000   |4|43|0|\
                 0000 1111   |4|25|0|\
                 0000 1110   |4|37|0|\
                 0000 1101   |4|26|0|\
                 0000 1100   |4|38|0|\
                 0000 1011   |4|29|0|\
                 0000 1010   |4|45|0|\
                 0000 1001   |4|53|0|\
                 0000 1000   |4|57|0|\
                 0000 0111   |4|30|0|\
                 0000 0110   |4|46|0|\
                 0000 0101   |4|54|0|\
                 0000 0100   |4|58|0|\
                 0000 0011 1 |4|31|0|\
                 0000 0011 0 |4|47|0|\
                 0000 0010 1 |4|55|0|\
                 0000 0010 0 |4|59|0|\
                 0000 0001 1 |4|27|0|\
                 0000 0001 0 |4|39|0|";

char B10_str[] = "0000 0011 001  |4|-16|0|\
                  0000 0011 011  |4|-15|0|\
                  0000 0011 101  |4|-14|0|\
                  0000 0011 111  |4|-13|0|\
                  0000 0100 001  |4|-12|0|\
                  0000 0100 011  |4|-11|0|\
                  0000 0100 11   |4|-10|0|\
                  0000 0101 01   |4|-9 |0|\
                  0000 0101 11   |4|-8 |0|\
                  0000 0111      |4|-7 |0|\
                  0000 1001      |4|-6 |0|\
                  0000 1011      |4|-5 |0|\
                  0000 111       |4|-4 |0|\
                  0001 1         |4|-3 |0|\
                  0011           |4|-2 |0|\
                  011            |4|-1 |0|\
                  1              |4|0  |0|\
                  010            |4|1  |0|\
                  0010           |4|2  |0|\
                  0001 0         |4|3  |0|\
                  0000 110       |4|4  |0|\
                  0000 1010      |4|5  |0|\
                  0000 1000      |4|6  |0|\
                  0000 0110      |4|7  |0|\
                  0000 0101 10   |4|8  |0|\
                  0000 0101 00   |4|9  |0|\
                  0000 0100 10   |4|10 |0|\
                  0000 0100 010  |4|11 |0|\
                  0000 0100 000  |4|12 |0|\
                  0000 0011 110  |4|13 |0|\
                  0000 0011 100  |4|14 |0|\
                  0000 0011 010  |4|15 |0|\
                  0000 0011 000  |4|16 |0|";

char B11_str[] = "11 |4|-1|0|\
                  0  |4|0 |0|\
                  10 |4|1 |0|";

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

// This ONLY for frame picture
u8 zz[64] = { 0,  1,  5,  6, 14, 15, 27, 28, \
              2,  4,  7, 13, 16, 26, 29, 42, \
              3,  8, 12, 17, 25, 30, 41, 43, \
              9, 11, 18, 24, 31, 40, 44, 53, \
             10, 19, 23, 32, 39, 45, 52, 54, \
             20, 22, 33, 38, 46, 51, 55, 60, \
             21, 34, 37, 47, 50, 56, 59, 61, \
             35, 36, 48, 49, 57, 58, 62, 63 };

u8 q_scale[2][32] = {
    { 0,  2,  4,  6,  8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62},
    { 0,  1,  2,  3,  4,  5,  6,  7,  8, 10, 12, 14, 16, 18, 20, 22, 24, 28, 32, 36, 40, 44, 48, 52, 56, 64, 72, 80, 88, 96, 104, 112}};

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

#define M_COS 200

