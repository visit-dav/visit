// ************************************************************************* //
//                            avtEigenvalueFilter.h                          //
// ************************************************************************* //

#ifndef AVT_EIGENVALUE_FILTER_H
#define AVT_EIGENVALUE_FILTER_H

#include <avtUnaryMathFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtEigenvalueFilter
//
//  Purpose:
//      A filter that calculates the eigenvalue of a tensor.
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class EXPRESSION_API avtEigenvalueFilter : public avtUnaryMathFilter
{
  public:
                              avtEigenvalueFilter();
    virtual                  ~avtEigenvalueFilter();

    virtual const char       *GetType(void)  
                                         { return "avtEigenvalueFilter"; };
    virtual const char       *GetDescription(void) 
                                         { return "Calculating eigenvalues"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples);
    virtual int               GetNumberOfComponentsInOutput(int)
                                          { return 3; };
    virtual int               GetVariableDimension(void)  { return 3; };
};


#endif


