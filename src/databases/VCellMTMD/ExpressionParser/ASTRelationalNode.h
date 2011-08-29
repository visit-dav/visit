/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#ifndef ASTRELATIONALNODE_H    
#define ASTRELATIONALNODE_H    
    
#include "SimpleNode.h"    
    
class ASTRelationalNode : public SimpleNode    
{    
public:    
    ASTRelationalNode(int i);    
    ~ASTRelationalNode();    
    void setOperationFromToken(std::string op);    
    std::string infixString(int lang, NameScope* nameScope);    
    void getStackElements(std::vector<StackElement>& elements);    
    double evaluate(int evalType, double* values=0);     
    bool isBoolean();    
    
    Node* copyTree();    
    bool equals(Node* node);    
    
private:    
    int operation;    
    std::string opString;    
    ASTRelationalNode();    
};    
    
#endif    
