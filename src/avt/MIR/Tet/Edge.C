// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "Edge.h"

// ****************************************************************************
//  Method:  Edge::HashFunction
//
//  Purpose:
//    Hash function for Edge's
//
//  Arguments:
//    edge       the edge
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
// ****************************************************************************
unsigned int
Edge::HashFunction(Edge &edge)
{
    return (edge.a<<4) ^ edge.b;
}

