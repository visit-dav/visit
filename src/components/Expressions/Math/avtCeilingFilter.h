// ************************************************************************* //
//                            avtCeilingFilter.h                             //
// ************************************************************************* //

#ifndef AVT_CEILING_FILTER_H
#define AVT_CEILING_FILTER_H


#include <avtUnaryMathFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtCeilingFilter
//
//  Purpose:
//      A filter that calculates the ceiling of a floating point quantity.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2005
//
// ****************************************************************************

class EXPRESSION_API avtCeilingFilter : public avtUnaryMathFilter
{
  public:
                              avtCeilingFilter();
    virtual                  ~avtCeilingFilter();

    virtual const char       *GetType(void)  { return "avtCeilingFilter"; };
    virtual const char       *GetDescription(void) 
                                         { return "Calculating the ceiling"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples);
};


#endif


