// ************************************************************************* //
//                         avtVectorCrossProductFilter.h                     //
// ************************************************************************* //

#ifndef AVT_VCROSSPRODUCT_FILTER_H
#define AVT_VCROSSPRODUCT_FILTER_H

#include <avtBinaryMathFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtVectorCrossProductFilter
//
//  Purpose:
//      A filter that calculates the vector cross product of its two inputs.
//
//  Programmer: Sean Ahern
//  Creation:   Wed Jun 11 13:29:38 PDT 2003
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class EXPRESSION_API avtVectorCrossProductFilter : public avtBinaryMathFilter
{
  public:
                              avtVectorCrossProductFilter();
    virtual                  ~avtVectorCrossProductFilter();

    virtual const char       *GetType(void)   { return "avtVectorCrossProductFilter"; };
    virtual const char       *GetDescription(void)
                                     { return "Calculating vector cross product"; };
    virtual int               NumVariableArguments() { return 2; }

  protected:
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                 vtkDataArray *out, int ncomps, int ntuples);
};


#endif
