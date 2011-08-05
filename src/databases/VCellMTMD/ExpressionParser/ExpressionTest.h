/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#ifndef EXPRESSIONTEST_H    
#define EXPRESSIONTEST_H    
    
class SymbolTable;    
    
class ExpressionTest    
{    
public:    
    ExpressionTest(void);    
    ~ExpressionTest(void);    
    static void testEvaluateVector(void);    
    static void testEvaluateConstant(void);    
    static void testParser(char* filename);    
    static void testParser(int count, char* javaresult, double cvalue, char* expStr, SymbolTable* symbolTable, double* values);    
};    
    
#endif    
