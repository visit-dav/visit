/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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
