// ************************************************************************* //
//                            avtTestLessThanFilter.h                        //
// ************************************************************************* //

#ifndef AVT_TEST_LESS_THAN_FILTER_H
#define AVT_TEST_LESS_THAN_FILTER_H

#include <avtComparisonFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtTestLessThanFilter
//
//  Purpose:
//      A filter that determines when one input is less than another.
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

class EXPRESSION_API avtTestLessThanFilter : public avtComparisonFilter
{
  public:
                              avtTestLessThanFilter();
    virtual                  ~avtTestLessThanFilter();

    virtual const char       *GetType(void)
                                  { return "avtTestLessThanFilter"; };
    virtual const char       *GetDescription(void) 
                                  { return "Testing <"; };

  protected:
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                 vtkDataArray *out, int ncomps, int ntuples);
};


#endif


