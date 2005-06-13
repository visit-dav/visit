#ifndef EXPRNODEFACTORY_H
#define EXPRNODEFACTORY_H

#include <expr_exports.h>
#include <ExprNode.h>
#include <Pos.h>

//  Modifications:
//    Jeremy Meredith, Wed Nov 24 12:22:32 PST 2004
//    Expression-language specific tokens are in a new,
//    more specific base class.
//
//    Jeremy Meredith, Mon Jun 13 15:46:22 PDT 2005
//    Made ConstExpr abstract and split it into multiple concrete
//    base classes.  Made FunctionExpr and MachExpr use names
//    instead of Identifier tokens.  These two changes were to
//    remove Token references from the parse tree node classes.


class EXPR_API ExprNodeFactory
{
public:
    ExprNodeFactory() {}
    virtual ~ExprNodeFactory() {}

    virtual ConstExpr*
        CreateIntegerConstExpr(const Pos & p, int v)
            { return new IntegerConstExpr(p, v); }

    virtual ConstExpr*
        CreateFloatConstExpr(const Pos & p, float v)
            { return new FloatConstExpr(p, v); }

    virtual ConstExpr*
        CreateStringConstExpr(const Pos & p, std::string v)
            { return new StringConstExpr(p, v); }

    virtual ConstExpr*
        CreateBooleanConstExpr(const Pos & p, bool v)
            { return new BooleanConstExpr(p, v); }

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
        CreateFunctionExpr(const Pos & p, std::string n,
                           ArgsExpr * e = NULL)
            { return new FunctionExpr(p, n, e); }

    virtual VarExpr*
        CreateVarExpr(const Pos & p, DBExpr * d, PathExpr * v,
                      bool exp)
            { return new VarExpr(p, d, v, exp); }
};

#endif
