/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2008 Scientific Computing and Imaging Institute,
   University of Utah.


   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/


/**
  \file    crc32.cpp
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \date    October 2008
*/

#pragma once

#ifndef CRC32_H
#define CRC32_H

#include "../../StdTuvokDefines.h"
#include "../MathTools.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef unsigned long DWORD;

/*
 * CRC32 (802.3) implementation using table lookups
 */
class CRC32 {
public:
  CRC32(UINT32 dwPoly=0x04C11DB7) {
    for (UINT32 ui=0; ui<256; ui++) {
      UINT32 dwR = reflect(ui);
      for (int i=0; i<8; i++) dwR = ((dwR&0x80000000) ? (dwR<<1) ^ dwPoly : (dwR<<1));
      m_dwTable[ui]=reflect(dwR);
    }
  }

  ~CRC32(void) {}

  inline DWORD get(const unsigned char *message, size_t stLength) const {
    DWORD dwR = 0xFFFFFFFF;
    chunk(message,stLength,dwR);
    return (dwR^0xFFFFFFFF);
  }

  inline DWORD get(const char *message, size_t stLength) const {
    return get((unsigned char*)(message),stLength);
  }

  inline void chunk(const unsigned char *message, size_t stLength, DWORD &dwR) const {
    for (size_t st=0; st<stLength; st++) dwR = (dwR >> 8) ^ m_dwTable[(dwR&0xFF) ^ message[st]];
  }

private:
  inline UINT32 reflect(UINT32 dw) {
    for (UINT32 ui=0; ui<16; ui++) {
      UINT32 dwR=(dw&(1<<ui));      // right bit
      UINT32 dwL=(dw&(1<<(31-ui)));  // left bit
      dw^=dwR^dwL^(dwR<<(32-2*ui-1))^(dwL>>(32-2*ui-1));  // swap bits
    }
    return dw;
  }
  UINT32 m_dwTable[256];
};

#endif // CRC32_H
