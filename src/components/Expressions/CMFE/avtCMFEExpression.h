// ************************************************************************* //
//                             avtCMFEExpression.h                           //
// ************************************************************************* //

#ifndef AVT_CMFE_EXPRESSION_H
#define AVT_CMFE_EXPRESSION_H

#include <avtExpressionFilter.h>
#include <avtSILRestriction.h>
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
//  Modifications:
//
//    Hank Childs, Fri Sep  9 09:38:08 PDT 2005
//    Add support for expressions.  Also add GetVariableDimension, which is
//    necessary for non-scalars.
//
//    Hank Childs, Thu Jan  5 15:34:17 PST 2006
//    Do a better job of getting centering of input variable.  Also add 
//    support for a "default" variable for when we are sampling onto 
//    non-overlapping meshes.
//
//    Hank Childs, Thu Jan 12 10:13:17 PST 2006
//    Change the number of variable arguments when we have a default
//    argument.
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
    virtual int               NumVariableArguments()
                                    { return (HasDefaultVariable() ? 2 : 1); };
    virtual int               GetVariableDimension() { return varDim; };
    virtual bool              IsPointVariable(void) { return isNodal; };
    virtual void              AddInputVariableName(const char *);

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
    avtSILRestriction_p       firstDBSIL;
    std::string               argument_expression;
    int                       varDim;
    bool                      isNodal;
    std::vector<std::string>  varnames;

    virtual void              PreExecute(void);
    virtual void              Execute(void);

    virtual avtDataTree_p     PerformCMFE(avtDataTree_p, avtDataTree_p,
                                          const std::string &, 
                                          const std::string &) = 0;
    virtual void              ExamineSpecification(avtPipelineSpecification_p);
    virtual bool              UseIdenticalSIL(void) { return false; };
    virtual bool              HasDefaultVariable(void) { return false; };
    int                       GetTimestate(ref_ptr<avtDatabase>);
};


#endif


