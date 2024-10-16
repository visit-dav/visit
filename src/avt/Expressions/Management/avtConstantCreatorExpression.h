// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtConstantCreatorExpression.h                     //
// ************************************************************************* //

#ifndef AVT_CONSTANT_CREATOR_FILTER_H
#define AVT_CONSTANT_CREATOR_FILTER_H

#include <avtUnaryMathExpression.h>


// ****************************************************************************
//  Class: avtConstantCreatorExpression
//
//  Purpose:
//      This class creates constant variables.
//
//  Programmer: Sean Ahern
//  Creation:   Fri Feb 21 23:28:13 America/Los_Angeles 2003
//
//  Modifications:
//
//    Hank Childs, Mon Nov  3 15:15:47 PST 2003
//    Override base class' definition of CreateArray.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs, Mon Jan 23 11:32:14 PST 2006
//    Make sure constants are always scalars.
//
//    Hank Childs, Sun Jan 13 20:07:56 PST 2008
//    Allow constants to be created as singletons.
//
//    Hank Childs, Thu Oct  9 09:44:37 PDT 2008
//    Define method "NullInputIsExpected".
//
//    Kevin Griffin, Wed Aug 17 19:40:04 PDT 2016
//    Define method "SetFilterCreatesSingleton".
//
// ****************************************************************************

class EXPRESSION_API avtConstantCreatorExpression 
    : public avtUnaryMathExpression
{
  public:
                             avtConstantCreatorExpression();
    virtual                 ~avtConstantCreatorExpression();

    void                     SetFilterCreatesSingleton(bool b) { isSingleton = b; }
    void                     SetValue(double v) {value = v;}
    virtual const char *     GetType(void) 
                                         { return "avtConstantCreatorExpression"; }
    virtual const char *     GetDescription(void) 
                                         { return "Generating constant"; }

    virtual bool             NullInputIsExpected(void) { return true; };

  protected:
    virtual void             DoOperation(vtkDataArray *in, vtkDataArray *out,
                                         int ncomponents, int ntuples, vtkDataSet *in_ds);
    virtual int              GetNumberOfComponentsInOutput(int) { return 1; };
    virtual int              GetVariableDimension(void) { return 1; };
    virtual vtkDataArray    *CreateArray(vtkDataArray *);
    virtual bool             FilterCreatesSingleton(void) { return isSingleton; };
    virtual bool             CanHandleSingletonConstants(void) {return true;};

    double                   value;
    bool                     isSingleton;
};


#endif


