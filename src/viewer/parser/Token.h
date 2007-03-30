#ifndef TOKEN_H
#define TOKEN_H
#include <viewer_parser_exports.h>

#include <string>
#include <stdlib.h>
#include <ExprGrammarNode.h>

enum TokenType {
    TT_NoToken    = 0,
    TT_EOF        = 256,
    TT_Identifier,
    TT_IntegerConst,
    TT_FloatConst,
    TT_StringConst,
    TT_BoolConst,
    TT_Space,
    TT_NTokens
};

std::string GetTokenString(int t);

// ****************************************************************************
//  Class:  Token
//
//  Purpose:
//    Base class of all tokens.  All tokens are also ExprGrammarNode, so
//    they will often be leaf elements of a parse tree.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
//  Modifications:
//      Sean Ahern, Thu Apr 18 11:25:16 PDT 2002
//      Made the base class ExprGrammarNode so that I can separate out the
//      code that talks to the engine.
//
// ****************************************************************************
class VIEWER_PARSER_API Token : public ExprGrammarNode
{
  public:
    Token(const Pos &p, int t) : ExprGrammarNode(p), type(t) { }
    virtual ~Token() { }
    int GetType() const { return type; }
  private:
    int type;
};

class VIEWER_PARSER_API EndOfInput : public Token
{
  public:
    EndOfInput(const Pos &p) : Token(p, TT_EOF) { }
    virtual ~EndOfInput() { }
    virtual void PrintNode(ostream &o) { o << "<<EOF>>\n"; }
};

class VIEWER_PARSER_API Space : public Token
{
  public:
    Space(const Pos &p, const std::string &v) : Token(p, TT_Space), val(v) { }
    virtual ~Space() { }
    virtual void PrintNode(ostream &o) { o << "Space\n"; }
  private:
    std::string val;
};

class VIEWER_PARSER_API Character : public Token
{
  public:
    Character(const Pos &p, const std::string &v) : Token(p, v[0]), val(v[0]) { }
    virtual ~Character() { }
    virtual void PrintNode(ostream &o) { o << "Char("<<val<<")\n"; }
    const char &GetVal() const { return val; }
  private:
    char val;
};

class VIEWER_PARSER_API Identifier : public Token
{
  public:
    Identifier(const Pos &p, const std::string &v) : Token(p, TT_Identifier), val(v) { }
    virtual ~Identifier() { }
    virtual void PrintNode(ostream &o) { o << "Ident("<<val.c_str()<<")\n"; }
    const std::string &GetVal() const { return val; }
  private:
    std::string val;
};

class VIEWER_PARSER_API IntegerConst : public Token
{
  public:
    IntegerConst(const Pos &p, const std::string &v) : Token(p, TT_IntegerConst), val(strtol(v.c_str(),NULL,10)) { }
    virtual ~IntegerConst() { }
    virtual void PrintNode(ostream &o) { o << "Integer("<<val<<")\n"; }
  private:
    int val;
};

class VIEWER_PARSER_API FloatConst : public Token
{
  public:
    FloatConst(const Pos &p, const std::string &v) : Token(p, TT_FloatConst), val(strtod(v.c_str(),NULL)) { }
    virtual ~FloatConst() { }
    virtual void PrintNode(ostream &o) { o << "Float("<<val<<")\n"; }
  private:
    double val;
};

class VIEWER_PARSER_API StringConst : public Token
{
  public:
    StringConst(const Pos &p, const std::string &v) : Token(p, TT_StringConst), val(v) { }
    virtual ~StringConst() { }
    virtual void PrintNode(ostream &o) { o << "String("<<val.c_str()<<")\n"; }
  private:
    std::string val;
};

class VIEWER_PARSER_API BoolConst : public Token
{
  public:
    BoolConst(const Pos &p, const std::string &v) : Token(p, TT_BoolConst), val(v == "true" || v == "on") { }
    virtual ~BoolConst() { }
    virtual void PrintNode(ostream &o) { o << "Bool("<<(val?"true":"false")<<")\n"; }
  private:
    bool val;
};


#endif
