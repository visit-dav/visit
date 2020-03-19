// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtEdgeNormalExpression.h                        //
// ************************************************************************* //

#ifndef AVT_EDGE_NORMAL_EXPRESSION_H
#define AVT_EDGE_NORMAL_EXPRESSION_H

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;
class     vtkRectilinearGrid;


// ****************************************************************************
//  Class: avtEdgeNormalExpression
//
//  Purpose:
//      Find normals for (assumed 2D) edges / line segments.
//          
//  Programmer: Jeremy Meredith
//  Creation:   March 10, 2014
//
//  Modifications:
//
// ****************************************************************************

class EXPRESSION_API avtEdgeNormalExpression 
    : public avtSingleInputExpressionFilter
{
  public:
                              avtEdgeNormalExpression();
    virtual                  ~avtEdgeNormalExpression();

    virtual const char       *GetType(void) 
                                      { return "avtEdgeNormalExpression"; };
    virtual const char       *GetDescription(void)
                                      {return "Calculating edge normal"; };
    void                      DoPointNormals(bool val) { isPoint = val; };

  protected:
    bool                      isPoint;
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual bool              IsPointVariable(void)  { return isPoint; };
    virtual int               GetVariableDimension(void) { return 2; };
    vtkDataArray             *RectilinearDeriveVariable(vtkRectilinearGrid *);
};


#endif


