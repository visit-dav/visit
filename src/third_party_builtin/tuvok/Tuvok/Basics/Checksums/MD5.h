#pragma once

#ifndef MD5_H
#define MD5_H

#include <vector>

typedef unsigned char BYTE;
typedef unsigned int  UINT;

class MD5  
{
public:
        MD5();
        virtual ~MD5() {};
 
        //RSA MD5 Implementation
        void Transform(BYTE Block[64], int& error);
        void Update(BYTE* Input, UINT nInputLen, int& error);
		std::vector<unsigned char> Final(int& iErrorCalculate);
 
protected:
        inline UINT RotateLeft(UINT x, int n);
        inline void FF( UINT& A, UINT B, UINT C, UINT D, UINT X, UINT S, UINT T);
        inline void GG( UINT& A, UINT B, UINT C, UINT D, UINT X, UINT S, UINT T);
        inline void HH( UINT& A, UINT B, UINT C, UINT D, UINT X, UINT S, UINT T);
        inline void II( UINT& A, UINT B, UINT C, UINT D, UINT X, UINT S, UINT T);
 
        // Helpers
        inline void UINTToByte(BYTE* Output, UINT* Input, UINT nLength, int& error);
        inline void ByteToUINT(UINT* Output, BYTE* Input, UINT nLength, int& error);
        void MemoryMove(BYTE* from, BYTE* to, UINT size);
 
private:
        BYTE  m_lpszBuffer[64];    // InputBuffer
        UINT m_nCount[2] ;        // bitcount, modulo 2^64 (lsb first)
        UINT m_lMD5[4] ;          // MD5 sum
};

#endif // MD5_H
