#ifndef QVIS_RENDERING_WINDOW_H
#define QVIS_RENDERING_WINDOW_H
#include <QvisPostableWindowSimpleObserver.h>
#include <gui_exports.h>

// Forward declarations
class QButtonGroup;
class QCheckBox;
class QLabel;
class QRadioButton;
class QSlider;
class QSpinBox;
class RenderingAttributes;
class WindowInformation;
class QvisOpacitySlider;

// ****************************************************************************
// Class: QvisRenderingWindow
//
// Purpose:
//   This class implements a window that displays rendering settings from the
//   viewer and also allows some settings to be changed.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 18 14:33:24 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu Oct 24 13:33:31 PST 2002
//   I made the stereo radio buttons class members.
//
//   Kathleen Bonnell, Wed Dec  4 18:42:48 PST 2002  
//   Removed quality slider, no longer needed. 
//
//   Jeremy Meredith, Fri Nov 14 16:03:31 PST 2003
//   Added specular lighting.
//
//   Mark C. Miller, Tue Apr 27 14:41:35 PDT 2004
//   Added stuff to deal with adjusting scalable threshold with a spinbox 
//
//   Hank Childs, Sun May  9 15:54:29 PDT 2004
//   Add support for multiple display list modes.
//
//   Hank Childs, Sun Oct 24 07:34:09 PDT 2004
//   Add shadows.
//
//   Mark C. Miller, Fri Mar  4 13:05:02 PST 2005
//   Changed approxNumTriangles to approxNumPrimitives
//
// ****************************************************************************

class GUI_API QvisRenderingWindow : public QvisPostableWindowSimpleObserver
{
    Q_OBJECT
public:
    QvisRenderingWindow(const char *caption = 0,
                        const char *shortName = 0,
                        QvisNotepadArea *n = 0);
    virtual ~QvisRenderingWindow();
    virtual void CreateWindowContents();
    virtual void SubjectRemoved(Subject *TheRemovedSubject);

    void ConnectRenderingAttributes(RenderingAttributes *);
    void ConnectWindowInformation(WindowInformation *);
protected slots:
    virtual void apply();
protected:
    virtual void UpdateWindow(bool doAll);
    void UpdateOptions(bool doAll);
    void UpdateInformation(bool doAll);
    void Apply(bool ignore = false);
    void InterpretScalableAutoThreshold(int,int*,QString*,int*) const;
private slots:
    void antialiasingToggled(bool);
    void objectRepresentationChanged(int);
    void displayListModeChanged(int);
    void stereoToggled(bool);
    void stereoTypeChanged(int);
    void renderNotifyToggled(bool);
    void scalrenActivationModeChanged(int);
    void scalrenAutoThresholdChanged(int val);
    void specularToggled(bool);
    void specularStrengthChanged(int, const void*);
    void specularPowerChanged(int, const void*);
    void shadowToggled(bool);
    void shadowStrengthChanged(int, const void*);
private:
    RenderingAttributes *renderAtts;
    WindowInformation   *windowInfo;

    // Controls
    QCheckBox    *antialiasingToggle;
    QButtonGroup *objectRepresentation;
    QButtonGroup *dlMode;
    QCheckBox    *stereoToggle;
    QButtonGroup *stereoType;
    QRadioButton *redblue;
    QRadioButton *interlace;
    QRadioButton *crystalEyes;
    QCheckBox    *renderNotifyToggle;
    QButtonGroup *scalrenActivationMode;
    QRadioButton *scalrenAuto;
    QRadioButton *scalrenAlways;
    QRadioButton *scalrenNever;
    QLabel       *scalrenGeometryLabel;
    QSpinBox     *scalrenAutoThreshold;
    QCheckBox         *specularToggle;
    QLabel            *specularStrengthLabel;
    QvisOpacitySlider *specularStrengthSlider;
    QLabel            *specularPowerLabel;
    QvisOpacitySlider *specularPowerSlider;
    QCheckBox         *shadowToggle;
    QLabel            *shadowStrengthLabel;
    QvisOpacitySlider *shadowStrengthSlider;

    // Labels to display renderer information.
    QLabel       *scalrenUsingLabel;
    QLabel       *fpsMinLabel;
    QLabel       *fpsAvgLabel;
    QLabel       *fpsMaxLabel;
    QLabel       *approxNumPrimitives;
    QLabel       *extents[6];
};

#endif
