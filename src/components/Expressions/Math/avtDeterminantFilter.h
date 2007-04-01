// ************************************************************************* //
//                           avtDeterminantFilter.h                          //
// ************************************************************************* //

#ifndef AVT_DETERMINANT_FILTER_H
#define AVT_DETERMINANT_FILTER_H

#include <avtUnaryMathFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtDeterminantFilter
//
//  Purpose:
//      A filter that calculates the determinant of a tensor.
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

class EXPRESSION_API avtDeterminantFilter : public avtUnaryMathFilter
{
  public:
                              avtDeterminantFilter();
    virtual                  ~avtDeterminantFilter();

    virtual const char       *GetType(void)  
                                         { return "avtDeterminantFilter"; };
    virtual const char       *GetDescription(void) 
                                         { return "Calculating determinant"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples);
    virtual int               GetNumberOfComponentsInOutput(int)
                                          { return 1; };
};


#endif


