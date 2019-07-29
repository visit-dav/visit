// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtZoneTypeRankExpression.h                         //
// ************************************************************************* //

#ifndef AVT_ZONETYPERANK_FILTER_H
#define AVT_ZONETYPERANK_FILTER_H

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;

// ****************************************************************************
//  Class: avtZoneTypeRankExpression
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

class EXPRESSION_API avtZoneTypeRankExpression : public avtSingleInputExpressionFilter
{
  public:
                              avtZoneTypeRankExpression();
    virtual                  ~avtZoneTypeRankExpression();

    virtual const char       *GetType(void) { return "avtZoneTypeRankExpression"; };
    virtual const char       *GetDescription(void)
                                           {return "Finding ZoneTypeRank";};
  protected:
    virtual void              UpdateDataObjectInfo(void);
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual bool              IsPointVariable(void)  { return false; };
};


#endif


