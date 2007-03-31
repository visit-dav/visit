// ************************************************************************* //
//                            avtTestNotEqualToFilter.h                      //
// ************************************************************************* //

#ifndef AVT_TEST_NOT_EQUAL_TO_FILTER_H
#define AVT_TEST_NOT_EQUAL_TO_FILTER_H

#include <avtComparisonFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtTestNotEqualToFilter
//
//  Purpose:
//      A filter that determines when two inputs are not equal.
//
//  Programmer: Hank Childs
//  Creation:   August 21, 2003
//
// ****************************************************************************

class EXPRESSION_API avtTestNotEqualToFilter 
    : public avtComparisonFilter
{
  public:
                              avtTestNotEqualToFilter() {;};
    virtual                  ~avtTestNotEqualToFilter() {;};

    virtual const char       *GetType(void)
                                  { return "avtTestNotEqualToFilter"; };
    virtual const char       *GetDescription(void) 
                                  { return "Testing for equality"; };

  protected:
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                 vtkDataArray *out, int ncomps, int ntuples);
};


#endif


