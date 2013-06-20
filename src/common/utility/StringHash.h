//
// Compile-time string hashing function.
//
// Example of usage:
//   printf( "Hash test: %u", StringHash("test").GetHash() );
//   printf( "Hash test: %u", StringHash("aVeryLongTestWhichStillWorks").GetHash() );
//
#ifndef STRING_HASH_H
#define STRING_HASH_H

// Try and use an inline that will force it to happen.
// icc -inline-level=1 -inline-forceinline, and with Apple's gcc you can use gcc -obey-inline.
#if defined(_MSC_VER)
    // MSVC is __forceinline.
    #define FORCE_INLINE __forceinline
#elif defined(__GNUC__)
    // gcc is an attribute.
    #define FORCE_INLINE __attribute__((always_inline))
#else
    // default to inline.
    #define FORCE_INLINE inline
#endif


// Hashing function. If the FNV is not working well, we can try Murmur Hash or City Hash
template <unsigned int N, unsigned int I>
struct FnvHash
{
    FORCE_INLINE static unsigned int Hash(const char (&str)[N])
    {
        return( (FnvHash<N, I-1>::Hash(str) ^ str[I-1]) * 16777619u );
    }
};
 
template <unsigned int N>
struct FnvHash<N, 1>
{
    FORCE_INLINE static unsigned int Hash(const char (&str)[N])
    {
        return( (2166136261u ^ str[0]) * 16777619u );
    }
};

class StringHash
{
    public:
        template <unsigned int N>
        FORCE_INLINE StringHash(const char (&str)[N])
        : m_hash(FnvHash<N, N>::Hash(str))
        {
        }

        FORCE_INLINE unsigned int GetHash() { return( m_hash ); }

    private:
        unsigned int m_hash;
};

#endif  // STRING_HASH_H

