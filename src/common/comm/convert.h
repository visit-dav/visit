#ifndef _CONVERT_H
#define _CONVERT_H
#include <comm_exports.h>

/* Int, Long format types. */
#define B32 0
#define B64 1
#define L32 2
#define L64 3

#define B32_B32 0
#define B32_B64 1
#define B32_L32 2
#define B32_L64 3
#define B64_B32 4
#define B64_B64 5
#define B64_L32 6
#define B64_L64 7
#define L32_B32 8
#define L32_B64 9
#define L32_L32 10
#define L32_L64 11
#define L64_B32 12
#define L64_B64 13
#define L64_L32 14
#define L64_L64 15

/* Conversion prototypes. */
int COMM_API IntConvert(int val, unsigned char *buf, int dest_format);
int COMM_API LongConvert(long val, unsigned char *buf, int dest_format);
int COMM_API FloatConvert(float val, unsigned char *buf, int dest_format);
int COMM_API DoubleConvert(double val, unsigned char *buf, int dest_format);
int COMM_API IntSourceFormat();
int COMM_API LongSourceFormat();
int COMM_API FloatSourceFormat();
int COMM_API DoubleSourceFormat();
#endif
