// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              StackTimer.h                                 //
// ************************************************************************* //

#ifndef STACK_TIMER_H
#define STACK_TIMER_H

#include <string>

#include <TimingsManager.h>

// ****************************************************************************
//  Class: StackTimer
//
//  Purpose:
//      Simplified mechanism for timing blocks of code.  Create a StackTimer on
//      the stack and it will record timing information when it goes out of
//      scope.  For example:
//
//         if(doLongComplicatedTask) {
//             StackTimer task_identifier("my task");
//             this->Function();
//         }
//
//      For short blocks, it may be easier to use the macro:
//
//         TimedCodeBlock("my task", this->Function());
//
//  Programmer: Tom Fogal
//  Creation:   July 2, 2007
//
//  Modifications:
//    Kathleen Bonnell, Fri Jun 20 08:15:27 PDT 2008
//    Moved constructor and destructor to 'C' file, in order for this class'
//    methods to be available on windows platform.
//
//    Tom Fogal, Mon Aug 25 09:54:55 EDT 2008
//    Made a std::string-based constructor.
//
// ****************************************************************************

class MISC_API StackTimer
{
  public:
    StackTimer(const std::string &);
    StackTimer(const char *msg); 
    ~StackTimer();

  private:

    // Unimplemented.  It does not make sense to have a timer without a
    // message: there will be no way to identify it.
    StackTimer();
    // Unimplemented.  You should not be trying to pass timers around.
    StackTimer(const StackTimer &);
    const StackTimer & operator =(const StackTimer &);

    // Unimplemented.  Allowing pointers would prevent automatic destruction
    // and defeat the whole purpose.
    static void *operator new(size_t);

  private:

    std::string                message;
    int                        timer_index;
};

#define TimedCodeBlock(msg, block)            \
    do {                                      \
        StackTimer _generic_stack_timer(msg); \
        block;                                \
    } while(0)

#endif /* STACK_TIMER_H */
