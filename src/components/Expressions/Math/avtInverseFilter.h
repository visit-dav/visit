// ************************************************************************* //
//                              avtInverseFilter.h                           //
// ************************************************************************* //

#ifndef AVT_INVERSE_FILTER_H
#define AVT_INVERSE_FILTER_H

#include <avtUnaryMathFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtInverseFilter
//
//  Purpose:
//      A filter that calculates the inverse of a tensor.
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2003
//
// ****************************************************************************

class EXPRESSION_API avtInverseFilter : public avtUnaryMathFilter
{
  public:
                              avtInverseFilter() {;};
    virtual                  ~avtInverseFilter() {;};

    virtual const char       *GetType(void)  
                                         { return "avtInverseFilter"; };
    virtual const char       *GetDescription(void) 
                                         { return "Calculating inverse"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples);
};


#endif


