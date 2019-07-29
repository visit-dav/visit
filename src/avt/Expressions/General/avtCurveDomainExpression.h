// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtCurveDomainExpression.h                         //
// ************************************************************************* //

#ifndef AVT_CURVEDOMAIN_EXPRESSION_H
#define AVT_CURVEDOMAIN_EXPRESSION_H

#include <expression_exports.h>

#include <string>

#include <avtMultipleInputExpressionFilter.h>


// ****************************************************************************
//  Class: avtCurveDomainExpression
//
//  Purpose:
//      This is an abstract type that allows derived types to create 
//      expressions one VTK dataset at a time.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 5, 2009
//
// ****************************************************************************

class EXPRESSION_API avtCurveDomainExpression 
    : virtual public avtMultipleInputExpressionFilter
{
  public:
                             avtCurveDomainExpression();
    virtual                 ~avtCurveDomainExpression();

    virtual const char      *GetType() { return "avtCurveDomainExpression"; }
    virtual const char      *GetDescription() 
                                 { return "Modifying domain of curve"; }

    virtual int              NumVariableArguments() { return 2; }

  protected:

    virtual vtkDataSet      *ExecuteData(vtkDataSet *, int, std::string);
    virtual vtkDataArray    *DeriveVariable(vtkDataSet *, int currentDomainsIndex) { return NULL;}
    virtual avtVarType       GetVariableType(void) { return AVT_CURVE; };
};


#endif
