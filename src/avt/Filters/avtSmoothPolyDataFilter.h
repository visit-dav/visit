// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtSmoothPolyDataFilter.h                        //
// ************************************************************************* //

#ifndef AVT_SMOOTH_POLY_DATA_FILTER_H
#define AVT_SMOOTH_POLY_DATA_FILTER_H

#include <filters_exports.h>

#include <avtDataTreeIterator.h>


// ****************************************************************************
//  Class: avtSmoothPolyDataFilter
//
//  Purpose:
//      Smooths geometry.
//
//  Programmer: Jeremy Meredith
//  Creation:   December  6, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Eric Brugger, Tue Jul 22 08:23:57 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class AVTFILTERS_API avtSmoothPolyDataFilter : public avtDataTreeIterator
{
  public:
                             avtSmoothPolyDataFilter();
    virtual                 ~avtSmoothPolyDataFilter();

    virtual const char      *GetType(void) 
                                 { return "avtSmoothPolyDataFilter"; };
    virtual const char      *GetDescription(void)
                                 { return "Smoothing geometry"; };

    void SetSmoothingLevel(int);
    
  protected:
    int     smoothingLevel;

    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
};


#endif


