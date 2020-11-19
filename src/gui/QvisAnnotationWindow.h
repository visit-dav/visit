// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_ANNOTATION_WINDOW_H
#define QVIS_ANNOTATION_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowSimpleObserver.h>

// Forward declarations.
class AnnotationAttributes;
class AnnotationObjectList;
class AxisAttributes;
class FontAttributes;
class PlotList;

class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QListWidget;
class QNarrowLineEdit;
class QPushButton;
class QSpinBox;
class QTabWidget;
class QvisAnnotationObjectInterface;
class QvisAxisAttributesWidget;
class QvisColorButton;
class QvisDialogLineEdit;
class QvisFontAttributesWidget;
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
//   Kathleen Bonnell, Tue Dec 16 11:34:33 PST 2003 
//   Added buttons for automatic label scaling, and for filling in exponents
//   of labels (when auto label scaling is off).  
//
//   Brad Whitlock, Wed Jul 27 16:25:15 PST 2005
//   Split the 2D and 3D tabs in 2 and added controls for setting axis titles
//   and units.
//
//   Brad Whitlock, Wed Mar 21 21:13:39 PST 2007
//   Added plotList.
//
//   Cyrus Harrison, Mon Jun 18 14:48:54 PDT 2007
//   Added path expansion mode widgets
//
//   Cyrus Harrison, Tue Oct  2 09:07:52 PDT 2007
//   Added create general tab
//
//   Brad Whitlock, Wed Nov 14 11:34:45 PDT 2007
//   Added background image support.
//
//   Brad Whitlock, Thu Feb 7 16:18:19 PST 2008
//   I rewrote the support for 2D,3D axes.
//
//   Brad Whitlock, Wed Apr  9 10:58:41 PDT 2008
//   QString for caption, shortName.
//
//   Brad Whitlock, Wed Jun 25 09:28:14 PDT 2008
//   Qt 4.
//
//   Jeremy Meredith, Tue Nov 18 15:49:29 EST 2008
//   Added AxisArray modality options.
//
//   Jeremy Meredith, Thu Jan 22 14:55:02 EST 2009
//   Change the 2D and 3D tabs to update just the sub-widget sensitivities,
//   not the whole tab itself, when "show axes" is unchecked.
//
//   Brad Whitlock, Mon Mar  2 14:31:48 PST 2009
//   I added support for scaling and offsetting time.
//
//   Hank Childs, Fri May 13 16:00:24 PDT 2011
//   Add ability to set location of bounding box.
//
//   Kathleen Biagas, Wed Sep  7 16:17:23 PDT 2011
//   Added timeInfo.
//
//   Kathleen Biagas, Wed Apr  8 07:52:35 PDT 2015
//   Added labels for 3d: line width, tick location and axes type.
//
//   Alister Maguire, Thu Mar  1 16:08:42 PST 2018
//   Added widgets and slots to handle altering the triad
//   attributes. 
//
// ****************************************************************************

class GUI_API QvisAnnotationWindow : public QvisPostableWindowSimpleObserver
{
    Q_OBJECT
public:
    QvisAnnotationWindow(const QString &caption = QString(),
                         const QString &shortName = QString(),
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisAnnotationWindow();
    virtual void CreateWindowContents();

    void ConnectAnnotationAttributes(AnnotationAttributes *a);
    void ConnectAnnotationObjectList(AnnotationObjectList *a);
    void ConnectPlotList(PlotList *pl);
    virtual void SubjectRemoved(Subject *TheRemovedSubject);

    virtual void CreateNode(DataNode *);
    virtual void SetFromNode(DataNode *, const int *borders);
protected:
    virtual void UpdateWindow(bool doAll);
    void UpdateAxes2D();
    void UpdateAxes3D();
    void UpdateAxesArray();
    void UpdateAnnotationControls(bool doAll);
    void UpdateAnnotationObjectControls(bool doAll);
    void Apply(bool dontIgnore = false);
    void ApplyObjectList(bool dontIgnore = false);
    void SetButtonGroup(QButtonGroup *bg, bool *vals);
    void GetCurrentValues(int which_widget);
    void CreateGeneralTab();
    void CreateArrayTab();
    QWidget *CreateGeneralTabArray(QWidget *);
    void Create2DTab();
    QWidget *CreateGeneralTab2D(QWidget *);
    void Create3DTab();
    QWidget *CreateGeneralTab3D(QWidget *);
    void CreateColorTab();
    void CreateObjectsTab();
private slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();

    // General option slots
    void tabSelected(int);
    void userInfoChecked(bool val);
    void userInfoFontChanged(const FontAttributes &);
    void databaseInfoChecked(bool val);
    void databasePathExpansionModeChanged(int index);
    void databaseInfoFontChanged(const FontAttributes &);
    void timeInfoChecked(bool val);
    void legendChecked(bool val);
    void turnOffAllAnnotations();
    void databaseTimeScaleChanged();
    void databaseTimeOffsetChanged();

    // array option slots
    void axesFlagCheckedArray(bool val);
    void axesTicksChangedArray(bool val);
    void axesAutoSetTicksCheckedArray(bool val);
    void labelAutoSetScalingCheckedArray(bool val);
    void axesLineWidthChangedArray(int index);
    void axisChangedArray(const AxisAttributes &);

    // 2D option slots
    void axesFlagChecked2D(bool val);
    void axesAutoSetTicksChecked2D(bool val);
    void labelAutoSetScalingChecked2D(bool val);
    void axesLineWidthChanged2D(int index);
    void axesTicksChanged2D(int index);
    void axesTickLocationChanged2D(int index);
    void xAxisChanged2D(const AxisAttributes &);
    void yAxisChanged2D(const AxisAttributes &);

    // 3D option slots
    void axes3DFlagChecked(bool val);
    void axesAutoSetTicksChecked(bool val);
    void labelAutoSetScalingChecked(bool val);
    void axes3DTickLocationChanged(int index);
    void axes3DTypeChanged(int index);
    void triadFlagChecked(bool val);
    void bboxFlagChecked(bool val);
    void axesLineWidthChanged(int index);
    void xAxisChanged(const AxisAttributes &);
    void yAxisChanged(const AxisAttributes &);
    void zAxisChanged(const AxisAttributes &);
    void setBBoxLocationChecked(bool);
    void bboxLocationChanged(void);
    void triadSetManualChecked(bool val);
    void setTriadColor(void);
    void triadLineWidthChanged(int index);
    void triadFontChanged(int index);
    void triadBoldToggleChecked(bool val);
    void triadItalicToggleChecked(bool val);

    // Color option slots
    void backgroundColorChanged(const QColor &c);
    void foregroundColorChanged(const QColor &c);
    void gradientColor1Changed(const QColor &c);
    void gradientColor2Changed(const QColor &c);
    void backgroundStyleChanged(int index);
    void gradientStyleChanged(int index);
    void backgroundImageChanged();
    void imageRepeatXChanged(int);
    void imageRepeatYChanged(int);

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
    PlotList             *plotList;

    QvisAnnotationObjectInterface **objectInterfaces;
    int                             nObjectInterfaces;
    QvisAnnotationObjectInterface  *displayInterface;

    QTabWidget               *tabs;
    int                       activeTab;

    // General Tab widgets
    QWidget                  *pageGeneral;
    QGroupBox                *userInfo;
    QvisFontAttributesWidget *userInfoFont;
    QGroupBox                *databaseInfo;
    QvisFontAttributesWidget *databaseInfoFont;
    QLabel                   *databasePathExpansionModeLabel;
    QComboBox                *databasePathExpansionMode;
    QGroupBox                *timeInfo;
    QCheckBox                *legendInfo;
    QPushButton              *turnOffAllButton;
    QNarrowLineEdit          *databaseTimeScale;
    QNarrowLineEdit          *databaseTimeOffset;

    // axisarray tab widgets
    QWidget                  *pageArray;
    QWidget                  *axesArrayGroup;
    QCheckBox                *axesFlagToggleArray;
    QCheckBox                *ticksToggleArray;
    QCheckBox                *axesAutoSetTicksToggleArray;
    QCheckBox                *labelAutoSetScalingToggleArray;
    QvisLineWidthWidget      *axesLineWidthArray;
    QvisAxisAttributesWidget *axesArray[1];
    

    // 2D tab widgets
    QWidget                  *page2D;
    QTabWidget               *page2DTabs;
    QCheckBox                *axesFlagToggle2D;
    QCheckBox                *axesAutoSetTicksToggle2D;
    QCheckBox                *labelAutoSetScalingToggle2D;
    QvisLineWidthWidget      *axesLineWidth2D;
    QComboBox                *axesTicksComboBox2D;
    QComboBox                *axesTickLocationComboBox2D;
    QvisAxisAttributesWidget *axes2D[2];

    // 3D tab widgets
    QWidget                  *page3D;
    QTabWidget               *page3DTabs;
    QGroupBox                *axesGroup;
    QCheckBox                *axes3DVisible;
    QCheckBox                *axesAutoSetTicksToggle;
    QCheckBox                *labelAutoSetScalingToggle;
    QLabel                   *axesLineWidthLabel;
    QvisLineWidthWidget      *axesLineWidth;
    QLabel                   *axes3DTickLocationLabel;
    QComboBox                *axes3DTickLocationComboBox;
    QLabel                   *axes3DTypeLabel;
    QComboBox                *axes3DTypeComboBox;
    QCheckBox                *triadFlagToggle;
    QGroupBox                *bboxGroup;
    QCheckBox                *bboxFlagToggle;
    QvisAxisAttributesWidget *axes3D[3];
    QCheckBox                *setBBoxLocationToggle;
    QNarrowLineEdit          *bboxLocations[6];
    QLabel                   *bboxLabels[6];
    QCheckBox                *triadSetManual;
    QGroupBox                *triadGroup;
    QLabel                   *triadLineWidthLabel;
    QLabel                   *triadColorLabel;
    QPushButton              *triadColorButton;
    QvisLineWidthWidget      *triadLineWidth;
    QLabel                   *triadFontLabel;
    QComboBox                *triadFontComboBox;
    QCheckBox                *triadBoldToggle;
    QCheckBox                *triadItalicToggle;

    // Color tab widgets
    QWidget                  *pageColor;
    QvisColorButton          *backgroundColorButton;
    QvisColorButton          *foregroundColorButton;
    QButtonGroup             *backgroundStyleButtons;
    QLabel                   *gradientStyleLabel;
    QComboBox                *gradientStyleComboBox;
    QLabel                   *gradientColor1Label;
    QvisColorButton          *gradientColor1Button;
    QLabel                   *gradientColor2Label;
    QvisColorButton          *gradientColor2Button;
    QvisDialogLineEdit       *backgroundImage;
    QLabel                   *backgroundImageLabel;
    QSpinBox                 *imageRepeatX;
    QLabel                   *imageRepeatXLabel;
    QSpinBox                 *imageRepeatY;
    QLabel                   *imageRepeatYLabel;

    // Objects tab widgets
    QWidget                  *pageObjects;
    QButtonGroup             *objButtonGroup;
    QListWidget              *annotationListBox;
    QPushButton              *hideShowAnnotationButton;
    QPushButton              *deleteAnnotationButton;
};

#endif
