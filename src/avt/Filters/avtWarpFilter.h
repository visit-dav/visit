// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtWarpFilter.h                               //
// ************************************************************************* //

#ifndef AVT_WARP_FILTER_H
#define AVT_WARP_FILTER_H

#include <filters_exports.h>

#include <avtDataTreeIterator.h>


// ****************************************************************************
//  Class: avtWarpFilter
//
//  Purpose:
//    A filter that 'warps' (elevates) a dataset by the point scalars.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   July 12, 2006 
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 17 13:05:53 PST 2006
//    Needs FILTERS api for windows build.
//
//    Eric Brugger, Tue Jul 22 12:24:30 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class AVTFILTERS_API avtWarpFilter : public avtDataTreeIterator
{
  public:
                              avtWarpFilter();
    virtual                  ~avtWarpFilter();

    virtual const char       *GetType(void)   { return "avtWarpFilter"; };
    virtual const char       *GetDescription(void)
                                  { return "Warping dataset"; };

  protected:
    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    virtual void              PostExecute(void);
    virtual void              UpdateDataObjectInfo(void);
    virtual avtContract_p
                              ModifyContract(avtContract_p);
};


#endif


