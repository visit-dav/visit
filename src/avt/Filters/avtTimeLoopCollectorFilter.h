// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                     avtTimeLoopCollectorFilter.h                          //
// ************************************************************************* //

#ifndef AVT_TIME_LOOP_COLLETOR_FILTER_H
#define AVT_TIME_LOOP_COLLETOR_FILTER_H

#include <filters_exports.h>

#include <vector>

#include <vectortypes.h>

#include <avtDataTree.h>
#include <avtTimeLoopFilter.h>
#include <avtDatasetToDatasetFilter.h>


// ****************************************************************************
//  Method: avtTimeLoopCollectorFilter
//
//  Purpose:
//    An abstract filter that collects the data from many time steps and 
//    sends that data to derived types via a virtual method call.
// 
//  Programmer: Hank Childs
//  Creation:   January 22, 2008
//
// ****************************************************************************

class AVTFILTERS_API avtTimeLoopCollectorFilter 
    : virtual public avtTimeLoopFilter,virtual public avtDatasetToDatasetFilter
{
  public:
                             avtTimeLoopCollectorFilter();
    virtual                 ~avtTimeLoopCollectorFilter();

  protected:
    std::vector<avtDataTree_p>    trees;

    virtual void             Execute(void);
    virtual void             CreateFinalOutput(void);
    virtual void             ReleaseData(void);
    virtual avtDataTree_p    ExecuteAllTimesteps(std::vector<avtDataTree_p> &) = 0;
};


#endif


