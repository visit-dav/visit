/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#ifndef DIVIDEBYZEROEXCEPTION_H    
#define DIVIDEBYZEROEXCEPTION_CPP    
    
#include "ExpressionException.h"    
    
class DivideByZeroException : public ExpressionException    
{    
public:    
    DivideByZeroException(std::string msg);    
    ~DivideByZeroException(void);    
};    
#endif    
