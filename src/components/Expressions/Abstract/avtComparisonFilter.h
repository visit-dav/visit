// ************************************************************************* //
//                           avtComparisonFilter.h                           //
// ************************************************************************* //

#ifndef AVT_COMPARISON_FILTER_H
#define AVT_COMPARISON_FILTER_H

#include <avtBinaryMathFilter.h>

#include <vtkUnsignedCharArray.h>


// ****************************************************************************
//  Class: avtComparisonFilter
//
//  Purpose:
//      This is the base class for any expression that compares two arrays
//
//  Programmer: Hank Childs
//  Creation:   August 20, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class avtComparisonFilter : public avtBinaryMathFilter
{
  public:
                           avtComparisonFilter();
    virtual               ~avtComparisonFilter();

  protected:
    virtual int            GetNumberOfComponentsInOutput() { return 1; };
    virtual vtkDataArray  *CreateArray(vtkDataArray *)
                                { return vtkUnsignedCharArray::New(); };
};


#endif


