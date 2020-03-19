// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef MIR_OPTIONS_H
#define MIR_OPTIONS_H
#include <mir_exports.h>

#define MAX_TETS_PER_CELL 10
#define MAX_TRIS_PER_CELL 10

#define MAX_NODES_PER_ZONE 8
#define MAX_FACES_PER_ZONE 8
#define MAX_EDGES_PER_ZONE 16

#define MAX_NODES_PER_POLY 8
#define MAX_EDGES_PER_POLY 8


// ****************************************************************************
//  Class:  MIROptions
//
//  Purpose:
//    store the options for material interface reconstructon
//
//  Note:   
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
//  Modifications:
//    Jeremy Meredith, Fri Dec 21 13:30:27 PST 2001
//    Added smoothing option.
//
//    Jeremy Meredith, Tue Aug 13 10:27:28 PDT 2002
//    Added leaveCleanZonesWhole.  Changed nature of structure.
//
//    Jeremy Meredith, Fri Oct 25 10:39:24 PDT 2002
//    Added cleanZonesOnly.
//
//    Jeremy Meredith, Thu Aug 18 16:36:42 PDT 2005
//    Added algorithm and isovolumeVF.
//
//    John C. Anderson, Thu Jan 15 10:20:20 2009
//    Added annealing time for Discrete MIR.
//
//    Jeremy Meredith, Fri Feb 13 11:22:39 EST 2009
//    Added MIR iteration and damping capability.
//
// ****************************************************************************
class MIR_API MIROptions
{
  public:
    enum SubdivisionLevel
    {
        Low,
        Med,
        High
    };

    int              algorithm;
    SubdivisionLevel subdivisionLevel;
    int              numIterations;
    float            iterationDamping;
    bool             smoothing;
    bool             leaveCleanZonesWhole;
    bool             cleanZonesOnly;
    float            isovolumeVF;
    int              annealingTime;

  public:
    MIROptions();
};

#endif
