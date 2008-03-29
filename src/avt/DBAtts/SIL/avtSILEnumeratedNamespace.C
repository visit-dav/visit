/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                        avtSILEnumeratedNamespace.C                        //
// ************************************************************************* //

#include <avtSILEnumeratedNamespace.h>

#include <NamespaceAttributes.h>


using std::vector;


// ****************************************************************************
//  Method: avtSILEnumeratedNamespace constructor
//
//  Arguments:
//      els     The element list to copy.
//
//  Programmer: Hank Childs
//  Creation:   March 9, 2001
//
// ****************************************************************************

avtSILEnumeratedNamespace::avtSILEnumeratedNamespace(const vector<int> &els)
{
    elements = els;
    sequentialElems = true;
    int ii;
    for (ii = 1; ii < els.size(); ii++)
    {
        if (els[ii-1]+1 != els[ii])
        {
            sequentialElems = false;
            break;
        }
    }
    sortedElems = true;
    for (ii = 1; ii < els.size(); ii++)
    {
        if (els[ii-1] > els[ii])
        {
            sortedElems = false;
            break;
        }
    }
}


// ****************************************************************************
//  Method: avtSILEnumeratedNamespace::GetAllElements
//
//  Purpose:
//      Returns the element list for this namespace.  Since this is an
//      enumerated namespace, this is exactly the elements we have previously
//      enumerated.
//
//  Returns:     The element list.
//
//  Programmer:  Hank Childs
//  Creation:    March 9, 2001
//
// ****************************************************************************

const vector<int> &
avtSILEnumeratedNamespace::GetAllElements(void) const
{
    return elements;
}


// ****************************************************************************
//  Method: avtSILEnumeratedNamespace::GetAttributes
//
//  Purpose:
//      Gets the namespace attributes when the namespace is enumerated.  Note
//      that it puts '0' in as the type and this type must be maintained in the
//      base class.  Every derived type should have its own unique number.
//
//  Returns:    The new attributes, the calling function owns this.
//
//  Programmer: Hank Childs
//  Creation:   March 29, 2001
//
// ****************************************************************************

NamespaceAttributes *
avtSILEnumeratedNamespace::GetAttributes(void) const
{
    NamespaceAttributes *rv = new NamespaceAttributes;
    rv->SetType(0);
    rv->SetSubsets(elements);

    return rv;
}



// ****************************************************************************
//  Method: avtSILEnumeratedNamespace::ContainsElement
//
//  Purpose:
//      Determine whether the element is contained in this namespace.  This 
//      class detects the common case of a sequence of elements, and uses that
//      info to speed up the query.
//
//  Programmer: Dave Bremer
//  Creation:   Thu Dec 20 10:31:43 PST 2007
//
// ****************************************************************************

bool
avtSILEnumeratedNamespace::ContainsElement(int e) const
{
    if (sequentialElems)
    {
        return (e >= elements[0] && e <= elements[elements.size()-1]);
    }
    else if (sortedElems)
    {
        int min = 0, max = elements.size()-1;
        int mid = (max+min)/2;
        while (min <= max)
        {
            if (elements[mid] == e)
            {
                return true;
            }
            else if (elements[mid] > e)
            {
                max = mid-1;
                mid = (max+min)/2;
            }
            else
            {
                min = mid+1;
                mid = (max+min)/2;
            }
        }
        return false;
    }
    else
    {
        for (int ii = 0; ii < elements.size(); ii++)
        {
            if (elements[ii] == e)
                return true;
        }
    }
    return false;
}


