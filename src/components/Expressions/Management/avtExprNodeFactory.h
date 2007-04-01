#ifndef AVT_EXPR_NODE_FACTORY_H
#define AVT_EXPR_NODE_FACTORY_H

#include <ExprNodeFactory.h>
#include <ExprToken.h>
#include <avtExprNode.h>
#include <expression_exports.h>

//  Modifications:
//    Jeremy Meredith, Wed Nov 24 12:24:12 PST 2004
//    Renamed Engine to avt.
//

class EXPRESSION_API avtExprNodeFactory: public ExprNodeFactory
{
public:
    virtual ~avtExprNodeFactory() {}
    avtExprNodeFactory() {}

    virtual ConstExpr*
        CreateConstExpr(const Pos & p, ExprToken * t);

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
avtExprNodeFactory::CreateConstExpr(const Pos & p, ExprToken * t)
{
    return new avtConstExpr(p, t);
}

inline UnaryExpr*
avtExprNodeFactory::CreateUnaryExpr(const Pos & p, char op, ExprNode * e)
{
    return new avtUnaryExpr(p, op, e);
}

inline BinaryExpr*
avtExprNodeFactory::CreateBinaryExpr(const Pos & p, char o, ExprNode * l,
                                     ExprNode * r)
{
    return new avtBinaryExpr(p, o, l, r);
}

inline IndexExpr*
avtExprNodeFactory::CreateIndexExpr(const Pos & p, ExprNode * e, int ind)
{
    return new avtIndexExpr(p, e, ind);
}

inline VectorExpr*
avtExprNodeFactory::CreateVectorExpr(const Pos & p, ExprNode * x,
                                     ExprNode * y, ExprNode * z )
{
    return new avtVectorExpr(p, x, y, z);
}

inline FunctionExpr*
avtExprNodeFactory::CreateFunctionExpr(const Pos & p, Identifier * i,
                                       ArgsExpr * e )
{
    return new avtFunctionExpr(p, i, e);
}

inline VarExpr*
avtExprNodeFactory::CreateVarExpr(const Pos & p, DBExpr * d, PathExpr * v,
                                  bool exp)
{
    return new avtVarExpr(p, d, v, exp);
}

#endif
