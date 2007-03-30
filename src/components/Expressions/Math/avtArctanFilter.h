// ************************************************************************* //
//                              avtArctanFilter.h                             //
// ************************************************************************* //

#ifndef AVT_ARCTAN_FILTER_H
#define AVT_ARCTAN_FILTER_H


#include <avtUnaryMathFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtArctanFilter
//
//  Purpose:
//      A filter that calculates the arctangent of its input.
//
//  Programmer: Sean Ahern
//  Creation:   Thu Jun 27 11:19:34 PDT 2002
//
//  Modifications:
//
//    Hank Childs, Fri Nov 15 15:25:26 PST 2002
//    Added support for vectors and arbitrary data types.
//
// ****************************************************************************

class EXPRESSION_API avtArctanFilter : public avtUnaryMathFilter
{
  public:
                              avtArctanFilter() {;};
    virtual                  ~avtArctanFilter() {;};

    virtual const char       *GetType(void)   { return "avtArctanFilter"; };
    virtual const char       *GetDescription(void) 
                                          { return "Calculating arctangent"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples);
};


#endif


