/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#ifndef EXPRESSIONBINDINGEXCEPTION_H    
#define EXPRESSIONBINDINGEXCEPTION_H    
    
#include "ExpressionException.h"    

namespace VCell {

class ExpressionBindingException : public ExpressionException    
{    
public:    
    ExpressionBindingException(std::string msg);    
    ~ExpressionBindingException(void);    
    std::string identify();    
};    

}    
#endif    
