#ifndef TOKEN_H
#define TOKEN_H
#include <parser_exports.h>

#include <string>
#include <stdlib.h>
#include <Pos.h>

#define EOF_TOKEN_ID 256

/*
enum TokenType {
    TT_NoToken    = 0,
    TT_EOF        = EOF_TOKEN_ID,
    ... your tokens here ...
};
*/

PARSER_API std::string GetTokenTypeString(int t);


// ****************************************************************************
//  Class:  Token
//
//  Purpose:
//    Base class of all tokens.  Inside a parse tree, tokens are
//    contained in TokenParseTreeNode objects, which will often be
//    leaf elements of a parse tree.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
//  Modifications:
//    Sean Ahern, Thu Apr 18 11:25:16 PDT 2002
//    Made the base class ExprParseTreeNode so that I can separate out the
//    code that talks to the engine.
//
//    Sean Ahern, Wed Oct 16 16:41:59 PDT 2002
//    Moved the engine communication code to a separate set of classes
//    that are owned by the viewer.  I may collapse ExprParseTreeNode again.
//
//    Jeremy Meredith, Wed Jun  9 10:20:08 PDT 2004
//    Added GetValue to BoolConst.
//
//    Jeremy Meredith, Wed Nov 24 12:11:33 PST 2004
//    Major refactoring.  First, moved all VisIt Expression specific tokens
//    to a new file (ExprToken).  Second, Token no longer derives from
//    the parse tree node class -- instead, there is a specific parse tree
//    node type (TokenParseTreeNode) that contains a token.  This avoids
//    virtual (diamond) inheritance for tokens.
//
// ****************************************************************************
class PARSER_API Token
{
  public:
    Token(const Pos &p, int t) : pos(p), type(t) { }
    virtual ~Token() { }
    int GetType() const { return type; }
    const Pos &GetPos() const {return pos;}
    Pos       &GetPos()       {return pos;}
    virtual void PrintNode(ostream &o) { o << "Unknown Token\n"; }
 protected:
    Pos pos;
    int type;
};

class PARSER_API EndOfInput : public Token
{
  public:
    EndOfInput(const Pos &p) : Token(p, EOF_TOKEN_ID) { }
    virtual ~EndOfInput() { }
    virtual void PrintNode(ostream &o) { o << "<<EOF>>\n"; }
};


#endif
