#ifndef QVIS_VIEW_WINDOW_H
#define QVIS_VIEW_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowSimpleObserver.h>

// Forward declarations.
class DataNode;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QTabWidget;
class QVBox;
class ViewAttributes;
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

    void Connect2DAttributes(ViewAttributes *v);
    void Connect3DAttributes(ViewAttributes *v);
    void ConnectWindowInformation(WindowInformation *);

    virtual void CreateNode(DataNode *parentNode);
    virtual void SetFromNode(DataNode *parentNode, const int *borders);
public slots:
    virtual void show();
protected:
    void Apply(bool ignore = false);
    void GetCurrentValues(int which_widget);
    void GetCurrentValues2d(int which_widget);
    void GetCurrentValues3d(int which_widget);

    virtual void UpdateWindow(bool doAll);
    void Update2D(bool doAll);
    void Update3D(bool doAll);
    void UpdateGlobal(bool doAll);
private slots:
    void apply();
    void processCommandText();

    void processViewportText2d();
    void processWindowText2d();

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

    void copyViewFromCameraChecked(bool);
    void makeViewKeyframe();

    void lockedViewChecked(bool);
    void extentTypeChanged(int);
    void undoView();
    void tabSelected(const QString &);
private:
    void ParseViewCommands(const char *str);
    void Pan(double panx, double pany);
    void RotateAxis(int axis, double angle);
    void Zoom(double zoom);
    void Viewport(const double *viewport);
    void Window(const double *window);

    ViewAttributes    *view2d;
    ViewAttributes    *view3d;
    WindowInformation *windowInfo;
    int               activeTab;

    // 2d widgets
    QVBox       *page2D;
    QGroupBox   *view2DGroup;
    QLineEdit   *viewportLineEdit;
    QLineEdit   *windowLineEdit;

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
};

#endif
