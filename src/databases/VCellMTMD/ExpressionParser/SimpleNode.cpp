/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#include <memory.h>    
#include <stdlib.h>    
#include <typeinfo>    
#include <iostream>    
#include <stdio.h>
#include <vector>
#include <string>
using std::cout;    
using std::endl;    
using std::vector;
using std::string;
    
#include "SimpleNode.h"    
#include "Expression.h"    
#include "Exception.h"    
#include "ExpressionException.h"    
#include "SymbolTableEntry.h"    
#include "ValueProxy.h"    

using namespace VCell;
    
SimpleNode::SimpleNode(int i) {    
    id = i;    
    numChildren = 0;    
    children = 0;    
    parent = 0;    
}    
    
SimpleNode::~SimpleNode() {    
    for (int i = 0; i < numChildren; i ++) {    
        delete children[i];    
    }    
    delete[] children;    
}    
    
void SimpleNode::jjtOpen() {    
}    
    
void SimpleNode::jjtClose() {    
}    
    
void SimpleNode::jjtSetParent(Node* n) {     
    parent = n;     
}    
    
Node* SimpleNode::jjtGetParent() {     
    return parent;     
}    
    
void SimpleNode::jjtAddChild(Node* n, int i) {    
    if (numChildren == 0) {    
        children = new Node*[i + 1];    
        memset(children, 0, (i + 1) * sizeof(Node*));    
        numChildren = i + 1;    
    } else if (i >= numChildren) {    
        Node** c = new Node*[i + 1];    
        memset(c, 0, (i + 1) * sizeof(Node*));    
        memcpy(c, children, numChildren * sizeof(Node*));    
        delete[] children;        
        children = c;    
        numChildren = i + 1;    
    }    
    children[i] = n;        
    ((SimpleNode*)children[i])->parent = this;    
}    
    
Node* SimpleNode::jjtGetChild(int i) {    
    return children[i];    
}    
    
int SimpleNode::jjtGetNumChildren() {    
    return numChildren;    
}    
    
void SimpleNode::dump(string prefix) {    
    cout << toString(prefix) << endl;    
    if (children != 0) {    
        for (int i = 0; i < numChildren; ++i) {    
            SimpleNode* n = (SimpleNode*)children[i];    
            if (n != 0) {    
                n->dump(prefix + " ");    
            }    
        }    
    }    
}    
    
string SimpleNode::toString(string prefix)    
{    
    return prefix + infixString(LANGUAGE_DEFAULT, 0);     
}    
    
void SimpleNode::getSymbols(vector<string>& symbols, int language, NameScope* nameScope)    
{    
    for (int i=0;i<jjtGetNumChildren();i++){    
        jjtGetChild(i)->getSymbols(symbols, language, nameScope);    
    }        
}    
    
SymbolTableEntry* SimpleNode::getBinding(string symbol)    
{    
    for (int i=0;i<jjtGetNumChildren();i++){    
        SymbolTableEntry* ste = jjtGetChild(i)->getBinding(symbol);    
        if (ste != NULL){    
            return ste;    
        }    
    }            
    return NULL;    
}    
    
void SimpleNode::bind(SymbolTable* symbolTable)    
{    
    for (int i=0;i<jjtGetNumChildren();i++){    
        jjtGetChild(i)->bind(symbolTable);    
    }    
}    
    
bool SimpleNode::isBoolean() {    
    return false;    
}    
    
string SimpleNode::getFunctionDomainError(string problem, double* values, string argumentName1, Node* node1, string argumentName2, Node* node2){    
    string errorMsg = problem + ": " + argumentName1 + "=" + getNodeSummary(values, node1);    
    if (node2 == 0) {    
        return errorMsg;    
    }    
    errorMsg += "\n" + argumentName2 + "=" + getNodeSummary(values, node2);    
    return errorMsg;    
}    
    
string SimpleNode::getNodeSummary(double* values, Node* node){    
    string errorMsg;    
    vector<string> symbols;    
    node->getSymbols(symbols, LANGUAGE_DEFAULT, 0);    
    if (symbols.size() > 0) {    
        errorMsg += "\"" + node->infixString(LANGUAGE_DEFAULT, 0) + "\"\n  where:\n";    
        SymbolTableEntry* symbolTableEntry = 0;    
        for (unsigned int i = 0; i < symbols.size(); i++) {    
            symbolTableEntry = node->getBinding(symbols.at(i));    
            double value = 0.0;            
            try {                    
                if (symbolTableEntry != 0) {                    
                    if (symbolTableEntry->getExpression() != NULL) {                                    
                        if (values == 0) {    
                            value = symbolTableEntry->getExpression()->evaluateConstant();    
                        } else {    
                            value = symbolTableEntry->getExpression()->evaluateVector(values);    
                        }    
                        char chrs[1000];    
                        sprintf(chrs, "\t%s = %lf\n", symbolTableEntry->getName().c_str(), value);    
                        chrs[999] = '\0';
                        errorMsg += chrs;    
                    } else if (symbolTableEntry->getIndex() > -1) {    
                        if (values == NULL) {    
                            if (symbolTableEntry->getValueProxy()!=NULL){    
                                value = symbolTableEntry->getValueProxy()->evaluate();    
                            }else{    
                                value = symbolTableEntry->getConstantValue();    
                            }    
                        } else {    
                            value = values[symbolTableEntry->getIndex()];    
                        }    
                        char chrs[1000];    
                        sprintf(chrs, "\t%s = %lf\n", symbolTableEntry->getName().c_str(), value);    
                        chrs[999] = '\0';
                        errorMsg += chrs;    
                    } else {    
                        errorMsg += "\t" + symbols[i] + " = <<<WRONG BINDING>>>\n";    
                    }    
                } else {    
                    errorMsg += "\t" + symbols[i] + " = <<<UNBOUND IDENTIFIER>>>\n";    
                }    
            } catch (Exception& e) {    
                throw ExpressionException(errorMsg + "\t" + symbols[i] + " = <<<ERROR>>> " + e.getMessage() + "\n");    
            }    
        }    
    } else {    
        errorMsg += node->infixString(LANGUAGE_DEFAULT, 0);    
    }    
    return errorMsg;    
}    
    
void SimpleNode::jjtAddChild(Node* n) {    
    jjtAddChild(n, jjtGetNumChildren());    
}        
    
void SimpleNode::substitute(Node* origNode, Node* newNode) {    
    
    for (int i=0;i<jjtGetNumChildren();i++){    
        if (jjtGetChild(i)->equals(origNode)) {    
            children[i] = newNode->copyTree();    
            newNode->jjtSetParent(this);    
        }else{    
            jjtGetChild(i)->substitute(origNode,newNode);    
        }    
    }    
}    
    
bool SimpleNode::equals(Node* node) {    
    //    
    // check to see if this node is the same    
    //    
    if (typeid(*node) != typeid(*this)){    
        return false;    
    }    
    //    
    // check for different number of children    
    //        
    if (jjtGetNumChildren() != node->jjtGetNumChildren()){    
        return false;    
    }        
    //    
    // now, check to see if all children are the same    
    //    
    //  (note: I'm assuming the children are in the same order)    
    //    
    for (int i=0;i<jjtGetNumChildren();i++){    
        Node* myChild = jjtGetChild(i);    
        Node* nodeChild = node->jjtGetChild(i);    
        if (!myChild->equals(nodeChild)){    
            return false;    
        }        
    }            
        
    return true;    
}    
