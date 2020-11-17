// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_VIEW_WINDOW_H
#define QVIS_VIEW_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowSimpleObserver.h>

// Forward declarations.
class DataNode;
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class QSlider;
class QTabWidget;
class QVBox;
class ViewAxisArrayAttributes;
class ViewCurveAttributes;
class View2DAttributes;
class View3DAttributes;
class WindowInformation;
class QPushButton;

// ****************************************************************************
// Class: QvisViewWindow
//
// Purpose:
//   This class implements the VisIt view window.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 26 16:03:25 PST 2001
//
// Modifications:
//   Eric Brugger, Tue Aug 21 13:45:36 PDT 2001
//   I redesigned the window fairly extensively.
//
//   Brad Whitlock, Mon Aug 27 11:35:11 PDT 2001
//   I made the window postable and added some default view buttons for 3d.
//
//   Eric Brugger, Tue Aug  6 10:30:24 PDT 2002
//   I added a view command processor.
//
//   Brad Whitlock, Tue Sep 17 12:58:39 PDT 2002
//   I reorganized the window.
//
//   Eric Brugger, Mon Jan 13 14:50:38 PST 2003
//   Add the vp and wp commands to the cli.
//
//   Jeremy Meredith, Tue Feb  4 17:44:53 PST 2003
//   Added controls for the camera and view keyframes.
//
//   Eric Brugger, Fri Apr 18 11:45:10 PDT 2003
//   Removed auto center view.
//
//   Eric Brugger, Tue Jun 10 12:48:03 PDT 2003
//   I added image pan and image zoom fields to the 3d view.
//
//   Eric Brugger, Wed Aug 20 14:02:49 PDT 2003
//   I added support for curve views and split the view attributes into
//   2d and 3d parts.
//
//   Brad Whitlock, Thu Sep 11 09:33:08 PDT 2003
//   I added slots to reset or recenter the view.
//
//   Eric Brugger, Thu Oct 16 12:21:13 PDT 2003
//   I added full frame mode to the 2D view tab.
//
//   Eric Brugger, Tue Feb 10 10:29:21 PST 2004
//   I added center of rotation controls to the advanced tab.
//
//   Mark C. Miller, Thu Jul 21 12:52:42 PDT 2005
//   Added stuff for auto full frame mode
//
//   Kathleen Bonnell, Thu Mar 22 16:07:56 PDT 2007 
//   Added radio buttons for log scaling.
//
//   Kathleen Bonnell, Wed May  9 11:15:13 PDT 2007 
//   Added radio buttons for 2d log scaling.
//
//   Jeremy Meredith, Mon Feb  4 13:44:33 EST 2008
//   Added support for axis-array views.  Renamed some curve view
//   buttons to avoid namespace collisions.
//
//   Brad Whitlock, Wed Apr  9 11:02:38 PDT 2008
//   QString for caption, shortName.
//
//   Cyrus Harrison, Thu Dec 18 09:36:57 PST 2008
//   Changed argument for tabSelected slot to an integer for Qt4.
//
//   Jeremy Meredith, Wed Feb  3 15:29:17 EST 2010
//   Added maintain view here.  (moved from main window)
//
//   Jeremy Meredith, Wed May 19 14:15:58 EDT 2010
//   Account for 3D axis scaling (3D equivalent of full-frame mode).
//
//   Jeremy Meredith, Mon Aug  2 13:55:42 EDT 2010
//   Added shear support for oblique projections.
//
// ****************************************************************************

class GUI_API QvisViewWindow : public QvisPostableWindowSimpleObserver
{
    Q_OBJECT
public:
    QvisViewWindow(const QString &caption = QString(),
                   const QString &shortName = QString(),
                   QvisNotepadArea *notepad = 0);
    virtual ~QvisViewWindow();
    virtual void CreateWindowContents();
    void SubjectRemoved(Subject *TheRemovedSubject);

    void ConnectAxisArrayAttributes(ViewAxisArrayAttributes *v);
    void ConnectCurveAttributes(ViewCurveAttributes *v);
    void Connect2DAttributes(View2DAttributes *v);
    void Connect3DAttributes(View3DAttributes *v);
    void ConnectWindowInformation(WindowInformation *);

    virtual void CreateNode(DataNode *parentNode);
    virtual void SetFromNode(DataNode *parentNode, const int *borders);
public slots:
    virtual void apply();
    virtual void show();
protected:
    void Apply(bool ignore = false);
    void GetCurrentValues(int which_widget);
    void GetCurrentValuesAxisArray(int which_widget);
    void GetCurrentValuesCurve(int which_widget);
    void GetCurrentValues2d(int which_widget);
    void GetCurrentValues3d(int which_widget);

    virtual void UpdateWindow(bool doAll);
    void UpdateAxisArray(bool doAll);
    void UpdateCurve(bool doAll);
    void Update2D(bool doAll);
    void Update3D(bool doAll);
    void UpdateGlobal(bool doAll);
private slots:
    void processCommandText();

    void processViewportAxisArrayText();
    void processDomainAxisArrayText();
    void processRangeAxisArrayText();

    void processViewportCurveText();
    void processDomainText();
    void processRangeText();
    void domainScaleModeChanged(int);
    void rangeScaleModeChanged(int);

    void processViewportText();
    void processWindowText();
    void fullFrameActivationModeChanged(int);
    void xScaleModeChanged(int);
    void yScaleModeChanged(int);

    void processNormalText();
    void processFocusText();
    void processUpVectorText();
    void processViewAngleText();
    void processParallelScaleText();
    void processNearText();
    void processFarText();
    void processImagePanText();
    void processImageZoomText();
    void perspectiveToggled(bool val);
    void viewButtonClicked(int index);
    void axis3DScaleFlagToggled(bool val);
    void processAxis3DScalesText();
    void processShearText();

    void processEyeAngleText();
    void eyeAngleSliderChanged(int val);
    void copyViewFromCameraChecked(bool);
    void makeViewKeyframe();
    void centerChecked(bool);
    void processCenterText();

    void lockedViewChecked(bool);
    void maintainViewChecked(bool);
    void extentTypeChanged(int);
    void resetView();
    void recenterView();
    void undoView();
    void tabSelected(int);
private:
    void ParseViewCommands(const char *str);
    void Pan(double panx, double pany);
    void RotateAxis(int axis, double angle);
    void Zoom(double zoom);
    void Viewport(const double *viewport);
    void Window(const double *window);
    void UpdateEyeAngleSliderFromAtts(void);

    ViewAxisArrayAttributes *viewAxisArray;
    ViewCurveAttributes *viewCurve;
    View2DAttributes    *view2d;
    View3DAttributes    *view3d;
    WindowInformation   *windowInfo;
    int                 activeTab;
    bool                activeTabSetBySlot;

    // Curve widgets
    QWidget      *pageCurve;
    QLineEdit    *viewportCurveLineEdit;
    QLineEdit    *domainCurveLineEdit;
    QLineEdit    *rangeCurveLineEdit;
    QLabel       *domainScaleLabel;
    QButtonGroup *domainScaleMode;
    QRadioButton *domainLinear;
    QRadioButton *domainLog;
    QLabel       *rangeScaleLabel;
    QButtonGroup *rangeScaleMode;
    QRadioButton *rangeLinear;
    QRadioButton *rangeLog;

    // 2d widgets
    QWidget      *page2D;
    QLineEdit    *viewportLineEdit;
    QLineEdit    *windowLineEdit;
    QLabel       *fullFrameLabel;
    QButtonGroup *fullFrameActivationMode;
    QRadioButton *fullFrameAuto;
    QRadioButton *fullFrameOn;
    QRadioButton *fullFrameOff;
    QLabel       *xScaleLabel;
    QButtonGroup *xScaleMode;
    QRadioButton *xLinear;
    QRadioButton *xLog;
    QLabel       *yScaleLabel;
    QButtonGroup *yScaleMode;
    QRadioButton *yLinear;
    QRadioButton *yLog;

    // 3d widgets
    QWidget     *page3D;
    QLineEdit   *normalLineEdit;
    QLineEdit   *focusLineEdit;
    QLineEdit   *upvectorLineEdit;
    QLineEdit   *viewAngleLineEdit;
    QLineEdit   *parallelScaleLineEdit;
    QLineEdit   *nearLineEdit;
    QLineEdit   *farLineEdit;
    QLineEdit   *imagePanLineEdit;
    QLineEdit   *imageZoomLineEdit;
    QLineEdit   *eyeAngleLineEdit;
    QSlider     *eyeAngleSlider;
    QCheckBox   *perspectiveToggle;
    QComboBox   *alignComboBox;
    QCheckBox   *axis3DScaleFlagToggle;
    QLineEdit   *axis3DScalesLineEdit;
    QLineEdit   *shearLineEdit;

    // AxisArray widgets
    QWidget      *pageAxisArray;
    QLineEdit    *viewportAxisArrayLineEdit;
    QLineEdit    *domainAxisArrayLineEdit;
    QLineEdit    *rangeAxisArrayLineEdit;

    // Global and advanced option widgets
    QTabWidget  *tabs;
    QLineEdit   *commandLineEdit;
    QComboBox   *extentComboBox;
    QCheckBox   *lockedViewToggle;
    QCheckBox   *maintainViewToggle;
    QCheckBox   *copyViewFromCameraToggle;
    QPushButton *makeViewKeyframeButton;
    QWidget     *pageAdvanced;
    QCheckBox   *centerToggle;
    QLineEdit   *centerLineEdit;
};

#endif
