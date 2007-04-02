/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
//   Kathleen Bonnell, Tue Dec 16 11:34:33 PST 2003 
//   Added buttons for automatic label scaling, and for filling in exponents
//   of labels (when auto label scaling is off).  
//
//   Brad Whitlock, Wed Jul 27 16:25:15 PST 2005
//   Split the 2D and 3D tabs in 2 and added controls for setting axis titles
//   and units.
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
    QWidget *Create2DTabForGridAndTicks(QWidget *);
    QWidget *Create2DTabForTitleAndLabels(QWidget *);
    void Create3DTab();
    QWidget *Create3DTabForGridAndTicks(QWidget *);
    QWidget *Create3DTabForTitleAndLabels(QWidget *);
    void CreateColorTab();
    void CreateObjectsTab();
private slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();
    void axesFlagChecked2D(bool val);
    void axesAutoSetTicksChecked2D(bool val);
    void labelAutoSetScalingChecked2D(bool val);
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
    void xLabelScalingChanged2D();
    void yLabelScalingChanged2D();
    void axesLineWidthChanged2D(int index);
    void axesTicksChanged2D(int index);
    void axesTickLocationChanged2D(int index);

    void xAxisUserTitleChecked2D(bool);
    void xAxisUserTitleLineEditChanged2D();
    void yAxisUserTitleChecked2D(bool);
    void yAxisUserTitleLineEditChanged2D();
    void xAxisUserUnitsChecked2D(bool);
    void xAxisUserUnitsLineEditChanged2D();
    void yAxisUserUnitsChecked2D(bool);
    void yAxisUserUnitsLineEditChanged2D();

    void xAxisUserTitleChecked(bool);
    void xAxisUserTitleLineEditChanged();
    void yAxisUserTitleChecked(bool);
    void yAxisUserTitleLineEditChanged();
    void zAxisUserTitleChecked(bool);
    void zAxisUserTitleLineEditChanged();
    void xAxisUserUnitsChecked(bool);
    void xAxisUserUnitsLineEditChanged();
    void yAxisUserUnitsChecked(bool);
    void yAxisUserUnitsLineEditChanged();
    void zAxisUserUnitsChecked(bool);
    void zAxisUserUnitsLineEditChanged();

    void axes3DFlagChecked(bool val);
    void labelAutoSetScalingChecked(bool val);
    void axisLabelsChanged(int index);
    void gridLinesChanged(int index);
    void axisTicksChanged(int index);
    void xLabelScalingChanged();
    void yLabelScalingChanged();
    void zLabelScalingChanged();
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
    QCheckBox       *labelAutoSetScalingToggle2D;
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
    QLabel          *labelScalingLabel2D;
    QNarrowLineEdit *xLabelScalingLineEdit2D;
    QNarrowLineEdit *yLabelScalingLineEdit2D;
    QvisLineWidthWidget *axesLineWidth2D;
    QComboBox       *axesTicksComboBox2D;
    QComboBox       *axesTickLocationComboBox2D;
    QCheckBox       *xAxisUserTitleToggle2D;
    QNarrowLineEdit *xAxisUserTitleLineEdit2D;
    QCheckBox       *yAxisUserTitleToggle2D;
    QNarrowLineEdit *yAxisUserTitleLineEdit2D;
    QCheckBox       *xAxisUserUnitsToggle2D;
    QNarrowLineEdit *xAxisUserUnitsLineEdit2D;
    QCheckBox       *yAxisUserUnitsToggle2D;
    QNarrowLineEdit *yAxisUserUnitsLineEdit2D;

    // 3D tab widgets
    QVBox           *page3D;
    QCheckBox       *axes3DFlagToggle;
    QCheckBox       *labelAutoSetScalingToggle;
    QGroupBox       *axes3DGroup;
    QButtonGroup    *axisLabelsButtons;
    QButtonGroup    *gridLinesButtons;
    QButtonGroup    *axisTicksButtons;
    QLabel          *labelScalingLabel;
    QNarrowLineEdit *xLabelScalingLineEdit;
    QNarrowLineEdit *yLabelScalingLineEdit;
    QNarrowLineEdit *zLabelScalingLineEdit;
    QComboBox       *axes3DTickLocationComboBox;
    QComboBox       *axes3DTypeComboBox;
    QCheckBox       *xAxisUserTitleToggle;
    QNarrowLineEdit *xAxisUserTitleLineEdit;
    QCheckBox       *yAxisUserTitleToggle;
    QNarrowLineEdit *yAxisUserTitleLineEdit;
    QCheckBox       *zAxisUserTitleToggle;
    QNarrowLineEdit *zAxisUserTitleLineEdit;
    QCheckBox       *xAxisUserUnitsToggle;
    QNarrowLineEdit *xAxisUserUnitsLineEdit;
    QCheckBox       *yAxisUserUnitsToggle;
    QNarrowLineEdit *yAxisUserUnitsLineEdit;
    QCheckBox       *zAxisUserUnitsToggle;
    QNarrowLineEdit *zAxisUserUnitsLineEdit;
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
