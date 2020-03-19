// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                    avtHSVColorComposeExpression.h                         //
// ************************************************************************* //

#ifndef AVT_HSV_COLOR_COMPOSE_FILTER_H
#define AVT_HSV_COLOR_COMPOSE_FILTER_H

#include <avtMultipleInputExpressionFilter.h>


// ****************************************************************************
//  Class: avtHSVColorComposeExpression
//
//  Purpose:
//      Creates a vector variable out of three components.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 30, 2007
//
//  Note:  "embraced and extended" from avtColorComposeExpression
//
//  Modifications:
//
// ****************************************************************************

class EXPRESSION_API avtHSVColorComposeExpression 
    : public avtMultipleInputExpressionFilter
{
  public:
                              avtHSVColorComposeExpression();
    virtual                  ~avtHSVColorComposeExpression();

    virtual const char       *GetType(void)  
                                    { return "avtHSVColorComposeExpression"; }
    virtual const char       *GetDescription(void)
                                 {return "Creating a color from HSV values";}
    virtual int               NumVariableArguments() { return 3; }

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual int               GetVariableDimension(void) { return 3; }
};


#endif


