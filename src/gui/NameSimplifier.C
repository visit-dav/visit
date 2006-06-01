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

#include <NameSimplifier.h>
#include <maptypes.h>
#include <Utility.h>

using std::string;
using std::vector;

// ****************************************************************************
// Method: NameSimplifier::NameSimplifier
//
// Purpose: 
//   Constructor for the NameSimplifier class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 4 13:46:35 PST 2004
//
// Modifications:
//   
// ****************************************************************************

NameSimplifier::NameSimplifier() : names()
{
}

// ****************************************************************************
// Method: NameSimplifier::~NameSimplifier
//
// Purpose: 
//   Destructor for the NameSimplifier class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 4 13:46:51 PST 2004
//
// Modifications:
//   
// ****************************************************************************

NameSimplifier::~NameSimplifier()
{
}

// ****************************************************************************
// Method: NameSimplifier::AddName
//
// Purpose: 
//   Adds a name to be simplified.
//
// Arguments:
//   name : The name to simplify.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 4 13:47:12 PST 2004
//
// Modifications:
//    Jeremy Meredith, Tue Apr  6 12:19:51 PDT 2004
//    Made use of the UniqueFileName construct here.
//
// ****************************************************************************

void
NameSimplifier::AddName(const std::string &n)
{
    AddName(QualifiedFilename(n));
}

void
NameSimplifier::AddName(const QualifiedFilename &n)
{
    int newindex = names.size();
    names.push_back(UniqueFileName(n));

    for (int j=0; j<newindex; j++)
    {
        UniqueFileName::Uniquify(names[newindex],names[j]);
    }
}

// ****************************************************************************
// Method: NameSimplifier::ClearNames
//
// Purpose: 
//   Clears out the name list.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 4 13:47:45 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
NameSimplifier::ClearNames()
{
    names.clear();
}


// ****************************************************************************
//  Constructor:  NameSimplifier::UniqueFileName::UniqueFileName
//
//  Arguments:
//    qfn        a QualifiedFilename to convert
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  6, 2004
//
//  Modifications:
//    Jeremy Meredith, Wed Apr  7 12:12:48 PDT 2004
//    Account for the fact that not all input names had full paths.
//
// ****************************************************************************

NameSimplifier::UniqueFileName::UniqueFileName(const QualifiedFilename &qfn)
{
    useHost = false;
    pathCount = 0;

    host = qfn.host;
    file = qfn.filename;
    separator = qfn.separator;
    leadingSlash = (qfn.path.size() > 0 && qfn.path[0] == separator);
    path = SplitValues(qfn.path, separator);
    pathLen = path.size();
}


// ****************************************************************************
//  Method:  NameSimplifier::UniqueFileName::GetAsString
//
//  Purpose:
//    Convert into a single string
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  6, 2004
//
//  Modifications:
//    Jeremy Meredith, Wed Apr  7 12:12:48 PDT 2004
//    Account for the fact that not all input names had full paths.
//
// ****************************************************************************

string
NameSimplifier::UniqueFileName::GetAsString() const
{
    string out;
    if (useHost)
        out += host + ":";
    if (leadingSlash && pathCount == pathLen)
        out += separator;
    for (int i=0; i<pathCount; i++)
        out += path[pathLen-pathCount + i] + separator;
    out += file;
    return out;
}


// ****************************************************************************
//  Method:  NameSimplifier::UniqueFileName::Unique
//
//  Purpose:
//    Returns true if we can consider the representations of two
//    file names unique.
//
//  Arguments:
//    a,b        the UniqueFileNames to test
//
//  Note:  This is definitely not an exact equality test.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  6, 2004
//
// ****************************************************************************

bool
NameSimplifier::UniqueFileName::Unique(const UniqueFileName &a,
                                       const UniqueFileName &b)
{
    string as = a.GetAsString();
    string bs = b.GetAsString();
    int    al = as.length();
    int    bl = bs.length();

    // Ensure that a string does not exactly match
    // the ending piece of the other one
    if (as == bs)
        return false;
    else if ((al < bl) && (as == bs.substr(bl - al)))
        return false;
    else if ((bl < al) && (bs == as.substr(al - bl)))
        return false;
    else
        return true;
}


// ****************************************************************************
//  Method:  NameSimplifier::UniqueFileName::Uniquify
//
//  Purpose:
//    Make two file names unique
//
//  Arguments:
//    a,b        the file names to make unique
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  6, 2004
//
// ****************************************************************************

void
NameSimplifier::UniqueFileName::Uniquify(UniqueFileName &a, UniqueFileName &b)
{
    // Filenames are often unique already
    if (Unique(a, b))
        return;

    // If we can make them unique by simply adding a host prefix, do it
    if (a.host != b.host)
    {
        a.useHost = true;
        b.useHost = true;
        // They should now be unique since our host name is prepended.
        // Test again for sanity, and return now that they are unique.
        if (Unique(a, b))
            return;
    }

    // If two filenames are the same on the same host, we need at 
    // least one path prefix for both
    if (a.pathCount == 0 && b.pathCount != 0 && a.pathLen > 0)
        a.pathCount++;
    if (b.pathCount == 0 && a.pathCount != 0 && b.pathLen > 0)
        b.pathCount++;

    // Start adding more path prefix segments as needed
    while (!Unique(a,b) &&
           (a.pathCount < a.pathLen || b.pathCount < b.pathLen))
    {
        // Add to the shorter one, or add to both if they are equal
        if (a.pathCount < b.pathCount && a.pathCount < a.pathLen)
            a.pathCount++;
        else if (b.pathCount < a.pathCount && b.pathCount < b.pathLen)
            b.pathCount++;
        else
        {
            if (a.pathCount < a.pathLen)
                a.pathCount++;
            if (b.pathCount < b.pathLen)
                b.pathCount++;
        }
    }
}

// ****************************************************************************
// Method: NameSimplifier::GetSimplifiedNames
//
// Purpose: 
//   Returns a stringVector of simplified names.
//   Uniqueness is now guaranteed as names are added.
//
// Arguments:
//   n : The return string vector.
//
// Returns:    
//
// Programmer: Jeremy Meredith
// Creation:   April  6, 2004
//
// Modifications:
//   
// ****************************************************************************

void
NameSimplifier::GetSimplifiedNames(stringVector &n) const
{
    n.clear();

    for (int i=0; i<names.size(); i++)
    {
        n.push_back(names[i].GetAsString());
    }
}
