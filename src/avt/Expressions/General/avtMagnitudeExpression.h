// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtMagnitudeExpression.h                           //
// ************************************************************************* //

#ifndef AVT_MAGNITUDE_FILTER_H
#define AVT_MAGNITUDE_FILTER_H


#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtMagnitudeExpression
//
//  Purpose:
//      A filter that takes vector data and creates a scalar variable based on
//      its magnitude. 
//          
//  Programmer: Matthew Haddox
//  Creation:   July 30, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs, Mon Jan 23 09:45:07 PST 2006
//    Declare the correct variable dimension.
//
//    Hank Childs, Mon Jan 14 17:58:58 PST 2008
//    Allow constants to be created as singletons.
//
//    Eric Brugger, Thu Dec  8 15:03:57 PST 2022
//    I removed the DeriveVariableVTKm method since vtkmExpressions is
//    no longer supported.
//
// ****************************************************************************

class EXPRESSION_API avtMagnitudeExpression : public avtSingleInputExpressionFilter
{
  public:
                              avtMagnitudeExpression();
    virtual                  ~avtMagnitudeExpression();

    virtual const char       *GetType(void)   { return "avtMagnitudeExpression"; };
    virtual const char       *GetDescription(void)
                              {return "Calculating Magnitude of Each Vector";};
  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);

    virtual int               GetVariableDimension(void) { return 1; };
    virtual bool              CanHandleSingletonConstants(void) {return true;};
};


#endif


