// ************************************************************************* //
//                        avtSingleInputExpressionFilter.h                   //
// ************************************************************************* //

#ifndef AVT_SINGLE_INPUT_EXPRESSION_FILTER_H
#define AVT_SINGLE_INPUT_EXPRESSION_FILTER_H

#include <avtExpressionStreamer.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtSingleInputExpressionFilter
//
//  Purpose:
//      A filter that performs a calculation on a single variable.
//
//  Programmer: Sean Ahern
//  Creation:   Wed Jun 12 16:43:09 PDT 2002
//
//  Modifications:
//    Sean Ahern, Fri Jun 13 11:22:43 PDT 2003
//    Added the NumVariableArguments function, specifying that all
//    subclasses of SingleInputExpressionFilter have one variable input.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs, Mon Dec 27 10:37:15 PST 2004
//    Changed inheritance from avtExpressionFilter to avtExpressionStreamer.
//
// ****************************************************************************

class EXPRESSION_API avtSingleInputExpressionFilter 
    : public avtExpressionStreamer
{
  public:
                              avtSingleInputExpressionFilter();
    virtual                  ~avtSingleInputExpressionFilter();

    virtual const char       *GetType(void) 
                                  { return "avtSingleInputExpressionFilter"; };
    virtual const char       *GetDescription(void) = 0;
    virtual int               NumVariableArguments() { return 1; }

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *) = 0;
};

#endif
