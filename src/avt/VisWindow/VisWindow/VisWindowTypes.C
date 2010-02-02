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

#include <VisWindowTypes.h>

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
// ****************************************************************************

std::string
INTERACTION_MODE_ToString(INTERACTION_MODE m)
{
    int index = int(m);
    if(m < NAVIGATE || m > SPREADSHEET_PICK) index = 0;
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

