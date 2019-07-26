// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtDistanceToBestFitLineExpression.h                 //
// ************************************************************************* //

#ifndef AVT_DISTANCE_TO_BEST_FIT_LINE_FILTER_H
#define AVT_DISTANCE_TO_BEST_FIT_LINE_FILTER_H

#include <avtBinaryMathExpression.h>

class     vtkDataArray;

// ****************************************************************************
//  Class: avtDistanceToBestFitLineExpression
//
//  Purpose:
//      Computes the best fit line for a 2-tuple of input variables and then,
//      as a second pass, calculates each data value's distance from that line.
//          
//  Programmer: Brad Whitlock
//  Creation:   Fri Nov 18 14:23:50 PST 2005
//
//  Modifications:
// 
//  Alister Maguire, Tue Nov 15 11:46:16 PST 2016
//  Added ThreadSafe method. 
//
// ****************************************************************************

class EXPRESSION_API avtDistanceToBestFitLineExpression 
    : public avtBinaryMathExpression
{
  public:
                              avtDistanceToBestFitLineExpression(bool v);
    virtual                  ~avtDistanceToBestFitLineExpression();

    virtual const char       *GetType(void)
                                 { return "avtDistanceToBestFitLineExpression"; }
    virtual const char       *GetDescription(void)
                                 { return "Distance to best fit line"; }
    virtual bool              ThreadSafe() { return true; };

  protected:
    bool                      verticalDifference;
    int                       pass;
    double                    sums[6];

    virtual void              PreExecute(void);
    virtual void              Execute(void);
    virtual void              DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                          vtkDataArray *out, int ncomps, int ntuples);
    virtual avtVarType        GetVariableType(void) { return AVT_SCALAR_VAR; };
};


#endif
