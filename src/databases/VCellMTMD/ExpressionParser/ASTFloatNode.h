/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#ifndef ASTFLOATNODE_H    
#define ASTFLOATNODE_H    
    
#include "SimpleNode.h"    
    
class ASTFloatNode : public SimpleNode    
{    
public:    
    ASTFloatNode(double i);    
    ASTFloatNode(int i);    
    ~ASTFloatNode();    
    double value;    
    std::string infixString(int lang, NameScope* nameScope);    
    void getStackElements(std::vector<StackElement>& elements);    
    double evaluate(int evalType, double* values=0);     
    
    Node* copyTree();    
    bool equals(Node* node);    
};    
    
#endif    
