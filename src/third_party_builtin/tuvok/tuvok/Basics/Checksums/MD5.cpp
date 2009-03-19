#include "MD5.h"
#include <memory.h>

#define MD5_INIT_STATE_0 0x67452301
#define MD5_INIT_STATE_1 0xefcdab89
#define MD5_INIT_STATE_2 0x98badcfe
#define MD5_INIT_STATE_3 0x10325476
 
#define MD5_S11  7
#define MD5_S12 12
#define MD5_S13 17
#define MD5_S14 22
#define MD5_S21  5
#define MD5_S22  9
#define MD5_S23 14
#define MD5_S24 20
#define MD5_S31  4
#define MD5_S32 11
#define MD5_S33 16
#define MD5_S34 23
#define MD5_S41  6
#define MD5_S42 10
#define MD5_S43 15
#define MD5_S44 21
 
//Transformation constants - Round 1
#define MD5_T01  0xd76aa478 //Transformation constant 1 
#define MD5_T02  0xe8c7b756 //Transformation constant 2
#define MD5_T03  0x242070db //Transformation constant 3
#define MD5_T04  0xc1bdceee //Transformation constant 4
#define MD5_T05  0xf57c0faf //Transformation constant 5
#define MD5_T06  0x4787c62a //Transformation constant 6
#define MD5_T07  0xa8304613 //Transformation constant 7
#define MD5_T08  0xfd469501 //Transformation constant 8
#define MD5_T09  0x698098d8 //Transformation constant 9
#define MD5_T10  0x8b44f7af //Transformation constant 10
#define MD5_T11  0xffff5bb1 //Transformation constant 11
#define MD5_T12  0x895cd7be //Transformation constant 12
#define MD5_T13  0x6b901122 //Transformation constant 13
#define MD5_T14  0xfd987193 //Transformation constant 14
#define MD5_T15  0xa679438e //Transformation constant 15
#define MD5_T16  0x49b40821 //Transformation constant 16
 
//Transformation constants - Round 2
#define MD5_T17  0xf61e2562 //Transformation constant 17
#define MD5_T18  0xc040b340 //Transformation constant 18
#define MD5_T19  0x265e5a51 //Transformation constant 19
#define MD5_T20  0xe9b6c7aa //Transformation constant 20
#define MD5_T21  0xd62f105d //Transformation constant 21
#define MD5_T22  0x02441453 //Transformation constant 22
#define MD5_T23  0xd8a1e681 //Transformation constant 23
#define MD5_T24  0xe7d3fbc8 //Transformation constant 24
#define MD5_T25  0x21e1cde6 //Transformation constant 25
#define MD5_T26  0xc33707d6 //Transformation constant 26
#define MD5_T27  0xf4d50d87 //Transformation constant 27
#define MD5_T28  0x455a14ed //Transformation constant 28
#define MD5_T29  0xa9e3e905 //Transformation constant 29
#define MD5_T30  0xfcefa3f8 //Transformation constant 30
#define MD5_T31  0x676f02d9 //Transformation constant 31
#define MD5_T32  0x8d2a4c8a //Transformation constant 32
 
//Transformation constants - Round 3
#define MD5_T33  0xfffa3942 //Transformation constant 33
#define MD5_T34  0x8771f681 //Transformation constant 34
#define MD5_T35  0x6d9d6122 //Transformation constant 35
#define MD5_T36  0xfde5380c //Transformation constant 36
#define MD5_T37  0xa4beea44 //Transformation constant 37
#define MD5_T38  0x4bdecfa9 //Transformation constant 38
#define MD5_T39  0xf6bb4b60 //Transformation constant 39
#define MD5_T40  0xbebfbc70 //Transformation constant 40
#define MD5_T41  0x289b7ec6 //Transformation constant 41
#define MD5_T42  0xeaa127fa //Transformation constant 42
#define MD5_T43  0xd4ef3085 //Transformation constant 43
#define MD5_T44  0x04881d05 //Transformation constant 44
#define MD5_T45  0xd9d4d039 //Transformation constant 45
#define MD5_T46  0xe6db99e5 //Transformation constant 46
#define MD5_T47  0x1fa27cf8 //Transformation constant 47
#define MD5_T48  0xc4ac5665 //Transformation constant 48
 
//Transformation constants - Round 4
#define MD5_T49  0xf4292244 //Transformation constant 49
#define MD5_T50  0x432aff97 //Transformation constant 50
#define MD5_T51  0xab9423a7 //Transformation constant 51
#define MD5_T52  0xfc93a039 //Transformation constant 52
#define MD5_T53  0x655b59c3 //Transformation constant 53
#define MD5_T54  0x8f0ccc92 //Transformation constant 54
#define MD5_T55  0xffeff47d //Transformation constant 55
#define MD5_T56  0x85845dd1 //Transformation constant 56
#define MD5_T57  0x6fa87e4f //Transformation constant 57
#define MD5_T58  0xfe2ce6e0 //Transformation constant 58
#define MD5_T59  0xa3014314 //Transformation constant 59
#define MD5_T60  0x4e0811a1 //Transformation constant 60
#define MD5_T61  0xf7537e82 //Transformation constant 61
#define MD5_T62  0xbd3af235 //Transformation constant 62
#define MD5_T63  0x2ad7d2bb //Transformation constant 63
#define MD5_T64  0xeb86d391 //Transformation constant 64
 
 
static BYTE PADDING[64] = {
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
 
using namespace std;


UINT32 MD5::RotateLeft(UINT32 x, int n) { return (x << n) | (x >> (32-n));}
 
void MD5::FF( UINT32& A, UINT32 B, UINT32 C, UINT32 D, UINT32 X, UINT32 S, UINT32 T)
{
        UINT32 F = (B & C) | (~B & D);
        A += F + X + T;
        A = RotateLeft(A, S);
        A += B;
}
 
void MD5::GG( UINT32& A, UINT32 B, UINT32 C, UINT32 D, UINT32 X, UINT32 S, UINT32 T)
{
        UINT32 G = (B & D) | (C & ~D);
        A += G + X + T;
        A = RotateLeft(A, S);
        A += B;
}
 
void MD5::HH( UINT32& A, UINT32 B, UINT32 C, UINT32 D, UINT32 X, UINT32 S, UINT32 T)
{
        UINT32 H = (B ^ C ^ D);
        A += H + X + T;
        A = RotateLeft(A, S);
        A += B;
}
 
void MD5::II( UINT32& A, UINT32 B, UINT32 C, UINT32 D, UINT32 X, UINT32 S, UINT32 T)
{
        UINT32 I = (C ^ (B | ~D));
        A += I + X + T;
        A = RotateLeft(A, S);
        A += B;
}
 
void MD5::ByteToUINT(UINT32* Output, BYTE* Input, UINT32 nLength, int& error)
{
        //entry invariants
        if( nLength % 4 != 0 ) error = -23;
 
        //Initialisierung
        UINT32 i=0;
        UINT32 j=0;
 
        for ( ; j < nLength; i++, j += 4)
        {
                Output[i] = (UINT32)Input[j]               | 
                            (UINT32)Input[j+1] << 8  | 
                            (UINT32)Input[j+2] << 16 | 
                            (UINT32)Input[j+3] << 24;
        }
}
 
void MD5::Transform(BYTE Block[64], int& error)
{
        UINT32 a = m_lMD5[0];
        UINT32 b = m_lMD5[1];
        UINT32 c = m_lMD5[2];
        UINT32 d = m_lMD5[3];
 
        UINT32 X[16];
        ByteToUINT( X, Block, 64, error);
 
        //Round 1 Transformation
        FF (a, b, c, d, X[ 0], MD5_S11, MD5_T01); 
        FF (d, a, b, c, X[ 1], MD5_S12, MD5_T02); 
        FF (c, d, a, b, X[ 2], MD5_S13, MD5_T03); 
        FF (b, c, d, a, X[ 3], MD5_S14, MD5_T04); 
        FF (a, b, c, d, X[ 4], MD5_S11, MD5_T05); 
        FF (d, a, b, c, X[ 5], MD5_S12, MD5_T06); 
        FF (c, d, a, b, X[ 6], MD5_S13, MD5_T07); 
        FF (b, c, d, a, X[ 7], MD5_S14, MD5_T08); 
        FF (a, b, c, d, X[ 8], MD5_S11, MD5_T09); 
        FF (d, a, b, c, X[ 9], MD5_S12, MD5_T10); 
        FF (c, d, a, b, X[10], MD5_S13, MD5_T11); 
        FF (b, c, d, a, X[11], MD5_S14, MD5_T12); 
        FF (a, b, c, d, X[12], MD5_S11, MD5_T13); 
        FF (d, a, b, c, X[13], MD5_S12, MD5_T14); 
        FF (c, d, a, b, X[14], MD5_S13, MD5_T15); 
        FF (b, c, d, a, X[15], MD5_S14, MD5_T16); 

        //Round 2 Transformation
        GG (a, b, c, d, X[ 1], MD5_S21, MD5_T17); 
        GG (d, a, b, c, X[ 6], MD5_S22, MD5_T18); 
        GG (c, d, a, b, X[11], MD5_S23, MD5_T19); 
        GG (b, c, d, a, X[ 0], MD5_S24, MD5_T20); 
        GG (a, b, c, d, X[ 5], MD5_S21, MD5_T21); 
        GG (d, a, b, c, X[10], MD5_S22, MD5_T22); 
        GG (c, d, a, b, X[15], MD5_S23, MD5_T23); 
        GG (b, c, d, a, X[ 4], MD5_S24, MD5_T24); 
        GG (a, b, c, d, X[ 9], MD5_S21, MD5_T25); 
        GG (d, a, b, c, X[14], MD5_S22, MD5_T26); 
        GG (c, d, a, b, X[ 3], MD5_S23, MD5_T27); 
        GG (b, c, d, a, X[ 8], MD5_S24, MD5_T28); 
        GG (a, b, c, d, X[13], MD5_S21, MD5_T29); 
        GG (d, a, b, c, X[ 2], MD5_S22, MD5_T30); 
        GG (c, d, a, b, X[ 7], MD5_S23, MD5_T31); 
        GG (b, c, d, a, X[12], MD5_S24, MD5_T32); 
 

        //Round 3 Transformation
        HH (a, b, c, d, X[ 5], MD5_S31, MD5_T33); 
        HH (d, a, b, c, X[ 8], MD5_S32, MD5_T34); 
        HH (c, d, a, b, X[11], MD5_S33, MD5_T35); 
        HH (b, c, d, a, X[14], MD5_S34, MD5_T36); 
        HH (a, b, c, d, X[ 1], MD5_S31, MD5_T37); 
        HH (d, a, b, c, X[ 4], MD5_S32, MD5_T38); 
        HH (c, d, a, b, X[ 7], MD5_S33, MD5_T39); 
        HH (b, c, d, a, X[10], MD5_S34, MD5_T40); 
        HH (a, b, c, d, X[13], MD5_S31, MD5_T41); 
        HH (d, a, b, c, X[ 0], MD5_S32, MD5_T42); 
        HH (c, d, a, b, X[ 3], MD5_S33, MD5_T43); 
        HH (b, c, d, a, X[ 6], MD5_S34, MD5_T44); 
        HH (a, b, c, d, X[ 9], MD5_S31, MD5_T45); 
        HH (d, a, b, c, X[12], MD5_S32, MD5_T46); 
        HH (c, d, a, b, X[15], MD5_S33, MD5_T47); 
        HH (b, c, d, a, X[ 2], MD5_S34, MD5_T48); 
 
        //Round 4 Transformation
        II (a, b, c, d, X[ 0], MD5_S41, MD5_T49); 
        II (d, a, b, c, X[ 7], MD5_S42, MD5_T50); 
        II (c, d, a, b, X[14], MD5_S43, MD5_T51); 
        II (b, c, d, a, X[ 5], MD5_S44, MD5_T52); 
        II (a, b, c, d, X[12], MD5_S41, MD5_T53); 
        II (d, a, b, c, X[ 3], MD5_S42, MD5_T54); 
        II (c, d, a, b, X[10], MD5_S43, MD5_T55); 
        II (b, c, d, a, X[ 1], MD5_S44, MD5_T56); 
        II (a, b, c, d, X[ 8], MD5_S41, MD5_T57); 
        II (d, a, b, c, X[15], MD5_S42, MD5_T58); 
        II (c, d, a, b, X[ 6], MD5_S43, MD5_T59); 
        II (b, c, d, a, X[13], MD5_S44, MD5_T60); 
        II (a, b, c, d, X[ 4], MD5_S41, MD5_T61); 
        II (d, a, b, c, X[11], MD5_S42, MD5_T62); 
        II (c, d, a, b, X[ 2], MD5_S43, MD5_T63); 
        II (b, c, d, a, X[ 9], MD5_S44, MD5_T64); 
 
        m_lMD5[0] += a;
        m_lMD5[1] += b;
        m_lMD5[2] += c;
        m_lMD5[3] += d;

}
 
MD5::MD5()
{
        memset( m_lpszBuffer, 0, 64 );
        m_nCount[0] = m_nCount[1] = 0;
 
        // load magic state
        m_lMD5[0] = MD5_INIT_STATE_0;
        m_lMD5[1] = MD5_INIT_STATE_1;
        m_lMD5[2] = MD5_INIT_STATE_2;
        m_lMD5[3] = MD5_INIT_STATE_3;
}
 
void MD5::UINTToByte(BYTE* Output, UINT32* Input, UINT32 nLength, int& error)
{
         //entry invariants
        if( nLength % 4 != 0 ) error = -22;
 
        UINT32 i = 0;
        UINT32 j = 0;
        for ( ; j < nLength; i++, j += 4) 
        {
                Output[j] =   (BYTE)(Input[i] & 0xff);
                Output[j+1] = (BYTE)((Input[i] >> 8) & 0xff);
                Output[j+2] = (BYTE)((Input[i] >> 16) & 0xff);
                Output[j+3] = (BYTE)((Input[i] >> 24) & 0xff);
        }
}
  
std::vector<BYTE> MD5::Final(int& error)
{
  BYTE Bits[8];
  UINTToByte( Bits, m_nCount, 8, error);

  UINT32 nIndex = (UINT32)((m_nCount[0] >> 3) & 0x3f);
  UINT32 nPadLen = (nIndex < 56) ? (56 - nIndex) : (120 - nIndex);
  Update( PADDING, nPadLen, error );

  Update( Bits, 8, error );

  const int nMD5Size = 16;
  std::vector<BYTE> lpszMD5;
  lpszMD5.resize(nMD5Size);
  UINTToByte( &lpszMD5[0], m_lMD5, nMD5Size,error );
 
  return lpszMD5;
}
 
 
void MD5::Update( BYTE* Input, UINT32 nInputLen, int& error )
{
        UINT32 nIndex = (UINT32)((m_nCount[0] >> 3) & 0x3F);
 
        if ((m_nCount[0] += nInputLen << 3) < (nInputLen << 3))
        {
                m_nCount[1]++;
        }
        m_nCount[1] += (nInputLen >> 29);
 
        UINT32 i=0;              
        UINT32 nPartLen = 64 - nIndex;
        if (nInputLen >= nPartLen)      
        {
                memmove( &m_lpszBuffer[nIndex], Input, nPartLen );
                
                Transform( m_lpszBuffer, error );
                for (i = nPartLen; i + 63 < nInputLen; i += 64) 
                {
                        Transform( &Input[i], error );
                }
                nIndex = 0;
        } 
        else 
        {
                i = 0;
        }
 
        memmove( &m_lpszBuffer[nIndex], &Input[i], nInputLen-i);

}
