// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtRadianToDegreeExpression.h                     //
// ************************************************************************* //

#ifndef AVT_RADIAN_TO_DEGREE_FILTER_H
#define AVT_RADIAN_TO_DEGREE_FILTER_H

#include <avtUnaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtRadianToDegreeExpression
//
//  Purpose:
//      Converts radian angles to degree angles.
//
//  Programmer: Hank Childs
//  Creation:   November 18, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class EXPRESSION_API avtRadianToDegreeExpression : public avtUnaryMathExpression
{
  public:
                              avtRadianToDegreeExpression();
    virtual                  ~avtRadianToDegreeExpression();

    virtual const char       *GetType(void) 
                                 { return "avtRadianToDegreeExpression"; };
    virtual const char       *GetDescription(void) 
                                 { return "Converting radians to degrees"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples, vtkDataSet *in_ds);
};


#endif


