/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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

class EXPRESSION_API avtExprNodeFactory: public ExprNodeFactory
{
public:
    virtual ~avtExprNodeFactory() {}
    avtExprNodeFactory() {}

    virtual ConstExpr*
        CreateIntegerConstExpr(const Pos & p, int v);

    virtual ConstExpr*
        CreateFloatConstExpr(const Pos & p, float v);

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
avtExprNodeFactory::CreateFloatConstExpr(const Pos & p, float v)
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
