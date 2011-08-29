/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#ifndef EXPRESSIONPARSERTREECONSTANTS_H    
#define EXPRESSIONPARSERTREECONSTANTS_H    
    
const int JJTEXPRESSION = 0;    
const int JJTVOID = 1;    
const int JJTORNODE = 2;    
const int JJTANDNODE = 3;    
const int JJTRELATIONALNODE = 4;    
const int JJTADDNODE = 5;    
const int JJTMINUSTERMNODE = 6;    
const int JJTMULTNODE = 7;    
const int JJTINVERTTERMNODE = 8;    
const int JJTPOWERNODE = 9;    
const int JJTNOTNODE = 10;    
const int JJTFUNCNODE = 11;    
const int JJTFLOATNODE = 12;    
const int JJTIDNODE = 13;    
    
const std::string jjtNodeName[] = {    
    "Expression",    
    "void",    
    "OrNode",    
    "AndNode",    
    "RelationalNode",    
    "AddNode",    
    "MinusTermNode",    
    "MultNode",    
    "InvertTermNode",    
    "PowerNode",    
    "NotNode",    
    "FuncNode",    
    "FloatNode",    
    "IdNode",    
};    
    
#endif    
