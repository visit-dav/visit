/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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
      if(mCachedHistograms.size() < mMaxCacheSize){
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
      if( mCachedHistograms.size() > mMaxCacheSize ){
            //remove objects from the oldest objects from the cache
            for(int i=mMaxCacheSize; i<mCachedHistograms.size() ; ++i){
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
