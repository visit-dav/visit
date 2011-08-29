/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#ifndef STACKMACHINE_H    
#define STACKMACHINE_H    
    
/**    
 * for expression: "(((1 < 2) * 3) + (5 * log(0.0) * (0.0 > 0.0)) + 2)"    
 *     
 * The following are the Stack Machine Instructions (e.g. FLOAT 1 means push 1.0 on the stack,    
 * and LT() means Stack[top] = (Stack[top] < Stack[top+1]))    
 *      
 * FLOAT 1    
 * FLOAT 2    
 * LT()    
 * BZ  2    
 * FLOAT 3    
 * FLOAT 0    
 * FLOAT 0    
 * GT()    
 * BZ  5    
 * FLOAT 5    
 * FLOAT 0    
 * LOG()    
 * MULT()    
 * ADD()    
 * FLOAT 2    
 * ADD()    
 *     
 * which results in the answer 5.0    
 *     
 * Note that "BZ offset" means branch to IP+offset if Stack[top] is zero and doesn't modify the stack.    
**/    
    
enum STACK_ELEMENT_TYPE {TYPE_BZ = 1, TYPE_LT, TYPE_GT, TYPE_LE, TYPE_GE, TYPE_EQ, TYPE_NE,     
    TYPE_AND, TYPE_OR, TYPE_NOT, TYPE_ADD, TYPE_SUB, TYPE_MULT,     
    TYPE_DIV, TYPE_FLOAT, TYPE_IDENTIFIER, TYPE_EXP, TYPE_SQRT, TYPE_ABS, TYPE_POW,     
    TYPE_LOG, TYPE_SIN, TYPE_COS, TYPE_TAN, TYPE_ASIN, TYPE_ACOS, TYPE_ATAN,     
    TYPE_ATAN2, TYPE_MAX, TYPE_MIN, TYPE_CEIL, TYPE_FLOOR, TYPE_CSC, TYPE_COT,     
    TYPE_SEC, TYPE_ACSC, TYPE_ACOT, TYPE_ASEC, TYPE_SINH, TYPE_COSH, TYPE_TANH,     
    TYPE_CSCH, TYPE_COTH, TYPE_SECH, TYPE_ASINH, TYPE_ACOSH, TYPE_ATANH, TYPE_ACSCH,     
    TYPE_ACOTH, TYPE_ASECH, TYPE_FACTORIAL};    // totally 52    
    
class ValueProxy;    
    
struct StackElement {    
    int type;    
    int branchOffset;    
    double value;    
    int vectorIndex;    
    ValueProxy* valueProxy;    
    
    StackElement() {    
        type = 0;    
        value = 0;    
        valueProxy = 0;    
        vectorIndex = 0;    
        branchOffset = 0;    
    }    
    
    StackElement(int arg_type) {    
        type = arg_type;    
        value = 0.0;    
        valueProxy = 0;    
        vectorIndex = 0;    
        branchOffset = 0;    
    }    
    
    StackElement(ValueProxy* arg_vp, int index) {    
        type = TYPE_IDENTIFIER;    
        value = 0.0;    
        valueProxy = arg_vp;    
        vectorIndex = index;    
        branchOffset = 0;    
    }    
    StackElement(double arg_value) {    
        type = TYPE_FLOAT;    
        value = arg_value;    
        valueProxy = 0;    
        vectorIndex = 0;    
        branchOffset = 0;    
    }    
};    
    
class StackMachine {    
private:    
    //double *workingStack;    
    StackElement* elements;    
    int elementSize;    
    
public:    
    StackMachine(StackElement* arg_elements, int size);    
    ~StackMachine();    
    double evaluate(double* values=0);        
    void showInstructions();    
};    
        
#endif    
