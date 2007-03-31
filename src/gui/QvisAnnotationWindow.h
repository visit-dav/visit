#ifndef QVIS_ANNOTATION_WINDOW_H
#define QVIS_ANNOTATION_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowSimpleObserver.h>

// Forward declarations.
class AnnotationAttributes;
class AnnotationObjectList;
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QListBox;
class QNarrowLineEdit;
class QPushButton;
class QTabWidget;
class QVBox;
class QvisAnnotationObjectInterface;
class QvisColorButton;
class QvisLineWidthWidget;

// ****************************************************************************
// Class: QvisAnnotationWindow
//
// Purpose:
//   This window displays and manipulates the viewer's annotation attributes.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Sun Jun 17 20:52:59 PST 2001
//
// Modifications:
//   Brad Whitlock, Mon Aug 27 14:34:56 PST 2001
//   Added controls for setting the background and foreground colors.
//
//   Brad Whitlock, Thu Apr 11 11:41:58 PDT 2002
//   Added toggles for the legend and the database info.
//
//   Eric Brugger, Mon Nov  4 12:09:23 PST 2002
//   Added more user control over the axes tick marks and labels.
//
//   Eric Brugger, Tue Jun 24 16:31:28 PDT 2003
//   Added the ability to control the 2d axes line width and replaced the
//   2d font size setting with individual controls for setting the x label,
//   y label, x title, and y title font heights.
//
//   Brad Whitlock, Mon Nov 10 16:27:45 PST 2003
//   I added a button to turn off all annotation.
//
//   Brad Whitlock, Thu Oct 30 16:44:49 PST 2003
//   I made the window observe another object so it's now a simple observer.
//   I added controls to create and manage lists of annotations.
//
// ****************************************************************************

class GUI_API QvisAnnotationWindow : public QvisPostableWindowSimpleObserver
{
    Q_OBJECT
public:
    QvisAnnotationWindow(const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisAnnotationWindow();
    virtual void CreateWindowContents();

    void ConnectAnnotationAttributes(AnnotationAttributes *a);
    void ConnectAnnotationObjectList(AnnotationObjectList *a);
    virtual void SubjectRemoved(Subject *TheRemovedSubject);

    virtual void CreateNode(DataNode *);
    virtual void SetFromNode(DataNode *, const int *borders);
protected:
    virtual void UpdateWindow(bool doAll);
    void UpdateAnnotationControls(bool doAll);
    void UpdateAnnotationObjectControls(bool doAll);
    void Apply(bool dontIgnore = false);
    void ApplyObjectList(bool dontIgnore = false);
    void SetButtonGroup(QButtonGroup *bg, bool *vals);
    void GetCurrentValues(int which_widget);
    void Create2DTab();
    void Create3DTab();
    void CreateColorTab();
    void CreateObjectsTab();
private slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();
    void axesFlagChecked2D(bool val);
    void axesAutoSetTicksChecked2D(bool val);
    void axisLabelsChanged2D(int index);
    void axisTitlesChanged2D(int index);
    void gridLinesChanged2D(int index);
    void xMajorTickMinimumChanged2D();
    void yMajorTickMinimumChanged2D();
    void xMajorTickMaximumChanged2D();
    void yMajorTickMaximumChanged2D();
    void xMajorTickSpacingChanged2D();
    void yMajorTickSpacingChanged2D();
    void xMinorTickSpacingChanged2D();
    void yMinorTickSpacingChanged2D();
    void xLabelFontHeightChanged2D();
    void yLabelFontHeightChanged2D();
    void xTitleFontHeightChanged2D();
    void yTitleFontHeightChanged2D();
    void axesLineWidthChanged2D(int index);
    void axesTicksChanged2D(int index);
    void axesTickLocationChanged2D(int index);
    void axes3DFlagChecked(bool val);
    void axisLabelsChanged(int index);
    void gridLinesChanged(int index);
    void axisTicksChanged(int index);
    void axes3DTickLocationChanged(int index);
    void axes3DTypeChanged(int index);
    void triadFlagChecked(bool val);
    void bboxFlagChecked(bool val);
    void backgroundColorChanged(const QColor &c);
    void foregroundColorChanged(const QColor &c);
    void gradientColor1Changed(const QColor &c);
    void gradientColor2Changed(const QColor &c);
    void backgroundStyleChanged(int index);
    void gradientStyleChanged(int index);
    void tabSelected(const QString &tabLabel);
    void userInfoChecked(bool val);
    void databaseInfoChecked(bool val);
    void legendChecked(bool val);
    void turnOffAllAnnotations();

    // Slots for the objects tab.
    void applyObjectListChanges();
    void setUpdateForWindow(bool);
    void addNewAnnotationObject(int);
    void setActiveAnnotations();
    void hideActiveAnnotations();
    void deleteActiveAnnotations();
private:
    AnnotationAttributes *annotationAtts;
    AnnotationObjectList *annotationObjectList;

    QvisAnnotationObjectInterface **objectInterfaces;
    int                             nObjectInterfaces;
    QvisAnnotationObjectInterface  *displayInterface;

    QCheckBox       *userInfo;
    QCheckBox       *databaseInfo;
    QCheckBox       *legendInfo;
    QPushButton     *turnOffAllButton;
    QTabWidget      *tabs;

    // 2D tab widgets
    QVBox           *page2D;
    QCheckBox       *axesFlagToggle2D;
    QCheckBox       *axesAutoSetTicksToggle2D;
    QGroupBox       *axesGroup2D;
    QButtonGroup    *axisLabelsButtons2D;
    QButtonGroup    *axisTitlesButtons2D;
    QButtonGroup    *gridLinesButtons2D;
    QLabel          *majorTickMinimumLabel2D;
    QNarrowLineEdit *xMajorTickMinimumLineEdit2D;
    QNarrowLineEdit *yMajorTickMinimumLineEdit2D;
    QLabel          *majorTickMaximumLabel2D;
    QNarrowLineEdit *xMajorTickMaximumLineEdit2D;
    QNarrowLineEdit *yMajorTickMaximumLineEdit2D;
    QLabel          *majorTickSpacingLabel2D;
    QNarrowLineEdit *xMajorTickSpacingLineEdit2D;
    QNarrowLineEdit *yMajorTickSpacingLineEdit2D;
    QLabel          *minorTickSpacingLabel2D;
    QNarrowLineEdit *xMinorTickSpacingLineEdit2D;
    QNarrowLineEdit *yMinorTickSpacingLineEdit2D;
    QNarrowLineEdit *xLabelFontHeightLineEdit2D;
    QNarrowLineEdit *yLabelFontHeightLineEdit2D;
    QNarrowLineEdit *xTitleFontHeightLineEdit2D;
    QNarrowLineEdit *yTitleFontHeightLineEdit2D;
    QvisLineWidthWidget *axesLineWidth2D;
    QComboBox       *axesTicksComboBox2D;
    QComboBox       *axesTickLocationComboBox2D;
    // 3D tab widgets
    QVBox           *page3D;
    QCheckBox       *axes3DFlagToggle;
    QGroupBox       *axes3DGroup;
    QButtonGroup    *axisLabelsButtons;
    QButtonGroup    *gridLinesButtons;
    QButtonGroup    *axisTicksButtons;
    QComboBox       *axes3DTickLocationComboBox;
    QComboBox       *axes3DTypeComboBox;
    QCheckBox       *triadFlagToggle;
    QCheckBox       *bboxFlagToggle;
    // Color tab widgets
    QGroupBox       *pageColor;
    QvisColorButton *backgroundColorButton;
    QvisColorButton *foregroundColorButton;
    QButtonGroup    *backgroundStyleButtons;
    QLabel          *gradientStyleLabel;
    QComboBox       *gradientStyleComboBox;
    QLabel          *gradientColor1Label;
    QvisColorButton *gradientColor1Button;
    QLabel          *gradientColor2Label;
    QvisColorButton *gradientColor2Button;
    // Objects tab widgets
    QGroupBox       *pageObjects;
    QButtonGroup    *objButtonGroup;
    QListBox        *annotationListBox;
    QPushButton     *hideShowAnnotationButton;
    QPushButton     *deleteAnnotationButton;

    int             activeTab;
};

#endif
