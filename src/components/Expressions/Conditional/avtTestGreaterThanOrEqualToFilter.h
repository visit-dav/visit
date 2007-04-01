// ************************************************************************* //
//                       avtTestGreaterThanOrEqualToFilter.h                 //
// ************************************************************************* //

#ifndef AVT_TEST_GREATER_THAN_OR_EQUAL_TO_FILTER_H
#define AVT_TEST_GREATER_THAN_OR_EQUAL_TO_FILTER_H

#include <avtComparisonFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtTestGreaterThanOrEqualToFilter
//
//  Purpose:
//      A filter that determines when one input is greater than or equal to
//      another.
//
//  Programmer: Hank Childs
//  Creation:   August 21, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class EXPRESSION_API avtTestGreaterThanOrEqualToFilter 
    : public avtComparisonFilter
{
  public:
                              avtTestGreaterThanOrEqualToFilter();
    virtual                  ~avtTestGreaterThanOrEqualToFilter();

    virtual const char       *GetType(void)
                               { return "avtTestGreaterThanOrEqualToFilter"; };
    virtual const char       *GetDescription(void) 
                               { return "Testing >="; };

  protected:
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                 vtkDataArray *out, int ncomps, int ntuples);
};


#endif


