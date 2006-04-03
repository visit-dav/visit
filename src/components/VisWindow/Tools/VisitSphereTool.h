#ifndef VISIT_SPHERE_TOOL_H
#define VISIT_SPHERE_TOOL_H
#include <viswindow_exports.h>
#include <VisitInteractiveTool.h>
#include <avtSphereToolInterface.h>
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
// Class: VisitSphereTool
//
// Purpose:
//   This class contains an interactive sphere tool that can be used to define
//   a slicing sphere.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu May 2 16:52:57 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002
//   Replace vtkActor2d/vtkTextMapper pairs with vtkTextActor.
//
//   Kathleen Bonnell, Wed May 28 16:14:22 PDT 2003 
//   Added method ReAddToWindow.
//
// ****************************************************************************

class VISWINDOW_API VisitSphereTool : public VisitInteractiveTool
{
  public:
             VisitSphereTool(VisWindowToolProxy &);
    virtual ~VisitSphereTool();

    virtual void Enable();
    virtual void Disable();
    virtual bool IsAvailable() const;

    virtual void Start2DMode();
    virtual void Stop3DMode();

    virtual void SetForegroundColor(double, double, double);

    virtual const char *  GetName() const { return "Sphere"; };
    virtual avtToolInterface &GetInterface() { return Interface; };

    virtual void UpdateView();
    virtual void UpdateTool();
    virtual void ReAddToWindow();

  protected:
    // Callback functions for the tool's hot points.
    static void TranslateCallback(VisitInteractiveTool *, CB_ENUM,
                                  int, int, int, int);
    static void ResizeCallback1(VisitInteractiveTool *, CB_ENUM,
                                int, int, int, int);
    static void ResizeCallback2(VisitInteractiveTool *, CB_ENUM,
                                int, int, int, int);
    static void ResizeCallback3(VisitInteractiveTool *, CB_ENUM,
                                int, int, int, int);
    static int activeResizeHotpoint;

    virtual void CallCallback();
    void Translate(CB_ENUM, int, int, int, int);
    void Resize(CB_ENUM, int, int, int, int);

    void CreateSphereActor();
    void CreateTextActors();
    void DeleteTextActors();
    void AddText();
    void RemoveText();
    void UpdateText();

    void InitialActorSetup();
    void FinalActorSetup();

    void DoTransformations();

    double              focalDepth;
    double              originalScale;
    double              originalDistance;
    double             translationDistance;
    bool               normalAway;
    bool               disableWhenNoPlots;
    vtkActor          *sphereActor;
    vtkPolyDataMapper *sphereMapper;
    vtkPolyData       *sphereData;
    vtkTextActor      *originTextActor;
    vtkTextActor      *radiusTextActor[3];

    avtSphereToolInterface Interface;

    HotPointVector     origHotPoints;
    avtMatrix          TMtx;
    avtMatrix          SMtx;

    bool               addedOutline;
    bool               addedBbox;
};

#endif
