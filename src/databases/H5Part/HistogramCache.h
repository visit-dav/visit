// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtHDF_UCFileFormat.h                           //
// ************************************************************************* //

#ifndef HISTOGRAMCACHE_H
#define HISTOGRAMCACHE_H

#include <visit-config.h>
#ifdef HAVE_LIBFASTBIT

#include <avtHistogramSpecification.h>


// ****************************************************************************
//  Class: HistogramCache
//
//  Purpose:
//      Cache histograms in order to avoid unnecessary recomputations
//
//  Programmer: oruebel
//  Creation:   Thu 10.04
//
// ****************************************************************************

class HistogramCache {
public:
      HistogramCache();
      ~HistogramCache();
      
      bool getCached(avtHistogramSpecification *spec);
      void addToCache(avtHistogramSpecification *spec);
      void clearCache();
      void setMaxCacheSize(unsigned int size);
      
private:
      inline bool compareHistogramSpecification(avtHistogramSpecification* spec1 , avtHistogramSpecification* spec2) {
            if( !spec1->IsValid() || !spec2->IsValid() )
                  return false;
            //Check timestep
            if( spec1->GetTimestep() != spec2->GetTimestep() )
                  return false;
            //Check binning strategy
            if( spec1->IsRegularBinning()  != spec2->IsRegularBinning()  )
                  return false;
            //Check dimensionality
            if( spec1->GetDimensionality() != spec2->GetDimensionality() )
                  return false;
            //Check conditions
            if( spec1->GetConditionExact() != spec2->GetConditionExact() )
                  return false;
            //Check the condition
            std::string                c1   = spec1->GetCondition();
            std::string                c2   = spec2->GetCondition();
            if( c1.compare(c2)!=0)
                  return false;
            //Check variables and numbre of bins
            const std::vector<std::string>& var1 = spec1->GetVariables();
            const std::vector<std::string>& var2 = spec2->GetVariables();
            const std::vector<int>&    nb1  = spec1->GetNumberOfBins();
            const std::vector<int>&    nb2  = spec2->GetNumberOfBins();
            for(unsigned int i=0 ; i<var1.size() ; ++i) {
                  if( var1[i].compare(var2[i]) != 0 )
                        return false;
                  if( nb1[i] != nb2[i] )
                        return false;
            }
            
            //Test if the bounds are the same if specified
            if( spec1->BoundsSpecified() && spec2->BoundsSpecified() ){
                  std::vector< std::vector<double> >& bounds1 = spec1->GetBounds();
                  std::vector< std::vector<double> >& bounds2 = spec2->GetBounds();
                  for( unsigned int i=0 ; i<bounds1.size() ; ++i){
                        for( unsigned int j=0 ; j<bounds1[i].size() ; ++j){
                              if( bounds1[i][j] != bounds2[i][j] )
                                    return false;
                        }
                  }
                  
            }
            
            //If all tests have been passed
            return true;
      }
      
      int mMaxCacheSize;
      std::vector<avtHistogramSpecification*> mCachedHistograms;
};


#endif

#endif
