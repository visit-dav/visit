#ifndef ENGINEEXPRNODEFACTORY_H
#define ENGINEEXPRNODEFACTORY_H

#include <ExprNodeFactory.h>
#include <EngineExprNode.h>

class EngineExprNodeFactory: public ExprNodeFactory
{
public:
    virtual ~EngineExprNodeFactory() {}
    EngineExprNodeFactory() {}

    virtual ConstExpr*
        CreateConstExpr(const Pos & p, Token * t);

    virtual UnaryExpr*
        CreateUnaryExpr(const Pos & p, char op, ExprNode * e);

    virtual BinaryExpr*
        CreateBinaryExpr(const Pos & p, char o, ExprNode * l,
                         ExprNode * r);

    virtual IndexExpr*
        CreateIndexExpr(const Pos & p, ExprNode * e, int ind);

    virtual VectorExpr*
        CreateVectorExpr(const Pos & p, ExprNode * x, ExprNode * y,
                         ExprNode * z = NULL);

    virtual FunctionExpr*
        CreateFunctionExpr(const Pos & p, Identifier * i,
                           ArgsExpr * e = NULL);

    virtual VarExpr*
        CreateVarExpr(const Pos & p, DBExpr * d, PathExpr * v,
                      bool exp);
};

inline ConstExpr*
EngineExprNodeFactory::CreateConstExpr(const Pos & p, Token * t)
{
    return new EngineConstExpr(p, t);
}

inline UnaryExpr*
EngineExprNodeFactory::CreateUnaryExpr(const Pos & p, char op, ExprNode * e)
{
    return new EngineUnaryExpr(p, op, e);
}

inline BinaryExpr*
EngineExprNodeFactory::CreateBinaryExpr(const Pos & p, char o, ExprNode * l,
                                      ExprNode * r)
{
    return new EngineBinaryExpr(p, o, l, r);
}

inline IndexExpr*
EngineExprNodeFactory::CreateIndexExpr(const Pos & p, ExprNode * e, int ind)
{
    return new EngineIndexExpr(p, e, ind);
}

inline VectorExpr*
EngineExprNodeFactory::CreateVectorExpr(const Pos & p, ExprNode * x,
                                      ExprNode * y, ExprNode * z = NULL)
{
    return new EngineVectorExpr(p, x, y, z);
}

inline FunctionExpr*
EngineExprNodeFactory::CreateFunctionExpr(const Pos & p, Identifier * i,
                                        ArgsExpr * e = NULL)
{
    return new EngineFunctionExpr(p, i, e);
}

inline VarExpr*
EngineExprNodeFactory::CreateVarExpr(const Pos & p, DBExpr * d, PathExpr * v,
                                   bool exp)
{
    return new EngineVarExpr(p, d, v, exp);
}

#endif
