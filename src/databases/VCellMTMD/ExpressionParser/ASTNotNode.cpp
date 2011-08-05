/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#include "ASTNotNode.h"    
#include "DivideByZeroException.h"    
#include "ExpressionException.h"    
#include "ExpressionParserTreeConstants.h"    
#include "StackMachine.h"    
    
ASTNotNode::ASTNotNode() : SimpleNode(JJTNOTNODE) {    
}    
    
ASTNotNode::ASTNotNode(int i) : SimpleNode(i) {    
}    
    
ASTNotNode::~ASTNotNode() {    
}    
    
bool ASTNotNode::isBoolean() {    
    return true;    
}    
    
std::string ASTNotNode::infixString(int lang, NameScope* nameScope)    
{    
    std::string buffer;    
    if (lang == LANGUAGE_VISIT){    
        buffer.append("not(");    
    }else{    
        buffer.append("!(");    
    }    
    buffer += jjtGetChild(0)->infixString(lang,nameScope);    
    buffer += ")";    
    
    return buffer;    
}    
    
void ASTNotNode::getStackElements(std::vector<StackElement>& elements) {    
    jjtGetChild(0)->getStackElements(elements);    
    elements.push_back(StackElement(TYPE_NOT));    
}    
    
    
double ASTNotNode::evaluate(int evalType, double* values) {    
    double childValue = jjtGetChild(0)->evaluate(evalType, values);    
    if (childValue==0.0){    
        return 1.0;    
    }else{    
        return 0.0;    
    }    
}    
    
Node* ASTNotNode::copyTree() {    
    ASTNotNode* node = new ASTNotNode();    
    for (int i=0;i<jjtGetNumChildren();i++){    
        node->jjtAddChild(jjtGetChild(i)->copyTree());    
    }    
    return node;        
}    
