#ifndef QUADRATIC_HASH_H
#define QUADRATIC_HASH_H
#include <visitstream.h>

// ****************************************************************************
//  Template Class: QuadraticHash
//
//  Purpose:
//      Handles hashes using open addressing (closed hashing) with a
//      quadratic probing function.  Hash table sizes are transparently
//      forced to prime numbers for an efficient implementation, and
//      rehashing is done automatically.
//
//  Programmer: Jeremy Meredith
//  Creation:   November 28, 2000
//
//  Modifications:
//    Jeremy Meredith, Wed Sep  4 11:29:34 PDT 2002
//    Added removal function and functions to return the valid values.
//
// ****************************************************************************
template <class K, class V = K>
class QuadraticHash
{
 public:
    QuadraticHash(int size_, unsigned int (*hashfunc_)(K&));
    ~QuadraticHash();

    bool  Find(K&);
    void  Insert(const V&);
    V    &Get();
    void  Remove();

    void  InvalidateAll();

    V    *GetFirstValidValue();
    V    *GetNextValidValue();

 private:
    void Rehash();

 private:
    enum State
    {
        Invalid,
        Valid,
        Removed
    };

    struct Entry
    {
        State state;
        K     key;
        V     value;

        Entry() : state(Invalid) {};
    };
    friend struct Entry;

 private:
    unsigned int (*hashfunc)(K&);
    unsigned int   size;
    int            sizeindex;
    unsigned int   currentcell;
    K              currentKey;

    int            iteratorpos;

    unsigned int  numvalid;
    Entry         *table;
};

extern int quadraticHashPrimes[];

// ----------------------------------------------------------------------------
//                               Template Methods
// ----------------------------------------------------------------------------


// ****************************************************************************
//  Constructor:  QuadraticHash::QuadraticHash
//
//  Arguments:
//    size_      the minimum size of the hash table
//    hashfunc_  the hash function to use for the keys
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 13, 2000
//
// ****************************************************************************
template <class K, class V>
QuadraticHash<K,V>::QuadraticHash(int size_, unsigned int (*hashfunc_)(K&))
{
    hashfunc = hashfunc_;

    sizeindex = 0;
    while (quadraticHashPrimes[sizeindex] < size_ &&
           quadraticHashPrimes[sizeindex] != -1)
        sizeindex++;

    size = quadraticHashPrimes[sizeindex];
    if (size == -1)
        throw;

    table = new Entry[size];
    currentcell = -1;
    numvalid = 0;
}

// ****************************************************************************
//  Destructor:  QuadraticHash::~QuadraticHash
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 13, 2000
//
// ****************************************************************************
template <class K, class V>
QuadraticHash<K,V>::~QuadraticHash()
{
    delete[] table;
}

// ****************************************************************************
//  Method:  QuadraticHash::Find
//
//  Purpose:
//    Find the position of a key, and return true if it is in the table.
//
//  Arguments:
//    key        the key to match
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 13, 2000
//
//  Modifications:
//    Jeremy Meredith, Wed Sep  4 11:30:08 PDT 2002
//    Added support for removal of entries.
//
// ****************************************************************************
template <class K, class V>
bool QuadraticHash<K,V>::Find(K &key)
{
    unsigned int emptycell  = -1;
    unsigned int startcell  = hashfunc(key) % size;;
    unsigned int probeindex = 0;

    currentcell = startcell;
    while (  table[currentcell].state != Invalid &&
           !(table[currentcell].key   == key))
    {
        if (emptycell == -1 && table[currentcell].state == Removed)
            emptycell = currentcell;
            
        probeindex++;
        currentcell = (startcell + probeindex*probeindex)%size;
    }
  
    if (table[currentcell].state == Valid)
    {
        return true;
    }
    else
    {
        if (emptycell != -1)
            currentcell = emptycell;
        currentKey = key;
        return false;
    }
}

// ****************************************************************************
//  Method:  QuadraticHash::Remove
//
//  Purpose:
//    Remove an entry from the hash table
//
//  Notes:
//    Removal is not straightforward in a closed hash.  A "Removed" sentinel
//    state is used.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September  3, 2002
//
// ****************************************************************************
template <class K, class V>
void QuadraticHash<K,V>::Remove()
{
    // we should use this instead (and remove the argument to the function:
    if (table[currentcell].state != Valid)
        throw;
    table[currentcell].state = Removed;
    numvalid--;
}

// ****************************************************************************
//  Method:  QuadraticHash::Get
//
//  Purpose:
//    Return the value at the position already found by Find(key)
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 13, 2000
//
// ****************************************************************************
template <class K, class V>
V &QuadraticHash<K,V>::Get()
{
    if (table[currentcell].state != Valid)
        throw;

    return table[currentcell].value;
}

// ****************************************************************************
//  Method:  QuadraticHash::Insert
//
//  Purpose:
//    Insert a value at the position already found by Find(key)
//
//  Arguments:
//    value      the value to insert
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 13, 2000
//
//  Modifications:
//    Jeremy Meredith, Wed Sep  4 11:30:08 PDT 2002
//    Added support for removal of entries.
//
// ****************************************************************************
template <class K, class V>
void QuadraticHash<K,V>::Insert(const V &value)
{
    if (table[currentcell].state == Valid)
        throw;

    table[currentcell].state = Valid;
    table[currentcell].key   = currentKey;
    table[currentcell].value = value;
    numvalid++;

    if (numvalid+1 > size/2)
        Rehash();
}

// ****************************************************************************
//  Method:  QuadraticHash::Rehash
//
//  Purpose:
//    increase the size by one factor of X and rehash the table 
//    -- this is private and is automatically called when needed
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 13, 2000
//
// ****************************************************************************
template<class K, class V>
void QuadraticHash<K,V>::Rehash()
{
    int    oldsize      = size;
    Entry *oldtable     = table;

    sizeindex++;
    size      = quadraticHashPrimes[sizeindex];
    table     = new Entry[size];

    for (int i=0; i<oldsize; i++)
    {
        if (oldtable[i].state == Valid)
        {
            Find(oldtable[i].key);
            Insert(oldtable[i].value);
        }
    }
    delete[] oldtable;
}

// ****************************************************************************
//  Method:  QuadraticHash::InvalidateAll
//
//  Purpose:
//    Clear all entries, but leave all memory allocated.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September  3, 2002
//
// ****************************************************************************
template<class K, class V>
void QuadraticHash<K,V>::InvalidateAll()
{
    for (int i=0; i<size; i++)
    {
        table[i].state = Invalid;
    }
}

// ****************************************************************************
//  Method:  QuadraticHash::GetFirstValidValue
//
//  Purpose:
//    Iterator begin and return first value (or NULL if at end).
//
//  Programmer:  Jeremy Meredith
//  Creation:    September  4, 2002
//
// ****************************************************************************
template<class K, class V>
V* QuadraticHash<K,V>::GetFirstValidValue()
{
    iteratorpos = -1;
    return GetNextValidValue();
}

// ****************************************************************************
//  Method:  QuadraticHash::GetNextValidValue
//
//  Purpose:
//    Increment iterator and return next value (or NULL if at end).
//
//  Programmer:  Jeremy Meredith
//  Creation:    September  4, 2002
//
// ****************************************************************************
template<class K, class V>
V* QuadraticHash<K,V>::GetNextValidValue()
{
    iteratorpos++;
    while (iteratorpos < size && table[iteratorpos].state != Valid)
    {
        iteratorpos++;
    }
    if (iteratorpos < size)
    {
        return &table[iteratorpos].value;
    }
    else
    {
        return NULL;
    }
}

#endif
