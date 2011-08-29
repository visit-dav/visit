/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
    
#include "SimpleNode.h"    
#include "StackMachine.h"    
#include <math.h>    
#include <stdio.h>
#include "MathUtil.h"    
#include "DivideByZeroException.h"    
#include "FunctionDomainException.h"    
#include "FunctionRangeException.h"    
#include "Exception.h"    
#include "ValueProxy.h"    
#include <iostream>    
#include <algorithm>    
#include <string>
using std::cout;    
using std::endl;    
using std::max;    
using std::min;    
using std::string;
using std::vector;
    
using namespace VCell;

StackMachine::StackMachine(StackElement* arg_elements, int size) {    
    elements = arg_elements;    
    elementSize = size;    
}    
    
StackMachine::~StackMachine() {    
    delete[] elements;    
}    
//-----------------------------------------------------------------    
//    
//  class StackMachine    
//    
//-----------------------------------------------------------------    
    
void StackMachine::showInstructions(){    
    char* opCodes[52] = { "unknown", "BZ", "LT", "GT", "LE", "GE", "EQ", "NE",     
                            "AND", "OR", "NOT", "ADD", "SUB", "MULT",     
                            "DIV", "FLOAT", "IDENTIFIER", "EXP", "SQRT", "ABS", "POW",     
                            "LOG", "SIN", "COS", "TAN", "ASIN", "ACOS", "ATAN",     
                            "TAN2", "MAX", "MIN", "CEIL", "FLOOR", "CSC", "COT",     
                            "SEC", "ACSC", "ACOT", "ASEC", "SINH", "COSH", "TANH",     
                            "CSCH", "COTH", "SECH", "ASINH", "ACOSH", "ATANH", "ACSCH",     
                            "ACOTH", "ASECH", "FACTORIAL"};            
    StackElement *token = elements;    
    for (int i=0; i<elementSize; i++, token++){    
        if (token->type==TYPE_BZ){    
            cout << "BZ  " << token->branchOffset << endl;    
        }else if (token->type==TYPE_IDENTIFIER){    
            cout << "PUSH " << "var[" << token->vectorIndex << "]" << endl;    
        }else if (token->type==TYPE_FLOAT){    
            cout << "PUSH " << token->value << endl;    
        }else{    
            cout << opCodes[token->type] << "()" << endl;    
        }    
    }    
}    
    
double StackMachine::evaluate(double* values){        
    double workingStack[20];    
    StackElement *token = elements;    
    double *tos = workingStack-1;    
    //    
    // iterate through tokens and push floats and identifiers (after evaluation)    
    // when get to an operator, evaluate and place results back on stack    
    //    
    int i=0;    
    double arg2;    
    for (i=0; i<elementSize; i++, token++){    
        switch (token->type){    
            case TYPE_BZ: // 1: read tos - branch if zero to offset    
                arg2 = *(tos);    
                if (arg2==0.0){    
                    //    
                    // branch offset    
                    //    
                    int offset = token->branchOffset;    
                    if (offset == 0) {    
                        showInstructions();    
                        throw "StackMachine::evaluate(), BZ with zero offset, should never happend";    
                    }    
                    token+=(offset-1); // because token++, i++ will add one    
                    i+=(offset-1);    
                } else {    
                    tos --;    
                }    
                break;    
            case TYPE_LT: // 2: pop 2 push 1    
                arg2 = *(tos--);    
                *tos = *tos < arg2;    
                break;    
            case TYPE_GT: // 3: pop 2 push 1    
                arg2 = *(tos--);    
                *tos = *tos > arg2;    
                break;    
            case TYPE_LE: // 4: pop 2 push 1    
                arg2 = *(tos--);    
                *tos = *tos <= arg2;    
                break;    
            case TYPE_GE: // 5: pop 2 push 1    
                arg2 = *(tos--);    
                *tos = *tos >= arg2;    
                break;    
            case TYPE_EQ: // 6, pop 2 push 1    
                arg2 = *(tos--);    
                *tos = *tos == arg2;    
                break;    
            case TYPE_NE: // 7, pop 2 push 1    
                arg2 = *(tos--);    
                *tos = *tos != arg2;    
                break;    
            case TYPE_AND: // 8, pop 2 push 1    
                arg2 = *(tos--);    
                *tos = *tos && arg2;    
                break;    
            case TYPE_OR: // 9, pop 2 push 1    
                arg2 = *(tos--);    
                *tos = *tos || arg2;    
                break;    
            case TYPE_NOT: // 10, pop 1 push 1    
                *tos = !*tos;    
                break;    
            case TYPE_ADD: // 11, pop 2 push 1    
                arg2 = *(tos--);    
                *tos += arg2;    
                break;    
            case TYPE_SUB: // 12, pop 1 push 1    
                *tos = -*tos;    
                break;    
            case TYPE_MULT: // 13, pop 2 push 1    
                arg2 = *(tos--);    
                *tos *= arg2;    
                break;                
            case TYPE_DIV: // 14, pop 1 push 1        
                if (*tos == 0.0) {    
                    throw DivideByZeroException("divide by zero");    
                }    
                *tos = 1/(*tos);    
                break;    
            case TYPE_FLOAT: // 15, push 1    
                *(++tos) = token->value; // push 1 float onto the stack.    
                break;     
            case TYPE_IDENTIFIER: // 16, push 1    
                if (values == 0){    
                    *(++tos) = token->valueProxy->evaluate(); // push identifier's value onto stack    
                } else {    
                    *(++tos) = values[token->vectorIndex]; // push identifier's value onto stack    
                }    
                break;     
            case TYPE_EXP: // 17, pop 1 push 1        
                *tos = exp(*tos);    
                break;    
            case TYPE_SQRT: // 18, pop 1 push 1        
                if (*tos < 0) {    
                    char problem[1000];    
                    sprintf(problem, "sqrt(u) where u=%lf<0 is undefined", *tos);                        
                    throw FunctionDomainException(string(problem));    
                }    
                *tos = sqrt(*tos);                    
                break;    
            case TYPE_ABS: // 19, pop 1 push 1        
                *tos = fabs(*tos);    
                break;    
            case TYPE_POW: // 20, pop 2 push 1        
                arg2 = *(tos--);    
                if (*tos < 0.0 && (MathUtil::round(arg2) != arg2)) {    
                    char problem[1000];    
                    sprintf(problem, "pow(u,v) and u=%lf<0 and v=%lf not an integer", *tos, arg2);                        
                    throw FunctionDomainException(string(problem));    
                } else if (*tos == 0.0 && arg2 < 0) {    
                    char problem[100];    
                    sprintf(problem, "pow(u,v) and u=0 and v=%lf<0 divide by zero", arg2);    
                    throw FunctionDomainException(string(problem));    
                }     
                if (arg2 == 0.0) {    
                    *tos = 1.0;    
                } else if (*tos == 1.0) {    
                    *tos = 1.0;    
                } else {    
                    double result = pow(*tos, arg2);        
                    if (MathUtil::double_infinity == -result || MathUtil::double_infinity == result || result != result) {    
                        char problem[1000];    
                        sprintf(problem, "u^v evaluated to %lf, u=%lf, v=%lf", result, *tos, arg2);    
                        throw FunctionDomainException(string(problem));    
                    }    
                    *tos = result;    
                }    
                break;     
            case TYPE_LOG: // 21, pop 1 push 1        
                if (*tos == 0.0) {                        
                    throw FunctionDomainException("log(u) and u==0.0 is undefined");    
                } else if (*tos < 0.0) {    
                    char problem[1000];                        
                    sprintf(problem, "log(u) and u=%lf < 0.0 is undefined", *tos);                        
                    throw FunctionDomainException(string(problem));    
                }     
                *tos = log(*tos);                    
                break;    
            case TYPE_SIN: // 22, pop 1 push 1        
                *tos = sin(*tos);    
                break;    
            case TYPE_COS: // 23, pop 1 push 1        
                *tos = cos(*tos);    
                break;    
            case TYPE_TAN: // 24, op 1 push 1        
                *tos = tan(*tos);    
                break;    
            case TYPE_ASIN: // 25, pop 1 push 1        
                if (fabs(*tos) > 1.0) {    
                    char problem[1000];    
                    sprintf(problem, "asin(u) and u=%lf and |u|>1.0 undefined", *tos);    
                    throw FunctionDomainException(string(problem));    
                }    
                *tos = asin(*tos);                    
                break;    
            case TYPE_ACOS: // 26, pop 1 push 1        
                if (fabs(*tos) > 1.0) {    
                    char problem[1000];    
                    sprintf(problem, "acos(u) and u=%lf and |u|>1.0 undefined", *tos);    
                    throw FunctionDomainException(string(problem));    
                }    
                *tos = acos(*tos);                
                break;    
            case TYPE_ATAN: // 27, pop 1 push 1        
                *tos = atan(*tos);    
                break;    
            case TYPE_ATAN2: // 28, pop 2 push 1        
                arg2 = *(tos--);    
                *tos = atan2(*tos, arg2);        
                break;     
            case TYPE_MAX: // 29, pop 2 push 1        
                arg2 = *(tos--);    
                *tos = max<double>(*tos, arg2);    
                break;    
            case TYPE_MIN: // 30, pop 2 push 1        
                arg2 = *(tos--);    
                *tos = min<double>(*tos, arg2);    
                break;    
            case TYPE_CEIL: // 31, pop 1 push 1        
                *tos = ceil(*tos);    
                break;    
            case TYPE_FLOOR: // 32, pop 1 push 1        
                *tos = floor(*tos);    
                break;    
            case TYPE_CSC: // 33, pop 1 push 1        
            {    
                double result = sin(*tos);    
                if (result == 0) {    
                    char problem[1000];    
                    sprintf(problem, "csc(u)=1/sin(u) and sin(u)=0 and u=%lf", *tos);    
                    throw FunctionDomainException(string(problem));    
                }    
                *tos = 1/result;                    
            }    
            break;    
            case TYPE_COT: // 34, pop 1 push 1        
            {    
                double result = tan(*tos);    
                if (result == 0) {    
                    char problem[1000];    
                    sprintf(problem, "cot(u)=1/tan(u) and tan(u)=0 and u=%lf", *tos);    
                    throw FunctionDomainException(string(problem));    
                }    
                *tos = 1/result;                    
            }    
                break;    
            case TYPE_SEC: // 35, pop 1 push 1        
            {    
                double result = cos(*tos);    
                if (result == 0) {    
                    char problem[1000];    
                    sprintf(problem, "sec(u)=1/cos(u) and cos(u)=0 and u=%lf", *tos);    
                    throw FunctionDomainException(string(problem));    
                }    
                *tos = 1/result;                    
            }    
                break;    
            case TYPE_ACSC: // 36, pop 1 push 1        
                if (fabs(*tos) < 1.0){    
                    char problem[1000];    
                    sprintf(problem, "acsc(u) and -1<u=%lf<1 undefined", *tos);    
                    throw FunctionDomainException(string(problem));    
                }    
                *tos = MathUtil::acsc(*tos);    
                break;    
            case TYPE_ACOT: // 37, pop 1 push 1        
                /*    
                if (*tos == 0) {    
                    throw FunctionDomainException("acot(u)=atan(1/u) and u=0");    
                }    
                */    
                *tos = MathUtil::acot(*tos);    
                break;    
            case TYPE_ASEC: // 38, pop 1 push 1        
                if (fabs(*tos) < 1.0){    
                    char problem[1000];    
                    sprintf(problem, "asec(u) and -1<u=%lf<1 undefined", *tos);    
                    throw FunctionDomainException(string(problem));    
                }    
                *tos = MathUtil::asec(*tos);    
                break;    
            case TYPE_SINH: // 39, pop 1 push 1        
                *tos = sinh(*tos);    
                break;    
            case TYPE_COSH: // 40, pop 1 push 1        
                *tos = cosh(*tos);    
                break;    
            case TYPE_TANH: // 41, pop 1 push 1        
                *tos = tanh(*tos);    
                break;     
            case TYPE_CSCH: // 42, pop 1 push 1        
                if (*tos == 0.0){    
                    throw FunctionDomainException("csch(u) and u = 0");    
                }    
                *tos = MathUtil::csch(*tos);    
                break;    
            case TYPE_COTH: // 43, pop 1 push 1        
                if (*tos == 0.0){    
                    throw FunctionDomainException("coth(u) and u = 0");    
                }    
                *tos = MathUtil::coth(*tos);    
                break;    
            case TYPE_SECH: // 44, pop 1 push 1        
                *tos = MathUtil::sech(*tos);    
                break;    
            case TYPE_ASINH: // 45, pop 1 push 1        
                *tos = MathUtil::asinh(*tos);    
                break;    
            case TYPE_ACOSH: // 46, pop 1 push 1        
                if (*tos < 1.0){    
                    char problem[1000];    
                    sprintf(problem, "acosh(u) and u=%lf<1.0", *tos);    
                    throw FunctionDomainException(string(problem));    
                }    
                *tos = MathUtil::acosh(*tos);    
                break;    
            case TYPE_ATANH: // 47, pop 1 push 1        
                if (fabs(*tos) >= 1.0){    
                    char problem[1000];    
                    sprintf(problem, "atanh(u) and |u| >= 1.0, u=%lf", *tos);    
                    throw FunctionDomainException(string(problem));    
                }    
                *tos = MathUtil::atanh(*tos);    
                break;    
            case TYPE_ACSCH: // 48, pop 1 push 1        
                if (*tos == 0.0){    
                    throw FunctionDomainException("acsch(u) and u=0");    
                }                    
                *tos = MathUtil::acsch(*tos);    
                break;    
            case TYPE_ACOTH: // 49, pop 1 push 1        
                if (fabs(*tos) <= 1.0){    
                    char problem[1000];    
                    sprintf(problem, "acoth(u) and |u| <= 1.0, u=%lf", *tos);    
                    throw FunctionDomainException(string(problem));    
                }    
                *tos = MathUtil::acoth(*tos);    
                break;    
            case TYPE_ASECH: // 50, pop 1 push 1    
                if (*tos <= 0.0 || *tos > 1.0){    
                    char problem[1000];    
                    sprintf(problem, "asech(u) and u <= 0.0 or u > 1.0, u=%lf", *tos);    
                    throw FunctionDomainException(string(problem));    
                }    
                *tos = MathUtil::asech(*tos);    
                break;    
            case TYPE_FACTORIAL: // 51, pop 1 push 1    
                if (*tos < 0.0 || (*tos-(int)*tos) != 0){    
                    char problem[1000];    
                    sprintf(problem, "factorial(u) and u=%lf < 0.0 or is not an integer", *tos);    
                    throw FunctionDomainException(string(problem));    
                }    
                *tos = MathUtil::factorial(*tos);    
                break;    
            default:    
                throw Exception("StackMachine: unknown stack element type " + token->type);    
        }    
        if (tos >= workingStack) {    
            if (MathUtil::double_infinity == -*tos || MathUtil::double_infinity == *tos) {    
                throw FunctionRangeException("Evaluated to infinity");    
            } else if (*tos != *tos) {    
                throw FunctionRangeException("Evaluated to NaN");    
            }    
        }    
        //cout << "------" << i << "--------" << endl;    
        //for (int c = 0; c < tos-workingStack+1; c++) {    
        //    cout << c << " " << workingStack[c]  << endl;    
        //}    
    }            
    return *tos;    
}    
