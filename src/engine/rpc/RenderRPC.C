// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "RenderRPC.h"
#include <DebugStream.h>


// ****************************************************************************
//  Constructor: RenderRPC::RenderRPC
//
//  Purpose: 
//    This is the constructor.
//
//  Arguments:
//
//  Programmer: Mark C. Miller
//  Creation:   07Apr03
//
//  Modifications:
//
//    Mark C. Miller, Mon Mar 29 14:11:42 PST 2004
//    Added bool for 3D annotations
//
//    Mark C. Miller, Wed Oct  6 18:12:29 PDT 2004
//    Changed 3D annotation flag to integer mode
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added window id
//
//    Brad Whitlock, Thu Jan 13 13:44:47 PST 2005
//    Initialized member values.
//
//    Mark C. Miller, Sat Jul 22 23:21:09 PDT 2006
//    Added leftEye to support stereo SR
//
//    Brad Whitlock, Mon Sep 25 14:02:31 PDT 2017
//    Added image type.
//
// ****************************************************************************

RenderRPC::RenderRPC() : NonBlockingRPC("i*biibi"), ids()
{
    sendZBuffer = false;
    annotMode = 0;
    windowID = 0;
    leftEye = true;
    imageType = ColorRGBImage;
}

// ****************************************************************************
//  Destructor: RenderRPC::~RenderRPC
//
//  Purpose: 
//    This is the destructor.
//
//  Arguments:
//
//  Programmer: Mark C. Miller
//  Creation:   07Apr03
//
// ****************************************************************************

RenderRPC::~RenderRPC()
{
}

// ****************************************************************************
//  Method: RenderRPC::operator()
//
//  Purpose: 
//    This is the RPC's invocation method.
//
//  Arguments:
//    id        the network id
//
//  Programmer: Mark C. Miller
//  Creation:   07Apr03
//
//  Modifications:
//
//    Mark C. Miller, Mon Mar 29 14:11:42 PST 2004
//    Added bool for 3D annotations
//
//    Mark C. Miller, Wed Oct  6 18:12:29 PDT 2004
//    Changed 3D annotation flag to integer mode
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added window id
//
//    Mark C. Miller, Sat Jul 22 23:21:09 PDT 2006
//    Added leftEye to support stereo SR
//
//    Brad Whitlock, 
//    Added image type.
//
// ****************************************************************************

void
RenderRPC::operator()(avtImageType imgT,
    const intVector& ids_, bool sendZBuffer_, 
    int annotMode_, int windowID_, bool leftEye_)
{
    SetImageType(imgT);
    SetIDs(ids_);
    SetSendZBuffer(sendZBuffer_);
    SetAnnotMode(annotMode_);
    SetWindowID(windowID_);
    SetLeftEye(leftEye_);

    Execute();
}

// ****************************************************************************
//  Method: RenderRPC::SelectAll
//
//  Purpose: 
//    This selects all variables for sending.
//
//  Arguments:
//
//  Programmer: Mark C. Miller
//  Creation:   07Apr03
//
//  Modifications:
//
//    Mark C. Miller, Wed Oct  6 18:12:29 PDT 2004
//    Changed 3D annotation flag to integer mode
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added window id
//
//    Mark C. Miller, Sat Jul 22 23:21:09 PDT 2006
//    Added leftEye to support stereo SR
// ****************************************************************************

void
RenderRPC::SelectAll()
{
    Select(0, (void*)&ids);
    Select(1, (void*)&sendZBuffer);
    Select(2, (void*)&annotMode);
    Select(3, (void*)&windowID);
    Select(4, (void*)&leftEye);
    Select(5, (void*)&imageType);
}


// ****************************************************************************
//  Method: RenderRPC::SetXXX methods
//
//  Purpose: 
//    This sets the id parameter.
//
//  Arguments:
//    ids_         the ids
//
//  Programmer: Mark C. Miller
//  Creation:   07Apr03
//
//  Modifications:
//    Mark C. Miller, added method for 3D annotations
//
//    Mark C. Miller, Wed Oct  6 18:12:29 PDT 2004
//    Changed 3D annotation flag to integer mode
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added window id
//
//    Brad Whitlock, Thu Jan 13 13:45:17 PST 2005
//    Fixed critical bug in SetWindowID where the address of the argument was
//    being taken for the Select call.
//
//    Mark C. Miller, Sat Jul 22 23:21:09 PDT 2006
//    Added leftEye to support stereo SR
// ****************************************************************************

void
RenderRPC::SetIDs(const intVector& ids_)
{
    ids = ids_;
    Select(0, (void*)&ids);
}

void
RenderRPC::SetSendZBuffer(bool sendZBuffer_)
{
    sendZBuffer = sendZBuffer_;
    Select(1, (void*)&sendZBuffer);
}

void
RenderRPC::SetAnnotMode(int annotMode_)
{
    annotMode = annotMode_;
    Select(2, (void*)&annotMode);
}

void
RenderRPC::SetWindowID(int windowID_)
{
    windowID = windowID_;
    Select(3, (void*)&windowID);
}

void
RenderRPC::SetLeftEye(bool leftEye_)
{
    leftEye = leftEye_;
    Select(4, (void*)&leftEye);
}

void
RenderRPC::SetImageType(avtImageType imgT)
{
    imageType = (int)imgT;
    Select(5, (void*)&imageType);
}

// ****************************************************************************
//  Method: RenderRPC::GetXXX methods
//
//  Purpose: 
//    This returns network id.
//
//  Arguments:
//
//  Programmer: Mark C. Miller
//  Creation:   07Apr03
//
//  Modifications:
//    Mark C. Miller, added method for 3D annotations
//
//    Mark C. Miller, Wed Oct  6 18:12:29 PDT 2004
//    Changed 3D annotation flag to integer mode
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added window id
//
//    Mark C. Miller, Sat Jul 22 23:21:09 PDT 2006
//    Added leftEye to support stereo SR
// ****************************************************************************

const intVector&
RenderRPC::GetIDs() const
{
    return ids;
}

bool
RenderRPC::GetSendZBuffer() const
{
    return sendZBuffer;
}

int
RenderRPC::GetAnnotMode() const
{
    return annotMode;
}

int
RenderRPC::GetWindowID() const
{
    return windowID;
}

bool
RenderRPC::GetLeftEye() const
{
    return leftEye;
}

avtImageType
RenderRPC::GetImageType() const
{
    return (avtImageType)imageType;
}
