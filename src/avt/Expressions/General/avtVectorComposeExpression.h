// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtVectorComposeExpression.h                         //
// ************************************************************************* //

#ifndef AVT_VECTOR_COMPOSE_FILTER_H
#define AVT_VECTOR_COMPOSE_FILTER_H

#include <avtMultipleInputExpressionFilter.h>


// ****************************************************************************
//  Class: avtVectorComposeExpression
//
//  Purpose:
//      Creates a vector variable out of three components.
//
//  Programmer: Sean Ahern
//  Creation:   Thu Mar  6 19:15:24 America/Los_Angeles 2003
//
//  Modifications:
//      Sean Ahern, Fri Jun 13 11:39:45 PDT 2003
//      Added NumVariableArguments, specifying that it processes three
//      variable arguments.
//
//      Hank Childs, Thu Feb  5 17:11:06 PST 2004
//      Moved inlined constructor and destructor definitions to .C files
//      because certain compilers have problems with them.
//
//      Hank Childs, Sun Jan 13 20:11:35 PST 2008
//      Add support for singleton constants.
//
// ****************************************************************************

class EXPRESSION_API avtVectorComposeExpression 
    : public avtMultipleInputExpressionFilter
{
  public:
                              avtVectorComposeExpression();
    virtual                  ~avtVectorComposeExpression();

    virtual const char       *GetType(void)  
                                    { return "avtVectorComposeExpression"; };
    virtual const char       *GetDescription(void)
                                 {return "Creating a vector from components";};
    virtual int               NumVariableArguments() { return 3; }

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual int               GetVariableDimension(void);
    virtual bool              CanHandleSingletonConstants(void) {return true;};
};


#endif


