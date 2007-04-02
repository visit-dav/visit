#ifndef AVT_IMAGE_COLLEAGUE_H
#define AVT_IMAGE_COLLEAGUE_H

#include <string>

#include <ColorAttribute.h>
#include <avtAnnotationColleague.h>
#include <viswindow_exports.h>

class vtkActor2D;
class vtkImageData;
class vtkImageMapper;
class vtkImageReader2;
class vtkImageResample;

// ****************************************************************************
// Class: avtImageColleague
//
// Purpose:
//   This colleague is a image that can be shown in the vis window.
//
// Notes:      
//
// Programmer: John C. Anderson
// Creation:   Thu Jul 15 08:04:46 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

class VISWINDOW_API avtImageColleague : public avtAnnotationColleague
{
public:
    avtImageColleague(VisWindowColleagueProxy &);
    virtual ~avtImageColleague();

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
    void CreateActorAndMapper();
    bool UpdateImage(std::string);

    vtkActor2D                 *actor;
    vtkImageMapper             *mapper;
    vtkImageResample           *resample;

    std::string                 currentImage;
    vtkImageData               *iData;

    int                         width, height;

    bool                        useOpacityColor;
    ColorAttribute              opacityColor;

    bool                        maintainAspectRatio;

    bool                        addedToRenderer;

    bool ShouldBeAddedToRenderer() const;
};


#endif


