/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#include "ExpressionException.h"    
    
ExpressionException::~ExpressionException(void)    
{    
}    
    
ExpressionException::ExpressionException(std::string msg) : Exception("ExpressionException", msg)    
{        
}    
    
ExpressionException::ExpressionException(std::string title, std::string msg) : Exception(title, msg)    
{        
}    
