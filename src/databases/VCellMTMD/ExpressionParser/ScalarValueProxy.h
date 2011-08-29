/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#ifndef SCALARVALUEPROXY_H    
#define SCALARVALUEPROXY_H    
    
#include "ValueProxy.h"    
    
class ScalarValueProxy : public ValueProxy    
{    
public:    
    ScalarValueProxy() : ValueProxy(0,  -1, 0) {    
        value = 0;    
    };    
    
    double evaluate() {    
        return value;    
    };    
    
    void setValue(double d) {    
        value = d;    
    }    
    
private:    
    double value;    
};    
    
#endif    
