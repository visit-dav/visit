/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                         VisWindowInteractorProxy.h                        //
// ************************************************************************* //

#ifndef VIS_WINDOW_INTERACTOR_PROXY_H
#define VIS_WINDOW_INTERACTOR_PROXY_H

#include <viswindow_exports.h>

#include <VisWindowProtectionProxy.h>


// ****************************************************************************
//  Class: VisWindowInteractorProxy
//
//  Purpose:
//      This is a proxy specific to VisWindow interactors.  They may use it
//      to access the VisWindow.  This object is not a friend to VisWindow,
//      but it may access it through its base class' methods and friend 
//      status.
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Jul  5 16:11:05 PDT 2000
//    Removed vtkPicker argument to Pick to match interface change.
//
//    Hank Childs, Fri Dec 29 10:12:13 PST 2000
//    Removed Picking altogether.
//
//    Brad Whitlock, Mon Oct 1 09:45:56 PDT 2001
//    Added the HasPlots method.
//
//    Brad Whitlock, Mon Oct 1 14:09:58 PST 2001
//    Added a method to return the HotPoint for a given x,y value.
//
//    Kathleen Bonnell, Wed Nov 12 10:54:55 PST 2001
//    Added Pick. 
//
//    Kathleen Bonnell, Wed May  8 15:02:28 PDT 2002  
//    Added Lineout. 
//
//    Hank Childs, Thu Jul 11 17:45:50 PDT 2002
//    Add support for motion tracking.
//
//    Brad Whitlock, Mon Mar 13 10:51:15 PDT 2006
//    Made it possible to get the foreground color.
//
// ****************************************************************************

class VISWINDOW_API VisWindowInteractorProxy : public VisWindowProtectionProxy
{
  public:
                        VisWindowInteractorProxy(VisWindow *vw) 
                             : VisWindowProtectionProxy(vw) {;};

    void                Render()
                                  { ProxiedRender(); };

    vtkRenderer        *GetBackground()
                                  { return ProxiedGetBackground(); };

    vtkRenderer        *GetCanvas()
                                  { return ProxiedGetCanvas(); };

    void                EndBoundingBox()
                                  { ProxiedEndBoundingBox(); };
    void                StartBoundingBox()
                                  { ProxiedStartBoundingBox(); };
    bool                GetBoundingBoxMode() const
                                  { return ProxiedGetBoundingBoxMode(); };
    bool                HasPlots() 
                                  { return ProxiedHasPlots(); };

    bool                GetHotPoint(int x, int y, HotPoint &h) const
                                  { return ProxiedGetHotPoint(x, y, h); };

    void                SetHighlightEnabled(bool val)
                                  { ProxiedSetHighlightEnabled(val); };

    void                Pick(int x, int y)
                                  { ProxiedPick(x, y); };

    void                Lineout(int x1, int y1, int x2, int y2)
                                  { ProxiedLineout(x1, y1, x2, y2); };

    void                MotionBegin(void) { ProxiedMotionBegin(); };
    void                MotionEnd(void)   { ProxiedMotionEnd(); };

    void                GetForegroundColor(double *fg)
                                  { ProxiedGetForegroundColor(fg); };
};


#endif


