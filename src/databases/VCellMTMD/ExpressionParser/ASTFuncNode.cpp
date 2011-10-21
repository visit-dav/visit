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
    
#include <math.h>    
#include <algorithm>    
#include <string>
using std::min;    
using std::max;    
using std::string;
    
#include "ASTFuncNode.h"    
#include "RuntimeException.h"    
#include "ExpressionException.h"    
#include "MathUtil.h"    
#include "FunctionDomainException.h"    
#include "FunctionRangeException.h"    
#include "ExpressionParserTreeConstants.h"    
#include "StackMachine.h"    
    
const int EXP = 0;    
const int SQRT = 1;    
const int ABS = 2;    
const int POW = 3;    
const int LOG = 4;    
const int SIN = 5;    
const int COS = 6;    
const int TAN = 7;    
const int ASIN = 8;    
const int ACOS = 9;    
const int ATAN = 10;    
const int ATAN2 = 11;    
const int MAX = 12;    
const int MIN = 13;    
const int CEIL = 14;    
const int FLOOR = 15;    
const int CSC = 16;    
const int COT = 17;    
const int SEC = 18;    
const int ACSC = 19;    
const int ACOT = 20;    
const int ASEC = 21;    
const int SINH = 22;    
const int COSH = 23;    
const int TANH = 24;    
const int CSCH = 25;    
const int COTH = 26;    
const int SECH = 27;    
const int ASINH = 28;    
const int ACOSH = 29;    
const int ATANH = 30;    
const int ACSCH = 31;    
const int ACOTH = 32;    
const int ASECH = 33;    
const int FACTORIAL = 34;        
    
int StackMachine_LookupTable[] = {TYPE_EXP, TYPE_SQRT, TYPE_ABS, TYPE_POW,     
    TYPE_LOG, TYPE_SIN, TYPE_COS, TYPE_TAN, TYPE_ASIN, TYPE_ACOS, TYPE_ATAN,     
    TYPE_ATAN2, TYPE_MAX, TYPE_MIN, TYPE_CEIL, TYPE_FLOOR, TYPE_CSC, TYPE_COT,     
    TYPE_SEC, TYPE_ACSC, TYPE_ACOT, TYPE_ASEC, TYPE_SINH, TYPE_COSH, TYPE_TANH,     
    TYPE_CSCH, TYPE_COTH, TYPE_SECH, TYPE_ASINH, TYPE_ACOSH, TYPE_ATANH, TYPE_ACSCH,     
    TYPE_ACOTH, TYPE_ASECH, TYPE_FACTORIAL    
};    
    
const string functionNamesVCML[] = {    
    "exp",        // 0    
    "sqrt",        // 1    
    "abs",        // 2    
    "pow",        // 3    
    "log",        // 4    
    "sin",        // 5    
    "cos",        // 6    
    "tan",        // 7    
    "asin",        // 8    
    "acos",        // 9    
    "atan",        // 10    
    "atan2",    // 11    
    "max",        // 12    
    "min",        // 13    
    "ceil",        // 14    
    "floor",    // 15    
    "csc",        // 16    
    "cot",        // 17    
    "sec",        // 18    
    "acsc",        // 19    
    "acot",        // 20    
    "asec",        // 21    
    "sinh",        // 22    
    "cosh",        // 23    
    "tanh",        // 24    
    "csch",        // 25    
    "coth",        // 26    
    "sech",        // 27    
    "asinh",    // 28    
    "acosh",    // 29    
    "atanh",    // 30    
    "acsch",    // 31    
    "acoth",    // 32    
    "asech",    // 33        
    "factorial"    // 34        
};    
    
const int parserNumFunctions = 35;    
    
ASTFuncNode::ASTFuncNode() : SimpleNode(JJTFUNCNODE) {    
    funcType = -1;    
}    
    
ASTFuncNode::ASTFuncNode(int i) : SimpleNode(i) {    
    funcType = -1;    
}    
    
ASTFuncNode::~ASTFuncNode() {    
}    
    
void ASTFuncNode::setFunctionFromParserToken(string parserToken)    
{    
    for (int i = 0; i < parserNumFunctions; i++){    
        string definedToken = functionNamesVCML[i];            
        if (definedToken == parserToken){    
            funcType = i;    
            funcName = parserToken;    
            return;    
        }    
    }    
    throw VCell::RuntimeException("unsupported function '" + parserToken + "'");    
}    
    
string ASTFuncNode::infixString(int lang, NameScope* nameScope)    
{    
    string buffer;    
    
    switch (funcType) {    
        case POW :    
        {    
            if (jjtGetNumChildren() != 2) {    
                throw VCell::ExpressionException("Function pow() expects 2 arguments");    
            }    
            if (lang == LANGUAGE_MATLAB) {    
                buffer += "(";    
                buffer += jjtGetChild(0)->infixString(lang, nameScope);    
                buffer += " ^ ";    
                buffer += jjtGetChild(1)->infixString(lang, nameScope);    
                buffer += ")";    
            } else if (lang == LANGUAGE_DEFAULT || lang == LANGUAGE_C) {    
                buffer += "pow(";    
                buffer += jjtGetChild(0)->infixString(lang, nameScope);    
                buffer += ",";    
                buffer += jjtGetChild(1)->infixString(lang, nameScope);    
                buffer += ")";    
            }    
            break;    
        }    
        default :    
        {    
            buffer += funcName;    
            buffer += "(";    
            for (int i = 0; i < jjtGetNumChildren(); i++) {    
                if (i > 0)    
                    buffer += ", ";    
                buffer += jjtGetChild(i)->infixString(lang, nameScope);    
            }    
            buffer += ")";    
            break;    
        }    
    }    
    
    return buffer;    
}    
    
void ASTFuncNode::getStackElements(std::vector<StackElement>& elements) {    
    for (int i = 0; i < jjtGetNumChildren(); i++) {    
        jjtGetChild(i)->getStackElements(elements);    
    }    
    elements.push_back(StackElement(StackMachine_LookupTable[funcType]));    
}    
    
double ASTFuncNode::evaluate(int evalType, double* values)    
{    
    double result = 0.0;    
    Node* child0 = jjtGetChild(0);    
    
    switch (funcType) {    
        case EXP :    
            {    
                if (jjtGetNumChildren() != 1)    
                    throw VCell::RuntimeException("exp() expects 1 argument");                    
                double argument = child0->evaluate(evalType, values);    
                result = exp(argument);    
                break;    
            }    
        case POW :    
            {    
                if (jjtGetNumChildren() != 2)    
                    throw VCell::RuntimeException("pow() expects 2 arguments");    
                Node* exponentChild = jjtGetChild(1);    
                Node* mantissaChild = child0;    
                double exponent = 0.0;    
                double mantissa = 0.0;    
                VCell::ExpressionException* exponentException = 0;    
                VCell::ExpressionException* mantissaException = 0;    
                try {    
                    exponent = exponentChild->evaluate(evalType, values);    
                } catch (VCell::ExpressionException& e) {    
                    if (evalType == EVALUATE_VECTOR)     
                        throw e;    
                    exponentException = new VCell::ExpressionException(e.getMessage());    
                }    
                try {    
                    mantissa = mantissaChild->evaluate(evalType, values);    
                } catch (VCell::ExpressionException& e) {    
                    if (evalType == EVALUATE_VECTOR)     
                        throw e;    
                    mantissaException = new VCell::ExpressionException(e.getMessage());    
                }    
    
                if (exponentException == NULL && mantissaException == NULL) {    
                    if (mantissa < 0.0 && (MathUtil::round(exponent) != exponent)) {    
                        char problem[100];    
                        sprintf(problem, "pow(u,v) and u=%lf<0 and v=%lf not an integer", mantissa, exponent);    
                        string errorMsg = getFunctionDomainError(problem, values, "u", mantissaChild, "v",  exponentChild);    
                        throw VCell::FunctionDomainException(errorMsg);    
                    }    
                    if (mantissa == 0.0 && exponent < 0) {    
                        char problem[100];    
                        sprintf(problem, "pow(u,v) and u=0 and v=%lf<0 divide by zero", exponent);    
                        string errorMsg = getFunctionDomainError(problem, values, "u", mantissaChild, "v", exponentChild);    
                        throw VCell::FunctionDomainException(errorMsg);    
                    }    
                    result = pow(mantissa, exponent);    
                    if (MathUtil::double_infinity == -result || MathUtil::double_infinity == result || result != result) {    
                        char problem[1000];    
                        sprintf(problem, "u^v evaluated to %lf, u=%lf, v=%lf", result, mantissa, exponent);    
                        string errorMsg = getFunctionDomainError(problem, values, "u", mantissaChild, "v", exponentChild);    
                        throw VCell::FunctionDomainException(errorMsg);    
                    }    
                } else if (exponentException == 0 && exponent == 0.0) {    
                        result = 1.0;    
                } else if (mantissaException == 0 && mantissa == 1.0) {    
                        result = 1.0;    
                } else {    
                    if (mantissaException != NULL) {    
                        throw (*mantissaException);    
                    } else if (exponentException != NULL) {    
                        throw (*exponentException);    
                    }    
                }    
                break;    
            }    
        case LOG :    
            {    
                if (jjtGetNumChildren() != 1)    
                    throw VCell::RuntimeException("log() expects 1 argument");                    
                double argument = child0->evaluate(evalType, values);    
                if (argument <= 0.0) {    
                    char problem[1000];                        
                    sprintf(problem, "log(u) and u=%lf <= 0.0 is undefined", argument);    
                    string errorMsg = getFunctionDomainError(problem, values, "u", child0);    
                    throw VCell::FunctionDomainException(errorMsg);    
                }    
                result = log(argument);    
                break;    
            }    
        case ABS :    
            {    
                if (jjtGetNumChildren() != 1)    
                    throw VCell::RuntimeException("abs() expects 1 argument");    
                double argument = child0->evaluate(evalType, values);    
                result = fabs(argument);    
                break;    
            }    
        case SQRT :    
            {    
                if (jjtGetNumChildren() != 1)    
                    throw VCell::RuntimeException("sqrt() expects 1 argument");                    
                double argument = child0->evaluate(evalType, values);    
                if (argument < 0) {    
                    char problem[1000];    
                    sprintf(problem, "sqrt(u) where u=%lf<0 is undefined", argument);    
                    string errorMsg = getFunctionDomainError(problem, values, "u", child0);    
                    throw VCell::FunctionDomainException(errorMsg);    
                }    
                result = sqrt(argument);    
                break;    
            }    
        case SIN :    
            {    
                if (jjtGetNumChildren() != 1)    
                    throw VCell::RuntimeException("sin() expects 1 argument");    
                double argument = child0->evaluate(evalType, values);    
                result = sin(argument);    
                break;    
            }    
        case COS :    
            {    
                if (jjtGetNumChildren() != 1)    
                    throw VCell::RuntimeException("cos() expects 1 argument");    
                double argument = child0->evaluate(evalType, values);    
                result = cos(argument);    
                break;    
            }    
        case TAN :    
            {    
                if (jjtGetNumChildren() != 1)    
                    throw VCell::RuntimeException("tan() expects 1 argument");    
                double argument = child0->evaluate(evalType, values);    
                result = tan(argument);    
                break;    
            }    
        case ASIN :    
            {    
                if (jjtGetNumChildren() != 1)    
                    throw VCell::RuntimeException("asin() expects 1 argument");                    
                double argument = child0->evaluate(evalType, values);    
                if (fabs(argument) > 1.0) {    
                    char problem[1000];    
                    sprintf(problem, "asin(u) and u=%lf and |u|>1.0 undefined", argument);    
                    string errorMsg = getFunctionDomainError(problem, values, "u", child0);    
                    throw VCell::FunctionDomainException(errorMsg);    
                }    
                result = asin(argument);    
                break;    
            }    
        case ACOS :    
            {    
                if (jjtGetNumChildren() != 1)    
                    throw VCell::RuntimeException("acos() expects 1 argument");                    
                double argument = child0->evaluate(evalType, values);    
                if (fabs(argument) > 1.0) {    
                    char problem[1000];    
                    sprintf(problem, "acos(u) and u=%lf and |u|>1.0 undefined", argument);    
                    string errorMsg = getFunctionDomainError(problem, values, "u", child0);    
                    throw VCell::FunctionDomainException(errorMsg);    
                }    
                result = acos(argument);    
                break;    
            }    
        case ATAN :    
            {    
                if (jjtGetNumChildren() != 1)    
                    throw VCell::RuntimeException("atan() expects 1 argument");    
                double argument = child0->evaluate(evalType, values);    
                result = atan(argument);    
                break;    
            }    
        case ATAN2 :    
            {    
                if (jjtGetNumChildren() != 2)    
                    throw VCell::RuntimeException("atan2() expects 2 arguments");    
                double argument0 = child0->evaluate(evalType, values);    
                double argument1 = jjtGetChild(1)->evaluate(evalType, values);    
                result = atan2(argument0, argument1);    
                break;    
            }    
        case MAX :    
            {    
                if (jjtGetNumChildren() != 2)    
                    throw VCell::RuntimeException("max() expects 2 arguments");    
                double argument0 = child0->evaluate(evalType, values);    
                double argument1 = jjtGetChild(1)->evaluate(evalType, values);    
                result = max<double>(argument0, argument1);    
                break;    
            }    
        case MIN :    
            {    
                if (jjtGetNumChildren() != 2)    
                    throw VCell::RuntimeException("min() expects 2 arguments");    
                double argument0 = child0->evaluate(evalType, values);    
                double argument1 = jjtGetChild(1)->evaluate(evalType, values);    
                result = min<double>(argument0, argument1);    
                break;    
            }    
        case CEIL :    
            {    
                if (jjtGetNumChildren() != 1)     
                    throw VCell::RuntimeException("ceil() expects 1 argument");        
                double argument = child0->evaluate(evalType, values);    
                result = ceil(argument);    
                break;    
            }    
        case FLOOR :    
            {    
                if (jjtGetNumChildren() != 1)     
                    throw VCell::RuntimeException("floor() expects 1 argument");    
                double argument = child0->evaluate(evalType, values);    
                result = floor(argument);    
                break;    
            }    
        case CSC:     
            {    
                if (jjtGetNumChildren() != 1)     
                    throw VCell::RuntimeException("csc() expects 1 argument");    
                double argument = child0->evaluate(evalType, values);    
                result = sin(argument);    
                if (result == 0) {    
                    char problem[1000];    
                    sprintf(problem, "csc(u)=1/sin(u) and sin(u)=0 and u=%lf", argument);    
                    string errorMsg = getFunctionDomainError(problem, values, "u", child0);    
                    throw VCell::FunctionDomainException(errorMsg);    
                }    
                result = 1/result;    
                break;    
            }    
        case COT:     
            {    
                if (jjtGetNumChildren() != 1)     
                    throw VCell::RuntimeException("cot() expects 1 argument");    
                double argument = child0->evaluate(evalType, values);    
                result = tan(argument);    
                if (result == 0) {    
                    char problem[1000];    
                    sprintf(problem, "cot(u)=1/tan(u) and tan(u)=0 and u=%lf", argument);    
                    string errorMsg = getFunctionDomainError(problem, values, "u", child0);    
                    throw VCell::FunctionDomainException(errorMsg);    
                }    
                result = 1/result;    
                break;    
            }    
        case SEC:     
            {    
                if (jjtGetNumChildren() != 1)     
                    throw VCell::RuntimeException("sec() expects 1 argument");    
                double argument = child0->evaluate(evalType, values);    
                result = cos(argument);    
                if (result == 0) {    
                    char problem[1000];    
                    sprintf(problem, "sec(u)=1/cos(u) and cos(u)=0 and u=%lf", argument);    
                    string errorMsg = getFunctionDomainError(problem, values, "u", child0);    
                    throw VCell::FunctionDomainException(errorMsg);    
                }    
                result = 1/result;    
                break;    
            }    
        case ACSC:     
            {    
                if (jjtGetNumChildren() != 1)     
                    throw VCell::RuntimeException("acsc() expects 1 argument");    
                double argument = child0->evaluate(evalType, values);    
                if (fabs(argument) < 1.0){    
                    char problem[1000];    
                    sprintf(problem, "acsc(u) and -1<u=%lf<1 undefined", argument);    
                    string errorMsg = getFunctionDomainError(problem, values, "u", child0);    
                    throw VCell::FunctionDomainException(errorMsg);    
                }    
                result = MathUtil::acsc(argument);    
                break;    
            }    
        case ACOT:     
            {    
                if (jjtGetNumChildren() != 1)     
                    throw VCell::RuntimeException("acot() expects 1 argument");    
                double argument = child0->evaluate(evalType, values);    
                if (argument == 0) {    
                    string errorMsg = getFunctionDomainError("acot(u)=atan(1/u) and u=0", values, "u", child0);    
                    throw VCell::FunctionDomainException(errorMsg);    
                }    
                result = MathUtil::acot(argument);    
                break;    
            }    
        case ASEC:     
            {    
                if (jjtGetNumChildren() != 1)     
                    throw VCell::RuntimeException("asec() expects 1 argument");    
                double argument = child0->evaluate(evalType, values);    
                if (fabs(argument) < 1.0){    
                    char problem[1000];    
                    sprintf(problem, "asec(u) and -1<u=%lf<1 undefined", argument);    
                    string errorMsg = getFunctionDomainError(problem, values, "u", child0);    
                    throw VCell::FunctionDomainException(errorMsg);    
                }    
                result = MathUtil::asec(argument);    
                break;    
            }    
        case SINH:     
            {    
                if (jjtGetNumChildren() != 1)     
                    throw VCell::RuntimeException("sinh() expects 1 argument");    
                double argument = child0->evaluate(evalType, values);    
                result = sinh(argument);    
                break;    
            }    
        case COSH:     
            {    
                if (jjtGetNumChildren() != 1)     
                    throw VCell::RuntimeException("cosh() expects 1 argument");    
                double argument = child0->evaluate(evalType, values);    
                result = cosh(argument);    
                break;    
            }    
        case TANH:     
            {    
                if (jjtGetNumChildren() != 1)     
                    throw VCell::RuntimeException("tanh() expects 1 argument");    
                double argument = child0->evaluate(evalType, values);    
                result = tanh(argument);    
                break;    
            }    
        case CSCH:     
            {    
                if (jjtGetNumChildren() != 1)     
                    throw VCell::RuntimeException("csch() expects 1 argument");    
                double argument = child0->evaluate(evalType, values);    
                if (argument == 0.0){    
                    char problem[1000];    
                    sprintf(problem, "csch(u) and |u| = 0, u=%lf", argument);    
                    string errorMsg = getFunctionDomainError(problem, values, "u", child0);    
                    throw VCell::FunctionDomainException(errorMsg);    
                }    
                result = MathUtil::csch(argument);    
                break;    
            }    
        case COTH:     
            {    
                if (jjtGetNumChildren() != 1)     
                    throw VCell::RuntimeException("coth() expects 1 argument");    
                double argument = child0->evaluate(evalType, values);    
                if (argument == 0.0){    
                    char problem[1000];    
                    sprintf(problem, "coth(u) and |u| = 0, u=%lf", argument);    
                    string errorMsg = getFunctionDomainError(problem, values, "u", child0);    
                    throw VCell::FunctionDomainException(errorMsg);    
                }    
                result = MathUtil::coth(argument);    
                break;    
            }    
        case SECH:     
            {    
                if (jjtGetNumChildren()!= 1)     
                    throw VCell::RuntimeException("sech() expects 1 argument");    
                double argument = child0->evaluate(evalType, values);    
                result = MathUtil::sech(argument);    
                break;    
            }    
        case ASINH:     
            {    
                if (jjtGetNumChildren() != 1 ){     
                    throw VCell::RuntimeException("asinh() expects 1 argument");    
                }    
                double argument = child0->evaluate(evalType, values);    
                result = MathUtil::asinh(argument);    
                break;    
            }    
        case ACOSH:     
            {    
                if (jjtGetNumChildren()!=1)     
                    throw VCell::RuntimeException("acosh() expects 1 argument");    
                double argument = child0->evaluate(evalType, values);    
                if (argument < 1.0){    
                    char problem[1000];    
                    sprintf(problem, "acosh(u) and u=%lf<1.0", argument);    
                    string errorMsg = getFunctionDomainError(problem, values, "u", child0);    
                    throw VCell::FunctionDomainException(errorMsg);    
                }    
                result = MathUtil::acosh(argument);    
                break;    
            }    
        case ATANH:     
            {    
                if (jjtGetNumChildren()!=1)     
                    throw VCell::RuntimeException("atanh() expects 1 argument");    
                double argument = child0->evaluate(evalType, values);    
                if (fabs(argument) >= 1.0){    
                    char problem[1000];    
                    sprintf(problem, "atanh(u) and |u| >= 1.0, u=%lf", argument);    
                    string errorMsg = getFunctionDomainError(problem, values, "u", child0);    
                    throw VCell::FunctionDomainException(errorMsg);    
                }    
                result = MathUtil::atanh(argument);    
                break;    
            }    
        case ACSCH:     
            {    
                if (jjtGetNumChildren()!=1)     
                    throw VCell::RuntimeException("acsch() expects 1 argument");    
                double argument = child0->evaluate(evalType, values);    
                if (argument == 0.0){    
                    string errorMsg = getFunctionDomainError("acsch(u) and u=0", values, "u", child0);    
                    throw VCell::FunctionDomainException(errorMsg);    
                }                    
                result = MathUtil::acsch(argument);    
                break;    
            }    
        case ACOTH:     
            {    
                if (jjtGetNumChildren()!= 1)     
                    throw VCell::RuntimeException("acoth() expects 1 argument");    
                double argument = child0->evaluate(evalType, values);    
                if (fabs(argument) <= 1.0){    
                    char problem[1000];    
                    sprintf(problem, "acoth(u) and |u| <= 1.0, u=%lf", argument);    
                    string errorMsg = getFunctionDomainError(problem, values, "u", child0);    
                    throw VCell::FunctionDomainException(errorMsg);    
                }    
                result = MathUtil::acoth(argument);    
                break;    
            }    
        case ASECH:     
            {    
                if (jjtGetNumChildren()!=1)     
                    throw VCell::RuntimeException("asech() expects 1 argument");                    
                double argument = child0->evaluate(evalType, values);    
                if (argument <= 0.0 || argument > 1.0){    
                    char problem[1000];    
                    sprintf(problem, "asech(u) and u <= 0.0 or u > 1.0, u=%lf", argument);    
                    string errorMsg = getFunctionDomainError(problem, values, "u", child0);    
                    throw VCell::FunctionDomainException(errorMsg);    
                }    
                result = MathUtil::asech(argument);    
                break;    
            }    
        case FACTORIAL:     
            {    
                if (jjtGetNumChildren()!= 1)     
                    throw VCell::RuntimeException("factorial() expects 1 argument");    
                double argument = child0->evaluate(evalType, values);    
                if (argument < 0.0 || (argument-(int)argument) != 0){    
                    char problem[1000];    
                    sprintf(problem, "factorial(u) and u=%lf < 0.0, or u is not an integer", argument);    
                    string errorMsg = getFunctionDomainError(problem, values, "u", child0);    
                    throw VCell::FunctionDomainException(errorMsg);    
                }    
                result = MathUtil::factorial(argument);    
                break;    
            }    
        default :    
            {    
                throw VCell::RuntimeException("undefined function");    
            }    
    }    
    //result is NAN    
    if (MathUtil::double_infinity == -result || MathUtil::double_infinity == result || result != result) {    
        char problem[1000];    
        sprintf(problem, "%s evaluated to infinity or NaN", infixString(LANGUAGE_DEFAULT,0).c_str(), functionNamesVCML[funcType].c_str());    
        throw VCell::FunctionRangeException(problem);    
    }    
    return result;    
}    
    
Node* ASTFuncNode::copyTree() {    
    ASTFuncNode* node = new ASTFuncNode();    
    node->funcType = funcType;    
    node->funcName = funcName;    
    for (int i=0;i<jjtGetNumChildren();i++){    
        node->jjtAddChild(jjtGetChild(i)->copyTree());    
    }    
    return node;        
}    
    
bool ASTFuncNode::equals(Node* node) {    
    //    
    // check to see if the types and children are the same    
    //    
    if (!SimpleNode::equals(node)){    
        return false;    
    }    
        
    //    
    // check this node for same state (function name)    
    //        
    ASTFuncNode* funcNode = (ASTFuncNode*)node;    
    if (funcNode->funcType != funcType){    
        return false;    
    }    
    if (funcNode->funcName != funcName){    
        return false;    
    }    
    
    return true;    
}    
