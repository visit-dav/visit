// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                  avtWhenConditionIsTrueExpression.h                       //
// ************************************************************************* //

#ifndef AVT_WHEN_CONDITION_IS_TRUE_EXPRESSION_H
#define AVT_WHEN_CONDITION_IS_TRUE_EXPRESSION_H

#include <avtTimeIteratorDataTreeIteratorExpression.h>


// ****************************************************************************
//  Class: avtWhenConditionIsTrueExpression
//
//  Purpose:
//      A derived type of time iterator/data tree iterator that calculates the
//      time/cycle/time index/separate variable when a condition is first/last
//      true.
//
//  Programmer: Hank Childs
//  Creation:   February 16, 2009
//
// ****************************************************************************

typedef enum
{
    WCT_OUTPUT_TIME,
    WCT_OUTPUT_CYCLE,
    WCT_OUTPUT_TIME_INDEX,
    WCT_OUTPUT_VARIABLE
} WCT_OutputType;


class EXPRESSION_API avtWhenConditionIsTrueExpression 
    : public avtTimeIteratorDataTreeIteratorExpression
{
  public:
                              avtWhenConditionIsTrueExpression();
    virtual                  ~avtWhenConditionIsTrueExpression();

    virtual const char       *GetType(void)   
                               { return "avtWhenConditionIsTrueExpression"; };
    virtual const char       *GetDescription(void)   
                               { return "Calculating when condition is true";};

    void                      SetWhenConditionIsFirstTrue(bool b){firstTrue=b;};
    void                      SetOutputType(WCT_OutputType t) {outputType=t;};

  protected:
    WCT_OutputType            outputType;
    bool                      firstTrue;

    virtual void              ExecuteDataset(std::vector<vtkDataArray *> &, 
                                             vtkDataArray *, int ts);
    virtual int               NumberOfVariables(void) 
                                   { return (outputType == WCT_OUTPUT_VARIABLE 
                                             ? 3 : 2); };
    virtual bool              VariableComesFromCurrentTime(int v) 
                                       { return (v==NumberOfVariables()-1); };
    virtual bool              CanHandleSingtonConstants(void) {return true;};
};


#endif


