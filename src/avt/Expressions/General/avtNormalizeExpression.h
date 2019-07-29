// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtNormalizeExpression.h                           //
// ************************************************************************* //

#ifndef AVT_NORMALIZE_FILTER_H
#define AVT_NORMALIZE_FILTER_H

#include <avtSingleInputExpressionFilter.h>


// ****************************************************************************
//  Class: avtNormalizeExpression
//
//  Purpose:
//      A filter that takes vector data and creates a new vector variable that
//      is the normalized version of the input.
//
//  Programmer: Sean Ahern
//  Creation:   Wed Feb  7 19:02:19 EST 2007
//
//  Modifications:
//
//    Hank Childs, Mon Jan 14 17:58:58 PST 2008
//    Allow constants to be created as singletons.
//
// ****************************************************************************

class EXPRESSION_API avtNormalizeExpression : public avtSingleInputExpressionFilter
{
  public:
                              avtNormalizeExpression();
    virtual                  ~avtNormalizeExpression();

    virtual const char       *GetType(void)   { return "avtNormalizeExpression"; };
    virtual const char       *GetDescription(void) { return "Normalizing Vector"; };

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual int               GetVariableDimension() { return 3; }
    virtual bool              CanHandleSingletonConstants(void) {return true;};
};


#endif
