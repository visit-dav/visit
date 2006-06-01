/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#ifndef MRU_CACHE_H
#define MRU_CACHE_H

#include <map>
using std::map;


// ****************************************************************************
//
//  Class: MRUCacheBase
//
//  Purpose: Provide a generic, map-like, container that also serves as a
//  most recently used cache. This class is implemented using 2 STL maps. One
//  for the cached entries and one for the 'age counters' of those entries.
//  We use two STL maps rather than a single map whose value is a pair
//  of <cached-value, age> so that we can easily return an iterator to
//  the caller to iterate over the cached entries as s/he ordinarily would.
//
//  The notion of 'most-recent' is implemented by keeping a sort-of 'clock'
//  for the cache. It is just a simple counter. Each time an operation on the
//  cache is performed that effects what is stored in the cache, the 'clock'
//  is incremented. Likewise, when entries in the cache are referenced there
//  age is updated to whatever the current 'clock' is.
//
//  All of the magic of occurs on a reference to
//  an item in the cache with the '[]' operator.  When items in the cache are
//  referenced using the '[]' operator, if the referenced item exists in the
//  cache, it is returned to the caller and it's "age" is also updated to
//  to the current 'clock' to indicate it is the most recently used item.
//  In this way, the oldest item in the cache is the one whose age is the
//  smallest. When an item referenced by the '[]' operator is not in the cache,
//  a new slot is made available in the cache for the item. If the cache is
//  full, the oldest entry in the cache is removed and its data is deleted.
//
//  The deletion of the cached data is handled by the 3rd argument to the
//  template, the MRUCache_DeleteMethod. The options are to not delete,
//  to use 'delete' to delete the data, to use 'delete []' to delete the
//  data or to use 'free()' to delete the data. Again, the particular
//  choice depends on the 3rd argument to the template which creates the
//  cache to begin with.
//
//  A cache is created using the following syntax
//
//     MRUCache< key-type, val-type, del-method, init-size> gorfo
//
//  where
//
//     key-type:   the type of key used to reference items in the cache.
//     val-type:   the type of values stored in the cache
//     del-method: how to delete the values that are cached
//     init-size:  initial number of slots in the cache
//
//  Notes: Unfortunately, there is no way to tell the difference between
//  a pointer obtained from a call such as 'new gorfo[...]' versus
//  'new gorfo'. This is important as the appropriate delete for the
//  former is 'delete []' while it is 'delete' for the latter. Consequently,
//  care must be taken to selecte the appropriate 'del-method' for the
//  val-type being used in the cache.
//
//  Programmer: Mark C. Miller 
//  Creation:   October 6, 2003 
//
// ****************************************************************************

// tags for which kind of delete to call on cache pre-emption
typedef enum {
   MRUCache_DontDelete,
   MRUCache_Delete,
   MRUCache_ArrayDelete,
   MRUCache_Free
} MRUCache_DeleteMethod;

template<class kT, class vT, MRUCache_DeleteMethod dM, size_t nS>
bool MRUCacheBase<kT,vT,dM,nS>::find(const kT& key) const

// kT is keyType, vT is valueType
template<class kT, class vT, MRUCache_DeleteMethod dM, size_t nS=20>
class MRUCacheBase {

   public:

      MRUCacheBase() : numSlots(nS) { ageCounter=0; } ;
     ~MRUCacheBase() { clear(); };

      // explicit existence test (won't change MRU history)
      bool find(const kT& key) const;

      // explicit remove of an entry from the cache
      void remove(const kT& key);

      // remove all entries from the cache
      void clear(void);

      // get reference to cached entry 
      //    a. hit moves cached entry to mru,
      //    b. miss removes lru entry, moves new entry to mru
      vT& operator[](const kT& key);

      // iterator (only allow const_iterator)
      typedef map<kT,vT>::const_iterator const_iterator;
      typedef map<kT,vT>::const_iterator iterator;
      const_iterator begin(void) const { return cache.begin(); };
      const_iterator end(void) const { return cache.end(); };

      // get most recently used entry 
      vT& mru(void) const;

      // return total memory used by all cached values
      size_t memsize(void) const { return cache.size() * sizeof(vT); };

      // return number of cache slots 
      size_t numslots(void) const { return numSlots; };

      // set number of cache slots, may result in
      // cache deletions if smaller than previous number
      size_t numslots(size_t newNumSlots);

   private:

      // returns key of the oldest entry
      kT oldest(void);

      // only method to be overridden based on item type 
      void deleteit(vT& item) {} ;

      // maximum number of slots in cache
      size_t numSlots;

      // a sort-of clock for aging entries
      size_t ageCounter;

      // cached values indexed by keys
      // (we use two maps so we can return an iterator to cache)
      map<kT,vT>  cache;
      map<kT,int> age;
};

// The MRUCache class
template<class kT, class vT, MRUCache_DeleteMethod dM, size_t nS=20>
class MRUCache : public MRUCacheBase<kT,vT,dM,nS>
{
   public:
      MRUCache() : MRUCacheBase<kT,vT,dM,nS>() {} ;
};

// DontDelete specialization
template<class kT, class vT, size_t nS>
class MRUCache<kT,vT,MRUCache_DontDelete,nS> : public MRUCacheBase<kT,vT,MRUCache_DontDelete,nS>
{
   public:
      MRUCache() : MRUCacheBase<kT,vT,MRUCache_DontDelete,nS>() {} ;
};

// Delete specialization
template<class kT, class vT, size_t nS>
class MRUCache<kT,vT,MRUCache_Delete,nS> : public MRUCacheBase<kT,vT,MRUCache_Delete,nS>
{
   public:
      MRUCache() : MRUCacheBase<kT,vT,MRUCache_Delete,nS>() {} ;
   private:
      void deleteit(vT& item) { delete item; } ;
};

// ArrayDelete specialization 
template<class kT, class vT, size_t nS>
class MRUCache<kT,vT,MRUCache_ArrayDelete,nS> : public MRUCacheBase<kT,vT,MRUCache_ArrayDelete,nS>
{
   public:
      MRUCache() : MRUCacheBase<kT,vT,MRUCache_ArrayDelete,nS>() {} ;
   private:
      void deleteit(vT& item) { delete [] item; } ;
};

// Free specialization 
template<class kT, class vT, size_t nS>
class MRUCache<kT,vT,MRUCache_Free,nS> : public MRUCacheBase<kT,vT,MRUCache_Free,nS>
{
   public:
      MRUCache() : MRUCacheBase<kT,vT,MRUCache_Free,nS>() {} ;
   private:
      void deleteit(vT& item) { free (item); } ;
};

// ****************************************************************************
//  Method: MRUCacheBase::find
//
//  Purpose: an explicit test for existence of an entry in the cache. Does not
//  change the MRU history
//
//  Arguments:
//     key: the key to search for 
//
//  Programmer: Mark C. Miller 
//  Creation:   October 6, 2003 
//
// ****************************************************************************
template<class kT, class vT, MRUCache_DeleteMethod dM, size_t nS>
bool MRUCacheBase<kT,vT,dM,nS>::find(const kT& key) const
{
   if (cache.find(key) != cache.end())
      return true;
   else
      return false;
}

// ****************************************************************************
//  Method: MRUCacheBase::remove
//
//  Purpose: an explicit removal of an entry in the cache
//
//  Arguments:
//     key: the key to the item to remove 
//
//  Programmer: Mark C. Miller 
//  Creation:   October 6, 2003 
//
// ****************************************************************************
template<class kT, class vT, MRUCache_DeleteMethod dM, size_t nS>
void MRUCacheBase<kT,vT,dM,nS>::remove(const kT& key)
{
   map<kT,vT>::iterator k = cache.find(key);
   if (k == cache.end())
      return;

   // delete the cached value
   deleteit(k->second);

   // erase slots from the cache
   cache.erase(k);
   map<kT,int>::iterator j = age.find(key);
   age.erase(j);
}

// ****************************************************************************
//  Method: MRUCacheBase::clear
//
//  Purpose: Remove all the items from the cache 
//
//  Note: we take a funny approach here to confine all work having to do with
//  deleting entries to the 'remove' method
//
//  Programmer: Mark C. Miller 
//  Creation:   October 6, 2003 
//
// ****************************************************************************
template<class kT, class vT, MRUCache_DeleteMethod dM, size_t nS>
void MRUCacheBase<kT,vT,dM,nS>::clear(void)
{
   map<kT,vT>::iterator i = cache.begin();

   while (i != cache.end())
   {
      remove(i->first);
      i = cache.begin();
   }
}

// ****************************************************************************
//  Method: MRUCacheBase::operator[]
//
//  Purpose: Reference an item in the cache. If the item is not found, make
//  room for it by removing the oldest item. Put the item's key at the front
//  of the mru history.
//
//  Arguments:
//     key: the key to the item
//
//  Programmer: Mark C. Miller 
//  Creation:   October 6, 2003 
//
// ****************************************************************************
template<class kT, class vT, MRUCache_DeleteMethod dM, size_t nS>
vT& MRUCacheBase<kT,vT,dM,nS>::operator[](const kT& key)
{
   map<kT,vT>::iterator mpos = cache.find(key);

   if (mpos == cache.end())
   {
      // Didn't find it. Erase oldest entry, add this as newest
      if (cache.size() > numSlots)
         remove(oldest());
   }

   age[key] = ageCounter++;
   return cache[key];
}

// ****************************************************************************
//  Method: MRUCacheBase::mru
//
//  Purpose: return most recently used entry in cache 
//
//  Programmer: Mark C. Miller 
//  Creation:   October 6, 2003 
//
// ****************************************************************************
template<class kT, class vT, MRUCache_DeleteMethod dM, size_t nS>
vT& MRUCacheBase<kT,vT,dM,nS>::mru(void) const
{
   map<kT,int>::iterator i = age.begin();
   kT mruKey               = i->first;
   int newest              = i->second; 

   for (i = age.begin(); i != age.end(); i++)
   {
      if (i->second > newest)
      {
         newest = i->second;
         mruKey = i->first;
      }
   }

   return cache[mruKey];
}

// ****************************************************************************
//  Method: MRUCacheBase::numslots
//
//  Purpose: change the number of slots in the cache 
//
//  Programmer: Mark C. Miller 
//  Creation:   October 6, 2003 
//
// ****************************************************************************
template<class kT, class vT, MRUCache_DeleteMethod dM, size_t nS>
size_t MRUCacheBase<kT,vT,dM,nS>::numslots(size_t newNumSlots)
{
   int oldNumSlots = numSlots;

   if (newNumSlots = 0)
      return oldNumSlots;

   // remove entries if we're making it smaller
   while (newNumSlots < cache.size())
      remove(oldest);

   numSlots = newNumSlots;

   return oldNumSlots;
}

// ****************************************************************************
//  Method: MRUCacheBase::oldest
//
//  Purpose: return key of oldest entry in cache 
//
//  Programmer: Mark C. Miller 
//  Creation:   October 6, 2003 
//
// ****************************************************************************
template<class kT, class vT, MRUCache_DeleteMethod dM, size_t nS>
kT MRUCacheBase<kT,vT,dM,nS>::oldest(void)
{
   map<kT,int>::iterator i = age.begin();
   kT retval               = i->first;
   int oldestAge           = i->second; 

   for (i = age.begin(); i != age.end(); i++)
   {
      if (i->second < oldestAge)
      {
         retval    = i->first;
         oldestAge = i->second;
      }
   }

   return retval;
}

#endif
