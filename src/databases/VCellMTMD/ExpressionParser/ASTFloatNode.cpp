/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#include <stdio.h>    
    
#include "ASTFloatNode.h"    
#include "RuntimeException.h"    
#include "ExpressionParserTreeConstants.h"    
#include "StackMachine.h"    
    
ASTFloatNode::ASTFloatNode(double doubleValue) : SimpleNode(JJTFLOATNODE) {    
    // is not a number    
    if (doubleValue != doubleValue){    
        throw VCell::RuntimeException("cannot set float node to NaN");    
    }    
    value = doubleValue;    
}    
    
ASTFloatNode::ASTFloatNode(int i) : SimpleNode(i) , value(0)    
{    
}    
    
ASTFloatNode::~ASTFloatNode() {    
}    
    
std::string ASTFloatNode::infixString(int lang, NameScope* nameScope)    
{    
    //if (value == NULL) {    
    //    return string("NULL");    
    //} else     
    if (value == 0.0) {    
        return std::string("0.0");    
    } else {    
        char s[256];            
        sprintf(s, "%.20lg", value);    
        s[255] = '\0';
        return std::string(s);    
    }    
}    
    
void ASTFloatNode::getStackElements(std::vector<StackElement>& elements) {    
    elements.push_back(StackElement(value));    
}    
    
double ASTFloatNode::evaluate(int evalType, double* values) {    
    return value;    
}    
    
Node* ASTFloatNode::copyTree(){    
    ASTFloatNode* node = new ASTFloatNode(value);    
    return node;        
}    
    
bool ASTFloatNode::equals(Node* node) {    
    //    
    // check to see if the types and children are the same    
    //    
    if (!SimpleNode::equals(node)){    
        return false;    
    }    
        
    //    
    // check this node for same state (value)    
    //        
    ASTFloatNode* floatNode = (ASTFloatNode*)node;    
    if (floatNode->value != value){    
        return false;    
    }        
    
    return true;    
}    
