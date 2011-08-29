/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#include "ASTAndNode.h"    
#include "ExpressionException.h"    
#include "ExpressionParserTreeConstants.h"    
#include "StackMachine.h"    
    
ASTAndNode::ASTAndNode() : SimpleNode(JJTANDNODE) {    
}    
    
ASTAndNode::ASTAndNode(int i) : SimpleNode(i) {    
}    
    
ASTAndNode::~ASTAndNode() {    
}    
    
bool ASTAndNode::isBoolean() {    
    return true;    
}    
    
std::string ASTAndNode::infixString(int lang, NameScope* nameScope)    
{    
    
    std::string buffer;    
    if(lang == LANGUAGE_VISIT){    
      for (int i=0;i<jjtGetNumChildren()-1;i++){    
          buffer += "and(";    
      }    
      buffer += jjtGetChild(0)->infixString(lang,nameScope);    
      for (int i=1;i<jjtGetNumChildren();i++){    
          buffer += ",";    
          buffer += jjtGetChild(i)->infixString(lang,nameScope);    
          buffer += ")";    
      }    
    }else{    
        buffer+= "(";    
        for (int i=0;i<jjtGetNumChildren();i++){    
            if (i>0)    
                buffer += " && ";    
            buffer += jjtGetChild(i)->infixString(lang, nameScope);    
        }    
        buffer += ")";    
    }    
    
    return buffer;    
}    
    
void ASTAndNode::getStackElements(std::vector<StackElement>& elements) {    
    for (int i=0;i<jjtGetNumChildren();i++){    
        jjtGetChild(i)->getStackElements(elements);;    
        if (i>0)     
            elements.push_back(StackElement(TYPE_AND));    
    }    
}    
    
double ASTAndNode::evaluate(int evalType, double* values) {    
    double sum = 1;    
    ExpressionException* savedException = NULL;    
    for (int i = 0; i < jjtGetNumChildren(); i++) {    
        try {    
            if (jjtGetChild(i)->evaluate(evalType, values) == 0) {    
                return 0;    
            }    
        } catch(ExpressionException& ex){    
            if (evalType == EVALUATE_VECTOR) {    
                throw ex;    
            }    
            savedException = new ExpressionException(ex.getMessage());    
        }    
    }    
    
    if (savedException != NULL){    
        throw (*savedException);    
    }else{    
        return sum;    
    }    
}    
    
Node* ASTAndNode::copyTree() {    
    ASTAndNode* node = new ASTAndNode();    
    for (int i=0;i<jjtGetNumChildren();i++){    
        node->jjtAddChild(jjtGetChild(i)->copyTree());    
    }    
    return node;        
}    
