// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtActualExtentsFilter.h                         //
// ************************************************************************* //

#ifndef AVT_ACTUAL_EXTENTS_FILTER_H
#define AVT_ACTUAL_EXTENTS_FILTER_H

#include <filters_exports.h>

#include <avtDatasetToDatasetFilter.h>


// ****************************************************************************
//  Class: avtActualExtentsFilter
//
//  Purpose:
//    Calculates the actual extents, both spatial and data.  Stores them
//    in the output's info. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 2, 2001 
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//
//    Hank Childs, Tue Aug 26 14:37:27 PDT 2008
//    Implement ModifyContract to prevent base class from declaring that it
//    can only work on floats.
//
//    Hank Childs, Thu Aug 26 13:47:30 PDT 2010
//    Renamed to avtActualExtentsFilter.
//
// ****************************************************************************

class AVTFILTERS_API avtActualExtentsFilter : public avtDatasetToDatasetFilter
{
  public:
                          avtActualExtentsFilter(){};
    virtual              ~avtActualExtentsFilter(){}; 

    virtual const char   *GetType(void) {return "avtActualExtentsFilter";};
    virtual const char   *GetDescription(void) 
                              { return "Calculating Actual Extents."; };

  protected:
    avtContract_p         lastContract;

    virtual void          Execute(void);
    virtual void          UpdateExtents(void);
    virtual bool          FilterUnderstandsTransformedRectMesh();
    virtual avtContract_p ModifyContract(avtContract_p);
};


#endif


