// ************************************************************************* //
//                           avtTestGreaterThanFilter.h                      //
// ************************************************************************* //

#ifndef AVT_TEST_GREATER_THAN_FILTER_H
#define AVT_TEST_GREATER_THAN_FILTER_H

#include <avtComparisonFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtTestGreaterThanFilter
//
//  Purpose:
//      A filter that determines when one input is greater than another.
//
//  Programmer: Hank Childs
//  Creation:   August 21, 2003
//
// ****************************************************************************

class EXPRESSION_API avtTestGreaterThanFilter : public avtComparisonFilter
{
  public:
                              avtTestGreaterThanFilter() {;};
    virtual                  ~avtTestGreaterThanFilter() {;};

    virtual const char       *GetType(void)
                                  { return "avtTestGreaterThanFilter"; };
    virtual const char       *GetDescription(void) 
                                  { return "Testing >"; };

  protected:
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                 vtkDataArray *out, int ncomps, int ntuples);
};


#endif


