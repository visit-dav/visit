// ************************************************************************* //
//                             ExprGrammarNode.C                             //
// ************************************************************************* //

#include <ExprGrammarNode.h>


// ****************************************************************************
//  Method: ExprGrammarNode::Print
//
//  Purpose:
//      The print method for an expression grammar node.
//
//  Programmer: Sean Ahern
//  Creation:   April 18, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Aug 14 08:25:29 PDT 2003
//    Moved this routine from the header, to prevent problems with inline 
//    functions and statics.
//
// ****************************************************************************

void 
ExprGrammarNode::Print(ostream &o, std::string s="")
{
    static int indent = 0;
    for (int i=0; i<indent; i++)
        o << "  ";
    o << s.c_str();
    indent++;
    PrintNode(o);
    indent--;
}


