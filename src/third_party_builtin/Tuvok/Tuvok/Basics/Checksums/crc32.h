#pragma once

#ifndef CRC32_H
#define CRC32_H

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
	CRC32(unsigned int dwPoly=0x04C11DB7) {
		for (unsigned int ui=0; ui<256; ui++) {
			unsigned int dwR = reflect(ui);
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
	inline unsigned int reflect(unsigned int dw) {
		for (unsigned int ui=0; ui<16; ui++) {
			unsigned int dwR=(dw&(1<<ui));			// right bit
			unsigned int dwL=(dw&(1<<(31-ui)));	// left bit
			dw^=dwR^dwL^(dwR<<(32-2*ui-1))^(dwL>>(32-2*ui-1));	// swap bits
		}
		return dw;
	}
	unsigned int m_dwTable[256];
};

#endif // CRC32_H
