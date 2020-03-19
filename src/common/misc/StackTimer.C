// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.
// ************************************************************************* //
//                              StackTimer.C                                 //
// ************************************************************************* //

#include <StackTimer.h>

// ****************************************************************************
//  StackTimer Constructor
//
//  Purpose:
//
//  Programmer: Tom Fogal
//  Creation:   July 2, 2007
//
//  Modifications:
//
// ****************************************************************************

StackTimer::StackTimer(const char *msg) 
{
    this->message = std::string(msg);
    this->timer_index = visitTimer->StartTimer();
}

// ****************************************************************************
//  StackTimer string Constructor
//
//  Purpose: Easier to use (sometimes) version of the timer.
//
//  Programmer: Tom Fogal
//  Creation:   July 25, 2007
//
// ****************************************************************************

StackTimer::StackTimer(const std::string &msg): message(msg)
{
    this->timer_index = visitTimer->StartTimer();
}

StackTimer::~StackTimer() 
{
    visitTimer->StopTimer(this->timer_index, this->message);
}
