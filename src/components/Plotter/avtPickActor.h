#ifndef PICK_ACTOR_H
#define PICK_ACTOR_H
#include <plotter_exports.h>
#include <visitstream.h>
#include <ref_ptr.h>

class vtkActor;
class vtkFollower;
class vtkLineSource;
class vtkPolyDataMapper;
class vtkRenderer;
class vtkGlyphSource2D;


// ****************************************************************************
//  Class:  avtPickActor
//
//  Purpose:  Responsible for creating the visual cue actors for a Pick. 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 22, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Wed May  8 13:50:44 PDT 2002
//    Moved ComputeScaleFactor to VisWinRendering.
//
//    Kathleen Bonnell, Fri Jun  6 15:08:45 PDT 2003  
//    Added Translate and ResetPosition methods. 
//
//    Kathleen Bonnell, Wed Jun 25 15:16:42 PDT 2003
//    Changed arguments to SetAttachmentPoint from an array to 3 values.
//    Added a glyph for NodePick.
//
//    Kathleen Bonnell, Tue Jun  8 17:42:59 PDT 2004 
//    Added 'GetLetterPosition'. 
//
//    Kathleen Bonnell, Wed Aug 18 09:59:02 PDT 2004 
//    Added 'GetMode3D'. 
//
// ****************************************************************************

class PLOTTER_API avtPickActor
{
  public:
                       avtPickActor();
    virtual           ~avtPickActor();

    void               Add(vtkRenderer *ren);
    void               Remove();
    void               Hide();
    void               UnHide();

    void               SetAttachmentPoint(const float newPos[3]);
    void               SetAttachmentPoint(float x, float y, float z);
    const float *      GetAttachmentPoint() { return attach; };
    const float *      GetLetterPosition(void); 
    void               SetMode3D(const bool);
    const bool         GetMode3D(void) const { return mode3D; };
    void               SetScale(float);
    void               SetDesignator(const char *l);
    void               SetForegroundColor(float fgr, float fgg, float fgb);
    void               SetForegroundColor(float fg[3]);
    void               UpdateView();
    void               Shift(const float vec[3]);
    void               Translate(const float vec[3]);
    void               ResetPosition(const float vec[3]);
    void               UseGlyph(const bool v) { useGlyph = v; } ;

  protected:
    bool               mode3D;
    bool               useGlyph;
    float              attach[3];
    vtkFollower       *letterActor;

    vtkActor          *lineActor;
    vtkLineSource     *lineSource;
    vtkPolyDataMapper *lineMapper;

    vtkFollower       *glyphActor;
    vtkGlyphSource2D  *glyphSource;
    vtkPolyDataMapper *glyphMapper;

    vtkRenderer       *renderer; 

  private:
};

typedef ref_ptr<avtPickActor> avtPickActor_p;

#endif
