// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                   avtPolarToCartesianFilter.h                             //
// ************************************************************************* //

#ifndef AVT_POLARTOCARTESIAN_FILTER_H
#define AVT_POLARTOCARTESIAN_FILTER_H

#include <avtDataTreeIterator.h>


// ****************************************************************************
//  Class: avtPolarToCartesianFilter
//
//  Purpose:
//    Converts a polar coordinate curve to cartesian coordinates.
//
//  Programmer: Kathleen Biagas 
//  Creation:   September 11, 2013
//
//  Modifications:
//      Eric Brugger, Tue Aug 19 10:03:09 PDT 2014
//      Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class avtPolarToCartesianFilter : public avtDataTreeIterator
{
  public:
                              avtPolarToCartesianFilter();
    virtual                  ~avtPolarToCartesianFilter();

    virtual const char       *GetType(void)   
                                  { return "avtPolarToCartesianFilter"; }
    virtual const char       *GetDescription(void)
                                  { return "PolarToCartesianing dataset"; }
    void                      SetSwapCoords(bool val)
                                  { swapCoords = val; }
    void                      SetUseDegrees(bool val)
                                  { useDegrees = val; }

  protected:
    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    virtual void              PostExecute(void);
    virtual void              UpdateDataObjectInfo(void);
    virtual avtContract_p     ModifyContract(avtContract_p);

  private:
    bool                      swapCoords;
    bool                      useDegrees;
};


#endif


