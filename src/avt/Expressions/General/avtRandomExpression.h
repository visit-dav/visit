// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtRandomExpression.h                             //
// ************************************************************************* //

#ifndef AVT_RANDOM_FILTER_H
#define AVT_RANDOM_FILTER_H

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;
class     ArgsExpr;
class     ExprPipelineState;

// ****************************************************************************
//  Class: avtRandomExpression
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
//    Hank Childs, Mon Jul 10 09:03:13 PDT 2006
//    Added PreExecute.
//
//    Hank Childs, Thu Aug 24 11:24:41 PDT 2006
//    Remove PreExecute, since it was not successfully accomplishing its
//    purpose (setting up the seed for the random number generator in a
//    reproducible way).
//
//    Hank Childs, Wed Aug 22 09:50:06 PDT 2007
//    Declare variable dimension to always be 1.
//
// ****************************************************************************

class EXPRESSION_API avtRandomExpression : public avtSingleInputExpressionFilter
{
  public:
                              avtRandomExpression();
    virtual                  ~avtRandomExpression();

    virtual const char       *GetType(void) { return "avtRandomExpression"; };
    virtual const char       *GetDescription(void)
                                           {return "Assigning random #.";};
    virtual void              ProcessArguments(ArgsExpr*, ExprPipelineState *);
  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual bool              IsPointVariable(void)  { return true; };
    virtual int               GetVariableDimension() { return 1; }
};


#endif


