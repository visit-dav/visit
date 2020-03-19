// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtDegreeExpression.h                             //
// ************************************************************************* //

#ifndef AVT_DEGREE_FILTER_H
#define AVT_DEGREE_FILTER_H


#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtDegreeExpression
//
//  Purpose:
//      A filter that creates a new vertex centered variable that counts the
//      number of cells incident to each vertex.
//
//  Programmer: Hank Childs
//  Creation:   June 7, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class EXPRESSION_API avtDegreeExpression : public avtSingleInputExpressionFilter
{
  public:
                              avtDegreeExpression();
    virtual                  ~avtDegreeExpression();

    virtual const char       *GetType(void)   { return "avtDegreeExpression"; };
    virtual const char       *GetDescription(void)
                                 { return "Calculating Degree of Each Node"; };

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
};


#endif


