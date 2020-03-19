// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            HistogramCache.C                               //
// ************************************************************************* //

#include <HistogramCache.h>

#ifdef HAVE_LIBFASTBIT

// ****************************************************************************
//  Method: HistogramCache constructor
//
//  Programmer: oruebel
//  Creation:   Thu Sept 04
//
// ****************************************************************************
HistogramCache::HistogramCache()
{
      mMaxCacheSize = 60;
      mCachedHistograms.clear();
      mCachedHistograms.reserve(mMaxCacheSize);
}
 
// ****************************************************************************
//  Method: HistogramCache destructor
//
//  Programmer: oruebel
//  Creation:   Thu Sept 04
//
// ****************************************************************************
HistogramCache::~HistogramCache()
{
      clearCache();
}

// ****************************************************************************
//  Method: HistogramCache::getCached
//
//  Purpose: Ask if a given histogram is available in this cache
//
//  Arguments:
//      spec    Specification of the desired histogram
//
//  Programmer: oruebel
//  Creation:   Thu Sept 04
//
// ****************************************************************************
bool 
HistogramCache::getCached(avtHistogramSpecification *spec)
{
      avtHistogramSpecification* found = NULL;
      int index =-1;
      for(int i=mCachedHistograms.size()-1 ; i>=0 ; --i){
            if( compareHistogramSpecification(spec , mCachedHistograms[i] ) ){
                  found = mCachedHistograms[i];
                  index =i;
                  break;
            }
      }
      
      if( found != NULL){
            spec->copyInfo( found );
            //Move the found histogram to the end of the vector to 
            //make sure it stays cached
            mCachedHistograms.erase(mCachedHistograms.begin()+index);
            mCachedHistograms.push_back(found);        
            return true;
      }else{
            return false;
      }
}

// ****************************************************************************
//  Method: HistogramCache::addToCache
//
//  Purpose: Add a given histogram to this histogram cache
//
//  Arguments:
//      spec    Complete specification of the histogram to be cached
//
//  Programmer: oruebel
//  Creation:   Thu Sept 04
//
// ****************************************************************************
void 
HistogramCache::addToCache(avtHistogramSpecification *spec)
{
      if( !spec->IsValid() )
            return;
      if( !spec->BoundsSpecified() )
            return;
            
      //make a deep copy of the object
      avtHistogramSpecification* histoCopy = new avtHistogramSpecification();
      histoCopy->copyInfo(spec);
      //save the object in the cache if there is enough space
      if(mCachedHistograms.size() < (size_t)mMaxCacheSize){
            mCachedHistograms.push_back( histoCopy );
      }else{
            //remove the oldest object from the cache
            avtHistogramSpecification* temp = mCachedHistograms[0];
            delete temp;
            mCachedHistograms.erase( mCachedHistograms.begin());
            //add the object to the cache
            mCachedHistograms.push_back(histoCopy);
      }
}

// ****************************************************************************
//  Method: HistogramCache::setMaxCacheSize
//
//  Purpose: Define the maximum size of the cache. The size is here 
//           specified in terms of the maximum number of histograms
//           to be stored in the cache not in terms of maximum size 
//           in memory.
//
//  Arguments:
//      size   Maximum number of histograms to be cached.
//
//  Programmer: oruebel
//  Creation:   Thu Sept 04
//
// ****************************************************************************void
void
HistogramCache::setMaxCacheSize(unsigned int size)
{
      if(size==0)
            return;
      mMaxCacheSize = size;
      if( mCachedHistograms.size() > (size_t)mMaxCacheSize ){
            //remove objects from the oldest objects from the cache
            for(size_t i=mMaxCacheSize; i<mCachedHistograms.size() ; ++i){
                  avtHistogramSpecification* temp = mCachedHistograms[i];
                  mCachedHistograms[i] = NULL;
                  delete temp;
            }
            mCachedHistograms.resize(mMaxCacheSize);      
      }
      mCachedHistograms.reserve(mMaxCacheSize);
}

// ****************************************************************************
//  Method: HistogramCache::clearCache
//
//  Purpose: Remove all data from this histogram cache
//
//  Programmer: oruebel
//  Creation:   Thu Sept 04
//
// ****************************************************************************
void 
HistogramCache::clearCache()
{
      for(unsigned int i=0 ; i<mCachedHistograms.size() ; ++i){
            avtHistogramSpecification* temp = mCachedHistograms[i];
            mCachedHistograms[i] = NULL;
            delete temp;
      }
      mCachedHistograms.clear();
            
}

#endif
