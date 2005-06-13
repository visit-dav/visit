#ifndef AVT_EXPR_NODE_H
#define AVT_EXPR_NODE_H

#include <ExprNode.h>
#include <expression_exports.h>

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
        : avtExprNode(p), IntegerConstExpr(p,v), ExprNode(p) {}
    virtual void CreateFilters(ExprPipelineState *);
};

class EXPRESSION_API avtFloatConstExpr
    : public avtExprNode, public FloatConstExpr
{
  public:
    avtFloatConstExpr(const Pos &p, float v)
        : avtExprNode(p), FloatConstExpr(p,v), ExprNode(p) {}
    virtual void CreateFilters(ExprPipelineState *);
};

class EXPRESSION_API avtStringConstExpr
    : public avtExprNode, public StringConstExpr
{
  public:
    avtStringConstExpr(const Pos &p, std::string v)
        : avtExprNode(p), StringConstExpr(p,v), ExprNode(p) {}
    virtual void CreateFilters(ExprPipelineState *);
};

class EXPRESSION_API avtBooleanConstExpr
    : public avtExprNode, public BooleanConstExpr
{
  public:
    avtBooleanConstExpr(const Pos &p, bool v)
        : avtExprNode(p), BooleanConstExpr(p,v), ExprNode(p) {}
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
    avtFunctionExpr(const Pos &p, std::string n, ArgsExpr *e=NULL)
        : avtExprNode(p), FunctionExpr(p,n,e), ExprNode(p) {}
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
