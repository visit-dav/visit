/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#include <math.h>    
#include "MathUtil.h"    
    
#if (defined(LINUX) || defined(CYGWIN))    
#include <cmath>    
static double MathUtil::double_infinity = INFINITY;    
#else    
#include <limits>    
double MathUtil::double_infinity = std::numeric_limits<double>::infinity();    
#endif    
    
double MathUtil::acosh(double arg){    
    return log(arg+sqrt(arg*arg-1));    
}    
    
double MathUtil::acot(double arg){    
    return atan(1.0/arg);    
}    
    
double MathUtil::acoth(double arg){    
    return 0.5*log((arg + 1.0)/(arg - 1.0));    
}    
    
double MathUtil::acsc(double arg){    
    return asin(1.0/arg);    
}    
    
double MathUtil::acsch(double arg){    
    if (arg > 0) {    
        return log( (1.0 + sqrt(1 + arg * arg)) / arg );    
    } else if (arg < 0) {    
        return log( (1.0 - sqrt(1 + arg * arg)) / arg );    
    }    
}     
    
double MathUtil::asec(double arg){    
    return acos(1.0/arg);    
}    
    
double MathUtil::asech(double arg){    
    return log( (1.0 + sqrt(1.0 - arg * arg)) / arg );    
}    
    
double MathUtil::asinh(double arg){    
    return log(arg + sqrt(arg * arg + 1));    
}    
    
double MathUtil::atanh(double arg){    
    return 0.5*(log( (1.0 + arg)/(1.0 - arg) ));    
}    
    
double MathUtil::cot(double arg){    
    return 1.0/tan(arg);    
}    
    
double MathUtil::coth(double arg){    
    return 1.0/tanh(arg);    
}    
    
double MathUtil::csc(double arg){    
    return 1.0/sin(arg);    
}    
    
double MathUtil::csch(double arg){    
    return 1.0/sinh(arg);    
}    
    
double MathUtil::factorial(double arg){    
    double f = 1.0;    
    int n = (int)arg;    
    for(int i = 1; i <= n; i ++) {    
        f *= i;    
    }    
    return f;    
}    
    
double MathUtil::round(double arg) {    
    return floor(arg + 0.5);    
}    
    
double MathUtil::sec(double arg){    
    return 1.0/cos(arg);    
}    
    
double MathUtil::sech(double arg){    
    return 1.0/cosh(arg);    
}    
