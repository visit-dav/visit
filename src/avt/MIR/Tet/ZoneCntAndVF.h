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

#ifndef ZONE_CNT_AND_VF_H
#define ZONE_CNT_AND_VF_H

// ****************************************************************************
//  Class:  ZoneCntAndVF
//
//  Purpose:
//    Store a volume fraction for a each mat for a face/node/edge/etc., and
//    keep track of how many zones share this face/node/edge/etc.
//
//  Note:   
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
// ****************************************************************************
struct ZoneCntAndVF
{
    int           zonecnt;
    vector<float> vf;
    ZoneCntAndVF();
    ZoneCntAndVF(int c, int n);
    ~ZoneCntAndVF();
    void operator=(const ZoneCntAndVF &rhs);
};


// ----------------------------------------------------------------------------
//                             Inline Methods
// ----------------------------------------------------------------------------


// ****************************************************************************
//  Default Constructor:  ZoneCntAndVF::ZoneCntAndVF
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
// ****************************************************************************
inline
ZoneCntAndVF::ZoneCntAndVF()
{
    zonecnt = 0;
}

// ****************************************************************************
//  Constructor:  ZoneCntAndVF::ZoneCntAndVF
//
//  Arguments:
//    c    the number of zones attached to this object
//    n    the number of materials in the problem
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
// ****************************************************************************
inline
ZoneCntAndVF::ZoneCntAndVF(int c, int n)
{
    zonecnt = c;
    vf.resize(n, 0.0);
}

// ****************************************************************************
//  Destructor:  ZoneCntAndVF::~ZoneCntAndVF
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
// ****************************************************************************
inline
ZoneCntAndVF::~ZoneCntAndVF()
{
    vf.clear();
}

// ****************************************************************************
//  Method:  ZoneCntAndVF::operator=
//
//  Purpose:
//    assignment operator
//
//  Arguments:
//    rhs        the source ZoneCntAndVF
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
// ****************************************************************************
inline void
ZoneCntAndVF::operator=(const ZoneCntAndVF &rhs)
{
    zonecnt = rhs.zonecnt;
    vf      = rhs.vf;
}


#endif
