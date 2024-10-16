// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                     avtConservativeSmoothingExpression.h                  //
// ************************************************************************* //

#ifndef AVT_CONSERVATIVE_SMOOTHING_EXPRESSION_H
#define AVT_CONSERVATIVE_SMOOTHING_EXPRESSION_H

#include <avtUnaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtConservativeSmoothingExpression
//
//  Purpose:
//      A filter that calculates the conservative smoothing of an input image.
//      Conservative smoothing is the following:
//      For each pixel, calculate the range of values from all of its 
//      neighbors (excluding the pixel itself).  Then if the pixel (ie
//      the center pixel), is outside of the range, clamp it to the range.
//      This is for salt-and-pepper noise.
//
//  Programmer: Hank Childs
//  Creation:   August 16, 2005
//
// ****************************************************************************

class EXPRESSION_API avtConservativeSmoothingExpression 
    : public avtUnaryMathExpression
{
  public:
                              avtConservativeSmoothingExpression();
    virtual                  ~avtConservativeSmoothingExpression();

    virtual const char       *GetType(void) 
                                 { return "avtConservativeSmoothingExpression"; };
    virtual const char       *GetDescription(void)
                                 { return "Conservative smoothing"; };

  protected:
    bool             haveIssuedWarning;

    virtual void     PreExecute(void);
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *out, 
                                 int ncomps, int ntuples, vtkDataSet *in_ds);
};


#endif


