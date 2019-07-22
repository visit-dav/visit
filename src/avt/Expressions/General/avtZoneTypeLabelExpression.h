// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtZoneTypeLabelExpression.h                         //
// ************************************************************************* //

#ifndef AVT_ZONETYPELABEL_FILTER_H
#define AVT_ZONETYPELABEL_FILTER_H

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;

// ****************************************************************************
//  Class: avtZoneTypeLabelExpression
//
//  Purpose:
//      Determines the number of materials in a zone.
//          
//  Programmer: Mark C. Miller (copied from Hank's NMats filter) 
//  Creation:   November 15, 2006 
//
//  Modifications:
//    Mark C. Miller, Wed Apr  2 09:46:47 PDT 2008
//    Added UpdateDataObjectInfo() to set treat as ascii
//
// ****************************************************************************

class EXPRESSION_API avtZoneTypeLabelExpression : public avtSingleInputExpressionFilter
{
  public:
                              avtZoneTypeLabelExpression();
    virtual                  ~avtZoneTypeLabelExpression();

    virtual const char       *GetType(void) { return "avtZoneTypeLabelExpression"; };
    virtual const char       *GetDescription(void)
                                           {return "Finding ZoneTypeLabel";};
  protected:
    virtual void              UpdateDataObjectInfo(void);
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual bool              IsPointVariable(void)  { return false; };
};


#endif


