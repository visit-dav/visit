/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
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
#include <vector>
using std::map;
using std::vector;


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
//  val-type being used in the cache. If all else fails, you can always
//  set the delete callback method so MRUCache will call your special
//  purpose delete method.
//
//  Programmer: Mark C. Miller 
//  Creation:   October 6, 2003 
//
//  Modifications:
//    Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//    Fixed problems with deleteit only getting invoked on base-class.
//    Fixed problems with clear getting called too late in deconstruction.
//    Added find methods. Renamed existing find to exists. Added
//    CallbackDelete specialization.
//
//    Jeremy Meredith, Mon Aug 28 18:04:57 EDT 2006
//    Newer gcc's won't resolve unqualified members of a dependent base in 
//    templates.  See [temp.dep]/3 in the ANSI C++ Standard.  Using explicit
//    this-> fixes it (as would adding explicit "using" declaration).
//
//    Mark C. Miller, Mon Jul 30 13:31:00 PDT 2007
//    Made oldest method public.
//    Added const operator[]
//
//    Tom Fogal, Thu Aug 21 16:06:29 EDT 2008
//    Made the destructor virtual, since there is a virtual method.
//
// ****************************************************************************

// tags for which kind of delete to call on cache pre-emption
typedef enum {
   MRUCache_DontDelete,
   MRUCache_Delete,
   MRUCache_ArrayDelete,
   MRUCache_Free,
   MRUCache_CallbackDelete
} MRUCache_DeleteMethod;

typedef void (*MRUCache_DeleteCallback)(void*);

// kT is keyType, vT is valueType
template<class kT, class vT, MRUCache_DeleteMethod dM, size_t nS=20>
class MRUCacheBase {

   public:

      MRUCacheBase() : numSlots(nS), ageCounter(0) {};
      virtual ~MRUCacheBase() {};

      // explicit existence test (won't change MRU history)
      bool exists(const kT& key) const;

      // explicit remove of an entry from the cache
      void remove(const kT& key);

      // remove all entries from the cache
      void clear(void);

      // get reference to cached entry 
      //    a. hit moves cached entry to mru,
      //    b. miss removes lru entry, moves new entry to mru
      vT& operator[](const kT& key);
      const vT& operator[](const kT& key) const;

      // iterator (only allow const_iterator)
      typedef typename map<kT,vT>::const_iterator const_iterator;
      typedef typename map<kT,vT>::const_iterator iterator;
      const_iterator begin(void) const { return cache.begin(); };
      const_iterator end(void) const { return cache.end(); };

      // find operators
      iterator find(const kT& key)
      {
          iterator mpos = cache.find(key);
          if (mpos != end())
              age[key] = ageCounter++;
          return mpos;
      };
      const_iterator find(const kT& key) const { return cache.find(key); };

      iterator find(const vector<kT>& keys)
      {
          iterator mpos;
          for (int i = 0; i < keys.size(); i++)
          {
              mpos = find(keys[i]);
              if (mpos != end())
                  return mpos;
          }
          return mpos;
      }
      const_iterator find(const vector<kT>& keys) const
      {
          iterator mpos;
          for (int i = 0; i < keys.size(); i++)
          {
              mpos = find(keys[i]);
              if (mpos != end())
                  return mpos;
          }
          return mpos;
      }

      // get most recently used entry 
      const vT& mru(void) const;

      // return total memory used by all cached values
      size_t memsize(void) const { return cache.size() * sizeof(vT); };

      // return number of cache slots 
      size_t numslots(void) const { return numSlots; };

      // set number of cache slots, may result in
      // cache deletions if smaller than previous number
      size_t numslots(size_t newNumSlots);

      // returns key of the oldest entry
      kT oldest(void);

   private:

      // only method to be overridden based on item type 
      virtual void deleteit(vT& item)  = 0;

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
     ~MRUCache() { this->clear(); };
   private:
      void deleteit(vT& item) {} ;
};

// Delete specialization
template<class kT, class vT, size_t nS>
class MRUCache<kT,vT,MRUCache_Delete,nS> : public MRUCacheBase<kT,vT,MRUCache_Delete,nS>
{
   public:
      MRUCache() : MRUCacheBase<kT,vT,MRUCache_Delete,nS>() {} ;
     ~MRUCache() { this->clear(); };
   private:
      void deleteit(vT& item) { delete item; } ;
};

// ArrayDelete specialization 
template<class kT, class vT, size_t nS>
class MRUCache<kT,vT,MRUCache_ArrayDelete,nS> : public MRUCacheBase<kT,vT,MRUCache_ArrayDelete,nS>
{
   public:
      MRUCache() : MRUCacheBase<kT,vT,MRUCache_ArrayDelete,nS>() {} ;
     ~MRUCache() { this->clear(); };
   private:
      void deleteit(vT& item) { delete [] item; } ;
};

// Free specialization 
template<class kT, class vT, size_t nS>
class MRUCache<kT,vT,MRUCache_Free,nS> : public MRUCacheBase<kT,vT,MRUCache_Free,nS>
{
   public:
      MRUCache() : MRUCacheBase<kT,vT,MRUCache_Free,nS>() {} ;
     ~MRUCache() { this->clear(); };
   private:
      void deleteit(vT& item) { free (item); } ;
};

// Delete callback specialization 
template<class kT, class vT, size_t nS>
class MRUCache<kT,vT,MRUCache_CallbackDelete,nS> : public MRUCacheBase<kT,vT,MRUCache_CallbackDelete,nS>
{
   public:
      MRUCache(MRUCache_DeleteCallback cb) : MRUCacheBase<kT,vT,MRUCache_CallbackDelete,nS>(), delCb(cb) {};
     ~MRUCache() { this->clear(); };
   private:
      MRUCache() : MRUCacheBase<kT,vT,MRUCache_CallbackDelete,nS>(), delCb(0) {};
      MRUCache_DeleteCallback  delCb;
      void deleteit(vT& item) { (*delCb) (item); } ;
};

// ****************************************************************************
//  Method: MRUCacheBase::exists
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
bool MRUCacheBase<kT,vT,dM,nS>::exists(const kT& key) const
{
   return cache.find(key) != cache.end();
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
//  Modifications:
//    Mark C. Miller, Mon Sep 18 14:22:13 PDT 2006
//    Worked around apparent STL bug on AIX where the const kT& key arg for kT
//    being type string was getting changed to "" after calling cache.erase(k)
//
// ****************************************************************************
template<class kT, class vT, MRUCache_DeleteMethod dM, size_t nS>
void MRUCacheBase<kT,vT,dM,nS>::remove(const kT& key)
{
   typename map<kT,vT>::iterator k = cache.find(key);
   if (k == cache.end())
      return;

   // delete the cached value
   deleteit(k->second);

   // erase slots from the cache
   typename map<kT,int>::iterator j = age.find(key);
   cache.erase(k);
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
   typename map<kT,vT>::iterator i = cache.begin();

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
   typename map<kT,vT>::iterator mpos = cache.find(key);

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
//  Method: MRUCacheBase::operator[]
//
//  Purpose: const version of above 
//
//  Arguments:
//     key: the key to the item
//
//  Programmer: Mark C. Miller 
//  Creation:   August 5, 2007 
//
// ****************************************************************************
template<class kT, class vT, MRUCache_DeleteMethod dM, size_t nS>
const vT& MRUCacheBase<kT,vT,dM,nS>::operator[](const kT& key) const
{
   typename map<kT,vT>::const_iterator mpos = cache.find(key);
   return mpos->second;
}

// ****************************************************************************
//  Method: MRUCacheBase::mru
//
//  Purpose: return most recently used entry in cache 
//
//  Programmer: Mark C. Miller 
//  Creation:   October 6, 2003 
//
//  Modifications:
//
//    Mark C. Miller, Mon Aug  6 13:36:16 PDT 2007
//    const qualified the return value
// ****************************************************************************
template<class kT, class vT, MRUCache_DeleteMethod dM, size_t nS>
const vT& MRUCacheBase<kT,vT,dM,nS>::mru(void) const
{
   typename map<kT,int>::iterator i = age.begin();
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
//  Modifications:
//
//    Mark C. Miller, Mon Aug  6 13:36:16 PDT 2007
//    Fixed error in missing '()' on refernece to oldest
// ****************************************************************************
template<class kT, class vT, MRUCache_DeleteMethod dM, size_t nS>
size_t MRUCacheBase<kT,vT,dM,nS>::numslots(size_t newNumSlots)
{
   int oldNumSlots = numSlots;

   if (newNumSlots <= 0)
      return oldNumSlots;

   // remove entries if we're making it smaller
   while (newNumSlots < cache.size())
      remove(oldest());

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
   typename map<kT,int>::iterator i = age.begin();
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
