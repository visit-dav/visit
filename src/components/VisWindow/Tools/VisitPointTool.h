#ifndef VISIT_POINT_TOOL_H
#define VISIT_POINT_TOOL_H
#include <viswindow_exports.h>
#include <VisitInteractiveTool.h>
#include <avtPointToolInterface.h>

// Forward declarations
class VisWindow;
class vtkActor;
class vtkPolyDataMapper;
class vtkPolyData;
class vtkTextActor;

// ****************************************************************************
// Class: VisitPointTool
//
// Purpose:
//   This class contains an interactive point tool.
//
// Notes:      
//
// Programmer: Akira Haddox
// Creation:   Mon Jun  9 09:20:16 PDT 2003
//
// Modifications:
//   Akira Haddox, Wed Jul  2 14:53:50 PDT 2003
//   Added translation along an axis.
//
//   Akira Haddox, Mon Aug  4 12:48:02 PDT 2003
//   Removed unneeded point actor.
//
// ****************************************************************************

class VISWINDOW_API VisitPointTool : public VisitInteractiveTool
{
  public:
             VisitPointTool(VisWindowToolProxy &);
    virtual ~VisitPointTool();

    virtual void Enable();
    virtual void Disable();
    virtual bool IsAvailable() const;

    virtual void Start2DMode();
    virtual void Start3DMode();
    virtual void Stop3DMode();
    
    virtual void SetForegroundColor(float, float, float);
    
    virtual const char *  GetName() const { return "Point"; };
    virtual avtToolInterface &GetInterface() { return Interface; };

    virtual void UpdateView();
    virtual void UpdateTool();

  protected:
    // Callback functions for the tool's hot points.
    static void TranslateCallback(VisitInteractiveTool *, CB_ENUM,
                                  int, int, int, int);

    virtual void CallCallback();
    void Translate(CB_ENUM, int, int, int, int, int);

    avtVector ComputeTranslationDistance(int);

    void CreateGuide();
    void DeleteGuide();
    void AddGuide();
    void RemoveGuide();
    void UpdateGuide();
    void GetGuidePoints(avtVector *pts);

    void CreateTextActors();
    void DeleteTextActors();
    void AddText();
    void RemoveText();
    void UpdateText();

    void InitialActorSetup();
    void FinalActorSetup();

    float                focalDepth;
    avtVector            translationDistance;
    vtkActor            *guideActor;
    vtkPolyDataMapper   *guideMapper;
    vtkPolyData         *guideData;
    vtkTextActor        *pointTextActor;

    avtPointToolInterface Interface;
    bool                 addedBbox;
    bool                 window3D;
    bool                 addedGuide;
    
    enum TranslateDirection
    {
        none = 0,
        leftAndRight,
        upAndDown,
        inAndOut
    };

    TranslateDirection   axisTranslate;
};

#endif
