// ************************************************************************* //
//                             avtLogicalOrFilter.h                          //
// ************************************************************************* //

#ifndef AVT_LOGICAL_OR_FILTER_H
#define AVT_LOGICAL_OR_FILTER_H

#include <avtComparisonFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtLogicalOrFilter
//
//  Purpose:
//      A filter that finds the logical or of two inputs.
//
//  Programmer: Hank Childs
//  Creation:   August 20, 2003
//
// ****************************************************************************

class EXPRESSION_API avtLogicalOrFilter : public avtComparisonFilter
{
  public:
                              avtLogicalOrFilter() {;};
    virtual                  ~avtLogicalOrFilter() {;};

    virtual const char       *GetType(void)
                                  { return "avtLogicalOrFilter"; };
    virtual const char       *GetDescription(void) 
                                  { return "Performing Logical \'Or\'"; };

  protected:
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                 vtkDataArray *out, int ncomps, int ntuples);
};


#endif


