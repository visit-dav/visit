#ifndef EXPRGRAMMARNODE_H
#define EXPRGRAMMARNODE_H
#include <viewer_parser_exports.h>

#include <vector>
#include "Pos.h"
class Identifier;
class Token;

class EngineProxy;
class ViewerPlot;

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
class VIEWER_PARSER_API ExprGrammarNode
{
  public:
    ExprGrammarNode(const Pos &p) : pos(p) { }
    virtual ~ExprGrammarNode() { }
    const Pos &GetPos() const {return pos;}
     Pos       &GetPos()       {return pos;}
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
