// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              VisWinUserInfo.h                             //
// ************************************************************************* //

#ifndef VIS_WIN_USER_INFO_H
#define VIS_WIN_USER_INFO_H
#include <viswindow_exports.h>
#include <avtAnnotationColleague.h>
#include <ColorAttribute.h>

class vtkVisItTextActor;

// ****************************************************************************
// Class: avtText2DColleague
//
// Purpose:
//   This colleague is a text label that can be shown in the vis window.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 14:09:57 PST 2003
//
// Modifications:
//    Jeremy Meredith, Wed Mar 11 12:33:20 EDT 2009
//    Added $cycle support.
//
//    Brad Whitlock, Mon Sep 19 15:44:40 PDT 2011
//    Switch to vtkVisItTextActor.
//
// ****************************************************************************

class VISWINDOW_API avtText2DColleague : public avtAnnotationColleague
{
public:
    avtText2DColleague(VisWindowColleagueProxy &);
    virtual ~avtText2DColleague();

    virtual void AddToRenderer();
    virtual void RemoveFromRenderer();
    virtual void Hide();

    virtual std::string TypeName() const { return "Text2D"; }

    // Methods to set and get the annotation's properties.
    virtual void SetOptions(const AnnotationObject &annot);
    virtual void GetOptions(AnnotationObject &annot);

    // Methods that are called in response to vis window events.
    virtual void SetForegroundColor(double r, double g, double b);
    virtual void HasPlots(void);
    virtual void NoPlots(void);
    virtual void UpdatePlotList(std::vector<avtActor_p> &lst);
protected:
    bool ShouldBeAddedToRenderer() const;
    void SetText(const char *text);

    static double initialTime;
    static int    initialCycle;

    vtkVisItTextActor *textActor;
    char           *textFormatString;
    char           *textString;
    double          currentTime;
    int             currentCycle;

    bool            useForegroundForTextColor;
    bool            addedToRenderer;
    ColorAttribute  textColor;
};


#endif


