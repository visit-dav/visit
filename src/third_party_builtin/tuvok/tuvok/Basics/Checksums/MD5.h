#pragma once

#ifndef MD5_H
#define MD5_H

#include <vector>
#include "../StdDefines.h"

class MD5
{
public:
        MD5();
        virtual ~MD5() {};

        //RSA MD5 Implementation
        void Transform(BYTE Block[64], int& error);
        void Update(BYTE* Input, UINT32 nInputLen, int& error);
        std::vector<BYTE> Final(int& iErrorCalculate);

protected:
        inline UINT32 RotateLeft(UINT32 x, int n);
        inline void FF( UINT32& A, UINT32 B, UINT32 C, UINT32 D, UINT32 X, UINT32 S, UINT32 T);
        inline void GG( UINT32& A, UINT32 B, UINT32 C, UINT32 D, UINT32 X, UINT32 S, UINT32 T);
        inline void HH( UINT32& A, UINT32 B, UINT32 C, UINT32 D, UINT32 X, UINT32 S, UINT32 T);
        inline void II( UINT32& A, UINT32 B, UINT32 C, UINT32 D, UINT32 X, UINT32 S, UINT32 T);

        // Helpers
        inline void UINTToByte(BYTE* Output, UINT32* Input, UINT32 nLength, int& error);
        inline void ByteToUINT(UINT32* Output, BYTE* Input, UINT32 nLength, int& error);
        void MemoryMove(BYTE* from, BYTE* to, UINT32 size);

private:
        BYTE  m_lpszBuffer[64];    // InputBuffer
        UINT32 m_nCount[2] ;        // bitcount, modulo 2^64 (lsb first)
        UINT32 m_lMD5[4] ;          // MD5 sum
};

#endif // MD5_H
