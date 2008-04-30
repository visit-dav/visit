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
//                                  avtSILArray.C                            //
// ************************************************************************* //

#include <avtSILArray.h>
#include <avtSILSet.h>
#include <SILArrayAttributes.h>
#include <avtSILRangeNamespace.h>
#include <avtSILEnumeratedNamespace.h>
#include <snprintf.h>

#include <cstring>

using std::vector;
using std::string;




// ****************************************************************************
//  Method: avtSILArray constructor
//
//  Programmer: Dave Bremer
//  Creation:   Thu Dec 20 12:12:30 PST 2007
//
// ****************************************************************************

avtSILArray::avtSILArray(const string &pfx, int nSets, 
                         int firstSetName, bool uniqueIDs,
                         const string &cat,
                         SILCategoryRole r, int parent)
{
    prefix = pfx;
    iNumSets = nSets;
    iFirstSetName = firstSetName;
    bUseUniqueIDs = uniqueIDs;

    iFirstSet = 0;
    iColIndex = 0;

    category = cat;
    role = r;
    iColParent = parent;
}


// ****************************************************************************
//  Method: avtSILArray constructor
//
//  Programmer: Dave Bremer
//  Creation:   Thu Mar 27 15:57:29 PDT 2008
//
// ****************************************************************************

avtSILArray::avtSILArray(const stringVector &domainNames, int nSets, 
                         int firstSetName, bool uniqueIDs,
                         const string &cat,
                         SILCategoryRole r, int parent)
{
    names = domainNames;
    iNumSets = nSets;
    iFirstSetName = firstSetName;
    bUseUniqueIDs = uniqueIDs;

    iFirstSet = 0;
    iColIndex = 0;

    category = cat;
    role = r;
    iColParent = parent;
}


// ****************************************************************************
//  Method: avtSILArray constructor
//
//  Programmer: Dave Bremer
//  Creation:   Thu Dec 20 12:12:30 PST 2007
//
//  Modifications:
//    Dave Bremer, Thu Mar 27 15:57:29 PDT 2008
//    Modified to handle arrays of domain names.
// ****************************************************************************

avtSILArray::avtSILArray(const SILArrayAttributes &atts)
{
    prefix = atts.GetPrefix();
    names = atts.GetNames();
    iNumSets = atts.GetNumSets();
    iFirstSetName = atts.GetFirstSetName();
    bUseUniqueIDs = atts.GetUseUniqueIDs();

    iFirstSet = atts.GetFirstSet();
    iColIndex = atts.GetColIndex();

    category = atts.GetCategory();
    role     = (SILCategoryRole)atts.GetRole();
    iColParent = atts.GetColParent();
}


// ****************************************************************************
//  Method: avtSILArray::MakeAttributes
//
//  Purpose:
//      Makes an attribute subject that represents this object.
//
//  Programmer: Dave Bremer
//  Creation:   Thu Dec 20 12:12:30 PST 2007
//
//  Modifications:
//    Dave Bremer, Thu Mar 27 15:57:29 PDT 2008
//    Modified to handle arrays of domain names.
// ****************************************************************************

SILArrayAttributes *
avtSILArray::MakeAttributes(void) const
{
    SILArrayAttributes *rv = new SILArrayAttributes;
    rv->SetPrefix(prefix);
    rv->SetNames(names);
    rv->SetNumSets(iNumSets);
    rv->SetFirstSetName(iFirstSetName);
    rv->SetUseUniqueIDs(bUseUniqueIDs);
    rv->SetFirstSet(iFirstSet);
    rv->SetColIndex(iColIndex);
    rv->SetCategory(category);
    rv->SetRole((int)role);
    rv->SetColParent(iColParent);
    return rv;
}


// ****************************************************************************
//  Method: avtSILArray::GetSILSet
//
//  Purpose:
//      Create an avtSILSet on demand, using a templated name.
//
//  Programmer: Dave Bremer
//  Creation:   Thu Dec 20 12:12:30 PST 2007
//
//  Modifications:
//    Dave Bremer, Thu Mar 27 15:57:29 PDT 2008
//    Modified to handle arrays of domain names.
// ****************************************************************************

avtSILSet_p  
avtSILArray::GetSILSet(int index) const
{
    int id = bUseUniqueIDs ? index : -1;

    char name[1024];
    avtSILSet_p rv;

    if (names.size() != 0)
    {
        rv = new avtSILSet(names[index], id);
    }
    else if (strstr(prefix.c_str(), "%") != NULL)
    {
        SNPRINTF(name, 1024, prefix.c_str(), index + iFirstSetName);
        rv = new avtSILSet(name, id);
    }
    else
    {
        SNPRINTF(name, 1024, "%s%d", prefix.c_str(), index + iFirstSetName);
        rv = new avtSILSet(name, id);
    }
    rv->AddMapIn(iColIndex);
    return rv;
}


// ****************************************************************************
//  Method: avtSILArray::GetSILCollection
//
//  Purpose:
//      Create an avtSILCollection on demand
//
//  Programmer: Dave Bremer
//  Creation:   Thu Dec 20 12:12:30 PST 2007
//
// ****************************************************************************

avtSILCollection_p
avtSILArray::GetSILCollection() const
{
    //May have to use enumerated namespace
    //avtSILRangeNamespace *ns = new avtSILRangeNamespace();
    vector<int> tmp(iNumSets);
    for (int ii = 0; ii < iNumSets; ii++)
    {
        tmp[ii] = ii+iFirstSet;
    }
    avtSILEnumeratedNamespace *ns = new avtSILEnumeratedNamespace(tmp);
    avtSILCollection_p rv         = new avtSILCollection(category, role, 
                                                         iColParent, ns);
    return rv;
}


// ****************************************************************************
//  Method: avtSILArray::GetSILSet
//
//  Purpose:
//      Create an avtSILSet on demand, using a templated name.
//
//  Programmer: Dave Bremer
//  Creation:   Thu Dec 20 12:12:30 PST 2007
//
// ****************************************************************************

int
avtSILArray::GetSILSetID(int index) const
{
    if (bUseUniqueIDs)
        return index;
    else
        return -1;
}


// ****************************************************************************
//  Method: avtSILArray::GetSetState
//
//  Purpose:
//      Determines if all, some, or none of the sets in this array are used.
//
//  Programmer: Dave Bremer
//  Creation:   Thu Dec 20 12:12:30 PST 2007
//
// ****************************************************************************

SetState
avtSILArray::GetSetState(const vector<unsigned char> &useSet) const
{
    int NoneUsedCount = 0;
    int SomeUsedCount = 0;
    int AllUsedCount = 0;

    for (int ii = 0; ii < iNumSets; ii++)
    {
        SetState s = (SetState)useSet[iFirstSet+ii];
        if (s == NoneUsed)
            NoneUsedCount++;
        else if (s == SomeUsed)
            SomeUsedCount++;
        else 
            AllUsedCount++;
    }

    SetState retval;
    if (SomeUsedCount > 0)
        retval = SomeUsed;
    else if (NoneUsedCount == 0 && AllUsedCount != 0)
        retval = AllUsed;
    else if (AllUsedCount == 0 && NoneUsedCount != 0)
        retval = NoneUsed;
    else
        retval = SomeUsed;

    return retval;
}


// ****************************************************************************
//  Method: avtSILArray::TurnSet
//
//  Purpose:
//      Turns all the sets on or off, or does something special for 
//      load balancing
//
//  Programmer: Dave Bremer
//  Creation:   Thu Dec 20 12:12:30 PST 2007
//
// ****************************************************************************

void
avtSILArray::TurnSet(vector<unsigned char> &useSet,
                     SetState val, bool forLoadBalance) const
{
    for (int ii = 0; ii < iNumSets; ii++)
    {
        int set = ii+iFirstSet;
        if (forLoadBalance && (val==NoneUsed) && 
            ((useSet[set]==AllUsed) || (useSet[set]==AllUsedOtherProc)))
            useSet[set] = AllUsedOtherProc;
        else
            useSet[set] = val;
    }
}


// ****************************************************************************
//  Method: avtSILArray::GetSetIndex
//
//  Purpose:
//      See if a given name could match this object's name template, and is in
//      right range.
//
//  Returns:    index of the set with the given name, in the containing
//              SIL's index space, or -1 if no match
//
//  Programmer: Dave Bremer
//  Creation:   Thu Dec 20 12:12:30 PST 2007
//
//  Modifications:
//    Dave Bremer, Thu Mar 27 15:57:29 PDT 2008
//    Modified to handle arrays of domain names.
// ****************************************************************************
int
avtSILArray::GetSetIndex(const std::string &name) const
{
    //Use sscanf to determine if this name could match an entry in the array.
    int nMatches = 0;
    int i = -999;

    if (names.size() != 0)
    {
        int ii = 0;
        for (ii = 0; ii < names.size(); ii++)
        {
            if (names[ii] == name)
            {
                return (iFirstSet + ii);
            }
        }
        return -1;
    }
    else if (strstr(prefix.c_str(), "%") != NULL)
    {
        nMatches = sscanf(name.c_str(), prefix.c_str(), &i);
    }
    else
    {
        char nameTemplate[1024];
        SNPRINTF(nameTemplate, 1024, "%s%%d", prefix.c_str());
        nMatches = sscanf(name.c_str(), nameTemplate, &i);
    }
    if (nMatches != 1)
        return -1;

    if (i < iFirstSetName || i >= iFirstSetName+iNumSets)
        return -1;

    return (i + iFirstSet - iFirstSetName);
}










