#ifndef EXPRGRAMMARNODE_H
#define EXPRGRAMMARNODE_H
#include <parser_exports.h>

#include <vector>
#include <Pos.h>
#include <set>
class Identifier;
class Token;

// ****************************************************************************
//  Class:  ExprGrammarNode
//
//  Purpose:
//    Base class for all grammar nodes in VisIt expression trees.
//
//  Programmer:  Sean Ahern
//  Creation:    Thu Apr 18 11:22:27 PDT 2002
//
//  Modifications:
//
// ****************************************************************************
class PARSER_API ExprGrammarNode
{
  public:
    ExprGrammarNode(const Pos &p) : pos(p) { }
    virtual ~ExprGrammarNode() { }
    const Pos &GetPos() const {return pos;}
    Pos       &GetPos()       {return pos;}
    virtual const std::string GetTypeName() {return "ExprGrammarNode";}
    virtual std::set<std::string> GetVarLeaves() {return std::set<std::string>();}
    virtual void Print(ostream &o, std::string s="")
    {
        static int indent = 0;
        for (int i=0; i<indent; i++) o << "  ";
        o << s.c_str();
        indent++;
        PrintNode(o);
        indent--;
    }
    virtual void PrintNode(ostream &o) = 0;
  protected:
    Pos pos;
};

#endif
