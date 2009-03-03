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

#ifndef AVT_TIME_SLIDER_COLLEAGUE_H
#define AVT_TIME_SLIDER_COLLEAGUE_H
#include <viswindow_exports.h>
#include <avtAnnotationColleague.h>
#include <ColorAttribute.h>

class vtkTextActor;
class vtkTimeSliderActor;

// ****************************************************************************
// Class: avtTimeSliderColleague
//
// Purpose:
//   This colleague is a time slider actor that can exist in the window and
//   indicate how far we are through the animation.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 16:27:10 PST 2003
//
// Modifications:
//   Kathleen Bonnell, Thu Jan 13 08:39:30 PST 2005
//   Added timeFormatString and another char* arg to SetText.
//
//   Brad Whitlock, Mon Mar  2 14:21:17 PST 2009
//   Added SetTimeScaleAndOffset.
//
// ****************************************************************************

class VISWINDOW_API avtTimeSliderColleague : public avtAnnotationColleague
{
public:
    avtTimeSliderColleague(VisWindowColleagueProxy &);
    virtual ~avtTimeSliderColleague();

    virtual void AddToRenderer();
    virtual void RemoveFromRenderer();
    virtual void Hide();

    virtual void SetTimeScaleAndOffset(double,double);

    virtual std::string TypeName() const { return "TimeSlider"; }

    // Methods to set and get the annotation's properties.
    virtual void SetOptions(const AnnotationObject &annot);
    virtual void GetOptions(AnnotationObject &annot);

    // Methods that are called in response to vis window events.
    virtual void SetForegroundColor(double r, double g, double b);
    virtual void HasPlots(void);
    virtual void NoPlots(void);
    virtual void SetFrameAndState(int, int, int, int, int, int, int);
    virtual void UpdatePlotList(std::vector<avtActor_p> &lst);

private:
    bool ShouldBeAddedToRenderer() const;
    void SetText(const char *text, const char *format);
    double SliderHeight(double height) const;
    void  GetTextRect(double, double, double, double, double *) const;
    void  GetSliderRect(double, double, double, double, double *) const;

    vtkTimeSliderActor *timeSlider;
    vtkTextActor       *textActor;
    char               *textFormatString;
    char               *textString;
    char               *timeFormatString;
    int                timeDisplayMode;

    double             currentTime;
    double             timeScale;
    double             timeOffset;
    bool               useForegroundForTextColor;
    bool               addedToRenderer;
    ColorAttribute     textColor;
};

#endif
