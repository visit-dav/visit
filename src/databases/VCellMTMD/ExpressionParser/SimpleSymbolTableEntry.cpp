/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#include "SimpleSymbolTableEntry.h"    
#include "ExpressionException.h"    
#include <string>
    
using std::string;
using namespace VCell;

SimpleSymbolTableEntry::SimpleSymbolTableEntry(string& nameValue, int indexVal, NameScope* namescopeVal, ValueProxy* proxyVal)    
 : name(nameValue), index(indexVal), namescope(namescopeVal), valueProxy(proxyVal)    
{    
    bConstant = false;    
    value = 0.0;    
}    
    
SimpleSymbolTableEntry::~SimpleSymbolTableEntry(void)    
{    
}    
    
double SimpleSymbolTableEntry::getConstantValue() {    
    if (bConstant){    
        return value;    
    }    
    
    throw ExpressionException("can't evaluate to constant");    
}    
    
Expression* SimpleSymbolTableEntry::getExpression() {    
    return NULL;    
}    
    
int SimpleSymbolTableEntry::getIndex() {    
    return index;    
}    
    
string& SimpleSymbolTableEntry::getName() {    
    return name;    
}    
    
NameScope* SimpleSymbolTableEntry::getNameScope() {    
    return namescope;    
}    
    
bool SimpleSymbolTableEntry::isConstant() {    
    return bConstant;    
}    
    
void SimpleSymbolTableEntry::setIndex(int indexVal)    
{    
    index = indexVal;    
}    
    
void SimpleSymbolTableEntry::setConstantValue(double v)    
{    
    value = v;    
    bConstant = true;    
}    
