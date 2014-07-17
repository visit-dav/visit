/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#ifndef    SIMPLESYMBOLTABLEENTRY_H    
#define SIMPLESYMBOLTABLEENTRY_H    
    
#include "SymbolTableEntry.h"    
    
class SimpleSymbolTableEntry : public SymbolTableEntry    
{    
public:    
    SimpleSymbolTableEntry(std::string& nameValue, int indexVal, NameScope* namescopeVal, ValueProxy* proxyVal);    
    virtual ~SimpleSymbolTableEntry();    
    double getConstantValue();    
    VCell::Expression* getExpression();    
    int getIndex();    
    std::string& getName();        
    NameScope* getNameScope();    
    //VCUnitDefinition getUnitDefinition()=0;    
    bool isConstant();        
    void setIndex(int symbolTableIndex);    
    void setConstantValue(double v);    
    ValueProxy* getValueProxy() { return valueProxy; };    
    
private:    
    std::string name;        
    int index;        
    NameScope* namescope;    
    bool bConstant;    
    double value;    
    ValueProxy* valueProxy;    
};    
#endif    
