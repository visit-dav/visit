/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#ifndef ASTMINUSTERMNODE_H    
#define ASTMINUSTERMNODE_H    
    
#include "SimpleNode.h"    
    
class ASTMinusTermNode : public SimpleNode    
{    
public:    
    ASTMinusTermNode(int i);    
    ~ASTMinusTermNode();    
    std::string infixString(int lang, NameScope* nameScope);    
    void getStackElements(std::vector<StackElement>& elements);    
    double evaluate(int evalType, double* values=0);     
    
    Node* copyTree();    
    
private:    
    ASTMinusTermNode();    
};    
    
#endif    
