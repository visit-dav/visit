// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtVectorDecomposeExpression.h                       //
// ************************************************************************* //

#ifndef AVT_VECTOR_DECOMPOSE_FILTER_H
#define AVT_VECTOR_DECOMPOSE_FILTER_H

#include <avtSingleInputExpressionFilter.h>


// ****************************************************************************
//  Class: avtVectorDecomposeExpression
//
//  Purpose:
//      Gets a component of a vector variable.
//
//  Programmer: Hank Childs
//  Creation:   November 18, 2002
//
//  Modifications:
//      Sean Ahern, Thu Mar  6 01:59:23 America/Los_Angeles 2003
//      Merged the vector component filters together.
//
//      Hank Childs, Thu Feb  5 17:11:06 PST 2004
//      Moved inlined constructor and destructor definitions to .C files
//      because certain compilers have problems with them.
//
// ****************************************************************************

class EXPRESSION_API avtVectorDecomposeExpression 
    : public avtSingleInputExpressionFilter
{
  public:
                              avtVectorDecomposeExpression(int w);
    virtual                  ~avtVectorDecomposeExpression();

    virtual const char       *GetType(void)  
                               {return "avtVectorDecomposeExpression";};
    virtual const char       *GetDescription(void)
                               {return "Pulling out a component of a vector";};

  protected:
    int                       which_comp;

    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual int               GetVariableDimension();
};


#endif


