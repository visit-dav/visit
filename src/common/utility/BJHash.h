// ************************************************************************* //
//                                 BJHash.h                                  //
// ************************************************************************* //

#ifndef BJHASH_H
#define BJHASH_H
#include <utility_exports.h>

namespace BJHash
{
    UTILITY_API unsigned int Hash(unsigned char *k, unsigned int length, unsigned int initval);
    unsigned int Mask(int n);
}

inline unsigned int BJHash::Mask(int n)
{
    return ((unsigned int)1<<n) - 1;
}

#endif
