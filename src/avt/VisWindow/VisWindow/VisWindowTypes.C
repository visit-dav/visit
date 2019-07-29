// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <VisWindowTypes.h>

static const char *WINDOW_MODE_strings[] = {
    "WINMODE_2D",
    "WINMODE_3D",
    "WINMODE_CURVE",
    "WINMODE_AXISARRAY",
    "WINMODE_PARALLELAXES",
    "WINMODE_VERTPARALLELAXES",
    "WINMODE_NONE"
};

// ****************************************************************************
// Function: WINDOW_MODE_ToString
//
// Purpose: 
//   Returns a string version of WINDOW_MODE.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 15 16:44:12 PST 2003
//
// Modifications:
//
// ****************************************************************************

std::string
WINDOW_MODE_ToString(WINDOW_MODE m)
{
    int index = int(m);
    if(m < WINMODE_2D || m > WINMODE_NONE) index = 0;
    return WINDOW_MODE_strings[index];
}

std::string
WINDOW_MODE_ToString(int m)
{
    int index = m;
    if(index < 0 || index > 6) index = 0;
    return WINDOW_MODE_strings[index];
}

// ****************************************************************************
// Function: WINDOW_MODE_FromString
//
// Purpose: 
//   Returns an WINDOW_MODE associated with a string value.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 15 16:44:44 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool
WINDOW_MODE_FromString(const std::string &s, WINDOW_MODE &m)
{
    m = WINMODE_NONE;

    for(int i = 0; i < 6; ++i)
    {
        if(s == WINDOW_MODE_strings[i])
        {
            m = WINDOW_MODE(i);
            return true;
        }
    }

    return false;
}

static const char *INTERACTION_MODE_strings[] = {"NAVIGATE", "ZONE_PICK",
    "NODE_PICK", "ZOOM", "LINEOUT", "SPREADSHEET_PICK"};

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
//    Gunther H. Weber, Wed Mar 19 16:08:41 PDT 2008
//    Updadated bounds for new SPREADSHEET_PICK
//
//   Jonathan Byrd (Allinea Software), Sun Dec 18, 2011
//   Added the DDT_PICK mode
//
// ****************************************************************************

std::string
INTERACTION_MODE_ToString(INTERACTION_MODE m)
{
    int index = int(m);
    if(m < NAVIGATE || m > DDT_PICK) index = 0;
    return INTERACTION_MODE_strings[index];
}

std::string
INTERACTION_MODE_ToString(int m)
{
    int index = m;
    if(index < 0 || index > 5) index = 0;
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
//    Gunther H. Weber, Wed Mar 19 16:08:41 PDT 2008
//    Updadated bounds for new SPREADSHEET_PICK
//   
// ****************************************************************************

bool
INTERACTION_MODE_FromString(const std::string &s, INTERACTION_MODE &m)
{
    m = NAVIGATE;

    for(int i = 0; i < 6; ++i)
    {
        if(s == INTERACTION_MODE_strings[i])
        {
            m = INTERACTION_MODE(i);
            return true;
        }
    }

    return false;
}

static const char *TOOLUPDATE_MODE_strings[] = {"CONTINUOUS",
                                                "ONRELEASE",
                                                "ONCLOSE"};

// ****************************************************************************
// Function: TOOLUPDATE_MODE_ToString
//
// Purpose: 
//   Returns a string version of TOOLUPDATE_MODE.
//
// Programmer: Jeremy Meredith
// Creation:   February  2, 2010
//
// Modifications:
//
// ****************************************************************************

std::string
TOOLUPDATE_MODE_ToString(TOOLUPDATE_MODE m)
{
    int index = int(m);
    if (m < UPDATE_CONTINUOUS || m > UPDATE_ONCLOSE)
        index = UPDATE_ONRELEASE;
    return TOOLUPDATE_MODE_strings[index];
}

std::string
TOOLUPDATE_MODE_ToString(int m)
{
    int index = m;
    if (index < 0 || index > 2)
        index = (int)UPDATE_ONRELEASE;
    return TOOLUPDATE_MODE_strings[index];
}

// ****************************************************************************
// Function: TOOLUPDATE_MODE_FromString
//
// Purpose: 
//   Returns an TOOLUPDATE_MODE associated with a string value.
//
// Programmer: Jeremy Meredith
// Creation:   February  2, 2010
//
// Modifications:
//   
// ****************************************************************************

bool
TOOLUPDATE_MODE_FromString(const std::string &s, TOOLUPDATE_MODE &m)
{
    m = UPDATE_ONRELEASE;

    for(int i = 0; i < 3; ++i)
    {
        if(s == TOOLUPDATE_MODE_strings[i])
        {
            m = TOOLUPDATE_MODE(i);
            return true;
        }
    }

    return false;
}

// ****************************************************************************
// Class: VisWinTextAttributes
//
// Purpose:
//   Simple text attributes class that we can use.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 29 16:35:07 PST 2008
//
// Modifications:
//   Brad Whitlock, Wed Mar 26 14:28:02 PDT 2008
//   Changed height to scale.
//
// ****************************************************************************

VisWinTextAttributes::VisWinTextAttributes()
{
    font = Arial;
    scale = 1.;
    useForegroundColor = true;
    color[0] = 0.f;
    color[1] = 0.f;
    color[2] = 0.f;
    color[3] = 1.f;
    bold = false;
    italic = false;
}

VisWinTextAttributes::VisWinTextAttributes(const VisWinTextAttributes &obj)
{
    font = obj.font;
    scale = obj.scale;
    useForegroundColor = obj.useForegroundColor;
    color[0] = obj.color[0];
    color[1] = obj.color[1];
    color[2] = obj.color[2];
    color[3] = obj.color[3];
    bold = obj.bold;
    italic = obj.italic;
}

VisWinTextAttributes::~VisWinTextAttributes()
{
}

VisWinTextAttributes
VisWinTextAttributes::operator = (const VisWinTextAttributes &obj)
{
    font = obj.font;
    scale = obj.scale;
    useForegroundColor = obj.useForegroundColor;
    color[0] = obj.color[0];
    color[1] = obj.color[1];
    color[2] = obj.color[2];
    color[3] = obj.color[3];
    bold = obj.bold;
    italic = obj.italic;
    return *this;
}

