// ************************************************************************* //
//                                avtTanFilter.h                             //
// ************************************************************************* //

#ifndef AVT_TAN_FILTER_H
#define AVT_TAN_FILTER_H


#include <avtUnaryMathFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtTanFilter
//
//  Purpose:
//      A filter that calculates the tangent of its input.
//
//  Programmer: Sean Ahern
//  Creation:   Tue Jun 11 16:23:45 PDT 2002
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class EXPRESSION_API avtTanFilter : public avtUnaryMathFilter
{
  public:
                              avtTanFilter();
    virtual                  ~avtTanFilter();

    virtual const char       *GetType(void)  { return "avtTanFilter"; };
    virtual const char       *GetDescription(void) 
                                             { return "Calculating tangent"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples);
};


#endif


