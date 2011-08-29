/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#ifndef JJEXPRESSIONPARSERSTATE_H    
#define JJEXPRESSIONPARSERSTATE_H    
    
#include <vector>    
    
#include "Node.h"    
    
class JJTExpressionParserState    
{    
public:    
    JJTExpressionParserState(void);    
    ~JJTExpressionParserState(void);    
    bool nodeCreated(void);    
    void reset(void);    
    Node* rootNode(void);    
    void pushNode(Node* n);    
    Node* popNode(void);    
    Node* peekNode(void);    
    int nodeArity(void);    
    void clearNodeScope(Node* n);    
    void openNodeScope(Node* n);    
    void closeNodeScope(Node* n, int num);    
    void closeNodeScope(Node* n, bool condition);    
    
private:    
    void popMark(void);    
    std::vector<Node*> nodes;    
    std::vector<int> marks;    
    int sp;    
    int mk;    
    bool node_created;    
    
};    
    
#endif    
