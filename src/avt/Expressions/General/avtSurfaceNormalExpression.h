// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtSurfaceNormalExpression.h                       //
// ************************************************************************* //

#ifndef AVT_SURFACE_NORMAL_EXPRESSION_H
#define AVT_SURFACE_NORMAL_EXPRESSION_H

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;
class     vtkRectilinearGrid;


// ****************************************************************************
//  Class: avtSurfaceNormalExpression
//
//  Purpose:
//      An expression that determines which nodes are external.
//          
//  Programmer: Hank Childs
//  Creation:   September 22, 2005
//
//  Modifications:
//
//    Hank Childs, Fri Sep 24 10:18:38 PDT 2010
//    Add a method for rectilinear generation.
//
//    Alister Maguire, Mon Apr 27 12:04:22 PDT 2020
//    Added override of UpdateDataObjectInfo and added zonesHaveBeenSplit.
//
// ****************************************************************************

class EXPRESSION_API avtSurfaceNormalExpression 
    : public avtSingleInputExpressionFilter
{
  public:
                              avtSurfaceNormalExpression();
    virtual                  ~avtSurfaceNormalExpression();

    virtual const char       *GetType(void) 
                                      { return "avtSurfaceNormalExpression"; };
    virtual const char       *GetDescription(void)
                                      {return "Calculating surface normal"; };
    void                      DoPointNormals(bool val) { isPoint = val; };

  protected:
    bool                      isPoint;
    bool                      zonesHaveBeenSplit;
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual bool              IsPointVariable(void)  { return isPoint; };
    virtual int               GetVariableDimension(void) { return 3; };
    vtkDataArray             *RectilinearDeriveVariable(vtkRectilinearGrid *);
    virtual void              UpdateDataObjectInfo(void);
};


#endif


