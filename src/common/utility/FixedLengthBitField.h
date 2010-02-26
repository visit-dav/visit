#ifndef FIXED_LENGTH_BIT_FIELD_H
#define FIXED_LENGTH_BIT_FIELD_H

//#include <iostream>

// ****************************************************************************
// Class:    FixedLengthBitField
//
// Purpose:
//   Simple template-based implementation of a fixed-length bitfield.
//   The template parameter is the number of bytes used to store the bits,
//   and thus the number of bits this supports is 8*NBYTES.
//
// Programmer:  Jeremy Meredith
// Creation:    February 26, 2010
//
// ****************************************************************************
template <int NBYTES>
class FixedLengthBitField
{
  private:
    unsigned char bytes[NBYTES];
  public:
    FixedLengthBitField()
    {
        for (int i=0; i<NBYTES; i++)
            bytes[i] = 0;
    }
    inline void SetBit(int bit)
    {
        int byte = (bit & ~0x07) >> 3;
        int lbit = bit & 0x07;
        bytes[byte] |= (1<<lbit);
    }
    inline void ClearBit(int bit)
    {
        int byte = (bit & ~0x07) >> 3;
        int lbit = bit & 0x07;
        bytes[byte] &= ~(1<<lbit);
    }
    inline bool TestBit(int bit)
    {
        int byte = (bit & ~0x07) >> 3;
        int lbit = bit & 0x07;
        return (bytes[byte] & (1<<lbit));
    }
    inline bool operator==(const FixedLengthBitField<NBYTES> &b) const
    {
        for (int i=0; i<NBYTES; i++)
            if (bytes[i] != b.bytes[i])
                return false;
        return true;
    }
    inline const FixedLengthBitField &operator=(const FixedLengthBitField<NBYTES> &b)
    {
        for (int i=0; i<NBYTES; i++)
            bytes[i] = b.bytes[i];
        return *this;
    }
#if 0
    void Print(std::ostream &out)
    {
        for (int i=NBYTES-1; i>=0; i--)
        {
            if (i==NBYTES-1)
                out << "0x";
            else
                out << "_";
            for (int j=7; j>=0; j--)
            {
                out << TestBit(i*8 + j) ? "1" : "0";
            }
        }
        out << endl;
    }
#endif
};

#endif
