// ************************************************************************* //
//                     avtConservativeSmoothingExpression.h                  //
// ************************************************************************* //

#ifndef AVT_CONSERVATIVE_SMOOTHING_EXPRESSION_H
#define AVT_CONSERVATIVE_SMOOTHING_EXPRESSION_H

#include <avtUnaryMathFilter.h>

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
    : public avtUnaryMathFilter
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
                                 int ncomps, int ntuples);
};


#endif


