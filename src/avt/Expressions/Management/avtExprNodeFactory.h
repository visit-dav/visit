// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//    Jeremy Meredith, Mon Jun 13 15:46:22 PDT 2005
//    Made ConstExpr abstract and split it into multiple concrete
//    base classes.  Made FunctionExpr and MachExpr use names
//    instead of Identifier tokens.  These two changes were to
//    remove Token references from the parse tree node classes.
//
//    Kathleen Biagas, Thu May  1 17:13:28 PDT 2014
//    Changed FloatConstExpr to double.

class EXPRESSION_API avtExprNodeFactory: public ExprNodeFactory
{
public:
    virtual ~avtExprNodeFactory() {}
    avtExprNodeFactory() {}

    virtual ConstExpr*
        CreateIntegerConstExpr(const Pos & p, int v);

    virtual ConstExpr*
        CreateFloatConstExpr(const Pos & p, double v);

    virtual ConstExpr*
        CreateStringConstExpr(const Pos & p, std::string v);

    virtual ConstExpr*
        CreateBooleanConstExpr(const Pos & p, bool v);

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
        CreateFunctionExpr(const Pos & p, std::string n,
                           ArgsExpr * e = NULL);

    virtual VarExpr*
        CreateVarExpr(const Pos & p, DBExpr * d, PathExpr * v,
                      bool exp);
};

inline ConstExpr*
avtExprNodeFactory::CreateIntegerConstExpr(const Pos & p, int v)
{
    return new avtIntegerConstExpr(p, v);
}

inline ConstExpr*
avtExprNodeFactory::CreateFloatConstExpr(const Pos & p, double v)
{
    return new avtFloatConstExpr(p, v);
}

inline ConstExpr*
avtExprNodeFactory::CreateStringConstExpr(const Pos & p, std::string v)
{
    return new avtStringConstExpr(p, v);
}

inline ConstExpr*
avtExprNodeFactory::CreateBooleanConstExpr(const Pos & p, bool v)
{
    return new avtBooleanConstExpr(p, v);
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
avtExprNodeFactory::CreateFunctionExpr(const Pos & p, std::string n,
                                       ArgsExpr * e )
{
    return new avtFunctionExpr(p, n, e);
}

inline VarExpr*
avtExprNodeFactory::CreateVarExpr(const Pos & p, DBExpr * d, PathExpr * v,
                                  bool exp)
{
    return new avtVarExpr(p, d, v, exp);
}

#endif
