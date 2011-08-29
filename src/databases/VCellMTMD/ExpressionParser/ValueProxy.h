/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#ifndef VALUEPROXY_H    
#define VALUEPROXY_H    
    
/*-----------------------------------------------------------    
    vartype is the index to the indices of array    
 ------------------------------------------------------------*/    
class ValueProxy    
{    
public:    
    ValueProxy(double* arg_values, int arg_vartype, int* arg_indices);      
    virtual double evaluate();    
    
protected:    
    double* values;    
    int indexindex;        
    int* indices;    
};    
    
#endif    
