/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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

#ifndef TRILIST_H
#define TRILIST_H
#include <mir_exports.h>

#include "Tri.h"
#include "VisItArray.h"


// ****************************************************************************
// ****************************************************************************
//                             class TriList
// ****************************************************************************
// ****************************************************************************

// ****************************************************************************
//  Class:  TriList
//
//  Purpose:
//    Encapsulation of a list of Tris.
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 14, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 14:09:13 PST 2001
//    Made it use a vector, since there is no current need to pop off the
//    front of the list.
//
//    Jeremy Meredith, Thu May 31 17:13:39 PDT 2001
//    Added the Clear method.
//
//    Jeremy Meredith, Mon Sep 17 15:33:35 PDT 2001
//    Made it use an Array instead of a vector, for faster push_backs.
//
//    Jeremy Meredith, Wed Dec 11 10:10:31 PST 2002
//    Added a "forced material" where if it is >=0, any added tet will have
//    the forced material instead of the normal requested material.
//
//    Mark C. Miller, Thu Feb  9 21:06:10 PST 2006
//    Renamed Array class to VisItArray to avoid name collisions with
//    third-party libs
// ****************************************************************************
class MIR_API TriList
{
  private:
    VisItArray<Tri> list;
    void operator=(const TriList &rhs) { };
  public:
    void       Clear()                 {list.clear();}
    bool       Empty()                 {return list.empty();}
    const int &Size()            const {return list.size();}
    Tri       &operator[](const int i) {return list[i];}
    void Add(const Tri&, int);
    void AddTri(int, int,  Tri::Node&,Tri::Node&,Tri::Node&, int);
    void AddQuad(int, int, Tri::Node&,Tri::Node&,Tri::Node&,Tri::Node&, int);
    static void Swap(TriList &a, TriList &b) {VisItArray<Tri>::Swap(a.list, b.list);}
};

// ****************************************************************************
//  Method:  TriList::Add
//
//  Purpose:
//    Add a tri directly to the list.
//
//  Arguments:
//    t      : the tri
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 14, 2001
//
// ****************************************************************************
void
TriList::Add(const Tri &t, int forcedMat)
{
    list.push_back(t);

    if (forcedMat >= 0)
        list[list.size()-1].mat = forcedMat;
}

// ****************************************************************************
//  Method:  TriList::AddTri
//
//  Purpose:
//    Construct a tri from its nodes and add it to the list.
//
//  Arguments:
//    id     : the cell id
//    mat    : the mat number
//    n0..n2 : the nodes of the tri
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 14, 2001
//
// ****************************************************************************
void
TriList::AddTri(int id,int mat,
                Tri::Node &n0, Tri::Node &n1, Tri::Node &n2,
                int forcedMat)
{
    if (forcedMat >= 0)
        mat = forcedMat;

    list.push_back(Tri(id, n0, n1, n2, mat));
}

// ****************************************************************************
//  Method:  TriList::AddQuad
//
//  Purpose:
//    Triangulate a quad and add the tris to the list.
//
//  Arguments:
//    id     : the cell id
//    mat    : the mat number
//    n0..n3 : the nodes of the quad
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 14, 2001
//
// ****************************************************************************
void
TriList::AddQuad(int id,int mat,
                 Tri::Node &n0, Tri::Node &n1, Tri::Node &n2, Tri::Node &n3,
                 int forcedMat)
{
    if (forcedMat >= 0)
        mat = forcedMat;

    list.push_back(Tri(id, n0, n1, n2, mat));
    list.push_back(Tri(id, n0, n2, n3, mat));
}

#endif
