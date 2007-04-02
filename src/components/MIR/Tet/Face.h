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

#ifndef FACE_H
#define FACE_H
#include <mir_exports.h>

#include <limits.h>

// ****************************************************************************
//  Class:  Face
//
//  Purpose:
//    Way to indentify a face within a mesh (uses three lowest node id's)
//
//  Note:   
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
// ****************************************************************************
class MIR_API Face
{
  public:
    Face();
    Face(int, const int *);
    void operator=(const Face &rhs);
    bool operator==(const Face &rhs);
    static unsigned int HashFunction(Face &face);
  private:
    int a,b,c;
};


// ----------------------------------------------------------------------------
//                             Inline Methods
// ----------------------------------------------------------------------------


// ****************************************************************************
//  Default Constructor:  Face::Face
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
// ****************************************************************************
inline
Face::Face()
{
    a=-1;
    b=-1;
    c=-1;
}


// ****************************************************************************
//  Constructor:  Face::Face
//
//  Purpose:
//    create yourself from a vtk face
//
//  Arguments:
//    cell       the vtk face
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Oct  8 17:02:05 PDT 2002
//    Removed costly VTK calls.
//
// ****************************************************************************
inline
Face::Face(int n, const int *ids)
{
    a = INT_MAX;
    b = INT_MAX;
    c = INT_MAX;

    for (int i=0; i<n; i++)
    {
        int node = ids[i];
        if (node < a)     { c=b; b=a; a=node; }
        else if (node < b)     { c=b; b=node; }
        else if (node < c)          { c=node; }
    }
}

// ****************************************************************************
//  Method:  Face::operator=
//
//  Purpose:
//    Assignment operator
//
//  Arguments:
//    rhs        the source face
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
// ****************************************************************************
inline void
Face::operator=(const Face &rhs)
{
    a=rhs.a;
    b=rhs.b;
    c=rhs.c;
}

// ****************************************************************************
//  Method:  Face::operator==
//
//  Purpose:
//    comparison operator
//
//  Arguments:
//    rhs        the face to compare with
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
// ****************************************************************************
inline bool
Face::operator==(const Face &rhs)
{
    return (a==rhs.a && b==rhs.b && c==rhs.c);
}

#endif
