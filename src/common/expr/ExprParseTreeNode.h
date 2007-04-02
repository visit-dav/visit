// ************************************************************************* //
//                            ExprParseTreeNode.h                            //
// ************************************************************************* //

#ifndef EXPRPARSETREENODE_H
#define EXPRPARSETREENODE_H

#include <expr_exports.h>

#include <vector>
#include <Pos.h>
#include <set>

#include <ParseTreeNode.h>

class Identifier;
class Token;

// ****************************************************************************
//  Class:  ExprParseTreeNode
//
//  Purpose:
//    Base class for all grammar nodes in VisIt expression trees.
//
//  Programmer:  Sean Ahern
//  Creation:    Thu Apr 18 11:22:27 PDT 2002
//
//  Modifications:
//
//    Hank Childs, Thu Aug 14 08:21:14 PDT 2003
//    Un-inlined Print method, since it contained statics.
//
//    Jeremy Meredith, Thu Nov 11 12:35:11 PST 2004
//    Refactored the non-expression related things to another class.
//
//    Hank Childs, Thu Sep  8 15:37:05 PDT 2005
//    Added method GetVarLeafNodes.
//
// ****************************************************************************
class EXPR_API ExprParseTreeNode : public ParseTreeNode
{
  public:
    ExprParseTreeNode(const Pos &p) : ParseTreeNode(p) { }
    virtual ~ExprParseTreeNode() { }
    virtual const std::string GetTypeName() {return "ExprParseTreeNode";}
    virtual std::set<std::string> GetVarLeaves() 
                                            {return std::set<std::string>();}
    virtual std::set<ExprParseTreeNode *> GetVarLeafNodes() 
                                      {return std::set<ExprParseTreeNode *>();}
};

#endif
