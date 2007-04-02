#ifndef VISIT_EXTENTS_TOOL_H
#define VISIT_EXTENTS_TOOL_H

#include <viswindow_exports.h>
#include <VisitInteractiveTool.h>
#include <avtExtentsToolInterface.h>
#include <avtMatrix.h>
#include <avtQuaternion.h>
#include <avtTrackball.h>

#define VET_MINIMUM_NUMBER_OF_EXTENTS   2

#define VET_HOTPOINT_RADIUS_FRACTION    0.015

/* Old pink color
#define VET_ARROW_RED                   1.0
#define VET_ARROW_GREEN                 0.5
#define VET_ARROW_BLUE                  0.75
*/

#define VET_ARROW_RED                   1.0     // Red to match
#define VET_ARROW_GREEN                 0.0     // tool hotpoint
#define VET_ARROW_BLUE                  0.0     // highlight color

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

    void InitializeHotPoints();
    void UpdateToolAttributesWithPlotAttributes();
    void CreateSliderActor();
    void CreateTextActors();
    void DeleteTextActors();

    void AddText();
    void RemoveText();
    void UpdateText();

    void MoveSliderMinimumArrow(int hotPointID);
    void MoveSliderMaximumArrow(int hotPointID);
    
    vtkActor            *sliderActor;
    vtkPolyDataMapper   *sliderMapper;
    vtkPolyData         *sliderData;

    vtkPoints           *arrowPoints;

    avtExtentsToolInterface Interface;
    
    int                  plotListIndex;
    
    int                  curTimeOrdinal;

    double               hotPointRadius;

    double               minSlidableX;
    double               maxSlidableX;
    double               minSlidableY;
    double               maxSlidableY;

    double               sliderXStride;
    double               normToWorldYScale;

    int                  activeAxisIndex;
    int                  activeHotPointID;
    double               activeHPMinY;
    double               activeHPMaxY;
};

#endif
