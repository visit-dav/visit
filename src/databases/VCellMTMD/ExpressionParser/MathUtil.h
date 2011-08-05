/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#ifndef MATHUTIL_H    
#define MATHUTIL_H    
    
class MathUtil {    
public:    
    static double acosh(double a);    
    static double acot(double a);     
    static double acoth(double a);    
    static double acsc(double a);    
    static double acsch(double a);        
    static double asec(double a);    
    static double asech(double a);        
    static double asinh(double a);        
    static double atanh(double a);    
    
    static double cot(double a);    
    static double coth(double a);        
    static double csc(double a);    
    static double csch(double a);    
    
    static double factorial(double n);    
    static double round(double x);    
    
    static double sec(double a);    
    static double sech(double a);    
    
    static double double_infinity;    
};    
#endif    
