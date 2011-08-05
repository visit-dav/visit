/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#ifndef NAMESCOPE_H    
#define NAMESCOPE_H    
    
#include "stdinc.h"    
    
class NameScope {    
public:    
    NameScope** getChildren()=0;    
    SymbolTableEntry getExternalEntry(string identifier)=0;;    
    String getName();    
    NameScope* getNameScopeFromPrefix(string prefix)=0;;    
    NameScope* getParent()=0;;    
    string getRelativeScopePrefix(NameScope referenceNameScope)=0;;    
    ScopedSymbolTable getScopedSymbolTable()=0;;    
    string getSymbolName(SymbolTableEntry* symbolTableEntry)=0;;    
    string getUnboundSymbolName(String unboundName)=0;;    
    bool isAncestor(NameScope nameScope)=0;;    
    bool isPeer(NameScope nameScope)=0;;    
}    
    
#endif    
