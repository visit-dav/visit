/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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
//                                 avtView2D.h                               //
// ************************************************************************* //

#ifndef AVT_VIEW_2D_H
#define AVT_VIEW_2D_H
#include <view_exports.h>

struct avtViewInfo;
class View2DAttributes;

// ****************************************************************************
//  Class: avtView2D
//
//  Purpose:
//    Contains the information for a 2D view.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2001
//
//  Modifications:
//    Kathleen Bonnell, Thu May 15 09:46:46 PDT 2003 
//    Added members axisScaleFactor and axisScaleType.
// 
//    Kathleen Bonnell, Wed Jul 16 16:46:02 PDT 2003 
//    Added methods ScaleWindow and ReverseScaleWindow.
// 
//    Eric Brugger, Wed Aug 20 09:35:51 PDT 2003
//    I replaced SetFromViewAttributes with SetFromView2DAttributes and
//    SetToViewAttributes with SetToView2DAttributes.
//
//    Eric Brugger, Wed Oct  8 16:43:28 PDT 2003
//    I replaced axisScaleFactor and axisScaleType with fullFrame.  I
//    deleted SetViewFromViewInfo.  I added window size arguments to
//    SetViewFromViewInfo.  I renamed SetViewportFromView to GetActualViewport.
//    I added GetScaleFactor.  I deleted ScaleWindow and ReverseScaleWindow.
//
//    Eric Brugger, Tue Nov 18 08:17:28 PST 2003
//    Replaced GetValidWindow with CheckAndCorrectWindow.
//
//    Mark C. Miller, Tue Mar 14 17:49:26 PST 2006
//    Added stuff to support auto full frame
//
// ****************************************************************************

struct AVTVIEW_API avtView2D
{
    double   viewport[4];
    double   window[4];
    bool     fullFrame;

    int      fullFrameActivationMode;
    float    fullFrameAutoThreshold;

  public:
                    avtView2D();
    avtView2D     & operator=(const avtView2D &);
    bool            operator==(const avtView2D &);

    // alternative to == that ignores autoff state
    bool            EqualViews(const avtView2D &);

    void            SetToDefault(void);
    void            SetViewInfoFromView(avtViewInfo &, int *);

    void            GetActualViewport(double *, const int, const int);
    double          GetScaleFactor(int *);

    void            SetFromView2DAttributes(const View2DAttributes *);
    void            SetToView2DAttributes(View2DAttributes *) const;

  protected:
    void            CheckAndCorrectWindow();
};


#endif

