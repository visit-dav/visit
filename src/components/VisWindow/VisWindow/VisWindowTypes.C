#include <VisWindowTypes.h>

static const char *INTERACTION_MODE_strings[] = {"NAVIGATE", "ZONE_PICK",
    "NODE_PICK", "ZOOM", "LINEOUT"};

// ****************************************************************************
// Function: INTERACTION_MODE_ToString
//
// Purpose: 
//   Returns a string version of INTERACTION_MODE.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 15 16:44:12 PST 2003
//
// Modifications:
//   
// ****************************************************************************

std::string
INTERACTION_MODE_ToString(INTERACTION_MODE m)
{
    int index = int(m);
    if(m < NAVIGATE || m > LINEOUT) index = 0;
    return INTERACTION_MODE_strings[index];
}

std::string
INTERACTION_MODE_ToString(int m)
{
    int index = m;
    if(index < 0 || index > 4) index = 0;
    return INTERACTION_MODE_strings[index];
}

// ****************************************************************************
// Function: INTERACTION_MODE_FromString
//
// Purpose: 
//   Returns an INTERACTION_MODE associated with a string value.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 15 16:44:44 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool
INTERACTION_MODE_FromString(const std::string &s, INTERACTION_MODE &m)
{
    m = NAVIGATE;

    for(int i = 0; i < 5; ++i)
    {
        if(s == INTERACTION_MODE_strings[i])
        {
            m = INTERACTION_MODE(i);
            return true;
        }
    }

    return false;
}
