// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtColorComposeExpression.h                         //
// ************************************************************************* //

#ifndef AVT_COLOR_COMPOSE_FILTER_H
#define AVT_COLOR_COMPOSE_FILTER_H

#include <avtMultipleInputExpressionFilter.h>


// ****************************************************************************
//  Class: avtColorComposeExpression
//
//  Purpose:
//      Creates a vector variable out of three components.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Apr 23 17:08:08 PST 2007
//
//  Modifications:
//
// ****************************************************************************

class EXPRESSION_API avtColorComposeExpression 
    : public avtMultipleInputExpressionFilter
{
  public:
                              avtColorComposeExpression(int nc);
    virtual                  ~avtColorComposeExpression();

    virtual const char       *GetType(void)  
                                    { return "avtColorComposeExpression"; }
    virtual const char       *GetDescription(void)
                                 {return "Creating a color vector from components";};
    virtual int               NumVariableArguments() { return ncomp; }

  protected:
    int ncomp;
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual int               GetVariableDimension(void) { return 3; }
};


#endif


