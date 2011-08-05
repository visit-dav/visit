/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#include "ASTAddNode.h"    
#include "ASTMinusTermNode.h"    
#include "ExpressionException.h"    
#include "ExpressionParserTreeConstants.h"    
#include "StackMachine.h"    
#include <string>
using std::string;
using std::vector;
    
ASTAddNode::ASTAddNode() : SimpleNode(JJTADDNODE) {    
}    
    
ASTAddNode::ASTAddNode(int i) : SimpleNode(i) {    
}    
    
ASTAddNode::~ASTAddNode() {    
}    
    
string ASTAddNode::infixString(int lang, NameScope* nameScope)    
{    
        string buffer("(");    
        for (int i = 0;i < jjtGetNumChildren(); i ++){    
                ASTMinusTermNode* pointer = dynamic_cast<ASTMinusTermNode*>(jjtGetChild(i));    
                if (pointer){    
                        buffer += jjtGetChild(i)->infixString(lang, nameScope);    
                }else{    
                        if (i>0)     
                                buffer += " + ";    
                        buffer += jjtGetChild(i)->infixString(lang, nameScope);    
                }    
        }    
        buffer += ")";    
    
        return buffer;    
}    
    
void ASTAddNode::getStackElements(vector<StackElement>& elements) {    
        for (int i = 0;i < jjtGetNumChildren(); i ++){    
                jjtGetChild(i)->getStackElements(elements);    
                if (i>0)     
                        elements.push_back(StackElement(TYPE_ADD));    
        }    
}    
    
double ASTAddNode::evaluate(int evalType, double* values) {    
    
        double sum = 0;    
        for (int i=0;i<jjtGetNumChildren();i++){    
                sum += jjtGetChild(i)->evaluate(evalType, values);    
        }    
        return sum;             
}    
    
Node* ASTAddNode::copyTree() {    
        ASTAddNode* node = new ASTAddNode();    
        for (int i=0;i<jjtGetNumChildren();i++){    
                node->jjtAddChild(jjtGetChild(i)->copyTree());    
        }    
        return node;            
}    
