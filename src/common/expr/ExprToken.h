/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#ifndef EXPRTOKEN_H
#define EXPRTOKEN_H
#include <expr_exports.h>

#include <string>
#include <stdlib.h>
#include <ExprParseTreeNode.h>
#include <Token.h>

enum TokenType {
    TT_NoToken    = 0,
    // All single-character tokens go in this range
    TT_EOF        = EOF_TOKEN_ID,
    TT_Identifier,
    TT_IntegerConst,
    TT_FloatConst,
    TT_StringConst,
    TT_BoolConst,
    TT_Space,
    TT_NTokens
};

// ****************************************************************************
//  Expression Language Tokens
//
//  Purpose:
//    ExprToken is the base class of all VisIt Expression specific
//    tokens.  Inside a parse tree, tokens are contained in
//    TokenParseTreeNode objects, which will often be leaf elements of
//    a parse tree.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
//  Modifications:
//    Sean Ahern, Thu Apr 18 11:25:16 PDT 2002
//    Made the base class ExprGrammarNode so that I can separate out the
//    code that talks to the engine.
//
//    Sean Ahern, Wed Oct 16 16:41:59 PDT 2002
//    Moved the engine communication code to a separate set of classes
//    that are owned by the viewer.  I may collapse ExprGrammarNode again.
//
//    Jeremy Meredith, Wed Jun  9 10:20:08 PDT 2004
//    Added GetValue to BoolConst.
//
//    Jeremy Meredith, Wed Nov 24 12:32:06 PST 2004
//    Split all these out of the new base Token class and into a new library.
//
// ****************************************************************************
class EXPR_API ExprToken : public Token
{
  public:
    ExprToken(const Pos &p, int t) : Token(p,t) { }
    virtual ~ExprToken() { }
    int GetType() const { return type; }
  private:
};

class EXPR_API Space : public ExprToken
{
  public:
    Space(const Pos &p, const std::string &v) : ExprToken(p, TT_Space), val(v) { }
    virtual ~Space() { }
    virtual void PrintNode(ostream &o) { o << "Space\n"; }
  private:
    std::string val;
};

class EXPR_API Character : public ExprToken
{
  public:
    Character(const Pos &p, const std::string &v) : ExprToken(p, v[0]), val(v[0]) { }
    virtual ~Character() { }
    virtual void PrintNode(ostream &o) { o << "Char("<<val<<")\n"; }
    const char &GetVal() const { return val; }
  private:
    char val;
};

class EXPR_API Identifier : public ExprToken
{
  public:
    Identifier(const Pos &p, const std::string &v) : ExprToken(p, TT_Identifier), val(v) { }
    virtual ~Identifier() { }
    virtual void PrintNode(ostream &o) { o << "Ident("<<val.c_str()<<")\n"; }
    const std::string &GetVal() const { return val; }
  private:
    std::string val;
};

class EXPR_API IntegerConst : public ExprToken
{
  public:
    IntegerConst(const Pos &p, const std::string &v) : ExprToken(p, TT_IntegerConst), val(strtol(v.c_str(),NULL,10)) { }
    virtual ~IntegerConst() { }
    virtual void PrintNode(ostream &o) { o << "Integer("<<val<<")\n"; }
    int GetValue(void) { return val; }
  private:
    int val;
};

class EXPR_API FloatConst : public ExprToken
{
  public:
    FloatConst(const Pos &p, const std::string &v) : ExprToken(p, TT_FloatConst), val(strtod(v.c_str(),NULL)) { }
    virtual ~FloatConst() { }
    virtual void PrintNode(ostream &o) { o << "Float("<<val<<")\n"; }
    double GetValue(void) { return val; }
  private:
    double val;
};

class EXPR_API StringConst : public ExprToken
{
  public:
    StringConst(const Pos &p, const std::string &v) : ExprToken(p, TT_StringConst), val(v) { }
    virtual ~StringConst() { }
    virtual void PrintNode(ostream &o) { o << "String("<<val.c_str()<<")\n"; }
    std::string GetValue(void) { return val; }
  private:
    std::string val;
};

class EXPR_API BoolConst : public ExprToken
{
  public:
    BoolConst(const Pos &p, const std::string &v) : ExprToken(p, TT_BoolConst), val(v == "true" || v == "on") { }
    virtual ~BoolConst() { }
    virtual void PrintNode(ostream &o) { o << "Bool("<<(val?"true":"false")<<")\n"; }
    bool GetValue(void) { return val; }
  private:
    bool val;
};


#endif
