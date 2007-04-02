// ************************************************************************* //
//                               avtModuloFilter.h                           //
// ************************************************************************* //

#ifndef AVT_MODULO_FILTER_H
#define AVT_MODULO_FILTER_H

#include <avtBinaryMathFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtModuloFilter
//
//  Purpose:
//      A filter that calculates the first variable module the second variable.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2005
//
// ****************************************************************************

class EXPRESSION_API avtModuloFilter : public avtBinaryMathFilter
{
  public:
                              avtModuloFilter();
    virtual                  ~avtModuloFilter();

    virtual const char       *GetType(void) { return "avtModuloFilter"; };
    virtual const char       *GetDescription(void)
                                            { return "Calculating modulo"; };

  protected:
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                 vtkDataArray *out, int ncomps, int ntuples);
};


#endif


