// ************************************************************************* //
//                             avtRandomFilter.h                             //
// ************************************************************************* //

#ifndef AVT_RANDOM_FILTER_H
#define AVT_RANDOM_FILTER_H

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;
class     ArgsExpr;
class     ExprPipelineState;

// ****************************************************************************
//  Class: avtRandomFilter
//
//  Purpose:
//      Creates a random number at each point.  Mostly used for odd effects in
//      movie-making.
//          
//  Programmer: Hank Childs
//  Creation:   March 7, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class EXPRESSION_API avtRandomFilter : public avtSingleInputExpressionFilter
{
  public:
                              avtRandomFilter();
    virtual                  ~avtRandomFilter();

    virtual const char       *GetType(void) { return "avtRandomFilter"; };
    virtual const char       *GetDescription(void)
                                           {return "Assigning random #.";};
    virtual void              ProcessArguments(ArgsExpr*, ExprPipelineState *);
  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual bool              IsPointVariable(void)  { return true; };
};


#endif


