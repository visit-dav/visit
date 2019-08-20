// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtNodeDegreeExpression.h                        //
// ************************************************************************* //

#ifndef AVT_NODE_DEGREE_FILTER_H
#define AVT_NODE_DEGREE_FILTER_H

#include <expression_exports.h>

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtNodeDegreeExpression
//
//  Purpose:
//      A filter that creates a new vertex centered variable that counts the
//      number of unique edges that go through each vertex.
//
//  Programmer: Akira Haddox
//  Creation:   June 27, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs, Wed May 21 15:55:53 PDT 2008
//    Re-define GetVariableDimension.
//
// ****************************************************************************

class EXPRESSION_API avtNodeDegreeExpression : public avtSingleInputExpressionFilter
{
  public:
                              avtNodeDegreeExpression();
    virtual                  ~avtNodeDegreeExpression();

    virtual const char       *GetType(void)  { return "avtNodeDegreeExpression"; };
    virtual const char       *GetDescription(void)
                             { return "Calculating NodeDegree of Each Node"; };

  protected:
    virtual bool              IsPointVariable(void) { return true; };
    virtual int               GetVariableDimension()   { return 1; };
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
};


#endif


