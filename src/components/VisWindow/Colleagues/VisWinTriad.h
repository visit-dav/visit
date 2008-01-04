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
//                               VisWinTriad.h                               //
// ************************************************************************* //

#ifndef VIS_WIN_TRIAD_H
#define VIS_WIN_TRIAD_H
#include <viswindow_exports.h>


#include <VisWinColleague.h>


class vtkTriad2D;

class VisWindowColleagueProxy;


// ****************************************************************************
//  Class: VisWinTriad
//
//  Purpose:
//      This is a concrete colleague for the mediator VisWindow.  It places
//      a triad in the lower left hand corner of the screen.
//
//  Programmer: Hank Childs
//  Creation:   June 8, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Jul  6 11:01:24 PDT 2000
//    Added HasPlots/NoPlots and addedTriad.
//
//    Hank Childs, Wed Jul 12 09:12:02 PDT 2000
//    Removed axis actors in favor of new vtkTriad2D class.
//
//    Kathleen Bonnell, Wed Jun 27 14:56:09 PDT 2001 
//    Added SetVisibility. 
//
// ****************************************************************************

class VISWINDOW_API VisWinTriad : public VisWinColleague
{
  public:
                              VisWinTriad(VisWindowColleagueProxy &);
    virtual                  ~VisWinTriad();

    virtual void              SetForegroundColor(double, double, double);

    virtual void              Start3DMode(void);
    virtual void              Stop3DMode(void);

    virtual void              HasPlots(void);
    virtual void              NoPlots(void);

    void                      SetVisibility(bool);

  protected:
    vtkTriad2D               *triad;

    bool                      addedTriad;

    void                      AddTriadToWindow(void);
    void                      RemoveTriadFromWindow(void);
    bool                      ShouldAddTriad(void);
};


#endif


