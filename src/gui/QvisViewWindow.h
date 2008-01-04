/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#ifndef QVIS_VIEW_WINDOW_H
#define QVIS_VIEW_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowSimpleObserver.h>

// Forward declarations.
class DataNode;
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class QSlider;
class QTabWidget;
class QVBox;
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
// ****************************************************************************

class GUI_API QvisViewWindow : public QvisPostableWindowSimpleObserver
{
    Q_OBJECT
public:
    QvisViewWindow(const char *caption = 0, const char *shortName = 0,
                   QvisNotepadArea *notepad = 0);
    virtual ~QvisViewWindow();
    virtual void CreateWindowContents();
    void SubjectRemoved(Subject *TheRemovedSubject);

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
    void GetCurrentValuesCurve(int which_widget);
    void GetCurrentValues2d(int which_widget);
    void GetCurrentValues3d(int which_widget);

    virtual void UpdateWindow(bool doAll);
    void UpdateCurve(bool doAll);
    void Update2D(bool doAll);
    void Update3D(bool doAll);
    void UpdateGlobal(bool doAll);
private slots:
    void processCommandText();

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

    void processEyeAngleText();
    void eyeAngleSliderChanged(int val);
    void copyViewFromCameraChecked(bool);
    void makeViewKeyframe();
    void centerChecked(bool);
    void processCenterText();

    void lockedViewChecked(bool);
    void extentTypeChanged(int);
    void resetView();
    void recenterView();
    void undoView();
    void tabSelected(const QString &);
private:
    void ParseViewCommands(const char *str);
    void Pan(double panx, double pany);
    void RotateAxis(int axis, double angle);
    void Zoom(double zoom);
    void Viewport(const double *viewport);
    void Window(const double *window);
    void UpdateEyeAngleSliderFromAtts(void);

    ViewCurveAttributes *viewCurve;
    View2DAttributes    *view2d;
    View3DAttributes    *view3d;
    WindowInformation   *windowInfo;
    int                 activeTab;
    bool                activeTabSetBySlot;

    // Curve widgets
    QVBox        *pageCurve;
    QGroupBox    *viewCurveGroup;
    QLineEdit    *viewportCurveLineEdit;
    QLineEdit    *domainLineEdit;
    QLineEdit    *rangeLineEdit;
    QLabel       *domainScaleLabel;
    QButtonGroup *domainScaleMode;
    QRadioButton *domainLinear;
    QRadioButton *domainLog;
    QLabel       *rangeScaleLabel;
    QButtonGroup *rangeScaleMode;
    QRadioButton *rangeLinear;
    QRadioButton *rangeLog;

    // 2d widgets
    QVBox        *page2D;
    QGroupBox    *view2DGroup;
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
    QVBox       *page3D;
    QGroupBox   *view3DGroup;
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

    // Global and advanced option widgets
    QTabWidget  *tabs;
    QLineEdit   *commandLineEdit;
    QComboBox   *extentComboBox;
    QCheckBox   *lockedViewToggle;
    QCheckBox   *copyViewFromCameraToggle;
    QPushButton *makeViewKeyframeButton;
    QVBox       *pageAdvanced;
    QCheckBox   *centerToggle;
    QLineEdit   *centerLineEdit;
};

#endif
