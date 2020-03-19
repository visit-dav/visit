// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PICK_ACTOR_H
#define PICK_ACTOR_H
#include <plotter_exports.h>
#include <visitstream.h>
#include <ref_ptr.h>

class vtkActor;
class vtkActor2D;
class vtkFollower;
class vtkLineSource;
class vtkMultiLineSource;
class vtkSphereSource;
class vtkPolyDataMapper;
class vtkPolyDataMapper2D;
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
//    Brad Whitlock, Fri Aug 27 10:54:33 PDT 2010
//    I added GetPickDesignator.
//
//    Burlen Loring, Thu Oct  8 10:36:25 PDT 2015
//    fix a compliler warning
//
//    Matt Larsen, Thu June  30 08:16:11 PDT 2016
//    Adding support for zone highlights.
//    Added members for Highlight source, mapper, and actor
//    Added method 'AddLine'
//    Added methods 'GetShowPickLetter' 'SetShowPickLetter'
//    Added member variable showPickLetter
//
//    Matt Larsen, Wed September 6 09:10:01 PDT 2017
//    Changed highlights to overlay(2D) to show internal zones 
//
//    Alister Maguire, Tue Sep 26 14:23:09 PDT 2017
//    Changed AddLine to include an rgb argument. 
//
//    Alister Maguire, Mon Aug 20 11:05:30 PDT 2018
//    Added PICK_TYPE for distinction between node and zone. 
//    Added ability to highlight a node. Added showHighlight and
//    highlightColor to generalized highlight abilities. Removed
//    rgb argument from AddLine. 
//
// ****************************************************************************

class PLOTTER_API avtPickActor
{
  public:
                       avtPickActor();
    virtual           ~avtPickActor();

    enum               PICK_TYPE {
                           NODE,
                           ZONE 
                       };

    void               Add(vtkRenderer *ren);
    void               Remove();
    void               Hide();
    void               UnHide();

    void               SetAttachmentPoint(const double newPos[3]);
    void               SetAttachmentPoint(double x, double y, double z);
    const double *     GetAttachmentPoint() { return attach; };
    const double *     GetLetterPosition(void); 
    void               SetMode3D(const bool);
    bool               GetMode3D(void) const { return mode3D; };
    void               SetScale(double);
    void               SetDesignator(const std::string &l);
    std::string        GetDesignator() const;
    void               SetForegroundColor(double fgr, double fgg, double fgb);
    void               SetForegroundColor(double fg[3]);
    void               UpdateView();
    void               Shift(const double vec[3]);
    void               Translate(const double vec[3]);
    void               ResetPosition(const double vec[3]);
    void               UseGlyph(const bool v) { useGlyph = v; } ;
    void               InitializePointHighlight();
    void               AddLine(double p0[3], double p1[3]);
    bool               GetShowPickLetter() const;
    void               SetShowPickLetter(const bool);
    void               SetShowHighlight(const bool);
    bool               GetShowHighlight() const;
    void               SetHighlightColor(const float *);
    float             *GetHighlightColor();
    void               SetPickType(PICK_TYPE);
    int                GetPickType();
  protected:
    bool               mode3D;
    bool               useGlyph;
    bool               showPickLetter;
    bool               showHighlight;
    float              highlightColor[3];
    double             attach[3];
    PICK_TYPE          pType;
    std::string        designator;
    vtkFollower       *letterActor;

    vtkActor          *lineActor;
    vtkLineSource     *lineSource;
    vtkPolyDataMapper *lineMapper;

    vtkFollower       *glyphActor;
    vtkGlyphSource2D  *glyphSource;
    vtkPolyDataMapper *glyphMapper;

    vtkSphereSource      *pointHighlightSource;
    vtkPolyDataMapper2D  *pointHighlightMapper;
    vtkActor2D           *pointHighlightActor;

    vtkMultiLineSource   *lineHighlightSource;
    vtkPolyDataMapper2D  *lineHighlightMapper;
    vtkActor2D           *lineHighlightActor;
    
    vtkRenderer        *renderer;
  private:
};

typedef ref_ptr<avtPickActor> avtPickActor_p;

#endif
