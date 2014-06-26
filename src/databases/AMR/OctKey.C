#include <OctKey.h>
#include <bitset>
#include <climits>

bool
operator < (const OctKey &k0, const OctKey &k1)
{
    return k0.eb < k1.eb;
}

std::ostream &
operator << (std::ostream &os, const OctKey &obj)
{
    const char* beg = reinterpret_cast<const char*>(&obj.eb);
    const char* end = beg + sizeof(OctKey);
    // print in reverse order
    const char *ptr = end-1;
    do
    {
        os << std::bitset<CHAR_BIT>(*ptr) << ' ';
        --ptr;
    }
    while(ptr >= beg);
    return os;
}

OctKey
OctKey_new()
{
    OctKey key;
    key.fv[0] = key.fv[1] = 0;
    return key;
}

OctKey
OctKey_new(unsigned long val)
{
    OctKey key;
    key.eb = val;
    return key;
}

int
OctKey_NumLevels(const OctKey &key)
{
    unsigned long k = key.eb;
    int nlevels = 0;
    for(int it = 0; it < 20; ++it)
    {
        if((k & 7) > 0)
            nlevels = it;
        k = k >> 3;    
    }
    return nlevels;
}

int
OctKey_OctCellForLevel(const OctKey &key, int level)
{
    int retval = -1;
    if(level >= 0 && level < 20)
    {
        int shift = ((OctKey_NumLevels(key)-1) - level) * 3;
        unsigned long mask = 7 << shift;
        retval = (key.eb & mask) >> shift;
    }
    return retval;
}

static const unsigned long ROOT_KEY   = 0x4000000000000000;
static const unsigned long ROOT_MASK  = 0x7fffffffffffffff;
static const unsigned long ROOT_MASK2 = 0x3fffffffffffffff;

OctKey
OctKey_Root()
{
    return OctKey_new(ROOT_KEY | 1);
}

OctKey
OctKey_AddLevel(const OctKey &key, int cell)
{
    return OctKey_new( (((key.eb << 3) | (cell & 7)) & ROOT_MASK) | ROOT_KEY);
}

bool
OctKey_Equal(const OctKey &k1, const OctKey &k2)
{
    return k1.eb == k2.eb;
}

OctKey
OctKey_RemoveLevel(const OctKey &key)
{
    // Mask off the top 2 bits and shift 3 to remove a level. Then add back the 2nd top bit.
    return OctKey_new( ((key.eb & ROOT_MASK2) >> 3) | ROOT_KEY);
}

bool
OctKey_HasImmediateParent(const OctKey &key, const OctKey &parent)
{
    return OctKey_Equal(OctKey_RemoveLevel(key), parent);
}

bool
OctKey_Inherits(const OctKey &key, const OctKey &parent)
{
    bool retval = false;
    int kLevel = OctKey_NumLevels(key);
    int pLevel = OctKey_NumLevels(parent);
    if(kLevel > pLevel)
    {
        // See that the key and parent take the same path through the octree.
        // NOTE: probably a better way to do this using the bits if we line up the levels.
        for(int level = 0; level < pLevel; ++level)
        {
            int kCell = OctKey_OctCellForLevel(key, level);
            int pCell = OctKey_OctCellForLevel(parent, level);
            if(kCell != pCell)
                return false;
        }
        retval = true;
    }
    return retval;
}

