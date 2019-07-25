// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef ICET_NETWORK_MANAGER_H
#define ICET_NETWORK_MANAGER_H

#include <NetworkManager.h>
#include <IceT.h>
#include <engine_main_exports.h>

class Engine;

// ****************************************************************************
//  Class: IceTNetworkManager
//
//  Purpose:
//      NetworkManager which uses IceT for rendering/readback of image data.
//
//  Programmer: Tom Fogal
//  Creation:   June 17, 2008
//
//  Modifications:
//
//    Tom Fogal, Tue Jun 24 13:27:48 EDT 2008
//    Defined `Readback' function.
//
//    Tom Fogal, Mon Jul 14 12:27:23 PDT 2008
//    Override parent's timer information.
//
//    Tom Fogal, Wed Jul 16 12:59:37 EDT 2008
//    Oops, destructor should be virtual.
//
//    Tom Fogal, Sat Jul 26 23:07:15 EDT 2008
//    Override RenderGeometry for a potential IceT-only optimization.
//
//    Tom Fogal, Sat Jul 26 23:07:15 EDT 2008
//    Override RenderTranslucent so we can avoid an avtImageCompositor.
//
//    Brad Whitlock, Mon Mar  2 16:38:12 PST 2009
//    I made Render return an avtDataObject_p.
//
//    Tom Fogal, Wed May 18 13:08:40 MDT 2011
//    Add new state for the number of renderings we've seen.
//
//    Burlen Loring, Tue Sep  1 14:26:30 PDT 2015
//    sync up with network manager(base class) order compositing refactor
//
//    Burlen Loring, Sat Oct 17 07:57:42 PDT 2015
//    temporarily remove the unused methods because they are
//    creating compiler warnings.
//
// ****************************************************************************

class ENGINE_MAIN_API IceTNetworkManager: public NetworkManager
{
 public:
               IceTNetworkManager(void);
    virtual   ~IceTNetworkManager(void);

    virtual avtDataObject_p Render(avtImageType imgT,
                                   bool getZBuffer,
                                   intVector networkIds,
                                   bool checkThreshold,
                                   int annotMode,
                                   int windowID,
                                   bool leftEye,
                                   int &outImgWidth,
                                   int &outImgHeight);
 protected:
    virtual avtImage_p RenderGeometry();

    void         ShrinkValidPixelsViewport(avtImage_p img, 
                                           IceTInt valid_pixels_viewport[4]) const;
    void         BroadcastFinishedImage(avtImage_p img, 
                                        int ncomps, 
                                        int width, 
                                        int height) const;
    void         CompositeBackground(avtImage_p img, avtImage_p bgImage) const;
    void         CompositeSolidBackground(avtImage_p img,  
                                          unsigned char bgColor[3]) const;
    void         ConvertChannels4to3(avtImage_p img) const;

    virtual void StopTimer();
    virtual void FormatDebugImage(char*, size_t, const char*) const;

 private:
    bool useIceTRenderGeometry;
    unsigned int renderings; ///< counts how many renderings we've done.
    int *compositeOrder;
    int batonTag;
    IceTCommunicator comm;
    IceTContext context;
 };
#endif /* ICET_NETWORK_MANAGER_H */
