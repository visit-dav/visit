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

#ifndef AVT_ANNOTATION_COLLEAGUE_H
#define AVT_ANNOTATION_COLLEAGUE_H
#include <VisWinColleague.h>
#include <avtLegend.h>

class AnnotationObject;

// ****************************************************************************
// Class: avtAnnotationColleague
//
// Purpose: 
//   Base class for certain annotations that can live in the window.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 16:06:04 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Mar 20 10:18:04 PDT 2007
//   Added support for setting legend properties.
//
// ****************************************************************************

class VISWINDOW_API avtAnnotationColleague : protected VisWinColleague
{
public:
    avtAnnotationColleague(VisWindowColleagueProxy &);
    virtual ~avtAnnotationColleague();

    virtual void AddToRenderer() = 0;
    virtual void RemoveFromRenderer() = 0;
    virtual void Hide() = 0;

    virtual std::string TypeName() const = 0;

    // Methods that return a little info about the annotation.
    void SetName(const std::string &n) { name = n; }
    const std::string &GetName() const { return name; }
    void SetActive(bool val) { active = val; }
    bool GetActive() const   { return active; }
    void SetVisible(bool val) { visible = val; }
    bool GetVisible() const   { return visible; }

    // Methods to set and get the annotation's properties.
    virtual void SetOptions(const AnnotationObject &annot) = 0;
    virtual void GetOptions(AnnotationObject &annot) = 0;

    // Legend methods.
    virtual bool ManageLayout(avtLegend_p legend) const { return false; }
    virtual void CustomizeLegend(avtLegend_p legend) { };

    // Methods that are called in response to vis window events. These
    // method can be overridden to let the annotation decide what to do when
    // these vis window events happen.
    virtual void SetBackgroundColor(double r, double g, double b) { };
    virtual void SetForegroundColor(double r, double g, double b) { };
    virtual void Start2DMode(void) { };
    virtual void Start3DMode(void) { };
    virtual void StartCurveMode(void) { };
    virtual void Stop2DMode(void) { };
    virtual void Stop3DMode(void) { };
    virtual void StopCurveMode(void) { };
    virtual void HasPlots(void) { };
    virtual void NoPlots(void) { };
    virtual void MotionBegin(void) { };
    virtual void MotionEnd(void) { };
    virtual void UpdateView(void) { };
    virtual void UpdatePlotList(std::vector<avtActor_p> &p) { };
    virtual void SetFrameAndState(int, int, int, int, int, int, int) { };

private:
    std::string name;
    bool        active;
    bool        visible;
};

#endif
