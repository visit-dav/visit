/*****************************************************************************
*
* Copyright (c) 2000 - 2018, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
