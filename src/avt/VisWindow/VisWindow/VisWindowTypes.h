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
//                             VisWindowTypes.h                              
//
//  Modifications:
//    Kathleen Bonnell, Fri Jun 27 16:25:01 PDT 2003  
//    For INTERACTION_MODE, renamed PICK to ZONE_PICK, added NODE_PICK.
//    Removed QUERY_TYPE, no longer needed.
//
//    Brad Whitlock, Tue Jul 15 16:34:53 PST 2003
//    Added methods to convert INTERACTION_MODE to and from string.
//
//    Jeremy Meredith, Mon Jan 28 17:36:52 EST 2008
//    Added new Axis Array window mode.
//
//    Jeremy Meredith, Fri Feb  1 15:45:02 EST 2008
//    Added two more data values to hotpoints: shape and an arbitrary data
//    value that gets passed along to the callbacks.  Also added a default
//    constructor so the new values get appropriate defaults.
//
//    Gunther H. Weber, Wed Mar 19 16:06:04 PDT 2008
//    Added SPREADSHEET_PICK to INTERACTION_MODE
//
// ************************************************************************* //


#ifndef VIS_WINDOW_TYPES_H
#define VIS_WINDOW_TYPES_H

#include <avtVector.h>
#include <viswindow_exports.h>
#include <string>

typedef enum
{
    NAVIGATE         = 0,
    ZONE_PICK,       /* 1 */
    NODE_PICK,       /* 2 */
    ZOOM,            /* 3 */
    LINEOUT,         /* 4 */
    SPREADSHEET_PICK /* 5*/
}  INTERACTION_MODE;

typedef enum
{
    WINMODE_2D          = 0,
    WINMODE_3D,        /* 1 */
    WINMODE_CURVE,     /* 2 */
    WINMODE_AXISARRAY, /* 3 */
    WINMODE_NONE       /* 4 */
}  WINDOW_MODE;

typedef enum {
    CB_START         = 0,
    CB_MIDDLE,      /* 1 */
    CB_END          /* 2 */
}  CB_ENUM;

// Forward declaration.
class VisitInteractiveTool;

typedef void (hotPointCallback)(VisitInteractiveTool *,CB_ENUM,int,int,int,int,int);

struct HotPoint
{
    avtVector             pt;
    double                radius;
    int                   data;  // passed along to callback
    int                   shape; // 0 = square, 1 = tri up, 2 = tri down
    VisitInteractiveTool *tool;
    hotPointCallback     *callback;

    HotPoint() : radius(0), data(0), shape(0), tool(NULL), callback(NULL) { }
};

// ****************************************************************************
// Class: VisWinTextAttributes
//
// Purpose:
//   Simple text attributes class that we can use in VisWin.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 29 16:35:07 PST 2008
//
// Modifications:
//   
// ****************************************************************************

struct VisWinTextAttributes
{
    typedef enum // This matches VTK
    { 
        Arial = 0,
        Courier,
        Times
    } FontID;

    VisWinTextAttributes();
    VisWinTextAttributes(const VisWinTextAttributes &);
    ~VisWinTextAttributes();
    VisWinTextAttributes operator = (const VisWinTextAttributes &);

    FontID font;
    double height;
    bool   useForegroundColor;
    double color[4];
    bool   bold;
    bool   italic;
};

// Functions to convert INTERACTION_MODE to/from string.
VISWINDOW_API std::string INTERACTION_MODE_ToString(INTERACTION_MODE);
VISWINDOW_API std::string INTERACTION_MODE_ToString(int);
VISWINDOW_API bool        INTERACTION_MODE_FromString(const std::string &,
                                                      INTERACTION_MODE &);

// ****************************************************************************
//  Function: ValidMode
//
//  Purpose: Determines if the argument represents a "valid" mode.  A "valid"
//           mode means WINMODE_2D or WINMODE_3D.
//
//  Arguments:
//     mode     The mode to check.
//
//  Returns:    True if a valid mode was specified, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   June 9, 2000
//
// ****************************************************************************

inline bool
ValidMode(WINDOW_MODE mode)
{
    if (mode == WINMODE_2D ||
        mode == WINMODE_3D ||
        mode == WINMODE_CURVE ||
        mode == WINMODE_AXISARRAY)
    {
        return true;
    }

    return false;
}


#endif


