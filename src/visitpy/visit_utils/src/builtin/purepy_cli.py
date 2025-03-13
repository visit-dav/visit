# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

###############################################################################
# file: purepy_cli.py
#
# Purpose: A place to put VisIt Python CLI methods implemented with pure python
#
# Mark C. Miller, Wed Mar 12 15:16:16 PDT 2025
#
# Modifications:
#
###############################################################################

try:
    import visit
except:
    pass

####################################################################################
# function: GetActiveWindow 
#
# Purpose: return the identifier for the active window
#
# Mark C. Miller, Wed Mar 12 13:38:50 PDT 2025
#
####################################################################################

def GetActiveWindow():
    """A method that returns the currently active window identifier.

    This is slightly subtle because the attributes object returned by GetGlobalAttributes()
    has both an integer member named activeWindow and a python tuple of integers named
    windows. A window is identified in SetActiveWindw(id) (and here also in GetActiveWindw())
    by one of the numbers in that list. The activeWindow member is merely an integer index
    into that list and NOT itself the window identifer. These numbers are also the ones that
    appear in window title strings in whatever window manager VisIt is being run within. 

    On success, a valid window identifer which will be a positive integer is returned.
    On failure, 0 is returned.

    See also SetActiveWindow.
    """
    try:
        ga = visit.GetGlobalAttributes()
        return ga.windows[ga.activeWindow]
    except:
        try:
            ga = GetGlobalAttributes()
            return ga.windows[ga.activeWindow]
        except:
            pass
    return 0
