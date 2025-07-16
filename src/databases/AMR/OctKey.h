#ifndef OCTKEY_H
#define OCTKEY_H
#include <iostream>
#include <cstdint>

// Modifications:
//   Kathleen Biagas, Wed Jun 11, 2025
//   Use uint64_t for eb since unsigned long is 32 bit on windows.

typedef union {
    unsigned int fv[2]; // force 64-bit?
    uint64_t eb;
} OctKey;

OctKey OctKey_new();
OctKey OctKey_new(uint64_t val);
OctKey OctKey_Root();
OctKey OctKey_Root(uint64_t iroot);
OctKey OctKey_AddLevel(const OctKey &key, int cell);
OctKey OctKey_RemoveLevel(const OctKey &key);
bool   OctKey_HasImmediateParent(const OctKey &key, const OctKey &parent);
bool   OctKey_Inherits(const OctKey &key, const OctKey &parent);
bool   OctKey_Equal(const OctKey &k1, const OctKey &k2);

int    OctKey_NumLevels(const OctKey &key);
int    OctKey_OctCellForLevel(const OctKey &key, int level);
int    OctKey_ExtractRootIndex(const OctKey &key);

void   OctKey_SetRootLen(int rl);

bool operator < (const OctKey &k0, const OctKey &k1);

std::ostream &operator << (std::ostream &os, const OctKey &obj);

#endif
