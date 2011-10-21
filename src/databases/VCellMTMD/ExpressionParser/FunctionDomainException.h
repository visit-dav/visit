/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#ifndef FUNCTIONDOMAINEXCEPTION_H    
#define FUNCTIONDOMAINEXCEPTION_H    
    
#include "ExpressionException.h"    

namespace VCell {

class FunctionDomainException : public ExpressionException    
{    
public:    
    FunctionDomainException(std::string msg);    
    ~FunctionDomainException(void);    
};

}    
#endif    
