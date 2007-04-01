#include <NameSimplifier.h>
#include <maptypes.h>

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
//   
// ****************************************************************************

void
NameSimplifier::AddName(const std::string &n)
{
    names.push_back(QualifiedFilename(n));
}

void
NameSimplifier::AddName(const QualifiedFilename &n)
{
    names.push_back(n);
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
// Method: NameSimplifier::GetSimplifiedNames
//
// Purpose: 
//   Returns a stringVector of simplified names.
//
// Arguments:
//   n : The return string vector.
//
// Returns:    
//
// Note:       This method will still give files that have the entire host
//             qualified path. It would be nice if the path was also
//             simplified instead of giving up and using the whole filename.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 4 13:48:00 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
NameSimplifier::GetSimplifiedNames(stringVector &n) const
{
    n.clear();

    StringIntMap fileCount, hostCount;
    int i;
    for(i = 0; i < names.size(); ++i)
    {
        ++fileCount[names[i].filename];
        ++hostCount[names[i].host];
    }

    bool needHosts = (hostCount.size() > 1);

    for(i = 0; i < names.size(); ++i)
    {
        if(fileCount[names[i].filename] > 1)
        {
            if(needHosts)
            {
                n.push_back(names[i].FullName());
            }
            else
                n.push_back(names[i].PathAndFile());
        }
        else if(needHosts)
        {
            n.push_back(names[i].host + ":" + names[i].filename);
        }
        else
            n.push_back(names[i].filename);
    }
}

