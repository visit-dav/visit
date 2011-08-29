/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#include "ASTRelationalNode.h"    
#include "RuntimeException.h"    
#include "ExpressionException.h"    
#include "ExpressionParserTreeConstants.h"    
#include "StackMachine.h"    
#include "ParseException.h"    
#include <string>
    
using std::string;
using std::vector;

const int LT = 1;    
const int GT = 2;    
const int LE = 3;    
const int GE = 4;    
const int EQ = 5;    
const int NE = 6;    
const int UNKNOWN = -1;    
    
int StackMachine_RelationalLookupTable[] = {0, TYPE_LT, TYPE_GT, TYPE_LE, TYPE_GE, TYPE_EQ, TYPE_NE};    
    
ASTRelationalNode::ASTRelationalNode() : SimpleNode(JJTRELATIONALNODE) {    
    operation = 0;    
    opString = "????";    
}    
    
ASTRelationalNode::ASTRelationalNode(int i) : SimpleNode(i) {    
    operation = 0;    
    opString = "????";    
}    
    
ASTRelationalNode::~ASTRelationalNode() {    
}    
    
bool ASTRelationalNode::isBoolean() {    
    return true;    
}    
    
void ASTRelationalNode::setOperationFromToken(string op)    
{    
    if (op == ">"){    
        operation = GT;    
    }else if (op == "<"){    
        operation = LT;    
    }else if (op == ">="){    
        operation = GE;    
    }else if (op == "<="){    
        operation = LE;    
    }else if (op == "=="){    
        operation = EQ;    
    }else if (op == "!="){    
        operation = NE;    
    }else{            
        throw RuntimeException("unknown relational operator token = '" + op + "'");    
    }    
    opString = op;    
}    
    
    
string ASTRelationalNode::infixString(int lang, NameScope* nameScope)    
{    
    string buffer;    
    if(lang == LANGUAGE_VISIT){    
      if(jjtGetNumChildren() != 2){    
          throw ParseException("ASTRelationalNode for VISIT expecting 2 children");    
      }    
      switch (operation){    
            case GT:{    
                buffer.append("gt(");    
                break;    
            }    
            case LT:{    
                buffer.append("lt(");    
                break;    
            }    
            case GE:{    
                buffer.append("gte(");    
                break;    
            }    
            case LE:{    
                buffer.append("lte(");    
                break;    
            }    
            case EQ:{    
                buffer.append("equal(");    
                break;    
            }    
            case NE:{    
                buffer.append("notequal(");    
                break;    
            }    
            default:{    
                throw ParseException("ASTRelationalNode VISIT unknown relational operator");    
            }    
        }    
        buffer.append(jjtGetChild(0)->infixString(lang,nameScope)+","+jjtGetChild(1)->infixString(lang,nameScope)+")");    
    
      }else{    
          buffer += "(";    
        for (int i = 0; i < jjtGetNumChildren(); i++) {    
            if (i > 0)    
                buffer += " " + opString + " ";    
            buffer += jjtGetChild(i)->infixString(lang, nameScope);    
        }    
        buffer += ")";    
      }    
    return buffer;    
}    
    
void ASTRelationalNode::getStackElements(vector<StackElement>& elements) {    
    for (int i=0;i<jjtGetNumChildren();i++){    
        jjtGetChild(i)->getStackElements(elements);;    
        if (i>0)     
            elements.push_back(StackElement(StackMachine_RelationalLookupTable[operation]));    
    }    
}    
    
double ASTRelationalNode::evaluate(int evalType, double* values)    
{    
    if (jjtGetNumChildren() != 2) {    
        throw ExpressionException("Expected two children");    
    }    
    double first = jjtGetChild(0)->evaluate(evalType, values);    
    double second = jjtGetChild(1)->evaluate(evalType, values);    
    
    switch (operation) {    
        case GT :    
            {    
                if (first > second)    
                    return 1.0;    
                else    
                    return 0.0;    
            }    
        case LT :    
            {    
                if (first < second)    
                    return 1.0;    
                else    
                    return 0.0;    
            }    
        case GE :    
            {    
                if (first >= second)    
                    return 1.0;    
                else    
                    return 0.0;    
            }    
        case LE :    
            {    
                if (first <= second)    
                    return 1.0;    
                else    
                    return 0.0;    
            }    
        case EQ :    
            {    
                if (first == second)    
                    return 1.0;    
                else    
                    return 0.0;    
            }    
        case NE :    
            {    
                if (first != second)    
                    return 1.0;    
                else    
                    return 0.0;    
            }    
    }    
    throw ExpressionException("unsupported relational operation");    
}    
    
Node* ASTRelationalNode::copyTree() {    
    ASTRelationalNode* node = new ASTRelationalNode();    
    node->operation = this->operation;    
    node->opString = this->opString;    
    for (int i=0;i<jjtGetNumChildren();i++){    
        node->jjtAddChild(jjtGetChild(i)->copyTree());    
    }    
    return node;        
}    
    
bool ASTRelationalNode::equals(Node* node) {    
    //    
    // check to see if the types and children are the same    
    //    
    if (!SimpleNode::equals(node)){    
        return false;    
    }    
        
    //    
    // check this node for same state (operation string and integer)    
    //        
    ASTRelationalNode* relNode = (ASTRelationalNode*)node;    
    if (relNode->opString != opString || relNode->operation != operation){    
        return false;    
    }        
    
    return true;    
}    
