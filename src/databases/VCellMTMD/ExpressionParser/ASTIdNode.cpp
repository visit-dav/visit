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
    
#include "ASTIdNode.h"    
#include "ExpressionException.h"    
#include "ExpressionBindingException.h"    
#include "RuntimeException.h"    
#include "Expression.h"    
#include "SymbolTable.h"    
#include "SymbolTableEntry.h"    
#include "StackMachine.h"    
#include "ValueProxy.h"    
    
using namespace VCell;    
using std::string;
using std::vector;
    
ASTIdNode::ASTIdNode(ASTIdNode* node) : SimpleNode(node->id) , name(node->name), symbolTableEntry(node->symbolTableEntry) {    
}    
    
ASTIdNode::ASTIdNode(int i) : SimpleNode(i), symbolTableEntry(NULL)    
{    
}    
    
    
ASTIdNode::~ASTIdNode() {    
}    
    
string ASTIdNode::infixString(int lang, NameScope* nameScope)    
{    
    string idName(name);    
    return idName;    
    
    if (nameScope == NULL) {    
        idName = name;    
    } else {    
    /*            
        if (symbolTableEntry != NULL) {    
            idName = nameScope->getSymbolName(symbolTableEntry);    
        } else {    
            idName = nameScope->getUnboundSymbolName(name);    
        }    
    }    
    if (lang == LANGUAGE_DEFAULT || lang == LANGUAGE_C) {    
        return idName;    
    } else if (lang == LANGUAGE_MATLAB) {    
        return cbit.util.TokenMangler.getEscapedTokenMatlab(idName);    
    } else if (lang == LANGUAGE_JSCL) {    
        return cbit.util.TokenMangler.getEscapedTokenJSCL(idName);    
    } else {            
    */    
        char chrs[20];    
        sprintf(chrs, "%d\0", lang);    
        throw RuntimeException(string("Lanaguage '") + chrs + " not supported");    
        
    }        
}    
    
void ASTIdNode::getStackElements(vector<StackElement>& elements) {    
    if (symbolTableEntry == NULL){    
        throw ExpressionException("tryin to evaluate unbound identifier '" + infixString(LANGUAGE_DEFAULT, 0)+"'");    
    }        
    elements.push_back(StackElement(symbolTableEntry->getValueProxy(), symbolTableEntry->getIndex()));    
}    
    
double ASTIdNode::evaluate(int evalType, double* values) {    
    if (symbolTableEntry == NULL){    
        throw ExpressionException("tryin to evaluate unbound identifier '" + infixString(LANGUAGE_DEFAULT, 0)+"'");    
    }        
    
    if (evalType == EVALUATE_CONSTANT) {    
        if (symbolTableEntry->isConstant()){    
            return symbolTableEntry->getConstantValue();    
        }            
        throw ExpressionException("Identifier '" + name + "' cannot be evaluated as a constant.");    
    } else {    
        Expression* exp = symbolTableEntry->getExpression();    
        if (exp != NULL) {    
            return exp->evaluateVector(values);    
        } else {    
            if (values == 0) {    
                if (symbolTableEntry->getValueProxy() == NULL) {    
                    throw ExpressionException("Value proxy not found for indentifier '" + name + "'");    
                }    
                return symbolTableEntry->getValueProxy()->evaluate();    
            } else {    
                if (symbolTableEntry->getIndex() < 0) {    
                    throw ExpressionBindingException("referenced symbol table entry " + name + " not bound to an index");    
                }    
                return values[symbolTableEntry->getIndex()];    
            }    
        }    
    }    
}    
    
SymbolTableEntry* ASTIdNode::getBinding(string symbol)    
{    
    if (name == symbol){    
        return symbolTableEntry;    
    }else{    
        return NULL;    
    }    
}    
    
void ASTIdNode::bind(SymbolTable* symbolTable)    
{    
    if (symbolTable == NULL){    
        symbolTableEntry = NULL;    
        return;    
    }        
    
    symbolTableEntry = symbolTable->getEntry(name);    
    
    if (symbolTableEntry == NULL){    
        string id = name;    
        throw ExpressionBindingException("error binding identifier '" + id + "'");    
    }    
}    
    
void ASTIdNode::getSymbols(vector<string>& symbols, int language, NameScope* nameScope) {    
    string infix = infixString(language, nameScope);    
    for (int i = 0; i < (int)symbols.size(); i ++) {    
        if (symbols[i] == infix) {    
            return;    
        }    
    }    
    symbols.push_back(infix);    
}    
    
Node* ASTIdNode::copyTree() {    
    ASTIdNode* node = new ASTIdNode(this);    
    return node;        
}    
    
bool ASTIdNode::equals(Node* node) {    
    //    
    // check to see if the types and children are the same    
    //    
    if (!SimpleNode::equals(node)){    
        return false;    
    }    
        
    //    
    // check this node for same state (identifier)    
    //        
    ASTIdNode* idNode = (ASTIdNode*)node;    
    if (idNode->name != name){    
        return false;    
    }    
    
    return true;    
}    
