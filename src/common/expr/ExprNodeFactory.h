#ifndef EXPRNODEFACTORY_H
#define EXPRNODEFACTORY_H

#include <expr_exports.h>
#include <ExprNode.h>
#include <Pos.h>

class Token;

//  Modifications:
//    Jeremy Meredith, Wed Nov 24 12:22:32 PST 2004
//    Expression-language specific tokens are in a new,
//    more specific base class.
//


class EXPR_API ExprNodeFactory
{
public:
    ExprNodeFactory() {}
    virtual ~ExprNodeFactory() {}

    virtual ConstExpr*
        CreateConstExpr(const Pos & p, ExprToken * t)
            { return new ConstExpr(p, t); }

    virtual UnaryExpr*
        CreateUnaryExpr(const Pos & p, char op, ExprNode * e)
            { return new UnaryExpr(p, op, e); }

    virtual BinaryExpr*
        CreateBinaryExpr(const Pos & p, char o, ExprNode * l,
                         ExprNode * r)
            { return new BinaryExpr(p, o, l, r); }

    virtual IndexExpr*
        CreateIndexExpr(const Pos & p, ExprNode * e, int i)
            { return new IndexExpr(p, e, i); }

    virtual VectorExpr*
        CreateVectorExpr(const Pos & p, ExprNode * x, ExprNode * y,
                         ExprNode * z = NULL)
            { return new VectorExpr(p, x, y, z); }

    virtual FunctionExpr*
        CreateFunctionExpr(const Pos & p, Identifier * i,
                           ArgsExpr * e = NULL)
            { return new FunctionExpr(p, i, e); }

    virtual VarExpr*
        CreateVarExpr(const Pos & p, DBExpr * d, PathExpr * v,
                      bool exp)
            { return new VarExpr(p, d, v, exp); }
};

#endif
