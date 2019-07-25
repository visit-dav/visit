// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtRectilinearLaplacianExpression.h                  //
// ************************************************************************* //

#ifndef AVT_RECTILINEAR_LAPLACIAN_FILTER_H
#define AVT_RECTILINEAR_LAPLACIAN_FILTER_H


#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtRectilinearLaplacianExpression
//
//  Purpose:
//      A filter that calculates the Laplacian by going out only one stencil.
//          
//  Programmer: Hank Childs
//  Creation:   May 7, 2008
//
// ****************************************************************************

class EXPRESSION_API avtRectilinearLaplacianExpression 
                                       : public avtSingleInputExpressionFilter
{
  public:
                              avtRectilinearLaplacianExpression();
    virtual                  ~avtRectilinearLaplacianExpression();

    virtual const char       *GetType(void)
                               { return "avtRectilinearLaplacianExpression"; };
    virtual const char       *GetDescription(void)
                                             {return "Calculating Laplacian";};
  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual int               GetVariableDimension(void) { return 1; };
};


#endif


