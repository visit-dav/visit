// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//    Cyrus Harrison, Tue Jul  3 08:22:37 PDT 2007
//    Changed get GetLeaves to return a vector b/c stl::set sorts
//    entries alphabetically causing problems when parsing apply_ddf.
//
// ****************************************************************************
class EXPR_API2 ExprParseTreeNode : public ParseTreeNode
{
  public:
    ExprParseTreeNode(const Pos &p) : ParseTreeNode(p) { }
    virtual ~ExprParseTreeNode() { }
    virtual const std::string GetTypeName() {return "ExprParseTreeNode";}
    virtual std::vector<std::string> GetVarLeaves()
                                      {return std::vector<std::string>();}
    virtual std::set<ExprParseTreeNode *> GetVarLeafNodes() 
                                      {return std::set<ExprParseTreeNode *>();}
};

#endif
