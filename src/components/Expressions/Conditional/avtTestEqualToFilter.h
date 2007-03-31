// ************************************************************************* //
//                             avtTestEqualToFilter.h                        //
// ************************************************************************* //

#ifndef AVT_TEST_EQUAL_TO_FILTER_H
#define AVT_TEST_EQUAL_TO_FILTER_H

#include <avtComparisonFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtTestEqualToFilter
//
//  Purpose:
//      A filter that determines when two inputs are equal.
//
//  Programmer: Hank Childs
//  Creation:   August 21, 2003
//
// ****************************************************************************

class EXPRESSION_API avtTestEqualToFilter : public avtComparisonFilter
{
  public:
                              avtTestEqualToFilter() {;};
    virtual                  ~avtTestEqualToFilter() {;};

    virtual const char       *GetType(void)
                                  { return "avtTestEqualToFilter"; };
    virtual const char       *GetDescription(void) 
                                  { return "Testing for equality"; };

  protected:
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                 vtkDataArray *out, int ncomps, int ntuples);
};


#endif


