// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtText3DColleague.h                         //
// ************************************************************************* //

#ifndef VIS_WIN_TEXT3D_COLLEAGUE_H
#define VIS_WIN_TEXT3D_COLLEAGUE_H
#include <viswindow_exports.h>
#include <avtAnnotationWithTextColleague.h>
#include <ColorAttribute.h>

class vtkFollower;
class vtkLinearExtrusionFilter;
class vtkPolyDataMapper;
class vtkPolyDataNormals;
class vtkVectorText;

// ****************************************************************************
// Class: avtText3DColleague
//
// Purpose:
//   This colleague is a 3D text label that can be shown in the vis window.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 7 14:09:57 PST 2007
//
// Modifications:
//    Jeremy Meredith, Wed Mar 11 12:33:20 EDT 2009
//    Added $cycle support.
//   
// ****************************************************************************

class VISWINDOW_API avtText3DColleague : public avtAnnotationWithTextColleague
{
public:
    avtText3DColleague(VisWindowColleagueProxy &);
    virtual ~avtText3DColleague();

    virtual void AddToRenderer();
    virtual void RemoveFromRenderer();
    virtual void Hide();

    virtual std::string TypeName() const { return "Text3D"; }

    // Methods to set and get the annotation's properties.
    virtual void SetOptions(const AnnotationObject &annot);
    virtual void GetOptions(AnnotationObject &annot);

    // Methods that are called in response to vis window events.
    virtual void SetForegroundColor(double r, double g, double b);
    virtual void HasPlots(void);
    virtual void NoPlots(void);
    virtual void UpdatePlotList(std::vector<avtActor_p> &lst);

    virtual void SetVisibility(int);

protected:
    bool ShouldBeAddedToRenderer() const;
    void SetText(const char *text);
    void UpdateActorScale();

    // Make a heap-allocated structure to prevent weird errors on MacOS X/gcc 4.0.1
    struct Text3DInformation
    {
        ColorAttribute  textColor;
        bool            positionInitialized;
        bool            scaleInitialized;

        bool            useForegroundForTextColor;
        bool            useRelativeHeight;
        int             relativeHeight;
        double          fixedHeight;
        double          rotations[3];
        bool            addedToRenderer;

        vtkVectorText            *textSource;
        vtkLinearExtrusionFilter *extrude;
        vtkPolyDataNormals       *normals;
        vtkPolyDataMapper        *mapper;
        vtkFollower              *textActor;
    };

    Text3DInformation *info;
};


#endif


