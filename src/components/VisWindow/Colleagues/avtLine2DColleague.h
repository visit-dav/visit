#ifndef AVT_LINE2D_COLLEAGUE_H
#define AVT_LINE2D_COLLEAGUE_H

#include <ColorAttribute.h>
#include <avtAnnotationColleague.h>
#include <viswindow_exports.h>

class vtkActor2D;
class vtkAppendPolyData;
class vtkPoints;
class vtkPolyData;
class vtkPolyDataMapper2D;

// ****************************************************************************
// Class: avtLine2DColleague
//
// Purpose:
//   This colleague is a line that can be shown in the vis window.
//
// Notes:      
//
// Programmer: John C. Anderson
// Creation:   Mon Jul 12 15:48:58 PDT 2004
//
// Modifications:
//   Brad Whitlock, Tue Jun 28 15:39:27 PST 2005
//   Changed so it does not use a cone source.
//
// ****************************************************************************

class VISWINDOW_API avtLine2DColleague : public avtAnnotationColleague
{
public:
    avtLine2DColleague(VisWindowColleagueProxy &);
    virtual ~avtLine2DColleague();

    virtual void AddToRenderer();
    virtual void RemoveFromRenderer();
    virtual void Hide();

    // Methods to set and get the annotation's properties.
    virtual void SetOptions(const AnnotationObject &annot);
    virtual void GetOptions(AnnotationObject &annot);

    // Methods that are called in response to vis window events.
    virtual void HasPlots(void);
    virtual void NoPlots(void);

protected:
    vtkActor2D          *actor;
    vtkPolyDataMapper2D *mapper;
    vtkAppendPolyData   *allData;
    vtkPolyData         *lineData;
    vtkPolyData         *beginArrowSolid;
    vtkPolyData         *endArrowSolid;
    vtkPolyData         *beginArrowLine;
    vtkPolyData         *endArrowLine;

    int                  beginArrowStyle;
    int                  endArrowStyle;

    bool                 addedToRenderer;

    bool ShouldBeAddedToRenderer() const;
    
    void makeArrows(vtkPolyData *, vtkPolyData *, bool);
    void updateArrows(vtkPolyData *, vtkPolyData *, double *, double *);
};


#endif


