/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#ifndef SYMBOLTABLEENTRY_H    
#define SYMBOLTABLEENTRY_H    
    
#include <string>    
    
class ValueProxy;    
namespace VCell {    
class Expression;    
}    
class NameScope;    
    
class SymbolTableEntry {    
public:
    virtual ~SymbolTableEntry(){}
    virtual double getConstantValue()=0;    
    virtual VCell::Expression* getExpression()=0;    
    virtual int getIndex()=0;    
    virtual std::string& getName()=0;        
    virtual NameScope* getNameScope()=0;    
    //VCUnitDefinition getUnitDefinition()=0;    
    virtual bool isConstant()=0;    
    virtual ValueProxy* getValueProxy()=0;    
};    
    
#endif    
