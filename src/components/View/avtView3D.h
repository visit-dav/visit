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

// ************************************************************************* //
//                                 avtView3D.h                               //
// ************************************************************************* //

#ifndef AVT_VIEW_3D_H
#define AVT_VIEW_3D_H
#include <view_exports.h>

struct avtViewInfo;
class View3DAttributes;

// ****************************************************************************
//  Class: avtView3D
//
//  Purpose:
//    Contains the information for a 3D view.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2001
//
//  Modifications:
//    Eric Brugger, Fri Mar 29 15:09:35 PST 2002
//    Remove the method SetViewFromViewInfo.
//
//    Eric Brugger, Fri Jun  6 15:20:43 PDT 2003
//    I added image pan and image zoom.
//
//    Eric Brugger, Wed Aug 20 09:38:17 PDT 2003
//    I replaced SetFromViewAttributes with SetFromView3DAttributes and
//    SetToViewAttributes with SetToView3DAttributes.
//
//    Hank Childs, Wed Oct 15 13:05:33 PDT 2003
//    Added eye angle.
//
//    Eric Brugger, Mon Feb  9 15:59:15 PST 2004
//    Added centerOfRotationSet and centerOfRotation.
//
// ****************************************************************************

struct AVTVIEW_API avtView3D
{
    double   normal[3];
    double   focus[3];
    double   viewUp[3];
    double   viewAngle;
    double   parallelScale;
    double   nearPlane;
    double   farPlane;
    double   imagePan[2];
    double   imageZoom;
    double   eyeAngle;
    bool     perspective;
    bool     centerOfRotationSet;
    double   centerOfRotation[3];

  public:
                    avtView3D();
    avtView3D     & operator=(const avtView3D &);
    bool            operator==(const avtView3D &);
    void            SetToDefault(void);
    void            SetViewInfoFromView(avtViewInfo &) const;

    void            SetFromView3DAttributes(const View3DAttributes *);
    void            SetToView3DAttributes(View3DAttributes *) const;
};


#endif

