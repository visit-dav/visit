/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#ifndef NODE_H    
#define NODE_H    
    
#include <string>    
#include <vector>    
    
#define LANGUAGE_DEFAULT  0    
#define LANGUAGE_C 1    
#define LANGUAGE_MATLAB  2    
#define LANGUAGE_JSCL 3    
#define LANGUAGE_VISIT 5    
    
#define EVALUATE_CONSTANT  111    
#define EVALUATE_VECTOR 112    
#define EVALUATE_PROXY  113    
    
class NameScope;    
class SymbolTableEntry;    
struct StackElement;    
class SymbolTable;    
    
class Node    
{    
public:        
    virtual ~Node() { }    
    /*    
    virtual std::string code()=0;    
    virtual Node copyTreeBinary()=0;    
    virtual Node differentiate(std::string independentVariable)=0;    
        
    virtual RealInterval evaluateInterval(RealInterval intervals[])=0;    
    virtual  RealInterval getInterval(RealInterval intervals[]) throws ExpressionBindingException;        
    virtual String infixString(int lang, NameScope nameScope);    
    */    
    virtual Node* copyTree()=0;    
    virtual void jjtAddChild(Node* n)=0;     
    
    virtual bool isBoolean()=0;    
    virtual void bind(SymbolTable* symbolTable)=0;    
    virtual SymbolTableEntry* getBinding(std::string symbol)=0;    
    virtual void getSymbols(std::vector<std::string>& symbols, int language, NameScope* nameScope)=0;      
    
    //virtual Node* flatten()=0;    
    virtual std::string infixString(int lang, NameScope* nameScope)=0;      
    virtual void jjtAddChild(Node* n, int i)=0;      
    virtual void jjtClose()=0;      
    virtual Node* jjtGetChild(int i)=0;      
    virtual int jjtGetNumChildren()=0;    
    virtual Node* jjtGetParent()=0;      
    virtual void jjtOpen()=0;      
    virtual void jjtSetParent(Node* n)=0;      
    
    virtual void getStackElements(std::vector<StackElement>& elements)=0;    
    virtual double evaluate(int type, double* values=0)=0;        
    /*    
    virtual bool narrow(RealInterval intervals[]) throws ExpressionBindingException;    
    virtual void roundToFloat();    
    virtual  void setInterval(RealInterval interval, RealInterval intervals[]) throws ExpressionBindingException;    
    virtual void substituteBoundSymbols() throws ExpressionException;    
    */    
    virtual bool equals(Node* node)=0;    
    virtual void substitute(Node* origNode, Node* newNode)=0;    
};    
    
#endif    
