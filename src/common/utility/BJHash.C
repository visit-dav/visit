// ************************************************************************* //
//                                BJHash.C                                   //
// ************************************************************************* //

#include <BJHash.h>

typedef unsigned int  ub4;
typedef unsigned char ub1;

// ****************************************************************************
//  Function: Hash 
//
//  Purpose:
//      Hash a variable length stream of bytes into a 32-bit value.
//
//      Can also be used effectively as a checksum.
//
//      The best hash table sizes are powers of 2.  There is no need to do
//      mod a prime (mod is sooo slow!).  If you need less than 32 bits,
//      use a bitmask.  For example, if you need only 10 bits, do
//        h = (h & BJHashmask(10));
//        In which case, the hash table should have hashsize(10) elements.
//      
//        If you are hashing n strings (ub1 **)k, do it like this:
//          for (i=0, h=0; i<n; ++i) h = hash( k[i], len[i], h);
//
//  Arguments:
//    k:       the key ((the unaligned variable-length array of bytes)
//    length:  the length of the key, in bytes
//    initval: can be any 4-byte value
//
//  Returns:  A 32-bit value.  Every bit of the key affects every bit of
//  the return value.  Every 1-bit and 2-bit delta achieves avalanche.
//
//  Programmer: By Bob Jenkins, 1996.  bob_jenkins@burtleburtle.net.
//
//  You may use this code any way you wish, private, educational, or
//  commercial.  It's free. However, do NOT use for cryptographic purposes.
//
//  See http://burtleburtle.net/bob/hash/evahash.html
// ****************************************************************************

#define mix(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8); \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15); \
}

ub4 BJHash::Hash(register ub1 *k, register ub4 length, register ub4 initval)
{
   register ub4 a,b,c,len;

   len = length;
   a = b = 0x9e3779b9;
   c = initval;

   while (len >= 12)
   {
      a += (k[0] +((ub4)k[1]<<8) +((ub4)k[2]<<16) +((ub4)k[3]<<24));
      b += (k[4] +((ub4)k[5]<<8) +((ub4)k[6]<<16) +((ub4)k[7]<<24));
      c += (k[8] +((ub4)k[9]<<8) +((ub4)k[10]<<16)+((ub4)k[11]<<24));
      mix(a,b,c);
      k += 12; len -= 12;
   }

   c += length;

   switch(len)
   {
      case 11: c+=((ub4)k[10]<<24);
      case 10: c+=((ub4)k[9]<<16);
      case 9 : c+=((ub4)k[8]<<8);
      case 8 : b+=((ub4)k[7]<<24);
      case 7 : b+=((ub4)k[6]<<16);
      case 6 : b+=((ub4)k[5]<<8);
      case 5 : b+=k[4];
      case 4 : a+=((ub4)k[3]<<24);
      case 3 : a+=((ub4)k[2]<<16);
      case 2 : a+=((ub4)k[1]<<8);
      case 1 : a+=k[0];
   }

   mix(a,b,c);

   return c;
}
