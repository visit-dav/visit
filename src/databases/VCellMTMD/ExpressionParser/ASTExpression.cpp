/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#include "ASTExpression.h"    
#include "ExpressionException.h"    
#include "ExpressionParserTreeConstants.h"    
    
ASTExpression::ASTExpression() : SimpleNode(JJTEXPRESSION) {    
}    
    
ASTExpression::ASTExpression(int i) : SimpleNode(i) {    
}    
    
ASTExpression::~ASTExpression() {    
}    
    
std::string ASTExpression::infixString(int lang, NameScope* nameScope)    
{    
   std::string buffer;    
    
    for (int i = 0; i < jjtGetNumChildren(); i++) {    
        buffer += jjtGetChild(i)->infixString(lang, nameScope);    
    }    
    return buffer;    
}    
    
void ASTExpression::getStackElements(std::vector<StackElement>& elements) {    
    for (int i = 0; i < jjtGetNumChildren(); i++) {    
        jjtGetChild(i)->getStackElements(elements);    
    }    
}    
    
double ASTExpression::evaluate(int evalType, double* values) {    
    return jjtGetChild(0)->evaluate(evalType, values);    
}    
    
Node* ASTExpression::copyTree() {    
    ASTExpression* node = new ASTExpression();    
    for (int i=0;i<jjtGetNumChildren();i++){    
        node->jjtAddChild(jjtGetChild(i)->copyTree());    
    }    
    return node;        
}    
