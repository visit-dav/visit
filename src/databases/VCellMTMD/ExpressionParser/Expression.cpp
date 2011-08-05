/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#include <typeinfo>    
#include <stdlib.h>    
#include <iostream>    
#include <sstream>    
#include <vector>
using std::cout;    
using std::endl;    
using std::istringstream;    
using std::istream;
using std::string;
using std::vector;
    
#include "Expression.h"    
#include "ExpressionParser.h"    
#include "ASTFloatNode.h"    
#include "ParseException.h"    
#include "ParserException.h"    
#include "StackMachine.h"    
    
//long Expression::flattenCount = 0;    
//long Expression::diffCount = 0;    
//long Expression::parseCount = 0;    
//long Expression::derivativeCount = 0;    
//long Expression::substituteCount = 0;    
//long Expression::bindCount = 0;    
    
VCell::Expression::Expression(void)    
{    
    rootNode = NULL;    
    stackMachine = NULL;    
}    
    
VCell::Expression::Expression(Expression* expression)    
{    
    this->rootNode = (SimpleNode*)expression->rootNode->copyTree();    
    stackMachine = NULL;    
}    
    
VCell::Expression::~Expression(void)    
{    
    delete rootNode;    
    delete stackMachine;    
}    
    
VCell::Expression::Expression(string expString)    
{    
    rootNode = 0;    
    stackMachine = NULL;    
    
    if (expString.length() == 0) {    
        throw ParserException("Empty expression");    
    }    
    
    bool bNumber = true;    
    for (unsigned int i = 0; i < expString.length(); i ++) {    
        if (!isdigit(expString[i]) && expString[i] != '.' && expString[i] != ';') {    
            bNumber = false;    
            break;    
        }    
    }    
    if (bNumber) {    
        double value = 0.0;    
        int n = sscanf(expString.c_str(), "%lf", &value);     
        if (n == 1) {    
            rootNode = new ASTFloatNode(value);    
            return;    
        }                    
    }     
    
    string trimstr = trim(expString);    
    if (trimstr[trimstr.length() - 1] != ';'){    
        trimstr += ";";    
    }    
    parseExpression(trimstr);    
}    
    
void VCell::Expression::showStackInstructions(void)    
{    
    getStackMachine()->showInstructions();    
    cout.flush();    
}    
    
double VCell::Expression::evaluateConstant(void)    
{    
    return getStackMachine()->evaluate(NULL);    
}    
    
double VCell::Expression::evaluateConstantTree()    
{    
    return rootNode->evaluate(EVALUATE_CONSTANT);    
}    
    
double VCell::Expression::evaluateVectorTree(double* values)    
{    
    try {    
        return rootNode->evaluate(EVALUATE_VECTOR, values);    
    } catch (Exception& ex) {    
        Exception::rethrowException(ex, ex.getMessage() + " in " + rootNode->getNodeSummary(values, rootNode));    
    }    
}    
    
double VCell::Expression::evaluateVector(double* values)    
{    
    try {    
        return getStackMachine()->evaluate(values);    
    } catch (Exception& ex) {    
        Exception::rethrowException(ex, ex.getMessage()+ " in " + rootNode->getNodeSummary(values, rootNode));    
    }    
}    
    
void VCell::Expression::parseExpression(string exp)    
{    
    //parseCount++;    
    try {    
        istringstream* iss = new istringstream(exp);    
        ExpressionParser* parser = new ExpressionParser(iss);    
        rootNode = parser->Expression();    
    
        if (typeid(*rootNode) == typeid(ASTExpression)){    
            if (rootNode->jjtGetNumChildren() == 1){ // we abandon the real root node here, so there is tiny memory leak;    
                rootNode = (SimpleNode*)rootNode->jjtGetChild(0);    
                rootNode->jjtSetParent(NULL);    
            }    
        }    
        delete iss;    
        delete parser;    
    } catch (Exception& e) {    
        throw ParserException("Parse Error while parsing expression " + e.getMessage());    
    }    
}    
    
string VCell::Expression::infix(void)    
{    
    return rootNode->infixString(LANGUAGE_DEFAULT, 0);    
}    
    
string VCell::Expression::infix_Visit(void)    
{    
    return rootNode->infixString(LANGUAGE_VISIT, 0);    
}    
    
void VCell::Expression::bindExpression(SymbolTable* symbolTable)    
{        
    //bindCount++;    
    rootNode->bind(symbolTable);    
}    
    
string VCell::Expression::trim(string str)    
{    
    int len = (int)str.length();    
    int st = 0;    
    const char* val = str.c_str();    
    
    while ((st < len) && (val[st] <= ' ')) {    
        st++;    
    }    
    while ((st < len) && (val[len - 1] <= ' ')) {    
        len--;    
    }    
    return ((st > 0) || (len < (int)str.length())) ?  str.substr(st, len-st) : str;    
}    
    
inline StackMachine* VCell::Expression::getStackMachine() {    
    if (stackMachine == NULL) {    
        vector<StackElement> elements_vector;    
        rootNode->getStackElements(elements_vector);    
        StackElement* elements = new StackElement[elements_vector.size()];    
        int i = 0;    
        for (vector<StackElement>::iterator iter = elements_vector.begin(); iter != elements_vector.end(); iter ++) {    
            elements[i ++] = *iter;    
        }    
        stackMachine = new StackMachine(elements, (int)elements_vector.size());    
        elements_vector.clear();    
    }    
    return stackMachine;    
}    
    
void VCell::Expression::getSymbols(vector<string>& symbols) {    
    rootNode->getSymbols(symbols, LANGUAGE_DEFAULT, 0);    
}    
    
SymbolTableEntry* VCell::Expression::getSymbolBinding(string symbol){    
    return rootNode->getBinding(symbol);    
}    
    
double VCell::Expression::evaluateProxy() {    
    return evaluateVector(0);    
}    
    
void VCell::Expression::substituteInPlace(Expression* origExp, Expression* newExp) {    
    SimpleNode* origNode = origExp->rootNode;    
    SimpleNode* newNode = (SimpleNode*)newExp->rootNode->copyTree();    
    //    
    // first check if must replace entire tree, if not then leaves can deal with it    
    //    
    if (origNode->equals(rootNode)){    
        rootNode = newNode;    
    } else {    
        rootNode->substitute(origNode, newNode);    
    }    
}    
