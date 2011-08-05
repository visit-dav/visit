/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#ifndef VCELL_EXPRESSION_H    
#define VCELL_EXPRESSION_H    
    
#include "SimpleNode.h"    
    
class SymbolTable;    
class SymbolTableEntry;    
class SimpleNode;    
class StackMachine;    
    
namespace VCell {    
    
class Expression    
{    
public:    
    Expression(void);    
    Expression(std::string expString);    
    Expression(Expression* expression);    
    ~Expression(void);    
    double evaluateConstant(void);    
    // exercise the old way of evaluating constant and vector by traversing abstract syntax tree    
    double evaluateConstantTree();    
    double evaluateVectorTree(double* values);    
    // exercise the new way of evaluating vector by using stack machine    
    double evaluateVector(double* values);    
    
    std::string infix(void);    
    void bindExpression(SymbolTable* symbolTable);    
    static std::string trim(std::string str);    
    void getSymbols(std::vector<std::string>& symbols);     
    
    SymbolTableEntry* getSymbolBinding(std::string symbol);    
    double evaluateProxy();    
        
    void showStackInstructions();    
    void substituteInPlace(Expression* origExp, Expression* newExp);    
    std::string infix_Visit(void);    
    
private:    
    SimpleNode  *rootNode;    
    
    //static long flattenCount;    
    //static long diffCount;    
    //static long parseCount;    
    //static long derivativeCount;    
    //static long substituteCount;    
    //static long bindCount;    
    void parseExpression(std::string exp);    
    StackMachine* stackMachine;    
    inline StackMachine* getStackMachine();    
};    
}    
#endif    
