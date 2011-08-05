/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#ifndef PARSEEXCEPTION_H    
#define PARSEEXCEPTION_H    
    
#include "Exception.h"    
#include "Token.h"    
    
class ParseException : public VCell::Exception    
{    
public:    
    ParseException();    
    ParseException(std::string msg);    
    ~ParseException(void);    
    Token* currentToken;    
    int** expectedTokenSequences;    
    int numETS;    
    int* etsLengthArray;    
    const std::string* tokenImage;    
    ParseException(Token* currentTokenVal, int** expectedTokenSequencesVal, int numETSVal, int* etsLengthArrayVal, const std::string* tokenImageVal);    
    std::string getExactMessage(void);    
    
protected:    
    bool specialConstructor;      
    static std::string eol;        
};    
#endif    
