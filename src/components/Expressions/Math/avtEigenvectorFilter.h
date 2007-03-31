// ************************************************************************* //
//                            avtEigenvectorFilter.h                         //
// ************************************************************************* //

#ifndef AVT_EIGENVECTOR_FILTER_H
#define AVT_EIGENVECTOR_FILTER_H

#include <avtUnaryMathFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtEigenvectorFilter
//
//  Purpose:
//      A filter that calculates the eigenvectors of a tensor.
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2003
//
// ****************************************************************************

class EXPRESSION_API avtEigenvectorFilter : public avtUnaryMathFilter
{
  public:
                              avtEigenvectorFilter() {;};
    virtual                  ~avtEigenvectorFilter() {;};

    virtual const char       *GetType(void)  
                                        { return "avtEigenvectorFilter"; };
    virtual const char       *GetDescription(void) 
                                        { return "Calculating eigenvectors"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples);
    virtual int               GetNumberOfComponentsInOutput(int)
                                          { return 9; };
    virtual int               GetVariableDimension(void) { return 9; };
};


#endif


