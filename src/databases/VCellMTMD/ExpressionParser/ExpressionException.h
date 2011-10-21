/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#ifndef EXPRESSIONEXCEPTION_H    
#define EXPRESSIONEXCEPTION_H    
    
#include "Exception.h"    
namespace VCell { 

class ExpressionException : public VCell::Exception    
{    
public:    
    ~ExpressionException(void);    
    ExpressionException(std::string msg);    
    ExpressionException(std::string title, std::string msg);    
};    

}

#endif    
