// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                    avtValueAtExtremaExpression.h                          //
// ************************************************************************* //

#ifndef AVT_VALUE_AT_EXTREMA_EXPRESSION_H
#define AVT_VALUE_AT_EXTREMA_EXPRESSION_H

#include <avtTimeIteratorDataTreeIteratorExpression.h>


// ****************************************************************************
//  Class: avtValueAtExtremaExpression
//
//  Purpose:
//      A derived type of time iterator/data tree iterator that calculates the
//      time/cycle/time index/separate variable at the minimum/maximum.
//
//  Programmer: Hank Childs
//  Creation:   February 16, 2009
//
// ****************************************************************************

typedef enum
{
    VE_OUTPUT_TIME,
    VE_OUTPUT_CYCLE,
    VE_OUTPUT_TIME_INDEX,
    VE_OUTPUT_VARIABLE
} VE_OutputType;


class EXPRESSION_API avtValueAtExtremaExpression 
    : public avtTimeIteratorDataTreeIteratorExpression
{
  public:
                              avtValueAtExtremaExpression();
    virtual                  ~avtValueAtExtremaExpression();

    virtual const char       *GetType(void)   
                               { return "avtValueAtExtremaExpression"; };
    virtual const char       *GetDescription(void)   
                               { return "Calculating value at extrema";};

    void                      SetAtMaximum(bool b) {atMaximum=b;};
    void                      SetOutputType(VE_OutputType t) {outputType=t;};

  protected:
    VE_OutputType             outputType;
    bool                      atMaximum;

    virtual void              ExecuteDataset(std::vector<vtkDataArray *> &, 
                                             vtkDataArray *, int ts);
    virtual int               NumberOfVariables(void) 
                                   { return (outputType == VE_OUTPUT_VARIABLE 
                                             ? 2 : 1); };
    virtual bool              CanHandleSingtonConstants(void) {return true;};

    virtual int               GetIntermediateSize(void)
                                    { return 1+GetVariableDimension(); };
    virtual vtkDataArray     *ConvertIntermediateArrayToFinalArray(vtkDataArray *);
};


#endif


