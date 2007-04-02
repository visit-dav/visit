// ************************************************************************* //
//                            avtPosCMFEExpression.h                         //
// ************************************************************************* //

#ifndef AVT_POS_CMFE_EXPRESSION_H
#define AVT_POS_CMFE_EXPRESSION_H

#include <avtCMFEExpression.h>

#include <avtIntervalTree.h>

class     vtkCell;
class     vtkDataArray;
class     ArgsExpr;
class     ExprPipelineState;
class     ConstExpr;


// ****************************************************************************
//  Class: avtPosCMFEExpression
//
//  Purpose:
//      Does a position based cross-mesh field evaluation.
//          
//  Programmer: Hank Childs
//  Creation:   October 10, 2005
//
// ****************************************************************************

class EXPRESSION_API avtPosCMFEExpression : public avtCMFEExpression
{
  public:
                              avtPosCMFEExpression();
    virtual                  ~avtPosCMFEExpression();

    virtual const char       *GetType(void){ return "avtPosCMFEExpression"; };
    virtual const char       *GetDescription(void)
                                           {return "Evaluating field";};
  protected:
    virtual avtDataTree_p     PerformCMFE(avtDataTree_p, avtDataTree_p,
                                          const std::string &,
                                          const std::string &);
    virtual bool              UseIdenticalSIL(void) { return false; };
    virtual bool              HasDefaultVariable(void) { return true; };
};


#endif


