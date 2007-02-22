/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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

#ifndef WEDGE_H
#define WEDGE_H

#include "Tet.h"

// ****************************************************************************
//  Class:  Wedge
//
//  Purpose:
//    A wedge used in the intermediate stages of MIR.
//
//  Note:   
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 13, 2002
//
//  Modifications:
//
// ****************************************************************************
struct Wedge
{
    Tet::Node  node[6];
    int        zone_num;
    int        mat;
  public:
    Wedge() {};
    Wedge(int c,const MaterialTetrahedron &t,int m,const Wedge&);
    Wedge(int,
          const Tet::Node&,const Tet::Node&,const Tet::Node&,
          const Tet::Node&,const Tet::Node&,const Tet::Node&,int);
};


// ----------------------------------------------------------------------------
//                             Inline Methods
// ----------------------------------------------------------------------------


// ****************************************************************************
//  Constructor:  Wedge::Wedge
//
//  Purpose:
//    Create this wedge from material m of a MaterialTetrahedron, but
//    conform it to another Wedge.
//
//  Arguments:
//    c         the cell number
//    t         the composite MaterialTetrahedron
//    m         the material to extract
//    conform   the Wedge to conform this new one to
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 13, 2002
//
// ****************************************************************************
inline
Wedge::Wedge(int c,const MaterialTetrahedron &t,int m, const Wedge &conform)
{
    zone_num = c;
    mat = m;
    for (int n=0; n<6; n++)
    {
        node[n].vf = 0.;
        node[n].index = conform.node[n].index;
        for (int w=0; w<4; w++)
        {
            node[n].weight[w] = conform.node[n].weight[w];
            node[n].vf += node[n].weight[w] * t.node[w].matvf[m];
        }
    }
}

// ****************************************************************************
//  Constructor:  Wedge::Wedge
//
//  Purpose:
//    Create this wedge from its constituent parts.
//
//  Arguments:
//    c          the cell number
//    n1..n6     the nodes
//    m          the material number
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 13, 2002
//
// ****************************************************************************
inline
Wedge::Wedge(int c,
             const Tet::Node &n1,
             const Tet::Node &n2,
             const Tet::Node &n3,
             const Tet::Node &n4,
             const Tet::Node &n5,
             const Tet::Node &n6,
             int m)
{
    node[0] = n1;
    node[1] = n2;
    node[2] = n3;
    node[3] = n4;
    node[4] = n5;
    node[5] = n6;
    zone_num = c;
    mat = m;
}

#endif
