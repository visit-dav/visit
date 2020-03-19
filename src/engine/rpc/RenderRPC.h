// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef RENDER_RPC_H
#define RENDER_RPC_H
#include <engine_rpc_exports.h>

#include <VisItRPC.h>
#include <avtImageType.h>

// ****************************************************************************
//  Class:  RenderRPC
//
//  Purpose:
//    Implements an RPC to render the plots on the specified networks 
//
//  Programmer:  Mark C. Miller 
//  Creation:    07Apr03
//
//  Modifications:
//
//    Mark C. Miller, Mon Mar 29 14:11:42 PST 2004 
//    Added bool controlling 3D annotations
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
//    Brad Whitlock, Thu Sep 21 16:40:58 PDT 2017
//    Added sendAlpha.
//
// ****************************************************************************

class ENGINE_RPC_API RenderRPC : public NonBlockingRPC
{
public:
    RenderRPC();
    virtual ~RenderRPC();

    virtual const std::string TypeName() const { return "RenderRPC"; }

    // Invokation method
    void operator()(avtImageType imgT,
                    const intVector& ids_, bool sendZBuffer_, 
                    int annotMode_, int windowID_, bool leftEye_);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetIDs(const intVector&);
    void SetSendZBuffer(bool);
    void SetAnnotMode(int);
    void SetWindowID(int);
    void SetLeftEye(bool);
    void SetImageType(avtImageType);

    // Property getting methods
    const intVector& GetIDs() const;
    bool GetSendZBuffer() const;
    int  GetAnnotMode() const;
    int GetWindowID() const;
    bool GetLeftEye() const;
    avtImageType GetImageType() const;

private:
    intVector ids;
    bool sendZBuffer;
    int annotMode;
    int windowID;
    bool leftEye;
    int imageType;
};

#endif
