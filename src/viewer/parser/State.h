#ifndef STATE_H
#define STATE_H
#include <viewer_parser_exports.h>

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
struct VIEWER_PARSER_API State
{
    std::map<const Symbol*, int>          shift;
    std::map<const Symbol*, int>          reduce;
    
    State() { }
    State(const ConfiguratingSet &cs) : shift(cs.shift), reduce(cs.reduce) { }
};

#endif
