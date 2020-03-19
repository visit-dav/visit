// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
