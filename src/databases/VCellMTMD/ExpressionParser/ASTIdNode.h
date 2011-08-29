/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#ifndef ASTIDNODE_H    
#define ASTIDNODE_H    
    
#include "SimpleNode.h"    
    
class SymbolTableEntry;    
    
class ASTIdNode : public SimpleNode    
{    
public:    
    ASTIdNode(int i);    
    ~ASTIdNode();    
    std::string name;    
    std::string infixString(int lang, NameScope* nameScope);    
    SymbolTableEntry* symbolTableEntry;    
    SymbolTableEntry* getBinding(std::string symbol);    
    void bind(SymbolTable* symbolTable);    
    void getStackElements(std::vector<StackElement>& elements);    
    double evaluate(int evalType, double* values=0);     
    void getSymbols(std::vector<std::string>& symbols, int language, NameScope* nameScope);    
    
    Node* copyTree();    
    bool equals(Node* node);    
    
private:    
    ASTIdNode(ASTIdNode*);    
};    
    
#endif    
