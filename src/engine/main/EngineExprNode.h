#ifndef ENGINE_EXPR_NODE_H
#define ENGINE_EXPR_NODE_H

#include <ExprNode.h>

class ExprPipelineState;

class EngineExprNode : public virtual ExprNode
{
  public:
    EngineExprNode(const Pos &p) : ExprNode(p) {}
    virtual void CreateFilters(ExprPipelineState *) = 0;
};

class EngineConstExpr
    : public EngineExprNode, public ConstExpr
{
  public:
    EngineConstExpr(const Pos &p, Token *t)
        : EngineExprNode(p), ConstExpr(p,t), ExprNode(p) {}
    virtual void CreateFilters(ExprPipelineState *);
};

class EngineUnaryExpr
    : public EngineExprNode, public UnaryExpr
{
  public:
    EngineUnaryExpr(const Pos &p, char o, ExprNode *e)
        : EngineExprNode(p), UnaryExpr(p,o,e), ExprNode(p) {}
    virtual void CreateFilters(ExprPipelineState *);
};

class EngineBinaryExpr : public EngineExprNode, public BinaryExpr
{
  public:
    EngineBinaryExpr(const Pos &p, char o, ExprNode *l, ExprNode *r)
        : EngineExprNode(p), BinaryExpr(p, o, l, r), ExprNode(p) {}
    virtual void CreateFilters(ExprPipelineState *);
};

class EngineIndexExpr : public EngineExprNode, public IndexExpr
{
  public:
    EngineIndexExpr(const Pos &p, ExprNode *e, int ind)
        : EngineExprNode(p), IndexExpr(p,e,ind), ExprNode(p) {}
    virtual void CreateFilters(ExprPipelineState *);
};

class EngineVectorExpr : public EngineExprNode, public VectorExpr
{
  public:
    EngineVectorExpr(const Pos &p, ExprNode *x, ExprNode *y, ExprNode *z=NULL)
        : EngineExprNode(p), VectorExpr(p,x,y,z), ExprNode(p) {}
    virtual void CreateFilters(ExprPipelineState *);
};

class EngineFunctionExpr : public EngineExprNode, public FunctionExpr
{
  public:
    EngineFunctionExpr(const Pos &p, Identifier *i, ArgsExpr *e=NULL)
        : EngineExprNode(p), FunctionExpr(p,i,e), ExprNode(p) {}
    virtual void CreateFilters(ExprPipelineState *);
};

class EngineVarExpr : public EngineExprNode, public VarExpr
{
  public:
    EngineVarExpr(const Pos &p, DBExpr *d, PathExpr *v, bool exp)
        : EngineExprNode(p), VarExpr(p,d,v,exp), ExprNode(p) {}
    virtual void CreateFilters(ExprPipelineState *);
};
    
#endif
