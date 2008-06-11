/*****************************************************************************
*
* Copyright (c) 2000 - 2008, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                              StackTimer.h                                 //
// ************************************************************************* //

#ifndef STACK_TIMER_H
#define STACK_TIMER_H

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
//
// ****************************************************************************

class MISC_API StackTimer
{
  public:
    StackTimer(const char *msg) {
        this->message = std::string(msg);
        this->timer_index = visitTimer->StartTimer();
    }
    ~StackTimer() {
        visitTimer->StopTimer(this->timer_index, this->message);
    }

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
