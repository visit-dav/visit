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
//                              VisWinQuery.h                                //
// ************************************************************************* //

#ifndef VIS_WIN_QUERY_H
#define VIS_WIN_QUERY_H
#include <viswindow_exports.h>

#include <VisualCueInfo.h>
#include <VisWinColleague.h>
#include <VisWindowTypes.h>
#include <avtPickActor.h>
#include <avtLineoutActor.h>

// ****************************************************************************
//  Class: VisWinQuery
//
//  Purpose:
//    Handles queries. (Currently only pick & lineout). 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 13, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Mon Mar 18 09:27:06 PST 2002
//    To better control size of pick letters, changed signature of method 
//    ComputePickScaleFactor.  Add method SetPickScale, member hidden.  
//    Removed member pickScaleFactor.
//
//    Kathleen Bonnell, Tue Mar 26 16:08:23 PST 2002  
//    Moved pick-specific code to avtPickActor.  Keep a list of pick actors.
//    
//    Kathleen Bonnell, Tue Apr 15 15:38:14 PDT 2002  
//    Added support for other query types.  Specifically, Lineout. 
//    
//    Kathleen Bonnell, Wed Jun 19 18:05:04 PDT 2002 
//    Added UpdateQuery, DeleteQuery.
//    
//    Kathleen Bonnell, Tue Oct  1 16:25:50 PDT 2002 
//    Changed argument for QueryIsValid and Lineout to Line* to convey more
//    information than simply color. 
//    
//    Kathleen Bonnell, Thu Dec 19 13:32:47 PST 2002 
//    Removed member designator and method GetNextDesignator. Added 
//    argument to QueryIsValid.
//    
//    Kathleen Bonnell, Fri Jan 31 11:34:03 PST 2003 
//    Replaced char * argument of QueryIsValid and Pick methods with 
//    PickAttributes.
//    
//    Kathleen Bonnell, Fri Jun  6 15:17:45 PDT 2003  
//    Added FullFrameOn/Off methods. 
//    
//    Kathleen Bonnell, Wed Jun 25 14:30:39 PDT 2003 
//    Removed type & its Set/Get methods.  Removed unused methods
//    ClearQueries, ClearAllQueries. Removed attachmentPoint/secondaryPoint
//    and their Set methods.
//    
//    Kathleen Bonnell, Tue Jul  8 20:02:21 PDT 2003 
//    Add method ReAddToWindow. 
//    
//    Kathleen Bonnell, Fri Feb 20 12:37:26 PST 2004 
//    Added methods CreateTranslationVector, CreateShiftVector, 
//    CalculateShiftDistance.
//
//    Mark C. Miller Wed Jun  9 17:44:38 PDT 2004
//    Modified with PickEntry and LineEntry data types in vectors of pickPoints
//    and refLines. Changed some interfaces to use VisualCueInfo.
//    
//    Kathleen Bonnell, Wed Aug 18 09:44:09 PDT 2004 
//    Added Start2DMode, StartCurveMode and Start3D mode so that pick actors
//    inappropriate for the new mode can be removed.   Added optional int arg
//    to ClearPickPoints method.
//    
// ****************************************************************************
class VISWINDOW_API VisWinQuery : public VisWinColleague
{
  public:
                                  VisWinQuery(VisWindowColleagueProxy &);
    virtual                      ~VisWinQuery();

    void                          EndBoundingBox(void);
    void                          StartBoundingBox(void);

    virtual void                  SetForegroundColor(double, double, double);
    virtual void                  UpdateView(void);

    void                          QueryIsValid(const VisualCueInfo *, const VisualCueInfo *);
    void                          UpdateQuery(const VisualCueInfo *);
    void                          DeleteQuery(const VisualCueInfo *);

    void                          Lineout(const VisualCueInfo *);
    void                          ClearLineouts(void);

    void                          Pick(const VisualCueInfo *);
    void                          ClearPickPoints(const int which = 0);

    virtual void                  FullFrameOn(const double, const int);
    virtual void                  FullFrameOff(void);
    virtual void                  ReAddToWindow(void);

    void                          GetVisualCues(const VisualCueInfo::CueType cueType,
                                      std::vector<const VisualCueInfo *>& cues) const;


    virtual void             Start2DMode();
    virtual void             Start3DMode();
    virtual void             StartCurveMode();

  protected:
    void                          CreateTranslationVector(const double, 
                                      const int, double vec[3]);
    void                          CreateTranslationVector(double vec[3]);
    void                          CreateShiftVector(double vec[3], const double);
    double                         CalculateShiftDistance(void);

    struct PickEntry {
        avtPickActor_p pickActor;
        VisualCueInfo vqInfo;
    };

    struct LineEntry {
        avtLineoutActor_p lineActor;
        VisualCueInfo vqInfo;
    };

    std::vector< PickEntry > pickPoints;
    std::vector< LineEntry > lineOuts;

    bool                       hidden;
};


#endif
