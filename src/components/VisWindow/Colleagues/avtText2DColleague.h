// ************************************************************************* //
//                              VisWinUserInfo.h                             //
// ************************************************************************* //

#ifndef VIS_WIN_USER_INFO_H
#define VIS_WIN_USER_INFO_H
#include <viswindow_exports.h>
#include <avtAnnotationColleague.h>
#include <ColorAttribute.h>

class vtkTextActor;

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

    // Methods to set and get the annotation's properties.
    virtual void SetOptions(const AnnotationObject &annot);
    virtual void GetOptions(AnnotationObject &annot);

    // Methods that are called in response to vis window events.
    virtual void SetForegroundColor(float r, float g, float b);
    virtual void HasPlots(void);
    virtual void NoPlots(void);
    virtual void UpdatePlotList(vector<avtActor_p> &lst);
protected:
    bool ShouldBeAddedToRenderer() const;
    void SetText(const char *text);

    vtkTextActor   *textActor;
    char           *textFormatString;
    char           *textString;
    double          currentTime;

    bool            useForegroundForTextColor;
    bool            addedToRenderer;
    ColorAttribute  textColor;
};


#endif


