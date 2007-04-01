// ************************************************************************* //
//                            avtNaturalLogFilter.h                          //
// ************************************************************************* //

#ifndef AVT_NATURALLOG_FILTER_H
#define AVT_NATURALLOG_FILTER_H

#include <avtUnaryMathFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtNaturalLogFilter
//
//  Purpose:
//      A filter that calculates the natural log of its input.
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

class EXPRESSION_API avtNaturalLogFilter : public avtUnaryMathFilter
{
  public:
                              avtNaturalLogFilter();
    virtual                  ~avtNaturalLogFilter();

    virtual const char       *GetType(void)  { return "avtNaturalLogFilter"; };
    virtual const char       *GetDescription(void) 
                                         { return "Calculating natural log"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples);
};


#endif


