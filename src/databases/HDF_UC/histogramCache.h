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
//                           avtHDF_UCFileFormat.h                           //
// ************************************************************************* //

#ifndef HISTOGRAMCACHE_H
#define HISTOGRAMCACHE_H

#include <avtHistogramSpecification.h>


// ****************************************************************************
//  Class: histogramCache
//
//  Purpose:
//      Cache histograms in order to avoid unnecessary recomputations
//
//  Programmer: oruebel
//  Creation:   Thu 10.04
//
// ****************************************************************************

class histogramCache {
public:
      histogramCache();
      ~histogramCache();
      
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
            string                c1   = spec1->GetCondition();
            string                c2   = spec2->GetCondition();
            if( c1.compare(c2)!=0)
                  return false;
            //Check variables and numbre of bins
            const vector<string>& var1 = spec1->GetVariables();
            const vector<string>& var2 = spec2->GetVariables();
            const vector<int>&    nb1  = spec1->GetNumberOfBins();
            const vector<int>&    nb2  = spec2->GetNumberOfBins();
            for(unsigned int i=0 ; i<var1.size() ; ++i) {
                  if( var1[i].compare(var2[i]) != 0 )
                        return false;
                  if( nb1[i] != nb2[i] )
                        return false;
            }
            
            //Test if the bounds are the same if specified
            if( spec1->BoundsSpecified() && spec2->BoundsSpecified() ){
                  vector< vector<double> >& bounds1 = spec1->GetBounds();
                  vector< vector<double> >& bounds2 = spec2->GetBounds();
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
      vector<avtHistogramSpecification*> mCachedHistograms;
};


#endif



