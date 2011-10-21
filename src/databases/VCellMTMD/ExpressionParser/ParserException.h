/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#ifndef PARSEREXCEPTION_H    
#define PARSEREXCEPTION_H    
    
#include "ExpressionException.h"    

namespace VCell {

class ParserException : public ExpressionException    
{    
public:    
    ParserException(std::string msg);    
    ~ParserException(void);    
};    

}    
#endif    
