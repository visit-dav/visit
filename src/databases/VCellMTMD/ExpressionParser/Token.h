/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#ifndef TOKEN_H    
#define TOKEN_H    
    
#include <string>    
    
class Token    
{    
public:    
    Token(void);    
    ~Token(void);    
    
    int kind;    
    int beginLine, beginColumn, endLine, endColumn;    
    std::string image;    
    
    Token* next;    
    Token* specialToken;    
    static Token* newToken(int ofKind);    
};    
    
#endif    
