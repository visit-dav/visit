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
// ****************************************************************************

SetWinAnnotAttsRPC::SetWinAnnotAttsRPC() : BlockingRPC("aa")
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
// ****************************************************************************

void
SetWinAnnotAttsRPC::operator()(const WindowAttributes *winAtts,
                               const AnnotationAttributes *annotAtts)
{
    if (winAtts)
       SetWindowAtts(winAtts);

    if (annotAtts)
       SetAnnotationAtts(annotAtts);

    if (winAtts || annotAtts)
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
// ****************************************************************************

void
SetWinAnnotAttsRPC::SelectAll()
{
    Select(0, (void*)&win);
    Select(1, (void*)&annot);
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
