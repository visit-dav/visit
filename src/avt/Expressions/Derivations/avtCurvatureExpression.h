// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtCurvatureExpression.h                         //
// ************************************************************************* //

#ifndef AVT_CURVATURE_EXPRESSION_H
#define AVT_CURVATURE_EXPRESSION_H

#include <avtSingleInputExpressionFilter.h>


// ****************************************************************************
//  Class: avtCurvatureExpression
//
//  Purpose:
//      Calculates the mean or gaussian curvature using a VTK filter.  The
//      input must be a surface.
//
//  Programmer: Hank Childs
//  Creation:   May 11, 2006
//
// ****************************************************************************

class EXPRESSION_API avtCurvatureExpression
    : public avtSingleInputExpressionFilter
{
  public:
                              avtCurvatureExpression();
    virtual                  ~avtCurvatureExpression();

    virtual const char       *GetType(void)
                                      { return "avtCurvatureExpression"; };
    virtual const char       *GetDescription(void)
                                      {return "Calculating curvature"; };
    void                      DoGaussCurvature(bool val) { doGauss = val; };

  protected:
    bool                      doGauss;
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual bool              IsPointVariable(void)  { return true; };
    virtual int               GetVariableDimension(void) { return 1; };
};



#endif


