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
// ****************************************************************************

NameSimplifier::UniqueFileName::UniqueFileName(const QualifiedFilename &qfn)
{
    useHost = false;
    pathCount = 0;

    host = qfn.host;
    file = qfn.filename;
    separator = qfn.separator;
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
// ****************************************************************************

string
NameSimplifier::UniqueFileName::GetAsString() const
{
    string out;
    if (useHost)
        out += host + ":";
    if (pathCount == pathLen)
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
