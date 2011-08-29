/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#include "ASTMinusTermNode.h"    
#include "ExpressionException.h"    
#include "ExpressionParserTreeConstants.h"    
#include "StackMachine.h"    
#include <string>
    
using std::string;
using std::vector;

ASTMinusTermNode::ASTMinusTermNode() : SimpleNode(JJTMINUSTERMNODE) {    
}    
    
ASTMinusTermNode::ASTMinusTermNode(int i) : SimpleNode(i) {    
}    
    
ASTMinusTermNode::~ASTMinusTermNode() {    
}    
    
string ASTMinusTermNode::infixString(int lang, NameScope* nameScope)    
{    
    string buffer(" - ");    
    buffer += jjtGetChild(0)->infixString(lang,nameScope);    
    return buffer;    
}    
    
    
void ASTMinusTermNode::getStackElements(vector<StackElement>& elements) {    
    jjtGetChild(0)->getStackElements(elements);    
    elements.push_back(StackElement(TYPE_SUB));    
}    
    
double ASTMinusTermNode::evaluate(int evalType, double* values) {    
    return (- jjtGetChild(0)->evaluate(evalType, values));    
}    
    
Node* ASTMinusTermNode::copyTree() {    
    ASTMinusTermNode* node = new ASTMinusTermNode();    
    for (int i=0;i<jjtGetNumChildren();i++){    
        node->jjtAddChild(jjtGetChild(i)->copyTree());    
    }    
    return node;        
}    
