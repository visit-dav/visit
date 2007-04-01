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
// ****************************************************************************

SetWinAnnotAttsRPC::SetWinAnnotAttsRPC() : BlockingRPC("aaasaI")
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
// ****************************************************************************

void
SetWinAnnotAttsRPC::operator()(const WindowAttributes *winAtts,
                               const AnnotationAttributes *annotAtts,
                               const AnnotationObjectList *aoList,
                               const string extStr,
                               const VisualCueList *cueList,
                               const int *frameAndState) 
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

    if (winAtts || annotAtts || aoList || extStr.size() || cueList ||
        frameAndState)
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
