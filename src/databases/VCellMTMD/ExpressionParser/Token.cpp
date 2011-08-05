/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#include "Token.h"    
    
Token::Token(void)    
{    
    next = 0;    
    specialToken = 0;    
    kind = 0;    
    beginLine = 0;    
    beginColumn = 0;    
    endLine = 0;    
    endColumn = 0;    
}    
    
Token::~Token(void)    
{    
}    
    
Token* Token::newToken(int ofKind)    
{    
    switch (ofKind) {    
        default:    
            return new Token();    
    }    
}    
