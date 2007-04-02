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
