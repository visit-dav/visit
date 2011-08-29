/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
    
#include "ValueProxy.h"    
    
//-----------------------------------------------------------------    
//    
//  class ValueProxy    
//    
//-----------------------------------------------------------------    
ValueProxy::ValueProxy(double* arg_values, int arg_indexindex, int* arg_indices)    
{        
    values = arg_values;    
    indexindex = arg_indexindex;    
    indices = arg_indices;    
}    
    
double ValueProxy::evaluate(){    
    return values[indices[indexindex]];    
}    
