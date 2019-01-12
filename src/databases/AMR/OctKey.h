#ifndef OCTKEY_H
#define OCTKEY_H
#include <iostream>

typedef union {
    unsigned int fv[2]; // force 64-bit?
    unsigned long eb;
} OctKey;

OctKey OctKey_new();
OctKey OctKey_new(unsigned long val);
OctKey OctKey_Root();
OctKey OctKey_AddLevel(const OctKey &key, int cell);
OctKey OctKey_RemoveLevel(const OctKey &key);
bool   OctKey_HasImmediateParent(const OctKey &key, const OctKey &parent);
bool   OctKey_Inherits(const OctKey &key, const OctKey &parent);
bool   OctKey_Equal(const OctKey &k1, const OctKey &k2);

int    OctKey_NumLevels(const OctKey &key);
int    OctKey_OctCellForLevel(const OctKey &key, int level);

bool operator < (const OctKey &k0, const OctKey &k1);

std::ostream &operator << (std::ostream &os, const OctKey &obj);

#endif
