#ifndef VISIT_BOX_TOOL_H
#define VISIT_BOX_TOOL_H
#include <viswindow_exports.h>
#include <VisitInteractiveTool.h>
#include <avtBoxToolInterface.h>
#include <avtMatrix.h>
#include <avtQuaternion.h>
#include <avtTrackball.h>

// Forward declarations
class VisWindow;
class vtkActor;
class vtkPolyDataMapper;
class vtkPolyData;
class vtkTextActor;

// ****************************************************************************
// Class: VisitBoxTool
//
// Purpose:
//   This class contains an interactive plane tool that can be used to define
//   a slice plane.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 23 07:13:08 PDT 2002
//
// Modifications:
//   Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002
//   Replace vtkActor2d/vtkTextMapper pairs with vtkTextActor.
//
//   Kathleen Bonnell, Wed May 28 16:09:47 PDT 2003 
//   Add method ReAddToWindow.
//
//   Brad Whitlock, Tue Jul 13 14:12:33 PST 2004
//   Added new handlers for the new hotpoints.
//
// ****************************************************************************

class VISWINDOW_API VisitBoxTool : public VisitInteractiveTool
{
  public:
             VisitBoxTool(VisWindowToolProxy &);
    virtual ~VisitBoxTool();

    virtual void Enable();
    virtual void Disable();
    virtual bool IsAvailable() const;

    virtual void Start2DMode();
    virtual void Stop3DMode();
    virtual void UpdateView();

    virtual void SetForegroundColor(float, float, float);

    virtual const char *  GetName() const { return "Box"; };
    virtual avtToolInterface &GetInterface() { return Interface; };

    virtual void UpdateTool();
    virtual void ReAddToWindow();
    void SetActiveHotPoint(int v) { activeHotPoint = v; };

  protected:
    // Callback functions for the tool's hot points.
    static void TranslateCallback(VisitInteractiveTool *, CB_ENUM,
                                  int, int, int, int);
    static void ResizeCallback(VisitInteractiveTool *, CB_ENUM,
                               int, int, int, int);
    static void XMINCallback(VisitInteractiveTool *, CB_ENUM,
                             int, int, int, int);
    static void XMAXCallback(VisitInteractiveTool *, CB_ENUM,
                             int, int, int, int);
    static void YMINCallback(VisitInteractiveTool *, CB_ENUM,
                             int, int, int, int);
    static void YMAXCallback(VisitInteractiveTool *, CB_ENUM,
                             int, int, int, int);
    static void ZMINCallback(VisitInteractiveTool *, CB_ENUM,
                             int, int, int, int);
    static void ZMAXCallback(VisitInteractiveTool *, CB_ENUM,
                             int, int, int, int);

    virtual void CallCallback();
    void Translate(CB_ENUM, int, int, int, int);
    void Resize(CB_ENUM, int, int, int, int);

    void CreateBoxActor();

    void CreateTextActors();
    void DeleteTextActors();
    void AddText();
    void RemoveText();
    void UpdateText();
    void GetHotPointLabel(int index, char *str);

    void CreateOutline();
    void DeleteOutline();
    void AddOutline();
    void RemoveOutline();
    void UpdateOutline();
    void GetBoundingBoxOutline(int a, avtVector *verts, bool giveMin);

    void InitialActorSetup();
    void FinalActorSetup();

    void DoTransformations();

    vtkActor           *boxActor;
    vtkPolyDataMapper  *boxMapper;
    vtkPolyData        *boxData;

    vtkActor           *outlineActor[3];
    vtkPolyDataMapper  *outlineMapper[3];
    vtkPolyData        *outlineData[3];
    vtkTextActor       *outlineTextActor[4];

    vtkTextActor       *originTextActor;
    vtkTextActor       *labelTextActor[7];

    avtBoxToolInterface Interface;

    avtTrackball        trackball;
    HotPointVector      origHotPoints;
    avtMatrix           SMtx;
    avtMatrix           TMtx;

    bool                addedOutline;
    bool                addedBbox;
    bool                depthTranslate;
    int                 activeHotPoint;
    float               focalDepth;
    float               originalDistance;
    avtVector           depthTranslationDistance;

};

#endif
