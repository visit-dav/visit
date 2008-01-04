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

#include "SetWinAnnotAttsRPC.h"
#include <DebugStream.h>


// ****************************************************************************
//  Constructor: SetWinAnnotAttsRPC::SetWinAnnotAttsRPC
//
//  Purpose: 
//    This is the constructor.
//
//  Arguments:
//
//  Programmer: Mark C. Miller
//  Creation:   15Jul03
//
//  Modifications:
//
//    Mark C. Miller, Wed Apr 14 16:41:32 PDT 2004
//    Added data member for extents type string
//
//    Mark C. Miller, Tue May 25 17:06:12 PDT 2004
//    Added data member for AnnotationObjectList
//
//    Mark C. Miller, Wed Jun  9 17:44:38 PDT 2004
//    Added data member for VisualCueList 
//
//    Mark C. Miller, Tue Jul 27 15:11:11 PDT 2004
//    Added stuff to support frame and state info
//
//    Mark C. Miller, Wed Oct  6 18:12:29 PDT 2004
//    Added view extents double vector 
//
//    Mark C. Miller, Tue Oct 19 19:44:00 PDT 2004
//    Added string for color table name
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added window id
// ****************************************************************************

SetWinAnnotAttsRPC::SetWinAnnotAttsRPC() : BlockingRPC("aaasaIDsi")
{
}

// ****************************************************************************
//  Destructor: SetWinAnnotAttsRPC::~SetWinAnnotAttsRPC
//
//  Purpose: 
//    This is the destructor.
//
//  Arguments:
//
//  Programmer: Mark C. Miller
//  Creation:   15Jul03
//
// ****************************************************************************

SetWinAnnotAttsRPC::~SetWinAnnotAttsRPC()
{
}

// ****************************************************************************
//  Method: SetWinAnnotAttsRPC::operator()
//
//  Purpose: 
//    This is the RPC's invocation method.
//
//  Arguments:
//    winAtts       the window atts. may be null if not needed
//    annotAtts     the annotation attributes. may be null if not needed.
//
//  Programmer: Mark C. Miller
//  Creation:   15Jul03
//
//  Modifications:
//
//    Mark C. Miller, Wed Apr 14 16:41:32 PDT 2004
//    Added argument for extents type string
//
//    Mark C. Miller, Tue May 25 17:06:12 PDT 2004
//    Added argument for AnnotationObjectList
//
//    Mark C. Miller, Wed Jun  9 17:44:38 PDT 2004
//    Added arg for VisualCueList
//
//    Mark C. Miller, Tue Jul 27 15:11:11 PDT 2004
//    Added stuff to support frame and state info
//
//    Mark C. Miller, Wed Oct  6 18:12:29 PDT 2004
//    Added view extents double vector
//
//    Mark C. Miller, Tue Oct 19 19:44:00 PDT 2004
//    Added string for color table name
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added window id
// ****************************************************************************

void
SetWinAnnotAttsRPC::operator()(const WindowAttributes *winAtts,
                               const AnnotationAttributes *annotAtts,
                               const AnnotationObjectList *aoList,
                               const string extStr,
                               const VisualCueList *cueList,
                               const int *frameAndState,
                               const double *viewExtents,
                               const string ctName,
                               int winID)
{
    if (winAtts)
       SetWindowAtts(winAtts);

    if (annotAtts)
       SetAnnotationAtts(annotAtts);

    if (aoList)
       SetAnnotationObjectList(aoList);

    if (extStr.size())
       SetExtentTypeString(extStr);

    if (cueList)
       SetVisualCueList(cueList);

    if (frameAndState)
       SetFrameAndState(frameAndState);

    if (viewExtents)
       SetViewExtents(viewExtents);

    if (ctName.size())
       SetChangedCtName(ctName);

    SetWindowID(winID);

    if (winAtts || annotAtts || aoList || extStr.size() || cueList ||
        frameAndState || viewExtents || ctName.size())
       Execute();
}

// ****************************************************************************
//  Method: SetWinAnnotAttsRPC::SelectAll
//
//  Purpose: 
//    This selects all variables for sending.
//
//  Arguments:
//
//  Programmer: Mark C. Miller
//  Creation:   15Jul03
//
//  Modifications:
//
//    Mark C. Miller, Wed Apr 14 16:41:32 PDT 2004
//    Added data member for extents type string
//
//    Mark C. Miller, Wed Jun  9 17:44:38 PDT 2004
//    Added visual cue list data member
//
//    Mark C. Miller, Tue Jul 27 15:11:11 PDT 2004
//    Added stuff to support frame and state info
//
//    Mark C. Miller, Wed Oct  6 18:12:29 PDT 2004
//    Added view extents double vector
//
//    Mark C. Miller, Tue Oct 19 19:44:00 PDT 2004
//    Added string for color table name
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added window id
// ****************************************************************************

void
SetWinAnnotAttsRPC::SelectAll()
{
    Select(0, (void*)&win);
    Select(1, (void*)&annot);
    Select(2, (void*)&aolist);
    Select(3, (void*)&extstr);
    Select(4, (void*)&cuelist);
    Select(5, (void*)fands, sizeof(fands)/sizeof(fands[0]));
    Select(6, (void*)vexts, sizeof(vexts)/sizeof(vexts[0]));
    Select(7, (void*)&ctname);
    Select(8, (void*)&windowID);
}


// ****************************************************************************
//  Method: SetWinAnnotAttsRPC::SetWindowAtts
//
//  Purpose: 
//    This sets the window atts.
//
//  Arguments:
//    atts      the window atts
//
//  Programmer: Mark C. Miller
//  Creation:   15Jul03
//
// ****************************************************************************

void
SetWinAnnotAttsRPC::SetWindowAtts(const WindowAttributes *atts)
{
    win = *atts;
    Select(0, (void*)&win);
}

// ****************************************************************************
//  Method: SetWinAnnotAttsRPC::SetAnnotationAtts
//
//  Purpose: 
//    This sets the annotation atts.
//
//  Arguments:
//    atts      the annotation atts
//
//  Programmer: Mark C. Miller
//  Creation:   15Jul03
//
// ****************************************************************************

void
SetWinAnnotAttsRPC::SetAnnotationAtts(const AnnotationAttributes *atts)
{
    annot = *atts;
    Select(1, (void*)&annot);
}

// ****************************************************************************
//  Method: SetWinAnnotAttsRPC::SetAnnotationObjectList
//
//  Purpose: 
//    This sets the annotation object list.
//
//  Arguments:
//    aolist      the annotation object list
//
//  Programmer: Mark C. Miller
//  Creation:   May 25, 2004 
//
// ****************************************************************************

void
SetWinAnnotAttsRPC::SetAnnotationObjectList(const AnnotationObjectList *list)
{
    aolist = *list;
    Select(2, (void*)&aolist);
}

// ****************************************************************************
//  Method: SetWinAnnotAttsRPC::SetExtentTypeString
//
//  Purpose: 
//    This sets the extent type.
//
//  Arguments:
//    ext       the extent type 
//
//  Programmer: Mark C. Miller
//  Creation:   14Apr04 
//
// ****************************************************************************

void
SetWinAnnotAttsRPC::SetExtentTypeString(const string extstr_)
{
    extstr = extstr_;
    Select(3, (void*)&extstr);
}

// ****************************************************************************
//  Method: SetWinAnnotAttsRPC::SetVisualCueList
//
//  Programmer: Mark C. Miller
//  Creation:   June 7, 2004 
//
// ****************************************************************************

void
SetWinAnnotAttsRPC::SetVisualCueList(const VisualCueList* cueList) 
{
    cuelist = *cueList;
    Select(4,(void*)&cuelist);
}

// ****************************************************************************
//  Method: SetWinAnnotAttsRPC::SetFrameAndState
//
//  Programmer: Mark C. Miller
//  Creation:   July 26, 2004 
//
// ****************************************************************************

void
SetWinAnnotAttsRPC::SetFrameAndState(const int *frameAndState)
{
    for (int i = 0; i < sizeof(fands)/sizeof(fands[0]); i++)
        fands[i] = frameAndState[i];
    Select(5, (void*)fands, sizeof(fands)/sizeof(fands[0]));
}

// ****************************************************************************
//  Method: SetWinAnnotAttsRPC::SetViewExtents
//
//  Programmer: Mark C. Miller
//  Creation:   August 16, 2004 
//
// ****************************************************************************

void
SetWinAnnotAttsRPC::SetViewExtents(const double *viewExtents)
{
    for (int i = 0; i < sizeof(vexts)/sizeof(vexts[0]); i++)
        vexts[i] = viewExtents[i];
    Select(6, (void*)vexts, sizeof(vexts)/sizeof(vexts[0]));
}

// ****************************************************************************
//  Method: SetWinAnnotAttsRPC::SetChangedCtName
//
//  Programmer: Mark C. Miller
//  Creation:   October 19, 2004 
//
// ****************************************************************************

void
SetWinAnnotAttsRPC::SetChangedCtName(const string ctname_)
{
    ctname = ctname_;
    Select(7, (void*)&ctname);
}

// ****************************************************************************
//  Method: SetWinAnnotAttsRPC::SetWindowID
//
//  Programmer: Mark C. Miller
//  Creation:   December 15, 2004 
//
// ****************************************************************************

void
SetWinAnnotAttsRPC::SetWindowID(const int id)
{
    windowID = id;
    Select(8, (void*)&windowID);
}

// ****************************************************************************
//  Method: SetWinAnnotAttsRPC::GetWindowAtts
//
//  Purpose: 
//    This returns the window atts.
//
//  Programmer: Mark C. Miller
//  Creation:   15Jul03
//
// ****************************************************************************

const WindowAttributes&
SetWinAnnotAttsRPC::GetWindowAtts() const
{
    return win;
}

// ****************************************************************************
//  Method: SetWinAnnotAttsRPC::GetAnnotationObjectList
//
//  Purpose: 
//    This returns the annotation object list 
//
//  Programmer: Mark C. Miller
//  Creation:   May 25, 2004 
//
// ****************************************************************************

const AnnotationObjectList&
SetWinAnnotAttsRPC::GetAnnotationObjectList() const
{
    return aolist;
}

// ****************************************************************************
//  Method: SetWinAnnotAttsRPC::GetAnnotationAtts
//
//  Purpose: 
//    This returns the annotation atts.
//
//  Programmer: Mark C. Miller
//  Creation:   15Jul03
//
// ****************************************************************************

const AnnotationAttributes&
SetWinAnnotAttsRPC::GetAnnotationAtts() const
{
    return annot;
}

// ****************************************************************************
//  Method: SetWinAnnotAttsRPC::GetExtentType
//
//  Purpose: 
//    This returns the extent type.
//
//  Programmer: Mark C. Miller
//  Creation:   14Apr04 
//
// ****************************************************************************

const string&
SetWinAnnotAttsRPC::GetExtentTypeString() const
{
    return extstr;
}

// ****************************************************************************
//  Method: SetWinAnnotAttsRPC::GetVisualCues
//
//  Programmer: Mark C. Miller
//  Creation:   June 7, 2004 
//
// ****************************************************************************

const VisualCueList&
SetWinAnnotAttsRPC::GetVisualCueList() const
{
    return cuelist;
}

// ****************************************************************************
//  Method: SetWinAnnotAttsRPC::GetFrameAndState
//
//  Programmer: Mark C. Miller
//  Creation:   July 26, 2004 
//
// ****************************************************************************

const int*
SetWinAnnotAttsRPC::GetFrameAndState() const
{
    return fands;
}

// ****************************************************************************
//  Method: SetWinAnnotAttsRPC::GetViewExtents
//
//  Programmer: Mark C. Miller
//  Creation:   August 16, 2004 
//
// ****************************************************************************

const double*
SetWinAnnotAttsRPC::GetViewExtents() const
{
    return vexts;
}

// ****************************************************************************
//  Method: SetWinAnnotAttsRPC::GetChangedCtName
//
//  Programmer: Mark C. Miller
//  Creation:   October 19, 2004 
//
// ****************************************************************************

const string&
SetWinAnnotAttsRPC::GetChangedCtName() const
{
    return ctname;
}

// ****************************************************************************
//  Method: SetWinAnnotAttsRPC::GetWindowID
//
//  Programmer: Mark C. Miller
//  Creation:   December 15, 2004 
//
// ****************************************************************************

const int 
SetWinAnnotAttsRPC::GetWindowID() const
{
    return windowID;
}
