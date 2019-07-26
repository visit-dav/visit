// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              ParseTreeNode.h                              //
// ************************************************************************* //

#ifndef PARSETREENODE_H
#define PARSETREENODE_H

#include <parser_exports.h>
#include <Pos.h>

class Identifier;

// ****************************************************************************
//  Class:  ParseTreeNode
//
//  Purpose:
//    Base class for all grammar nodes in parse trees.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 11, 2004
//
//  Modifications:
//
// ****************************************************************************
class PARSER_API2 ParseTreeNode
{
  public:
    ParseTreeNode() { }
    ParseTreeNode(const Pos &p) : pos(p) { }
    virtual ~ParseTreeNode() { }
    const Pos &GetPos() const {return pos;}
    Pos       &GetPos()       {return pos;}
    virtual const std::string GetTypeName() {return "ParseTreeNode";}
    virtual void Print(ostream &o, std::string s="");
    virtual void PrintNode(ostream &o) = 0;
  protected:
    Pos pos;
};


#endif
