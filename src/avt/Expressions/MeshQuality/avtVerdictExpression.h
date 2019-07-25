// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtVerdictExpression.h                          //
// ************************************************************************* //

// Caveat: Verdict filters currently support triangles, but not triangle strips

#ifndef AVT_VERDICT_FILTER_H
#define AVT_VERDICT_FILTER_H

#include <expression_exports.h>

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtVerdictExpression
//
//  Purpose:
//    This is a abstract base class for the verdict metric filters.
//
//  Programmer: Akira Haddox
//  Creation:   June 13, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
//    Hank Childs, Thu May 19 10:44:29 PDT 2005
//    Add support for sub-types operating directly on the mesh.
//
//    Hank Childs, Mon Aug 28 10:29:49 PDT 2006
//    Declare the variable dimension.
//
//    Hank Childs, Thu Jul 24 12:49:19 PDT 2008
//    Added a virtual method to help with supporting polygonal and polyhedral
//    data.
//
// ****************************************************************************

class EXPRESSION_API avtVerdictExpression : public avtSingleInputExpressionFilter
{
  public:
                              avtVerdictExpression();
    virtual                  ~avtVerdictExpression();

    virtual void              PreExecute();

    virtual const char       *GetType(void)   { return "avtVerdictExpression"; };
    virtual const char       *GetDescription(void)
                                 { return "Calculating Verdict expression."; };

    virtual bool              OperateDirectlyOnMesh(vtkDataSet *)
                                       { return false; };
    virtual void              MetricForWholeMesh(vtkDataSet *, vtkDataArray *);

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);

    virtual double            Metric(double coordinates[][3], int type) = 0;

    virtual bool              RequiresSizeCalculation() { return false; };
    virtual bool              SummationValidForOddShapes(void) { return false; };

    virtual bool              IsPointVariable() {  return false; };
    virtual int               GetVariableDimension(void) { return 1; };
};

#endif
