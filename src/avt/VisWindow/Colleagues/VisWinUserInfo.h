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
//                              VisWinUserInfo.h                             //
// ************************************************************************* //

#ifndef VIS_WIN_USER_INFO_H
#define VIS_WIN_USER_INFO_H
#include <viswindow_exports.h>


#include <VisWinColleague.h>


class vtkTextActor;

class VisWindowColleagueProxy;


// ****************************************************************************
//  Class: VisWinUserInfo
//
//  Purpose:
//      This is a concrete colleague for the mediator VisWindow.  It displays
//      the user info (user name and start time).
//
//  Programmer: Hank Childs
//  Creation:   June 8, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Jul  6 10:45:43 PDT 2000
//    Added HasPlots/NoPlots and addedUserInfo.
//
//    Hank Childs, Tue Sep 18 11:58:33 PDT 2001
//    Made destructor virtual.
//
//    Brad Whitlock, Thu Jan 10 08:15:14 PDT 2002
//    Added the SetVisibility method.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002  
//    vtkTextMapper/vtkScaledTextActor pairs have been replaced by 
//    vtkTextActor which defines its own mapper. (new vtk api).
//
//    Brad Whitlock, Wed Oct 29 08:46:49 PDT 2003
//    Added UpdateUserText method. Added override of UpdatePlotList method.
//
//    Brad Whitlock, Wed Jan 30 15:14:50 PST 2008
//    Added SetTextAttributes.
//
// ****************************************************************************

class VISWINDOW_API VisWinUserInfo : public VisWinColleague
{
  public:
                         VisWinUserInfo(VisWindowColleagueProxy &);
    virtual             ~VisWinUserInfo();

    virtual void         SetForegroundColor(double, double, double);

    virtual void         HasPlots(void);
    virtual void         NoPlots(void);
    virtual void         UpdatePlotList(std::vector<avtActor_p> &);

    void                 SetVisibility(bool);
    void                 SetTextAttributes(const VisWinTextAttributes &textAtts);

  protected:
    vtkTextActor        *infoActor;
    char                *infoString;
    VisWinTextAttributes textAttributes;

    bool                 addedUserInfo;

    void                 AddToWindow(void);
    void                 RemoveFromWindow(void);
    void                 UpdateUserText();

    static const float   defaultUserInfoHeight;
    static const float   defaultUserInfoWidth;
};


#endif


