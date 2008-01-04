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
//                          VisWinBackground.h                               //
// ************************************************************************* //

#ifndef VIS_WIN_BACKGROUND_H
#define VIS_WIN_BACKGROUND_H
#include <viswindow_exports.h>

#include <VisWinColleague.h>

// Forward declarations.
class vtkBackgroundActor;
class vtkTexturedBackgroundActor;
class VisWindowColleagueProxy;

// ****************************************************************************
// Class: VisWinBackground
//
// Purpose:
//   This colleague displays a gradient background or an image background
//   for the Vis window.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 28 10:08:09 PDT 2001
//
// Modifications:
//    Brad Whitlock, Wed Nov 14 15:23:23 PST 2007
//    Added background image support.
//   
// ****************************************************************************

class VISWINDOW_API VisWinBackground : public VisWinColleague
{
  public:
                              VisWinBackground(VisWindowColleagueProxy &);
    virtual                  ~VisWinBackground();

    virtual void              SetGradientBackgroundColors(int,
                                                    double, double, double,
                                                    double, double, double);
    virtual void              SetBackgroundMode(int);
    virtual void              SetBackgroundImage(const std::string &,int,int);
    virtual void              UpdatePlotList(std::vector<avtActor_p> &);
  protected:
    static bool                 sphereModeError1;
    static bool                 sphereModeError2;

    vtkBackgroundActor         *bgActor;
    vtkTexturedBackgroundActor *textureActor;
    bool                        addedBackground;

    void                        AddBackgroundToWindow(int);
    void                        RemoveBackgroundFromWindow();
};

#endif
