// ************************************************************************* //
//                         avtRelativeDifferenceFilter.h                     //
// ************************************************************************* //

#ifndef AVT_RELATIVE_DIFFERENCE_FILTER_H
#define AVT_RELATIVE_DIFFERENCE_FILTER_H

#include <avtBinaryMathFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtRelativeDifferenceFilter
//
//  Purpose:
//      A filter that calculates the relative difference between its two 
//      inputs.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2004
//
// ****************************************************************************

class EXPRESSION_API avtRelativeDifferenceFilter : public avtBinaryMathFilter
{
  public:
                              avtRelativeDifferenceFilter();
    virtual                  ~avtRelativeDifferenceFilter();

    virtual const char       *GetType(void) 
                                 { return "avtRelativeDifferenceFilter"; };
    virtual const char       *GetDescription(void)
                                 { return "Calculating relative difference"; };

  protected:
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                 vtkDataArray *out, int ncomps, int ntuples);
};


#endif


