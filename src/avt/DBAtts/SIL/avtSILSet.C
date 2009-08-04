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

// ************************************************************************* //
//                                 avtSILSet.C                               //
// ************************************************************************* //

#include <avtSILSet.h>


using std::string;
using std::vector;


// ****************************************************************************
//  Method: avtSILSet constructor
//
//  Arguments:
//      n       The name of the set.
//      i       An identifier for the set.
//
//  Programmer: Hank Childs
//  Creation:   March 9, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Jun 15 10:42:51 PDT 2001
//    Added identifier argument.
//
//    Dave Bremer, Thu Dec 20 10:31:43 PST 2007
//    Changed input type from "string" to "const string &"
// ****************************************************************************

avtSILSet::avtSILSet(const string &n, int i)
{
    name = n;
    id   = i;
}


// ****************************************************************************
//  Method: avtSILSet::AddMapIn
//
//  Arguments:
//      in      The index in the SIL object of the collection that maps
//              into this set.
//
//  Programmer: Hank Childs
//  Creation:   March 9, 2001
//
// ****************************************************************************

void
avtSILSet::AddMapIn(int in)
{
    mapsIn.push_back(in);
}


// ****************************************************************************
//  Method: avtSILSet::AddMapOut
//
//  Arguments:
//      out     The index in the SIL object of the collection that maps
//              out of this set.
//
//  Programmer: Hank Childs
//  Creation:   March 9, 2001
//
// ****************************************************************************

void
avtSILSet::AddMapOut(int out)
{
    mapsOut.push_back(out);
    allMapsOut.push_back(out);
}


// ****************************************************************************
//  Method: avtSILSet::AddMatrixMapOut
//
//  Purpose:
//      Tells the set that it is a member in a matrix
//
//  Arguments:
//      coll    The index of the collection this corresponds to in the SIL.
//
//  Programmer: Hank Childs
//  Creation:   November 15, 2002
//
//  Modifications:
//    Dave Bremer, Thu Dec 20 10:31:43 PST 2007
//    There used to be two methods named AddMatrixRow and AddMatrixColumn.  I
//    removed some unused class data, and then there was no reason to distinguish
//    between the two use cases, so the methods have been consolidated into this
//    AddMatrixMapOut method.
// ****************************************************************************

void
avtSILSet::AddMatrixMapOut(int coll)
{
    allMapsOut.push_back(coll);
}


// ****************************************************************************
//  Method: avtSILSet::Print
//
//  Purpose:
//      Prints out a SIL set -- meant for debugging purposes only.
//
//  Programmer: Hank Childs
//  Creation:   March 12, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Dec  4 09:16:53 PST 2002
//    Don't print out unnecessary lines.
//
// ****************************************************************************

void
avtSILSet::Print(ostream &out) const
{
    out << "\tSet name = " << name.c_str() << ", identifier = " << id << endl;
    std::vector<int>::const_iterator it;
    if (mapsIn.size() > 0)
    {
        out << "\t\tMaps in ";
        for (it = mapsIn.begin() ; it != mapsIn.end() ; it++)
        {
            out << *it << ", ";
        }
        out << endl;
    }
    if (mapsOut.size() > 0)
    {
        out << "\t\tMaps out ";
        for (it = mapsOut.begin() ; it != mapsOut.end() ; it++)
        {
            out << *it << ", ";
        }
        out << endl;
    }
}


