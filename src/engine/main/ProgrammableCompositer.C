/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                       ProgrammableCompositer.C                            //
// ************************************************************************* //
#include <ProgrammableCompositer.h>

// ****************************************************************************
//  Helper Functions:
//
//  makeIteration -- given a rank ordering, make a list of 
//          sender/reciever pairs where the receiver composites
//          it's data (background) with the sender's data (foreground).
//          the reciever then has the data for the next iteration.
//
//  updateRanks -- remove senders from the previous iteration.
//          they no longer have data.
//
//  makeProgram -- Build up a program, a list of lists of pairs, where
//          each list of pairs, called an iteration, describes one
//          round of communication in a reduction.
//
//  printProgram -- display the program for debuging.
//
//  Purpose:
//      some helper code used to construct the program describing
//      compositing communications. Currently these methods build
//      a program describing a communication organized as a binary
//      tree, however the point of the program abstraction is that
//      other communication patterns can be easily implemented,
//      without modification to the consumer of the program.
//
//  Programmer: Burlen Loring
//  Creation: Thu Sep  3 22:24:59 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

// --------------------------------------------------------------------------
std::ostream &operator<<(std::ostream &os, const RankPair &p)
{
    os << p.first << " <- " << p.second;
    return os;
}

// --------------------------------------------------------------------------
void makeIteration(const RankOrdering &o, Iteration &p)
{
    size_t n = o.size();
    for (size_t i = 0; i < n; i += 2)
    {
        if ((i+1) < n)
            p.push_back(RankPair(o[i], o[i+1]));
    }
}

// --------------------------------------------------------------------------
void updateRanks(const RankOrdering &p, RankOrdering &q)
{
    q.resize(0);

    size_t n = p.size();

    if (n == 1)
        return;

    for (size_t i = 0; i < n; i += 2)
        q.push_back(p[i]);
}

// --------------------------------------------------------------------------
void makeProgram(const RankOrdering &o, Program &prog)
{
    Iteration it;
    makeIteration(o, it);
    if (it.size())
        prog.push_back(it);
    else
        return;

    RankOrdering no;
    updateRanks(o, no);

    makeProgram(no, prog);
}

// --------------------------------------------------------------------------
Program makeProgram(const RankOrdering &o)
{
    Program prog;
    makeProgram(o, prog);
    return prog;
}

// --------------------------------------------------------------------------
void printProgram(Program &prog)
{
    size_t n = prog.size();
    for (size_t i = 0; i < n; ++i)
        std::cerr << "round " << i << ":" << std::endl << prog[i] << std::endl;
}
