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
//                              VisWinLegends.h                              //
// ************************************************************************* //

#ifndef VIS_WIN_LEGENDS_H
#define VIS_WIN_LEGENDS_H
#include <viswindow_exports.h>


#include <VisWinColleague.h>

class     vtkTextActor;


// ****************************************************************************
//  Class: VisWinLegends
//
//  Purpose:
//      Manages the legends for each plot -- their placement and whether they
//      should have annotation for cycles/times, etc.
//
//  Programmer: Hank Childs
//  Creation:   March 12, 2002
//
//  Modifications:
//    Brad Whitlock, Thu Apr 11 12:11:13 PDT 2002
//    Added flag to turn off the main database.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002 
//    vtkScaledTextActor has been deprecated in favor of vtkTextActor. 
//    vtkTextActor defines its own mapper, so vtkTextMapper not needed. 
//
//    Eric Brugger, Mon Jul 14 16:27:28 PDT 2003
//    Changed the way database information is handled.
//
//    Eric Brugger, Wed Jul 16 09:47:15 PDT 2003
//    Removed GetPosition.
//
//    Kathleen Bonnell, Thu Nov 13 12:21:25 PST 2003 
//    Added bool return type to CreateDatabaseInfo. 
//
//    Brad Whitlock, Thu Mar 22 02:17:55 PDT 2007
//    Renamed PositionLegends to UpdateLegendInfo.
//
// ****************************************************************************

class VISWINDOW_API VisWinLegends : public VisWinColleague
{
  public:
                                  VisWinLegends(VisWindowColleagueProxy &);
    virtual                      ~VisWinLegends();

    virtual void                  SetForegroundColor(double, double, double);
    virtual void                  UpdatePlotList(std::vector<avtActor_p> &);

    void                          SetVisibility(bool db, bool legend);

  protected:
    vtkTextActor                 *dbInfoActor;
    bool                          dbInfoIsAdded;
    bool                          mainDBInfoVisible;
    bool                          legendVisible;
    bool                          homogeneous;
    
    static const double            leftColumnPosition;
    static const double            rightColumnPosition;
    static const double            dbInfoHeight;
    static const double            dbInfoWidth;

    void                          UpdateLegendInfo(std::vector<avtActor_p> &);
    void                          UpdateDBInfo(std::vector<avtActor_p> &);

  private:
    static bool                   CreateDatabaseInfo(char *,
                                                     avtDataAttributes &);
};


#endif


