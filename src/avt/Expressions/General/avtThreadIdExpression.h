// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtThreadIdExpression.h                             //
// ************************************************************************* //

#ifndef AVT_THREAD_ID_FILTER_H
#define AVT_THREAD_ID_FILTER_H

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtThreadIdExpression
//
//  Purpose:
//      Identifies the thread id for each dataset block.  This is mostly good for
//      debugging.
//          
//  Programmer: David Camp
//  Creation:   Feb 13, 2013
//
//  Modifications:
//
// ****************************************************************************

class EXPRESSION_API avtThreadIdExpression : public avtSingleInputExpressionFilter
{
  public:
                              avtThreadIdExpression();
    virtual                  ~avtThreadIdExpression();

    virtual const char       *GetType(void) { return "avtThreadIdExpression"; };
    virtual const char       *GetDescription(void)
                                           {return "Assigning thread ID.";};
  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual bool              IsPointVariable(void)  { return true; };
    virtual int               GetVariableDimension() { return 1; }
};

#endif

