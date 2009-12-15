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
//                             avtSILCollection.C                            //
// ************************************************************************* //

#include <avtSILCollection.h>

#include <vector>

#include <avtSILNamespace.h>


using     std::string;
using     std::vector;


// ****************************************************************************
//  Method: avtSILCollection constructor
//
//  Arguments:
//      cat         The name of the category.
//      r           The role of the category.
//      si          The superset index.
//      ns          The namespace that has the subset relations.  This object
//                  will be owned by the collection and deleted when the
//                  collection is deleted.
//
//  Programmer: Hank Childs
//  Creation:   March 9, 2001
//
// ****************************************************************************

avtSILCollection::avtSILCollection(string cat, SILCategoryRole r, int si,
                                   avtSILNamespace *ns)
{
    category      = cat;
    role          = r;
    supersetIndex = si;
    subsets       = ns;
}


// ****************************************************************************
//  Method: avtSILCollection destructor
//
//  Programmer: Hank Childs
//  Creation:   March 9, 2001
//
// ****************************************************************************

avtSILCollection::~avtSILCollection()
{
    if (subsets != NULL)
    {
        delete subsets;
        subsets = NULL;
    }
}


// ****************************************************************************
//  Method: avtSILCollection::Print
//
//  Purpose:
//      Prints out a collection -- for debugging purposed only.
//
//  Programmer: Hank Childs
//  Creation:   March 12, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Sep 18 10:20:35 PDT 2001
//    Added case for boundary.
//
//    Jeremy Meredith, Fri Dec 14 16:25:54 PST 2001
//    Added species.
//
//    Jeremy Meredith, Fri Aug 25 17:16:38 EDT 2006
//    Added enumerated scalars.
//
//    Hank Childs, Fri Dec 11 11:37:48 PST 2009
//    Push printing off the name spaces ... that way we don't have to
//    enumerate all the values.
//
// ****************************************************************************

void
avtSILCollection::Print(ostream &out) const
{
    out << "\tCollection for category = " << category.c_str() << ", role = ";
    string roleS = "";
    switch (role)
    {
      case SIL_TOPOLOGY:
        roleS = "topology";
        break;
      case SIL_PROCESSOR:
        roleS = "processor";
        break;
      case SIL_BLOCK:
        roleS = "block";
        break;
      case SIL_DOMAIN:
        roleS = "domain";
        break;
      case SIL_ASSEMBLY:
        roleS = "assembly";
        break;
      case SIL_MATERIAL:
        roleS = "material";
        break;
      case SIL_BOUNDARY:
        roleS = "boundary";
        break;
      case SIL_SPECIES:
        roleS = "species";
        break;
      case SIL_ENUMERATION:
        roleS = "enumeration";
        break;
      case SIL_USERD:
        roleS = "user defined";
        break;
    }
    out << roleS.c_str() << endl;

    out << "\t\tSuperset is " << supersetIndex << endl;

    out << "\t\t";
    subsets->Print(out);
}


// ****************************************************************************
//  Method: avtSILCollection::ContainsElement
//
//  Purpose:
//      Returns true if the element is contained in this collection.
//
//  Programmer: Dave Bremer
//  Creation:   Thu Dec 20 14:23:29 PST 2007
//
// ****************************************************************************

bool
avtSILCollection::ContainsElement(int e) const
{
    return (subsets->ContainsElement(e));
}
