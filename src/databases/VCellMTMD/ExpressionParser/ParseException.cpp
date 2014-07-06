/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#include <stdio.h>    
#include <string>
    
#include "ParseException.h"    
using std::string;
    
string VCell::ParseException::eol = string("\n");    
    
VCell::ParseException::ParseException() : VCell::Exception("ParseException", "")    
{        
    specialConstructor = false;    
    currentToken = NULL;    
    expectedTokenSequences = NULL;    
    numETS = 0;    
    etsLengthArray = NULL;    
    tokenImage = 0;    
}    
    
VCell::ParseException::ParseException(string msg) : VCell::Exception("ParseException", msg)    
{        
    specialConstructor = false;    
    currentToken = NULL;    
    expectedTokenSequences = NULL;    
    numETS = 0;    
    etsLengthArray = NULL;    
    tokenImage = 0;    
}    
    
VCell::ParseException::~ParseException(void)    
{    
}    
    
VCell::ParseException::ParseException(Token* currentTokenVal, int** expectedTokenSequencesVal, int numETSVal, int* etsLengthArrayVal, const string* tokenImageVal) : Exception("ParseException: ", "")    
{    
    specialConstructor = true;    
    currentToken = currentTokenVal;    
    expectedTokenSequences = expectedTokenSequencesVal;    
    numETS = numETSVal;    
    etsLengthArray = etsLengthArrayVal;    
    tokenImage = tokenImageVal;    
}    
    
string VCell::ParseException::getExactMessage(void)    
{    
    if (!specialConstructor) {    
        return Exception::getExactMessage();    
    }    
            
    string expected = "";    
    int maxSize = 0;    
    for (int i = 0; i < numETS; i++) {    
        if (maxSize < etsLengthArray[i]) {    
            maxSize = etsLengthArray[i];    
        }    
        for (int j = 0; j < etsLengthArray[i]; j++) {    
            expected += tokenImage[expectedTokenSequences[i][j]] + " ";    
        }    
        if (expectedTokenSequences[i][etsLengthArray[i] - 1] != 0) {    
            expected += "...";    
        }    
        expected += eol + "    ";    
    }    
    string retval("Encountered \"");    
    Token* tok = currentToken->next;    
    for (int i = 0; i < maxSize; i++) {    
        if (i != 0)     
            retval += " ";    
        if (tok->kind == 0) {    
            retval += tokenImage[0];    
            break;    
        }    
        retval += add_escapes(tok->image);    
        tok = tok->next;     
    }    
    char chrs[128];    
    sprintf(chrs, "\" at line %d, column %d", currentToken->next->beginLine, currentToken->next->beginColumn);    
    chrs[127] = '\0';
    retval += chrs;    
    retval += "." + eol;    
    if (numETS == 1) {    
        retval += "Was expecting:" + eol + "    ";    
    } else {    
        retval += "Was expecting one of:" + eol + "    ";    
    }    
    retval += expected;    
    return retval;    
}    
