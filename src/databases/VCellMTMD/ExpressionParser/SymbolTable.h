/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#ifndef SYMBOLTABLE_H    
#define SYMBOLTABLE_H    
    
#include <string>    
    
class SymbolTableEntry;    
    
class SymbolTable {    
public:    
    virtual SymbolTableEntry* getEntry(std::string identifierString)=0;     
    virtual ~SymbolTable() {}    
};    
    
#endif    
