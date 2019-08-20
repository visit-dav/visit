// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//    Eric Brugger, Tue Dec  9 14:32:10 PST 2008
//    Added the ParallelAxes window mode.
//
//    Mark C. Miller, Wed Mar  4 17:59:51 PST 2009
//    Adjusted for dbio-only build.
//
//    Jeremy Meredith, Tue Feb  2 13:46:25 EST 2010
//    Added new tool update mode.
//
//    Jonathan Byrd (Allinea Software) Sun 18 Dec 2011
//    Added DDT_PICK mode
//
//    Eric Brugger, Mon Nov  5 15:03:55 PST 2012
//    I added the ability to display the parallel axes either horizontally
//    or vertically.
//
// ************************************************************************* //


#ifndef VIS_WINDOW_TYPES_H
#define VIS_WINDOW_TYPES_H

typedef enum
{
    WINMODE_2D                = 0,
    WINMODE_3D,               /* 1 */
    WINMODE_CURVE,            /* 2 */
    WINMODE_AXISARRAY,        /* 3 */
    WINMODE_PARALLELAXES,     /* 4 */
    WINMODE_VERTPARALLELAXES, /* 5 */
    WINMODE_NONE              /* 6 */
}  WINDOW_MODE;

#ifndef DBIO_ONLY

#include <avtVector.h>
#include <viswindow_exports.h>
#include <string>

typedef enum
{
    NAVIGATE         = 0,
    ZOOM,             /* 1 */
    ZONE_PICK,        /* 2 */
    NODE_PICK,        /* 3 */
    SPREADSHEET_PICK, /* 4 */
    DDT_PICK,         /* 5 */
    LINEOUT           /* 6 */
}  INTERACTION_MODE;

typedef enum {
    CB_START         = 0,
    CB_MIDDLE,      /* 1 */
    CB_END          /* 2 */
}  CB_ENUM;

typedef enum
{
    UPDATE_CONTINUOUS  = 0,
    UPDATE_ONRELEASE, /* 1 */
    UPDATE_ONCLOSE    /* 2 */
}  TOOLUPDATE_MODE;


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
//   Brad Whitlock, Wed Mar 26 14:26:51 PDT 2008
//   Changed height to scale so we can think in terms of scaling the preferred
//   size by some factor.
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
    double scale;
    bool   useForegroundColor;
    double color[4];
    bool   bold;
    bool   italic;
};

// Functions to convert WINDOW_MODE to/from string.
VISWINDOW_API std::string WINDOW_MODE_ToString(WINDOW_MODE);
VISWINDOW_API std::string WINDOW_MODE_ToString(int);
VISWINDOW_API bool        WINDOW_MODE_FromString(const std::string &,
                                                 WINDOW_MODE &);

// Functions to convert INTERACTION_MODE to/from string.
VISWINDOW_API std::string INTERACTION_MODE_ToString(INTERACTION_MODE);
VISWINDOW_API std::string INTERACTION_MODE_ToString(int);
VISWINDOW_API bool        INTERACTION_MODE_FromString(const std::string &,
                                                      INTERACTION_MODE &);

// Functions to convert TOOLUPDATE_MODE to/from string.
VISWINDOW_API std::string TOOLUPDATE_MODE_ToString(TOOLUPDATE_MODE);
VISWINDOW_API std::string TOOLUPDATE_MODE_ToString(int);
VISWINDOW_API bool        TOOLUPDATE_MODE_FromString(const std::string &,
                                                     TOOLUPDATE_MODE &);

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
//  Modifications:
//    Eric Brugger, Tue Dec  9 14:32:10 PST 2008
//    Added the ParallelAxes window mode.
//
// ****************************************************************************

inline bool
ValidMode(WINDOW_MODE mode)
{
    if (mode == WINMODE_2D ||
        mode == WINMODE_3D ||
        mode == WINMODE_CURVE ||
        mode == WINMODE_AXISARRAY ||
        mode == WINMODE_PARALLELAXES ||
        mode == WINMODE_VERTPARALLELAXES)
    {
        return true;
    }

    return false;
}

#endif // DBIO_ONLY


#endif


