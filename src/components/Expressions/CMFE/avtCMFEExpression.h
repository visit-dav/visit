// ************************************************************************* //
//                             avtCMFEExpression.h                           //
// ************************************************************************* //

#ifndef AVT_CMFE_EXPRESSION_H
#define AVT_CMFE_EXPRESSION_H

#include <avtExpressionFilter.h>
#include <ExprNode.h>

class     vtkDataArray;
class     ArgsExpr;
class     ExprPipelineState;
class     ConstExpr;
class     avtDatabase;

// ****************************************************************************
//  Class: avtCMFEExpression
//
//  Purpose:
//      The base class for doing CMFE expressions.
//          
//  Programmer: Hank Childs
//  Creation:   August 26, 2005
//
// ****************************************************************************

class EXPRESSION_API avtCMFEExpression : public avtExpressionFilter
{
  public:
                              avtCMFEExpression();
    virtual                  ~avtCMFEExpression();

    virtual const char       *GetType(void){ return "avtCMFEExpression"; };
    virtual const char       *GetDescription(void)
                                           {return "Evaluating field";};
    virtual void              ProcessArguments(ArgsExpr*, ExprPipelineState *);
    virtual int               NumVariableArguments() { return 1; };

  protected:
    bool                      issuedWarning;
    std::string               db;
    std::string               var;
    TimeExpr::Type            timeType;
    int                       timeIndex;
    int                       cycle;
    double                    dtime;
    bool                      isDelta;
    int                       firstDBTime;

    virtual void              PreExecute(void);
    virtual void              Execute(void);

    virtual avtDataTree_p     PerformCMFE(avtDataTree_p, avtDataTree_p,
                                          const std::string &, 
                                          const std::string &) = 0;
    virtual void              ExamineSpecification(avtPipelineSpecification_p);
    int                       GetTimestate(ref_ptr<avtDatabase>);
};


#endif


