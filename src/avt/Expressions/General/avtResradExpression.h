// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtResradExpression.h                            //
// ************************************************************************* //

#ifndef AVT_RESRAD_FILTER_H
#define AVT_RESRAD_FILTER_H

#include <avtBinaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtResradExpression
//
//  Purpose:
//      A filter that calculates resrad -- which adjusts the resolution
//      using a monte carlo resampling with a given radius.
//
//  Programmer: Hank Childs
//  Creation:   January 20, 2005
//
// ****************************************************************************

class EXPRESSION_API avtResradExpression : public avtBinaryMathExpression
{
  public:
                              avtResradExpression();
    virtual                  ~avtResradExpression();

    virtual const char       *GetType(void) 
                                 { return "avtResradExpression"; };
    virtual const char       *GetDescription(void)
                                 { return "Calculating resrad"; };

  protected:
    bool             haveIssuedWarning;

    virtual void     PreExecute(void);
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                 vtkDataArray *out, int ncomps, int ntuples);
};


#endif


