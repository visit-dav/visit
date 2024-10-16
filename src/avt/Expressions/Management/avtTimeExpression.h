// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtTimeExpression.h                           //
// ************************************************************************* //

#ifndef AVT_TIME_EXPRESSION_H
#define AVT_TIME_EXPRESSION_H

#include <avtUnaryMathExpression.h>


// ****************************************************************************
//  Class: avtTimeExpression
//
//  Purpose:
//      Creates a constant, where the constant is the time.
//
//  Programmer: Hank Childs
//  Creation:   March 5, 2006
//
//  Modifications:
//
//    Hank Childs, Mon Jan 14 20:47:27 PST 2008
//    Allow constants to be created as singletons.
//
//    Hank Childs, Thu Oct  9 09:44:37 PDT 2008
//    Define method "NullInputIsExpected".
//
//    Jeremy Meredith, Wed Mar 11 12:35:18 EDT 2009
//    Added support for cycle and timestep values.
//
// ****************************************************************************

class EXPRESSION_API avtTimeExpression : public avtUnaryMathExpression
{
  public:
    enum TimeMode {MODE_TIME, MODE_CYCLE, MODE_INDEX};

                             avtTimeExpression(TimeMode);
    virtual                 ~avtTimeExpression();

    virtual const char *     GetType(void) 
                                         { return "avtTimeExpression"; };
    virtual const char *     GetDescription(void) 
                                     { return "Generating constant (time)"; };

    virtual bool             NullInputIsExpected(void) { return true; };

  protected:
    virtual void             DoOperation(vtkDataArray *in, vtkDataArray *out,
                                         int ncomponents, int ntuples, vtkDataSet *in_ds);
    virtual int              GetNumberOfComponentsInOutput(int) { return 1; };
    virtual int              GetVariableDimension(void) { return 1; };
    virtual vtkDataArray    *CreateArray(vtkDataArray *);
    virtual bool             FilterCreatesSingleton(void) { return true; };
    virtual bool             CanHandleSingletonConstants(void) {return true;};

    TimeMode mode;
};


#endif


