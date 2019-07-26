// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtTimeIteratorExpression.h                      //
// ************************************************************************* //

#ifndef AVT_TIME_ITERATOR_EXPRESSION_H
#define AVT_TIME_ITERATOR_EXPRESSION_H

#include <avtExpressionFilter.h>


// ****************************************************************************
//  Class: avtTimeIteratorExpression
//
//  Purpose:
//      This is a base class for all derived types of time iterator expressions.
//      Its primary role is to (1) handle AVT magic to get multiple time slices
//      read in and (2) process the arguments from the expression language.
//
//  Notes:      This class seems very much like it _should_ inherit from the
//              time loop filter.  Although they are similar, there is a subtle
//              difference.  This is that this module always operates on the
//              mesh from the current time slice and uses CMFEs to apply the 
//              fields from different time slices onto that mesh, where a time
//              loop filter isn't as concerned about correlating things from
//              time slice to time slice.
//
//  Programmer: Hank Childs
//  Creation:   February 14, 2009
//
// ****************************************************************************

typedef enum
{
    CONN_CMFE,
    POS_CMFE
} CMFEType;


class EXPRESSION_API avtTimeIteratorExpression 
    : public avtExpressionFilter
{
  public:
                              avtTimeIteratorExpression();
    virtual                  ~avtTimeIteratorExpression();

    virtual const char       *GetType(void)   
                                  { return "avtTimeIteratorExpression"; };
    virtual const char       *GetDescription(void)
                                  { return "Iterating over time"; };

    virtual void              Execute(void);
    virtual void              ProcessArguments(ArgsExpr*, ExprPipelineState *);
    virtual int               NumVariableArguments(void)
                                  { return NumberOfVariables()+
                                           (cmfeType == POS_CMFE ? 1: 0); };
    virtual void              AddInputVariableName(const char *);

  protected:
    int                       firstTimeSlice;
    int                       lastTimeSlice;
    int                       timeStride;
    int                       numTimeSlicesToProcess;
    int                       actualLastTimeSlice;

    CMFEType                  cmfeType;
    std::vector<std::string>  varnames;
    avtContract_p             executionContract;

    int                       currentCycle;
    double                    currentTime;

    virtual void              ProcessDataTree(avtDataTree_p, int ts) = 0;
    virtual void              InitializeOutput(void) = 0;
    virtual void              FinalizeOutput(void) = 0;
    virtual avtContract_p     ModifyContract(avtContract_p);
    virtual bool              IsPointVariable(void);
    virtual bool              VariableComesFromCurrentTime(int) { return false; };

    virtual int               NumberOfVariables(void) = 0;

    void                      FinalizeTimeLoop(void);
    avtContract_p             ConstructContractWithVarnames(void);
    void                      UpdateExpressions(int);

    std::string               GetInternalVarname(int);
};


#endif


