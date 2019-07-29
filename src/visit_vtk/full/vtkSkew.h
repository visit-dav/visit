// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VTK_SKEW_H
#define VTK_SKEW_H
#include <math.h>

//
// Define skew and inverse skew in one place.
//

template <class T>
inline T
vtkSkewValue(T val, T min, T max, T factor)
{
    if (factor <= 0 || factor == 1. || min == max) 
        return val;

    T range = max - min; 
    T k = range / (factor - 1.);
    T t = (val - min) / range;
    T rv =  k * ((T)exp(t * (T)log(factor)) -1.) + min;
    return rv;
}

template <class T>
inline T
vtkInverseSkewValue(T val, T min, T max, T factor)
{
    if (factor <= 0 || factor == 1. || min == max) 
        return val;

    T rangeDif = max - min;
    T t = (val - min) / rangeDif;
    T t2 = (T)log(t * (factor - 1.) + 1.) / (T)log(factor);
    T rv = t2 * rangeDif + min;
    return rv;
}

#endif
