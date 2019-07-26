// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtNeighborExpression.h                            //
// ************************************************************************* //

#ifndef AVT_NEIGHBOR_FILTER_H
#define AVT_NEIGHBOR_FILTER_H

#include <expression_exports.h>

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtNeighborExpression
//
//  Purpose:
//    This is a filter that takes a mesh and decomposes it into a mesh of
//    vertexes. The points are also assigned a variable based on the distance
//    to the next nearest node.
//
//  Programmer: Akira Haddox
//  Creation:   June 27, 2002
//
//  Modifications:
//
//    Hank Childs, Fri Feb 20 15:51:54 PST 2004
//    Re-define GetVariableDimension.
//
//    Eric Brugger, Wed Aug 20 16:18:18 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class EXPRESSION_API avtNeighborExpression : public avtSingleInputExpressionFilter
{
  public:
                             avtNeighborExpression();
    virtual                 ~avtNeighborExpression();

  protected:
    static const char        *variableName;

    virtual const char       *GetType(void)   { return "avtNeighborExpression"; };
    virtual const char       *GetDescription(void)
                             { return "Create vertex mesh,"
                                      " find distance to nearest node"; };

    // Used to fullfill parent's requirement, but unused since
    // ExecuteData exists for this derived class.
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex) { return NULL; }

    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    virtual void             UpdateDataObjectInfo(void);

    virtual bool             IsPointVariable()     { return true; };
    virtual int              GetVariableDimension()   { return 1; };
};

#endif
