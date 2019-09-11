// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              ParseTreeNode.C                              //
// ************************************************************************* //

#include <ParseTreeNode.h>


// ****************************************************************************
//  Method: ParseTreeNode::Print
//
//  Purpose:
//      The print method for a grammar node.
//
//  Programmer: Jeremy Meredith
//  Creation:   November 11, 2004
//
//  Modifications:
//    Jeremy Meredith, Wed Jul 23 13:15:15 EDT 2008
//    Separate the parent printout from this one if necessary.
//    
// ****************************************************************************

void 
ParseTreeNode::Print(ostream &o, std::string s)
{
    static int indent = 0;
    for (int i=0; i<indent; i++)
        o << "  ";
    if (!s.empty())
        o << s.c_str() << ": "; 
    indent++;
    PrintNode(o);
    indent--;
}

