// ************************************************************************* //
//                               avtCosFilter.h                              //
// ************************************************************************* //

#ifndef AVT_COS_FILTER_H
#define AVT_COS_FILTER_H

#include <avtUnaryMathFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtCosFilter
//
//  Purpose:
//      A filter that calculates the sin of its input.
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

class EXPRESSION_API avtCosFilter : public avtUnaryMathFilter
{
  public:
                              avtCosFilter();
    virtual                  ~avtCosFilter();

    virtual const char       *GetType(void)   { return "avtCosFilter"; };
    virtual const char       *GetDescription(void) 
                                              { return "Calculating cosine"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples);
};


#endif


