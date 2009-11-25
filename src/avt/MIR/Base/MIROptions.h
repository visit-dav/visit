/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
