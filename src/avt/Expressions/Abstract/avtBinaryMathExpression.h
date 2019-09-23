// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtBinaryMathExpression.h                         //
// ************************************************************************* //

#ifndef AVT_BINARY_MATH_FILTER_H
#define AVT_BINARY_MATH_FILTER_H

#include <expression_exports.h>
#include <avtMultipleInputExpressionFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtBinaryMathExpression
//
//  Purpose:
//      A filter that performs a calculation on two variables.
//
//  Programmer: Sean Ahern
//  Creation:   Tue Jun 25 19:40:54 PDT 2002
//
//  Modifications:
//      Sean Ahern, Fri Jun 13 11:16:09 PDT 2003
//      Added the NumVariableArguments function, specifying that all
//      subclasses of BinaryMathFilter process two variable arguments.
//
//      Hank Childs, Thu Feb  5 17:11:06 PST 2004
//      Moved inlined constructor and destructor definitions to .C files
//      because certain compilers have problems with them.
//
//      Hank Childs, Thu Jan 20 17:30:09 PST 2005
//      Add data member cur_mesh so derived types can access mesh when 
//      deriving variables.
//
//      Hank Childs, Fri Aug 19 14:04:25 PDT 2005
//      Do a better job of determining the variable dimension.
//
//      Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//      Added support for rectilinear grids with an inherent transform.
//      Binary math filters can handle these with no modifications.
//
//      Kathleen Biagas, Thu Apr 5 08:55:32 PDT 2012
//      Added second array to CreateArray method.
//
// ****************************************************************************

class EXPRESSION_API avtBinaryMathExpression 
    : public avtMultipleInputExpressionFilter
{
  public:
                              avtBinaryMathExpression();
    virtual                  ~avtBinaryMathExpression();

    virtual const char       *GetType(void)   { return "avtBinaryMathExpression";};
    virtual const char       *GetDescription(void) = 0;
    virtual int               NumVariableArguments() { return 2; }

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual vtkDataArray     *CreateArray(vtkDataArray *, vtkDataArray *);
    virtual void              DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                          vtkDataArray *out, int, int) = 0;
    virtual int               GetNumberOfComponentsInOutput(int ncompsIn1,
                                                            int ncompsIn2)
                                         { return (ncompsIn1 > ncompsIn2
                                                    ? ncompsIn1 
                                                    : ncompsIn2);
                                         };
    virtual int               GetVariableDimension(void);

    virtual bool              FilterUnderstandsTransformedRectMesh();

    avtCentering              centering;
    vtkDataSet               *cur_mesh;
};


#endif


