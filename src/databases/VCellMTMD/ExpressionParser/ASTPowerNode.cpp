/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#include <math.h>    
#include <stdio.h>
#include <string>

#include "ASTPowerNode.h"    
#include "RuntimeException.h"    
#include "DivideByZeroException.h"    
#include "FunctionDomainException.h"    
#include "ExpressionParserTreeConstants.h"    
#include "StackMachine.h"    
#include "MathUtil.h"    
    
using std::string;

ASTPowerNode::ASTPowerNode() : SimpleNode(JJTPOWERNODE) {    
}    
    
ASTPowerNode::ASTPowerNode(int i) : SimpleNode(i) {    
}    
    
ASTPowerNode::~ASTPowerNode() {    
}    
    
string ASTPowerNode::infixString(int lang, NameScope* nameScope)    
{    
    if (jjtGetNumChildren() != 2) {    
        char ch[20];    
        sprintf(ch, "%d\0", jjtGetNumChildren());    
        throw RuntimeException("There are" + string(ch) + " arguments for the power operator, expecting 2");    
    }    
    
    string buffer;    
    if (lang == LANGUAGE_DEFAULT || lang == LANGUAGE_MATLAB) {    
        buffer += "(";    
        buffer += jjtGetChild(0)->infixString(lang, nameScope);    
        buffer += " ^ ";    
        buffer += jjtGetChild(1)->infixString(lang, nameScope);    
        buffer += ")";    
    } else if (lang == LANGUAGE_C) {    
        buffer += "pow(";    
        buffer += jjtGetChild(0)->infixString(lang, nameScope);    
        buffer += ",";    
        buffer += jjtGetChild(1)->infixString(lang, nameScope);    
        buffer += ")";    
    }    
    
    return buffer;    
}    
    
void ASTPowerNode::getStackElements(std::vector<StackElement>& elements) {    
    jjtGetChild(0)->getStackElements(elements);;    
    jjtGetChild(1)->getStackElements(elements);;    
    elements.push_back(StackElement(TYPE_POW));    
}    
    
double ASTPowerNode::evaluate(int evalType, double* values) {    
    if (jjtGetNumChildren() != 2) {    
        char chrs[1000];    
        sprintf(chrs, "ASTPowerNode: wrong number of arguments for '^' (%d), expected 2\0", jjtGetNumChildren());    
        throw ExpressionException(chrs);    
    }    
    //    
    // see if there are any constant 0.0's, if there are simplify to 0.0    
    //    
    ExpressionException* exponentException = NULL;    
    ExpressionException* baseException = NULL;    
    Node* exponentChild = jjtGetChild(1);    
    Node* baseChild = jjtGetChild(0);    
    
    double exponentValue = 0.0;    
    double baseValue = 0.0;    
    try {    
        exponentValue = exponentChild->evaluate(evalType, values);    
    } catch (ExpressionException& e) {    
        if (evalType == EVALUATE_VECTOR)    
            throw e;    
        exponentException = new ExpressionException(e.getMessage());    
    }        
    try {    
        baseValue = baseChild->evaluate(evalType, values);    
    } catch (ExpressionException& e) {    
        if (evalType == EVALUATE_VECTOR)    
            throw e;    
        baseException = new ExpressionException(e.getMessage());    
    }    
    
    if (exponentException == NULL && baseException == NULL) {    
        if (baseValue == 0.0 && exponentValue < 0.0) {    
            string childString = infixString(LANGUAGE_DEFAULT,0);    
            char problem[1000];    
            sprintf(problem, "u^v and u=0 and v=%lf<0", exponentValue);    
            string errorMsg = getFunctionDomainError(problem, values, "u", baseChild, "v", exponentChild);    
            throw DivideByZeroException(errorMsg);    
        } else if (baseValue < 0.0 && exponentValue != MathUtil::round(exponentValue)) {    
            char problem[1000];    
            sprintf(problem, "u^v and u=%lf<0 and v=%lf not an integer: undefined", baseValue, exponentValue);    
            string errorMsg = getFunctionDomainError(problem, values, "u", baseChild, "v", exponentChild);    
            throw FunctionDomainException(errorMsg);    
        } else {    
            double result = pow(baseValue, exponentValue);    
            if (MathUtil::double_infinity == -result || MathUtil::double_infinity == result || result != result) {    
                char problem[1000];    
                sprintf(problem, "u^v evaluated to %lf, u=%lf, v=%lf", result, baseValue);    
                string errorMsg = getFunctionDomainError(problem, values, "u", baseChild, "v", exponentChild);    
                throw FunctionDomainException(errorMsg);    
            }    
            return result;    
        }    
    } else if (exponentException == 0 && exponentValue == 0.0) {    
        return 1.0;    
    } else if (baseException == 0 && baseValue == 1.0) {    
        return 1.0;    
    } else {    
        if (baseException != NULL) {    
            throw (*baseException);    
        } else if (exponentException != NULL) {    
            throw (*exponentException);    
        }     
    }    
}    
    
Node* ASTPowerNode::copyTree() {    
    ASTPowerNode* node = new ASTPowerNode();    
    for (int i=0;i<jjtGetNumChildren();i++){    
        node->jjtAddChild(jjtGetChild(i)->copyTree());    
    }    
    return node;        
}    
