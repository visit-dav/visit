/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#ifndef SIMPLENODE_H    
#define SIMPLENODE_H    
    
#include "Node.h"    
    
class ExpressionParser;    
class NameScope;    
    
class SimpleNode : public Node    
{    
public:    
    SimpleNode(int i);    
    virtual ~SimpleNode(void);    
    
    void jjtOpen();    
    void jjtClose();    
    void jjtSetParent(Node* n);    
    Node* jjtGetParent();    
    void jjtAddChild(Node* n, int i);    
    Node* jjtGetChild(int i);    
    int jjtGetNumChildren();    
    void dump(std::string prefix);    
    virtual std::string infixString(int lang, NameScope* nameScope)=0;    
    std::string toString(std::string prefix);    
    virtual void getSymbols(std::vector<std::string>& symbols, int language, NameScope* nameScope);    
    virtual SymbolTableEntry* getBinding(std::string symbol);    
    virtual void bind(SymbolTable* symbolTable);    
    static std::string getFunctionDomainError(std::string problem, double* values, std::string argumentName1, Node* node1, std::string argumentName2="", Node* node2=0);    
    static std::string getNodeSummary(double* values, Node* node);    
    virtual bool isBoolean();    
    
    void jjtAddChild(Node* n);    
    void substitute(Node* origNode, Node* newNode);    
    virtual bool equals(Node* node);    
    
protected:    
    Node* parent;    
    Node** children;    
    int id;    
    int numChildren;    
};    
#endif    
