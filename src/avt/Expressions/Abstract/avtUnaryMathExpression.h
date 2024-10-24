// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtUnaryMathExpression.h                       //
// ************************************************************************* //

#ifndef AVT_UNARY_MATH_FILTER_H
#define AVT_UNARY_MATH_FILTER_H

#include <expression_exports.h>

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtUnaryMathExpression
//
//  Purpose:
//      A filter that performs a calculation on a single variable.
//
//  Programmer: Sean Ahern
//  Creation:   Wed Jun 12 16:43:09 PDT 2002
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs, Tue Aug 16 09:03:39 PDT 2005
//    Add data member cur_mesh so derived types can access mesh when
//    deriving variables.
//
//    Hank Childs, Fri Aug 19 14:04:25 PDT 2005
//    Do a better job of determining the variable dimension.
//
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//    Unary math filters can handle these with no modifications.
//
//    Hank Childs, Sun Jan 13 20:07:56 PST 2008
//    Add support for constants creating a singleton.
//
//    Hank Childs, Thu Oct  9 09:44:37 PDT 2008
//    Define method "NullInputIsExpected".
//
// ****************************************************************************

class EXPRESSION_API avtUnaryMathExpression : public avtSingleInputExpressionFilter
{
  public:
                              avtUnaryMathExpression();
    virtual                  ~avtUnaryMathExpression();

    virtual const char       *GetType(void)   { return "avtUnaryMathExpression";};
    virtual const char       *GetDescription(void) = 0;

    virtual bool              NullInputIsExpected(void) { return false; };

    static std::vector<int>   IdentifyGhostedNodes(vtkDataSet *in_ds,
                                                   vtkDataArray *ghost_zones,
                                                   vtkDataArray *ghost_nodes);

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples, 
                                          vtkDataSet *in_ds) = 0;
    virtual vtkDataArray     *CreateArray(vtkDataArray *);
    virtual int               GetNumberOfComponentsInOutput(int numInInput)
                                          { return numInInput; };
    virtual int               GetVariableDimension(void);

    virtual bool              FilterUnderstandsTransformedRectMesh();
    virtual bool              FilterCreatesSingleton(void) { return false; };
    virtual bool              CanHandleSingletonConstants(void){ return true; };

    avtCentering              centering;
    vtkDataSet               *cur_mesh;
};


#endif


