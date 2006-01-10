// ************************************************************************* //
//                           avtConnCMFEExpression.h                         //
// ************************************************************************* //

#ifndef AVT_CONN_CMFE_EXPRESSION_H
#define AVT_CONN_CMFE_EXPRESSION_H

#include <avtCMFEExpression.h>

class     vtkDataArray;
class     ArgsExpr;
class     ExprPipelineState;
class     ConstExpr;

// ****************************************************************************
//  Class: avtConnCMFEExpression
//
//  Purpose:
//      Does a connectivity based cross-mesh field evaluation.
//          
//  Programmer: Hank Childs
//  Creation:   August 26, 2005
//
//  Modifications:
//
//    Hank Childs, Mon Oct 10 17:08:05 PDT 2005
//    Remove virtual designation from ExecuteTree, since its not really 
//    intended to be a virtual method.
//
// ****************************************************************************

class EXPRESSION_API avtConnCMFEExpression : public avtCMFEExpression
{
  public:
                              avtConnCMFEExpression();
    virtual                  ~avtConnCMFEExpression();

    virtual const char       *GetType(void){ return "avtConnCMFEExpression"; };
    virtual const char       *GetDescription(void)
                                           {return "Evaluating field";};
  protected:
    virtual avtDataTree_p     PerformCMFE(avtDataTree_p, avtDataTree_p,
                                          const std::string &,
                                          const std::string &);
    avtDataTree_p             ExecuteTree(avtDataTree_p, avtDataTree_p,
                                          const std::string &,
                                          const std::string &);
    virtual bool              UseIdenticalSIL(void) { return true; };
};


#endif


