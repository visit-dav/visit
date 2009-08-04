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
//                            avtSILRangeNamespace.C                         //
// ************************************************************************* //

#include <avtSILRangeNamespace.h>

#include <NamespaceAttributes.h>


using std::vector;


// ****************************************************************************
//  Method: avtSILRangeNamespace constructor
//
//  Arguments:
//      s       A set has been put in the SIL as the union of all the sets this
//              namespace is over for conceptual purposes.  This is the index
//              of that set.
//      min     The minimum in the range.
//      max     The maximum in the range.
//
//  Programmer: Hank Childs
//  Creation:   March 9, 2001
//
// ****************************************************************************

avtSILRangeNamespace::avtSILRangeNamespace(int s, int min, int max)
{
    set = s;
    minRange = min;
    maxRange = max;

    //
    // We need to return a reference to a vector, so create one with the object
    // that contains the one set we will point at.
    //
    setAsElementList.push_back(set);
}


// ****************************************************************************
//  Method: avtSILRangeNamespace::GetAllElements
//
//  Purpose:
//      Returns the element list for this namespace.  There are actually
//      elements from minRange to maxRange, but they are compressed into one
//      set for conceptual purposes.  That set is the data member 'set'.  Since
//      we need to return a reference to a vector, there is a data member
//      kept around just to return -- setAsElementList.
//
//  Returns:     The element list for this namespace.
//
//  Programmer:  Hank Childs
//  Creation:    March 9, 2001
//
// ****************************************************************************

const vector<int> &
avtSILRangeNamespace::GetAllElements(void) const
{
    return setAsElementList;
}


// ****************************************************************************
//  Method: avtSILRangeNamespace::GetAttributes
//
//  Purpose:
//      Gets the namespace attributes when the namespace is for ranges.  Note
//      that it puts '1' in as the type and this type must be maintained in the
//      base class.  Every derived type should have its own unique number.
//
//  Returns:    The new attributes, the calling function owns this.
//
//  Programmer: Hank Childs
//  Creation:   March 29, 2001
//
// ****************************************************************************

NamespaceAttributes *
avtSILRangeNamespace::GetAttributes(void) const
{
    NamespaceAttributes *rv = new NamespaceAttributes;
    rv->SetType(1);
    vector<int> ss;
    ss.push_back(set);
    rv->SetSubsets(ss);
    rv->SetMin(minRange);
    rv->SetMax(maxRange);

    return rv;
}


// ****************************************************************************
//  Method: avtSILRangeNamespace::ContainsElement
//
//  Purpose:
//      Determine whether the element is contained in this namespace.
//
//  Programmer: Dave Bremer
//  Creation:   Thu Dec 20 10:31:43 PST 2007
//
// ****************************************************************************

bool
avtSILRangeNamespace::ContainsElement(int e) const
{
    return (e >= minRange && e <= maxRange);
}





