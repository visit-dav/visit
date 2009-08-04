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

#ifndef EDGE_H
#define EDGE_H
#include <mir_exports.h>

#include <limits.h>

// ****************************************************************************
//  Class:  Edge
//
//  Purpose:
//    Way to indentify an edge within a mesh (uses two lowest node id's)
//
//  Note:   
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
// ****************************************************************************
class MIR_API Edge
{
  public:
    Edge();
    Edge(int *);
    void operator=(const Edge &rhs);
    bool operator==(const Edge &rhs);
    static unsigned int HashFunction(Edge &edge);
  private:
    int a,b;
};


// ----------------------------------------------------------------------------
//                             Inline Methods
// ----------------------------------------------------------------------------


// ****************************************************************************
//  Default Constructor:  Edge::Edge
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
// ****************************************************************************
inline
Edge::Edge()
{
    a=-1;
    b=-1;
}

// ****************************************************************************
//  Constructor:  Edge::Edge
//
//  Purpose:
//    Create yourself for a vtk edge
//
//  Arguments:
//    cell       the vtk edge
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Oct  8 16:29:24 PDT 2002
//    Removed costly VTK calls.
//
// ****************************************************************************
inline
Edge::Edge(int *edges)
{
    a = INT_MAX;
    b = INT_MAX;

    for (int i=0; i<2; i++)
    {
        int node = edges[i];
        if (node < a)     { b=a; a=node; }
        else if (node < b)     { b=node; }
    }
}

// ****************************************************************************
//  Method:  Edge::operator=
//
//  Purpose:
//    Assignment operator
//
//  Arguments:
//    rhs        the source edge
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
// ****************************************************************************
inline void
Edge::operator=(const Edge &rhs)
{
    a=rhs.a;
    b=rhs.b;
}

// ****************************************************************************
//  Method:  Edge::operator==
//
//  Purpose:
//    comparison operator
//
//  Arguments:
//    rhs        the edge to compare with
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
// ****************************************************************************
inline bool
Edge::operator==(const Edge &rhs)
{
    return (a==rhs.a && b==rhs.b);
}



#endif
