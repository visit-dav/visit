/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#ifndef SIMPLESYMBOLTABLE_H    
#define SIMPLESYMBOLTABLE_H    
    
#include <vector>    
    
#include "SymbolTable.h"    
#include "SimpleSymbolTableEntry.h"    
    
class SimpleSymbolTable : public SymbolTable    
{    
public:    
    SimpleSymbolTable(std::string* symbols, int symbolCount, ValueProxy** valueProxies=0);    
    //SimpleSymbolTable(std::string* symbols, NameScope* namescopeVal);    
    virtual ~SimpleSymbolTable();    
    SymbolTableEntry* getEntry(std::string identifier);    
    SymbolTableEntry* getLocalEntry(std::string identifier);    
    
private:    
    std::vector<SimpleSymbolTableEntry*> steArray;    
};    
    
#endif    
