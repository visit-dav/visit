# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

###############################################################################
# file: eval.py
# Purpose: Python helper functions for evaluating math functions.
#
# Programmer: Cyrus Harrison
# Creation: Thu Apr  5 14:55:25 PDT 2012
#
#
# Modifications:
#
#
###############################################################################


def EvalCubicSpline(t, allX, allY):
    n = len(allY)
    if((allX[0] > t) or (allX[n-1] < t)):
        raise Exception('t must be in the range between the first and last X')
    for i in range(1, n):
        if(allX[i] >= t):
            break
    i1 = max(i-2, 0)
    i2 = max(i-1, 0)
    i3 = i
    i4 = min(i+1, n-1)
    X = (allX[i1], allX[i2], allX[i3], allX[i4])
    Y = (allY[i1], allY[i2], allY[i3], allY[i4])
    dx = (X[2] - X[1])
    invdx = 1. / dx
    dy1   = (Y[2] + (Y[0] * -1.)) * (1. / (X[2] - X[0]))
    dy2   = (Y[2] + (Y[1] * -1.)) * invdx
    dy3   = (Y[3] + (Y[1] * -1.)) * (1. / (X[3] - X[1]))
    ddy2  = (dy2 + (dy1 * -1)) * invdx
    ddy3  = (dy3 + (dy2 * -1)) * invdx
    dddy3 = (ddy3 + (ddy2 * -1)) * invdx
    u = (t - X[1])
    return (Y[1] + dy1*u + ddy2*u*u + dddy3*u*u*(u-dx));

def EvalLinear(t, c0, c1):
    return ((c0*(1. - float(t))) + (c1*float(t)));

def EvalQuadratic(t, c0, c1, c2):
    T = float(t)
    T2 = T * T
    OMT = 1. - T
    OMT2 = OMT * OMT
    return ((c0*OMT2) + (c1*(2.*OMT*T)) + (c2*T2));

def EvalCubic(t, c0, c1, c2, c3):
    T = float(t)
    T2 = T * T
    T3 = T * T2
    OMT = 1. - T
    OMT2 = OMT * OMT
    OMT3 = OMT2 * OMT
    return ((c0*OMT3) + (c1*(3.*OMT2*T)) + (c2*(3.*OMT*T2)) + (c3*T3));
