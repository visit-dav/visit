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

#ifndef WEDGELIST_H
#define WEDGELIST_H
#include <mir_exports.h>

#include "Wedge.h"
#include "VisItArray.h"
#include "mat3d_tet.h"

// ****************************************************************************
// ****************************************************************************
//                             class WedgeList
// ****************************************************************************
// ****************************************************************************

// ****************************************************************************
//  Class:  WedgeList
//
//  Purpose:
//    Encapsulation of a list of Wedges.
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 13, 2002
//
//  Modifications:
//
//    Mark C. Miller, Thu Feb  9 21:06:10 PST 2006
//    Renamed Array class to VisItArray to avoid name collisions with
//    third-party libs
// ****************************************************************************
class MIR_API WedgeList
{
  private:
    VisItArray<Wedge> list;
    void operator=(const WedgeList &rhs) { };
  public:
    void       Clear()                 {list.clear();}
    bool       Empty()                 {return list.empty();}
    const int &Size()            const {return list.size();}
    Wedge     &operator[](const int i) {return list[i];}
    void Add(const Wedge&, int);
    void AddWedge(int, int, const Tet::Node&,const Tet::Node&,const Tet::Node&,
                            const Tet::Node&,const Tet::Node&,const Tet::Node&, int);
    static void Swap(WedgeList &a, WedgeList &b) {VisItArray<Wedge>::Swap(a.list, b.list);}
};

// ****************************************************************************
//  Method:  WedgeList::Add
//
//  Purpose:
//    Add a wedge directly to the list.
//
//  Arguments:
//    w      : the Wedge
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 13, 2002
//
// ****************************************************************************
void
WedgeList::Add(const Wedge &t, int forcedMat)
{
    list.push_back(t);

    if (forcedMat >= 0)
        list[list.size()-1].mat = forcedMat;
}

// ****************************************************************************
//  Method:  WedgeList::AddWedge
//
//  Purpose:
//    Construct a wedge from its nodes and add it to the list.
//
//  Arguments:
//    id     : the cell id
//    mat    : the mat number
//    n0..n5 : the nodes of the wedge
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 13, 2002
//
// ****************************************************************************
void
WedgeList::AddWedge(int id,int mat,
                    const Tet::Node &n0, const Tet::Node &n1, const Tet::Node &n2,
                    const Tet::Node &n3, const Tet::Node &n4, const Tet::Node &n5,
                    int forcedMat)
{
    if (forcedMat >= 0)
        mat = forcedMat;

    list.push_back(Wedge(id, n0, n1, n2, n3, n4, n5, mat));
}


#endif
