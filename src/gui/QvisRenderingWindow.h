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
class RenderingAttributes;
class WindowInformation;

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
protected:
    virtual void UpdateWindow(bool doAll);
    void UpdateOptions(bool doAll);
    void UpdateInformation(bool doAll);
    void Apply(bool ignore = false);
private slots:
    void apply();
    void antialiasingToggled(bool);
    void objectRepresentationChanged(int);
    void displayListToggled(bool);
    void stereoToggled(bool);
    void stereoTypeChanged(int);
    void renderNotifyToggled(bool);
    void scalableRenderingToggled(bool);
    void scalableThresholdChanged(int);
private:
    RenderingAttributes *renderAtts;
    WindowInformation   *windowInfo;

    // Controls
    QCheckBox    *antialiasingToggle;
    QButtonGroup *objectRepresentation;
    QCheckBox    *dislayListToggle;
    QCheckBox    *stereoToggle;
    QButtonGroup *stereoType;
    QRadioButton *redblue;
    QRadioButton *interlace;
    QRadioButton *crystalEyes;
    QCheckBox    *renderNotifyToggle;
    QCheckBox    *scalableRenderingToggle;
    QButtonGroup *scalableThreshold;
    QRadioButton *scalrenAlways;
    QRadioButton *scalrenBySize;
    QRadioButton *scalrenAuto;

    // Labels to display renderer information.
    QLabel       *fpsMinLabel;
    QLabel       *fpsAvgLabel;
    QLabel       *fpsMaxLabel;
    QLabel       *approxNumTriangles;
    QLabel       *extents[6];
};

#endif
