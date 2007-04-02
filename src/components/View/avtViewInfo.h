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
//                                avtViewInfo.h                              //
// ************************************************************************* //

#ifndef AVT_VIEW_INFO_H
#define AVT_VIEW_INFO_H
#include <view_exports.h>

class vtkCamera;

// ****************************************************************************
//  Class: avtViewInfo
//
//  Purpose:
//    Contains all of the information for the view.
//
//  Programmer: Hank Childs
//  Creation:   November 27, 2000
//
//  Modifications:
//   Hank Childs, Wed Dec 27 14:28:01 PST 2000
//   Added viewUp.
//
//   Kathleen Bonnell, Thu Jan  4 15:40:32 PST 2001 
//   Added method to set the data members from a vtkCamera.
//
//   Kathleen Bonnell, Mon Jan  8 11:56:50 PST 2001 
//   Added method to set the data members to a vtkCamera.
//
//   Hank Childs, Mon Jun 18 09:03:19 PDT 2001
//   Added constructor.
//
//   Eric Brugger, Fri Jun  6 15:29:28 PDT 2003
//   I added image pan and image zoom.
//
//   Hank Childs, Wed Oct 15 13:09:03 PDT 2003
//   Added eye angle.
//
// ****************************************************************************

struct AVTVIEW_API avtViewInfo
{
    double   camera[3];
    double   focus[3];
    double   viewUp[3];
    double   viewAngle;
    double   eyeAngle;
    double   parallelScale;
    bool     setScale;
    double   nearPlane;
    double   farPlane;
    double   imagePan[2];
    double   imageZoom;
    bool     orthographic;

  public:
                    avtViewInfo();
    avtViewInfo   & operator=(const avtViewInfo &);
    bool            operator==(const avtViewInfo &);
    void            SetToDefault(void);
    void            SetViewFromCamera(vtkCamera *);
    void            SetCameraFromView(vtkCamera *) const;
};


#endif


