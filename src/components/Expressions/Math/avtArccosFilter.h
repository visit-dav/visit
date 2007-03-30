// ************************************************************************* //
//                              avtArccosFilter.h                            //
// ************************************************************************* //

#ifndef AVT_ARCCOS_FILTER_H
#define AVT_ARCCOS_FILTER_H


#include <avtUnaryMathFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtArccosFilter
//
//  Purpose:
//      A filter that calculates the arccosine of its input.
//
//  Programmer: Sean Ahern
//  Creation:   Thu Jun 27 11:22:28 PDT 2002
//
//  Modifications:
//
//    Hank Childs, Fri Nov 15 15:23:54 PST 2002
//    Added support for vectors and non-float arrays.
//
// ****************************************************************************

class EXPRESSION_API avtArccosFilter : public avtUnaryMathFilter
{
  public:
                              avtArccosFilter() {;};
    virtual                  ~avtArccosFilter() {;};

    virtual const char       *GetType(void)   { return "avtArccosFilter"; };
    virtual const char       *GetDescription(void) 
                                           { return "Calculating arccosine"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples);
};


#endif


