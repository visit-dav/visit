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

#ifndef VISIT_EXTENTS_TOOL_H
#define VISIT_EXTENTS_TOOL_H

#include <viswindow_exports.h>
#include <VisitInteractiveTool.h>
#include <avtExtentsToolInterface.h>

#define VET_MINIMUM_NUMBER_OF_EXTENTS   2
#define VET_MAX_HORIZONTAL_TEXT_AXES    7

#define VET_HOTPOINT_RADIUS             0.015

#define VET_H_TITLE_SIZE_MIN_DIM_RATIO  0.020 // Size (not the ratio) is in pixels
#define VET_H_TITLE_MAX_CHARS           15    // Must be >= VET_V_TITLE_MAX_CHARS
#define VET_H_TITLE_Y_OFFSET           -0.085
#define VET_H_BOUND_SIZE_MIN_DIM_RATIO  0.016 // Size (not the ratio) is in pixels
#define VET_H_BOUND_MAX_CHARS           25    // Must be >= VET_V_BOUND_MAX_CHARS
#define VET_H_AXIS_MIN_Y_OFFSET        -0.065
#define VET_H_AXIS_MAX_Y_OFFSET         0.065
#define VET_H_SLIDER_MIN_Y_OFFSET      -0.045
#define VET_H_SLIDER_MAX_Y_OFFSET       0.045

#define VET_V_TITLE_SIZE_MIN_DIM_RATIO  0.020 // Size is in pixels
#define VET_V_TITLE_MAX_CHARS           12    // Must be <= VET_H_TITLE_MAX_CHARS
#define VET_V_TITLE_X_OFFSET            0.006
#define VET_V_BOUND_SIZE_MIN_DIM_RATIO  0.016 // Size (not the ratio) is in pixels
#define VET_V_BOUND_MAX_CHARS           25    // Must be <= VET_H_BOUND_MAX_CHARS
#define VET_V_AXIS_MIN_X_OFFSET        -0.01
#define VET_V_AXIS_MAX_X_OFFSET        -0.005
#define VET_V_SLIDER_MIN_X_OFFSET       0.02
#define VET_V_SLIDER_MAX_X_OFFSET       0.01
#define VET_V_AXIS_BOTTOM_MARGIN        0.018
#define VET_V_AXIS_TOP_MARGIN           0.018

/* Old pink color
#define VET_ARROW_RED                   1.0
#define VET_ARROW_GREEN                 0.5
#define VET_ARROW_BLUE                  0.75
*/

#define VET_ARROW_RED                   1.0     // Red to match
#define VET_ARROW_GREEN                 0.0     // tool hotpoint
#define VET_ARROW_BLUE                  0.0     // highlight color

#define VET_TITLE_RED                   0.0
#define VET_TITLE_GREEN                 0.0
#define VET_TITLE_BLUE                  1.0

#define VET_AXIS_BOUND_RED              0.0
#define VET_AXIS_BOUND_GREEN            0.75
#define VET_AXIS_BOUND_BLUE             0.75

#define VET_SLIDER_BOUND_RED            1.0
#define VET_SLIDER_BOUND_GREEN          0.0
#define VET_SLIDER_BOUND_BLUE           0.0

// Forward declarations
class VisWindow;
class vtkActor;
class vtkPoints;
class vtkPolyDataMapper;
class vtkPolyData;
class vtkTextActor;

// ****************************************************************************
// Class: VisitExtentsTool
//
// Purpose: This class contains an interactive tool to manipulate the values
//          of individual minima and maxima within a set of (min,max) pairs.
//          So far its use is restricted to 2-D VisIt plots.
//
// Programmer: Mark Blair
// Creation:   Mon Oct 31 18:35:00 PST 2005
//
// Modifications:
//
//    Mark Blair, Wed Sep 13 12:18:53 PDT 2006
//    Added ShowsHotPointHighlights method.  Extents tool shows no highlights.
//
//    Mark Blair, Fri Sep 15 16:41:45 PDT 2006
//    Added methods for updating minima and maxima on an individual basis.
//    Also added running time ordinal for tagging slider changes.
//
//    Mark Blair, Thu Oct 26 18:40:28 PDT 2006
//    Added UpdateToolAttributesWithPlotAttributes method.
//
//    Mark Blair, Thu Nov  2 12:33:23 PST 2006
//    Added methods and members for selective axis labeling in associated plot.
//
//    Mark Blair, Tue Nov 28 13:25:41 PST 2006
//    Added support for improved plot versus tool interaction.
//
// ****************************************************************************

class VISWINDOW_API VisitExtentsTool : public VisitInteractiveTool
{
  public:
             VisitExtentsTool(VisWindowToolProxy &);
    virtual ~VisitExtentsTool();

    virtual void Enable();
    virtual void Disable();
    virtual bool IsAvailable() const;
    virtual bool ShowsHotPointHighlights() const { return false; };

    virtual void SetForegroundColor(float, float, float);

    virtual const char       *GetName() const { return "Extents"; };
    virtual avtToolInterface &GetInterface() { return Interface; };

    virtual void UpdateTool();

    virtual void ReAddToWindow();
    virtual void FullFrameOn(const double, const int);
    virtual void FullFrameOff(void);

  protected:
    static void SliderMinimumCallback(VisitInteractiveTool *it, CB_ENUM e,
                                      int crtl, int shift, int x, int y);
    static void SliderMaximumCallback(VisitInteractiveTool *it, CB_ENUM e,
                                      int crtl, int shift, int x, int y);

    void SliderMinimum(CB_ENUM e, int crtl, int shift, int x, int y);
    void SliderMaximum(CB_ENUM e, int crtl, int shift, int x, int y);
    void CallMinCallback();
    void CallMaxCallback();

    void GetCurrentPlotAttributes();
    void UpdateToolAttributesWithPlotAttributes();
    void InitializeHotPoints();
    void CreateSliderActor();
    void CreateTextActors();
    void CreateAxisTitleActors();
    void CreateAxisBoundActors();
    void CreateSliderBoundActors();
    void DeleteTextActors();

    void AddText();
    void RemoveText();
    void UpdateText();

    void MoveSliderMinimumArrow(int hotPointID);
    void MoveSliderMaximumArrow(int hotPointID);
    
    void MakeAxisTitleText(
        char titleText[], const std::string &axisTitle, int maxTitleChars);
    void MakeDataBoundText(char boundText[], double boundValue);
    
    vtkActor            *sliderActor;
    vtkPolyDataMapper   *sliderMapper;
    vtkPolyData         *sliderData;

    vtkPoints           *arrowPoints;
    
    std::vector<vtkTextActor *> axisTitleActors;
    std::vector<vtkTextActor *> axisMinActors;
    std::vector<vtkTextActor *> axisMaxActors;
    std::vector<vtkTextActor *> sliderMinActors;
    std::vector<vtkTextActor *> sliderMaxActors;

    avtExtentsToolInterface Interface;
    
    int                  plotListIndex;
    
    int                  curTimeOrdinal;
    int                  toolExtentCount;

    double               hotPointRadius;
    double               normToWorldYScale;
    double               sliderXStride;
        
    double               minSlidableX;
    double               maxSlidableX;
    double               minSlidableY;
    double               maxSlidableY;

    int                  axisTitleTextSize;
    double               axisTitleY;
    double               axisTitleDispXOffset;
    
    int                  boundTextSize;

    double               axisMinY;
    double               axisMaxY;
    double               axisMinDispXOffset;
    double               axisMaxDispXOffset;

    double               sliderMinY;
    double               sliderMaxY;
    double               sliderMinDispXOffset;
    double               sliderMaxDispXOffset;

    int                  activeAxisIndex;
    int                  activeHotPointID;
    double               activeHPMinY;
    double               activeHPMaxY;
};

#endif
