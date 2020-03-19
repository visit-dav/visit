// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_EXPR_NODE_H
#define AVT_EXPR_NODE_H

#include <ExprNode.h>
#include <expression_exports.h>

class avtExpressionFilter;
class ExprPipelineState;

//  Modifications:
//    Jeremy Meredith, Wed Nov 24 12:24:12 PST 2004
//    Renamed Engine to avt.
//
//    Jeremy Meredith, Mon Jun 13 15:46:22 PDT 2005
//    Made ConstExpr abstract and split it into multiple concrete
//    base classes.  Made FunctionExpr and MachExpr use names
//    instead of Identifier tokens.  These two changes were to
//    remove Token references from the parse tree node classes.
//
//    Kathleen Bonnell, Fri Nov 17 08:32:54 PST 2006 
//    Added private CreateFilters(string) for FuncExpr class. 
//
//    Jeremy Meredith, Thu Aug  7 14:34:01 EDT 2008
//    Reorder constructor initializers to be the correct order.
//
//    Kathleen Biagas, Thu May  1 17:11:14 PDT 2014
//    Change avtFloatContExpr arg to double.
//


class EXPRESSION_API avtExprNode : public virtual ExprNode
{
  public:
    avtExprNode(const Pos &p) : ExprNode(p) {}
    virtual void CreateFilters(ExprPipelineState *) = 0;
};

class EXPRESSION_API avtIntegerConstExpr
    : public avtExprNode, public IntegerConstExpr
{
  public:
    avtIntegerConstExpr(const Pos &p, int v)
        : ExprNode(p), avtExprNode(p), IntegerConstExpr(p,v) {}
    virtual void CreateFilters(ExprPipelineState *);
};

class EXPRESSION_API avtFloatConstExpr
    : public avtExprNode, public FloatConstExpr
{
  public:
    avtFloatConstExpr(const Pos &p, double v)
        : ExprNode(p), avtExprNode(p), FloatConstExpr(p,v) {}
    virtual void CreateFilters(ExprPipelineState *);
};

class EXPRESSION_API avtStringConstExpr
    : public avtExprNode, public StringConstExpr
{
  public:
    avtStringConstExpr(const Pos &p, std::string v)
        : ExprNode(p), avtExprNode(p), StringConstExpr(p,v) {}
    virtual void CreateFilters(ExprPipelineState *);
};

class EXPRESSION_API avtBooleanConstExpr
    : public avtExprNode, public BooleanConstExpr
{
  public:
    avtBooleanConstExpr(const Pos &p, bool v)
        : ExprNode(p), avtExprNode(p), BooleanConstExpr(p,v) {}
    virtual void CreateFilters(ExprPipelineState *);
};

class EXPRESSION_API avtUnaryExpr
    : public avtExprNode, public UnaryExpr
{
  public:
    avtUnaryExpr(const Pos &p, char o, ExprNode *e)
        : ExprNode(p), avtExprNode(p), UnaryExpr(p,o,e) {}
    virtual void CreateFilters(ExprPipelineState *);
};

class EXPRESSION_API avtBinaryExpr : public avtExprNode, public BinaryExpr
{
  public:
    avtBinaryExpr(const Pos &p, char o, ExprNode *l, ExprNode *r)
        : ExprNode(p), avtExprNode(p), BinaryExpr(p, o, l, r) {}
    virtual void CreateFilters(ExprPipelineState *);
};

class EXPRESSION_API avtIndexExpr : public avtExprNode, public IndexExpr
{
  public:
    avtIndexExpr(const Pos &p, ExprNode *e, int ind)
        : ExprNode(p), avtExprNode(p), IndexExpr(p,e,ind) {}
    virtual void CreateFilters(ExprPipelineState *);
};

class EXPRESSION_API avtVectorExpr : public avtExprNode, public VectorExpr
{
  public:
    avtVectorExpr(const Pos &p, ExprNode *x, ExprNode *y, ExprNode *z=NULL)
        : ExprNode(p), avtExprNode(p), VectorExpr(p,x,y,z) {}
    virtual void CreateFilters(ExprPipelineState *);
};

class EXPRESSION_API avtFunctionExpr : public avtExprNode, public FunctionExpr
{
  public:
    avtFunctionExpr(const Pos &p, std::string n, ArgsExpr *e=NULL)
        : ExprNode(p), avtExprNode(p), FunctionExpr(p,n,e) {}
    virtual void CreateFilters(ExprPipelineState *);
  private:
    avtExpressionFilter *CreateFilters(std::string);

    avtExpressionFilter *CreateMeshQualityFilters(const std::string &) const;
    avtExpressionFilter *CreateMeshFilters(const std::string &) const;
    avtExpressionFilter *CreateImageProcessingFilters(const std::string &) const;
    avtExpressionFilter *CreateMaterialFilters(const std::string &) const;
    avtExpressionFilter *CreateCMFEFilters(const std::string &) const;
    avtExpressionFilter *CreateConditionalFilters(const std::string &) const;
    avtExpressionFilter *CreateMathFilters(const std::string &) const;
    avtExpressionFilter *CreateVectorMatrixFilters(const std::string &) const;
    avtExpressionFilter *CreateTimeAndValueFilters(const std::string &) const;
};

class EXPRESSION_API avtVarExpr : public avtExprNode, public VarExpr
{
  public:
    avtVarExpr(const Pos &p, DBExpr *d, PathExpr *v, bool exp)
        : ExprNode(p), avtExprNode(p), VarExpr(p,d,v,exp) {}
    virtual void CreateFilters(ExprPipelineState *);
};
    
#endif
