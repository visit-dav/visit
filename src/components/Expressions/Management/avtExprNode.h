#ifndef AVT_EXPR_NODE_H
#define AVT_EXPR_NODE_H

#include <ExprNode.h>
#include <expression_exports.h>

class ExprPipelineState;

//  Modifications:
//    Jeremy Meredith, Wed Nov 24 12:24:12 PST 2004
//    Renamed Engine to avt.
//


class EXPRESSION_API avtExprNode : public virtual ExprNode
{
  public:
    avtExprNode(const Pos &p) : ExprNode(p) {}
    virtual void CreateFilters(ExprPipelineState *) = 0;
};

class EXPRESSION_API avtConstExpr
    : public avtExprNode, public ConstExpr
{
  public:
    avtConstExpr(const Pos &p, ExprToken *t)
        : avtExprNode(p), ConstExpr(p,t), ExprNode(p) {}
    virtual void CreateFilters(ExprPipelineState *);
};

class EXPRESSION_API avtUnaryExpr
    : public avtExprNode, public UnaryExpr
{
  public:
    avtUnaryExpr(const Pos &p, char o, ExprNode *e)
        : avtExprNode(p), UnaryExpr(p,o,e), ExprNode(p) {}
    virtual void CreateFilters(ExprPipelineState *);
};

class EXPRESSION_API avtBinaryExpr : public avtExprNode, public BinaryExpr
{
  public:
    avtBinaryExpr(const Pos &p, char o, ExprNode *l, ExprNode *r)
        : avtExprNode(p), BinaryExpr(p, o, l, r), ExprNode(p) {}
    virtual void CreateFilters(ExprPipelineState *);
};

class EXPRESSION_API avtIndexExpr : public avtExprNode, public IndexExpr
{
  public:
    avtIndexExpr(const Pos &p, ExprNode *e, int ind)
        : avtExprNode(p), IndexExpr(p,e,ind), ExprNode(p) {}
    virtual void CreateFilters(ExprPipelineState *);
};

class EXPRESSION_API avtVectorExpr : public avtExprNode, public VectorExpr
{
  public:
    avtVectorExpr(const Pos &p, ExprNode *x, ExprNode *y, ExprNode *z=NULL)
        : avtExprNode(p), VectorExpr(p,x,y,z), ExprNode(p) {}
    virtual void CreateFilters(ExprPipelineState *);
};

class EXPRESSION_API avtFunctionExpr : public avtExprNode, public FunctionExpr
{
  public:
    avtFunctionExpr(const Pos &p, Identifier *i, ArgsExpr *e=NULL)
        : avtExprNode(p), FunctionExpr(p,i,e), ExprNode(p) {}
    virtual void CreateFilters(ExprPipelineState *);
};

class EXPRESSION_API avtVarExpr : public avtExprNode, public VarExpr
{
  public:
    avtVarExpr(const Pos &p, DBExpr *d, PathExpr *v, bool exp)
        : avtExprNode(p), VarExpr(p,d,v,exp), ExprNode(p) {}
    virtual void CreateFilters(ExprPipelineState *);
};
    
#endif
