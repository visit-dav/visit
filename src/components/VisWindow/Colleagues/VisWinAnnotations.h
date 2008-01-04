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

#ifndef VIS_WIN_ANNOTATIONS_H
#define VIS_WIN_ANNOTATIONS_H
#include <viswindow_exports.h>
#include <VisWinColleague.h>

class AnnotationObjectList;
class avtAnnotationColleague;

// ****************************************************************************
// Class: VisWinAnnotations
//
// Purpose: 
//   Contains some of the annotations for the vis window and manages them.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 14:21:00 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Mar 20 10:10:34 PDT 2007
//   Name the annotation objects.
//
// ****************************************************************************

class VISWINDOW_API VisWinAnnotations : public VisWinColleague
{
public:
                 VisWinAnnotations(VisWindowColleagueProxy &);
    virtual      ~VisWinAnnotations();

    // Overrides from VisWinColleague
    virtual void SetBackgroundColor(double, double, double);
    virtual void SetForegroundColor(double, double, double);

    virtual void Start2DMode(void);
    virtual void Start3DMode(void);
    virtual void StartCurveMode(void);
    virtual void Stop2DMode(void);
    virtual void Stop3DMode(void);
    virtual void StopCurveMode(void);

    virtual void HasPlots(void);
    virtual void NoPlots(void);

    virtual void MotionBegin(void);
    virtual void MotionEnd(void);

    virtual void UpdateView(void);

    virtual void UpdatePlotList(std::vector<avtActor_p> &);

    virtual void SetFrameAndState(int, int, int, int, int, int, int);

    // New methods for annotations
    bool         AddAnnotationObject(int annotType, const std::string &annotName);
    void         HideActiveAnnotationObjects();
    void         DeleteActiveAnnotationObjects();
    bool         DeleteAnnotationObject(const std::string &);
    void         DeleteAllAnnotationObjects();
    void         RaiseActiveAnnotationObjects();
    void         LowerActiveAnnotationObjects();
    void         SetAnnotationObjectOptions(const AnnotationObjectList &al);
    void         UpdateAnnotationObjectList(AnnotationObjectList &al);
    void         CreateAnnotationObjectsFromList(const AnnotationObjectList &al);
private:
    void UpdateLegends();

    std::vector<avtAnnotationColleague *> annotations;

    // DO NOT USE avtActor_p BECAUSE WHEN WE CLEAR THE VECTOR, THE ACTORS GET 
    // DELETED AGAIN!
    std::vector<avtActor *>               actorList;
};

#endif
