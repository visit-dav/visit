/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#include "ASTMultNode.h"    
#include "ASTInvertTermNode.h"    
#include "ExpressionException.h"    
#include "ExpressionParserTreeConstants.h"    
#include "StackMachine.h"    
#include "Expression.h"    
//#include "ParseException.h"    
    
#include <sstream>    
#include <string>
using std::stringstream;    
using std::string;
using std::vector;
    
using namespace VCell;

ASTMultNode::ASTMultNode() : SimpleNode(JJTMULTNODE) {    
}    
    
ASTMultNode::ASTMultNode(int i) : SimpleNode(i) {    
}    
    
ASTMultNode::~ASTMultNode() {    
}    
    
bool ASTMultNode::isBoolean() {    
    for (int i=0;i<jjtGetNumChildren();i++){    
        if (!jjtGetChild(i)->isBoolean()) {    
            return false;    
        }    
    }    
      return true;    
}    
    
string ASTMultNode::infixString(int lang, NameScope* nameScope)    
{    
    bool* boolChildFlags = new bool[jjtGetNumChildren()];    
    bool bAllBoolean = true;    
    bool bNoBoolean = true;    
    for (int i=0;i<jjtGetNumChildren();i++){    
        boolChildFlags[i] = jjtGetChild(i)->isBoolean();    
        if (boolChildFlags[i]) {    
            boolChildFlags[i] = true;    
            bNoBoolean = false;    
        } else {    
            bAllBoolean = false;    
        }    
    
    }    
    
    stringstream buffer;    
    buffer << "(";    
    
    if (bAllBoolean || bNoBoolean || (lang != LANGUAGE_C && lang != LANGUAGE_VISIT)) { // old way    
        for (int i=0;i<jjtGetNumChildren();i++){    
            ASTInvertTermNode* pointer = dynamic_cast<ASTInvertTermNode*>(jjtGetChild(i));    
            if (pointer){    
                buffer << " / ";    
                buffer << jjtGetChild(i)->infixString(lang,nameScope);    
            }else{    
                if (i>0)     
                    buffer << " * ";    
                buffer << jjtGetChild(i)->infixString(lang,nameScope);    
            }    
        }            
    } else {            
        stringstream conditionBuffer;    
        stringstream valueBuffer;    
        for (int i=0;i<jjtGetNumChildren();i++){    
            if (boolChildFlags[i]) {    
                if (conditionBuffer.str().length() > 0) {    
                    conditionBuffer << " && ";    
                }    
                conditionBuffer << jjtGetChild(i)->infixString((lang == LANGUAGE_VISIT?LANGUAGE_DEFAULT:lang),nameScope);    
            } else {    
                if (valueBuffer.str().length() == 0) {                        
                    ASTInvertTermNode* pointer = dynamic_cast<ASTInvertTermNode*>(jjtGetChild(i));    
                    if (pointer){    
                        valueBuffer << " 1.0 / ";    
                    }    
                } else {                        
                    ASTInvertTermNode* pointer = dynamic_cast<ASTInvertTermNode*>(jjtGetChild(i));    
                    if (pointer){        
                        valueBuffer << " / ";        
                    } else {    
                        valueBuffer << " * ";    
                    }    
                }    
                valueBuffer << jjtGetChild(i)->infixString(lang,nameScope);    
            }    
        }    
        if(lang == LANGUAGE_VISIT){    
            //try{    
                VCell::Expression * exp = new VCell::Expression(conditionBuffer.str());    
                buffer << "if(" << exp->infix_Visit() << " , " << valueBuffer.str() << " , 0.0)";    
                delete exp;    
            //}catch(...){    
            //    throw ParseException("Error pasing Expr");    
            //}    
        }else{    
            buffer << "((" << conditionBuffer.str() << ") ? (" << valueBuffer.str() << ") : 0.0)";    
        }    
    }    
    buffer << ")";    
    string s = buffer.str();    
    delete [] boolChildFlags;
    return s;    
}    
    
void ASTMultNode::getStackElements(vector<StackElement>& elements) {    
        
    int startSize = (int)elements.size();    
        
    //    
    // Add the stack machine instructions for each boolean child (followed by a branch "BZ" with unknown offset).    
    // If it is the last term in the product, there is no need to insert the branch instruction.    
    //    
    // The "BZ" instruction is inserted before the offset is known (will be updated at the end).    
    // this enforces any "false" evaluation to skip the rest of the terms in this product (children).    
    //    
    int indexBooleanChildren = 0;    
    for (int i=0;i<jjtGetNumChildren();i++){    
        if (jjtGetChild(i)->isBoolean()){    
            jjtGetChild(i)->getStackElements(elements);    
            if (indexBooleanChildren < jjtGetNumChildren()-1){    
                elements.push_back(StackElement(TYPE_BZ));    
            }    
            indexBooleanChildren++;    
        }    
    }    
        
    //    
    // add stack machine instructions for all non-boolean children    
    //    
    int indexNonBooleanChildren = 0;    
    for (int i=0;i<jjtGetNumChildren();i++){    
        if (!jjtGetChild(i)->isBoolean()){    
            jjtGetChild(i)->getStackElements(elements);    
            if (indexNonBooleanChildren > 0){    
                elements.push_back(StackElement(TYPE_MULT));    
            }    
            indexNonBooleanChildren++;    
        }    
    }    
    
    //    
    // go back an fill in the offsets for each "BZ" added earlier.  This is done by starting at the end with a    
    // reverse iterator and counting the instructions to each "BZ" that has a zero offset and is part of this product.    
    // The check for zero offset eliminates setting "BZ" instructions of children of this product.    
    // This will ensure that any "false" evaluation of a boolean child will skip all non-boolean children     
    // so will protect against function domain exceptions.      
    //    
    if (indexBooleanChildren>0){    
        int finalSize = (int)elements.size();    
        int size = finalSize-startSize;    
        vector<StackElement>::reverse_iterator iter = elements.rbegin();    
        for (int offset = 0; offset < size; ++offset) {    
            if ((*iter).type==TYPE_BZ && (*iter).branchOffset==0){    
                (*iter).branchOffset = offset+1;    
            }    
            iter++;                
        }    
    }    
}    
    
double ASTMultNode::evaluate(int evalType, double* values) {    
    Exception* childException = 0;    
    
    // evaluate boolean children first for conditional expressions.     
    // if any one of them is false, just return 0;    
    // this protects against evaluating the terms outside of    
    // domain defined by the boolean conditions.    
    for (int i=0;i<jjtGetNumChildren();i++){    
        if (jjtGetChild(i)->isBoolean()) {    
            try {    
                if (jjtGetChild(i)->evaluate(evalType, values) == 0) {    
                    return 0.0;    
                }    
            } catch (ExpressionException& e){    
                if (evalType == EVALUATE_VECTOR) {    
                    throw e;    
                }    
                childException = new ExpressionException(e.getMessage());    
            }            
        }    
    }    
    if (childException != NULL){    
        throw (*childException);    
    }        
    
    double product = 1.0;    
    for (int i=0;i<jjtGetNumChildren();i++){    
        try {    
            double value = jjtGetChild(i)->evaluate(evalType, values);    
            product *= value;    
        }catch (ExpressionException& e){    
            if (evalType == EVALUATE_VECTOR) {    
                throw e;    
            }    
            childException = new ExpressionException(e.getMessage());    
        }            
    }    
    if (product == -0.0){    
        return 0.0;    
    }        
    if (childException != NULL){    
        throw (*childException);    
    }        
    return product;    
}    
    
Node* ASTMultNode::copyTree() {    
    ASTMultNode* node = new ASTMultNode();    
    for (int i=0;i<jjtGetNumChildren();i++){    
        node->jjtAddChild(jjtGetChild(i)->copyTree());    
    }    
    return node;        
}    
