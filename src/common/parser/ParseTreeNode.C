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
//    
// ****************************************************************************

void 
ParseTreeNode::Print(ostream &o, std::string s)
{
    static int indent = 0;
    for (int i=0; i<indent; i++)
        o << "  ";
    o << s.c_str();
    indent++;
    PrintNode(o);
    indent--;
}

