// ************************************************************************* //
//                              avtArcsinFilter.h                            //
// ************************************************************************* //

#ifndef AVT_ARCSIN_FILTER_H
#define AVT_ARCSIN_FILTER_H


#include <avtUnaryMathFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtArcsinFilter
//
//  Purpose:
//      A filter that calculates the arcsine of its input.
//
//  Programmer: Sean Ahern
//  Creation:   Thu Jun 27 11:21:41 PDT 2002
//
//  Modifications:
//
//    Hank Childs, Fri Nov 15 15:23:54 PST 2002
//    Added support for vectors and non-float arrays.
//
// ****************************************************************************

class EXPRESSION_API avtArcsinFilter : public avtUnaryMathFilter
{
  public:
                              avtArcsinFilter() {;};
    virtual                  ~avtArcsinFilter() {;};

    virtual const char       *GetType(void)   { return "avtArcsinFilter"; };
    virtual const char       *GetDescription(void) 
                                             { return "Calculating arcsine"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples);
};


#endif


