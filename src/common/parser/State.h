// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef STATE_H
#define STATE_H
#include <parser_exports.h>

#include <map>
#include "ConfiguratingSet.h"

// ****************************************************************************
//  Class:  State
//
//  Purpose:
//    A parse state; maps from symbols to shift/reduce rules.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
struct PARSER_API State
{
    std::map<const Symbol*, int>          shift;
    std::map<const Symbol*, int>          reduce;
    
    State() { }
    State(const ConfiguratingSet &cs) : shift(cs.shift), reduce(cs.reduce) { }
};

#endif
