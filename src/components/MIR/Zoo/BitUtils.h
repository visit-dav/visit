#ifndef BIT_UTILS_H
#define BIT_UTILS_H

// ----------------------------------------------------------------------------
//  Programmer: Jeremy Meredith
//  Date:       August 21, 2003
// ----------------------------------------------------------------------------

#define byteForBit(m) int((m)/8)
#define bitForBit(m)  (1<<((m)%8))
extern const unsigned char nBitsSet[256];
extern const signed   char bitsSet[256][8];
extern const signed   char bitPos[256][8];


#endif
