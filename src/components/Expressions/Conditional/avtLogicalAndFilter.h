// ************************************************************************* //
//                             avtLogicalAndFilter.h                         //
// ************************************************************************* //

#ifndef AVT_LOGICAL_AND_FILTER_H
#define AVT_LOGICAL_AND_FILTER_H

#include <avtComparisonFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtLogicalAndFilter
//
//  Purpose:
//      A filter that finds the logical and of two inputs.
//
//  Programmer: Hank Childs
//  Creation:   August 20, 2003
//
// ****************************************************************************

class EXPRESSION_API avtLogicalAndFilter : public avtComparisonFilter
{
  public:
                              avtLogicalAndFilter() {;};
    virtual                  ~avtLogicalAndFilter() {;};

    virtual const char       *GetType(void)
                                  { return "avtLogicalAndFilter"; };
    virtual const char       *GetDescription(void) 
                                  { return "Performing Logical \'And\'"; };

  protected:
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                 vtkDataArray *out, int ncomps, int ntuples);
};


#endif


