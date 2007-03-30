#ifndef VISIT_LINE_TOOL_H
#define VISIT_LINE_TOOL_H
#include <viswindow_exports.h>
#include <VisitInteractiveTool.h>
#include <avtLineToolInterface.h>
#include <avtMatrix.h>
#include <avtQuaternion.h>
#include <avtTrackball.h>

// Forward declarations
class VisWindow;
class vtkActor;
class vtkLineSource;
class vtkPolyDataMapper;
class vtkPolyData;
class vtkTextActor;

// ****************************************************************************
// Class: VisitLineTool
//
// Purpose:
//   This class contains an interactive line tool that can be used to define
//   a lineout curve.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 18 15:23:54 PST 2002
//
// Modifications:
//   Brad Whitlock, Tue Jul 23 10:23:18 PDT 2002
//   I removed the NoPlots redeclaration since it now uses the base class's
//   method implementation.
//
//   Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002
//   Replace vtkActor2d/vtkTextMapper pairs with vtkTextActor.
//
//   Kathleen Bonnell, Wed May 28 16:14:22 PDT 2003 
//   Add method ReAddToWindow.
//
//   Kathleen Bonnell, Fri Jun  6 15:36:24 PDT 2003 
//   Add FullFrameOn/Off methods. 
//
// ****************************************************************************

class VISWINDOW_API VisitLineTool : public VisitInteractiveTool
{
  public:
             VisitLineTool(VisWindowToolProxy &);
    virtual ~VisitLineTool();

    virtual void Enable();
    virtual void Disable();
    virtual bool IsAvailable() const;

    virtual void Start2DMode();
    virtual void Start3DMode();
    virtual void Stop3DMode();

    virtual void SetForegroundColor(float, float, float);

    virtual const char *  GetName() const { return "Line"; };
    virtual avtToolInterface &GetInterface() { return Interface; };

    virtual void UpdateView();
    virtual void UpdateTool();
    virtual void ReAddToWindow();

    virtual void FullFrameOn(const double, const int);
    virtual void FullFrameOff(void);

  protected:
    // Callback functions for the tool's hot points.
    static void TranslatePoint1Callback(VisitInteractiveTool *, CB_ENUM,
                                        int, int, int, int);
    static void TranslatePoint2Callback(VisitInteractiveTool *, CB_ENUM,
                                        int, int, int, int);
    static void TranslateCallback(VisitInteractiveTool *, CB_ENUM,
                                  int, int, int, int);

    virtual void CallCallback();
    void Translate(CB_ENUM, int, int, int, int, int);
    void InitializePoints();

    void CreateLineActor();
    void UpdateLine();

    void CreateGuide();
    void DeleteGuide();
    void AddGuide(int);
    void RemoveGuide();
    void UpdateGuide(int);
    void GetGuidePoints(int pi, avtVector *pts);

    void CreateTextActors();
    void DeleteTextActors();
    void AddText();
    void RemoveText();
    void UpdateText();

    void InitialActorSetup(int);
    void FinalActorSetup();

    float                focalDepth;
    float                translationDistance;
    avtVector            depthTranslationDistance;
    vtkLineSource       *lineSource;
    vtkActor            *lineActor;
    vtkPolyDataMapper   *lineMapper;
    vtkPolyData         *lineData;
    vtkActor            *guideActor;
    vtkPolyDataMapper   *guideMapper;
    vtkPolyData         *guideData;
    vtkTextActor        *pointTextActor[2];

    avtLineToolInterface Interface;
    bool                 addedBbox;
    bool                 window3D;
    bool                 addedGuide;
    bool                 enlongating;
    bool                 depthTranslate;
};

#endif
