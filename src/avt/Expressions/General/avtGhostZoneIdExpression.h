// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtGhostZoneIdExpression.h                        //
// ************************************************************************* //

#ifndef AVT_GHOST_ZONE_ID_FILTER_H
#define AVT_GHOST_ZONE_ID_FILTER_H

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtGhostZoneIdExpression
//
//  Purpose:
//      Provides an interface for requesting ghost zone ids.
//          
//  Programmer: Justin Privitera
//  Creation:   Wed Mar 30 11:40:46 PDT 2022
//
// ****************************************************************************

class EXPRESSION_API avtGhostZoneIdExpression : public avtSingleInputExpressionFilter
{
  public:
                            avtGhostZoneIdExpression();
    virtual                 ~avtGhostZoneIdExpression();

    virtual const char      *GetType(void) { return "avtGhostZoneIdExpression"; };
    virtual const char      *GetDescription(void)
                                { return "Assigning Ghost Zone IDs."; };
    virtual void            PreExecute(void);

    void                    CreateGhostZoneIds(void) 
                                { doGhostZoneIds = true; };

  protected:
    bool                        doGhostZoneIds;

    virtual vtkDataArray    *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual avtContract_p   ModifyContract(avtContract_p);
};


#endif


