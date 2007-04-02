// ************************************************************************* //
//                          avtDistanceToBestFitLineFilter.h                 //
// ************************************************************************* //

#ifndef AVT_DISTANCE_TO_BEST_FIT_LINE_FILTER_H
#define AVT_DISTANCE_TO_BEST_FIT_LINE_FILTER_H

#include <avtBinaryMathFilter.h>

class     vtkDataArray;

// ****************************************************************************
//  Class: avtDistanceToBestFitLineFilter
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
// ****************************************************************************

class EXPRESSION_API avtDistanceToBestFitLineFilter 
    : public avtBinaryMathFilter
{
  public:
                              avtDistanceToBestFitLineFilter(bool v);
    virtual                  ~avtDistanceToBestFitLineFilter();

    virtual const char       *GetType(void)
                                 { return "avtDistanceToBestFitLineFilter"; }
    virtual const char       *GetDescription(void)
                                 { return "Distance to best fit line"; }

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
