/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

#ifndef QVIS_STREAMLINE_PLOT_WINDOW_H
#define QVIS_STREAMLINE_PLOT_WINDOW_H
#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

// Forward declarations
class QLabel;
class QGroupBox;
class QListWidget;
class QListWidgetItem;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QSpinBox;
class QvisColorTableButton;
class QvisColorButton;
class QvisLineWidthWidget;
class QvisVariableButton;
class QvisOpacitySlider;
class QPushButton;
class QButtonGroup;
class QRadioButton;
class StreamlineAttributes;

// ****************************************************************************
// Class: QvisStreamlinePlotWindow
//
// Purpose: 
//   Defines QvisStreamlinePlotWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 21 14:12:49 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Dec 22 13:03:49 PST 2004
//   Added support for coloring by vorticity and ribbons.
//
//   Brad Whitlock, Mon Jan 3 15:32:24 PST 2005
//   Added override of ProcessOldVersions.
//
//   Hank Childs, Sat Mar  3 09:11:44 PST 2007
//   Added useWholeBox.
//
//   Dave Pugmire, Thu Nov 15 12:09:08 EST 2007
//   Add streamline direction option.
//
//   Dave Pugmire, Mon Aug 4 2:49:38 EDT 2008
//   Added termination, algorithm and integration options.
//
//   Dave Pugmire, Wed Aug 6 15:16:23 EST 2008
//   Add accurate distance calculate option.
//
//   Dave Pugmire, Tue Aug 19 17:18:03 EST 2008
//   Removed the accurate distance calculation option.
//
//   Dave Pugmire, Thu Feb  5 12:20:15 EST 2009
//   Added workGroupSize for the masterSlave algorithm.
//
//   Dave Pugmire, Mon Feb 23, 09:11:34 EST 2009
//   Added number of steps as a termination criterion.
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Add pathline GUI.
//    
//   Hank Childs, Sat May  2 22:10:26 PDT 2009
//   Added option for specifying seed points as a point list.
//
//   Dave Pugmire, Wed Jun 10 16:26:25 EDT 2009
//   Add color by variable.
//
//   Dave Pugmire, Mon Nov 23 09:38:53 EST 2009
//   Add min/max options to color table.
//
//   Dave Pugmire, Tue Dec 29 14:37:53 EST 2009
//   Add lots of appearance options to the streamlines plots.
//
//   Dave Pugmire, Wed Jan 20 09:28:59 EST 2010
//   Add ramp opacity, show head geom.
//
//   Dave Pugmire, Tue Feb 16 09:08:32 EST 2010
//   Add display head geom as cone.
//
//   Allen Sanderson, Sun Mar  7 12:49:56 PST 2010
//   Change layout of window for 2.0 interface changes.
//
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources.
//
//   Dave Pugmire, Wed Jun 23 16:44:36 EDT 2010
//   Repace fill interior checkbox with radio button.
//
//   Hank Childs, Wed Sep 29 20:22:36 PDT 2010
//   Add methods for the maximum time step (DoPri), which is distinct from
//   the step length (Adams/Bashforth).
//
//   Hank Childs, Thu Sep 30 01:22:24 PDT 2010
//   Add widgets for choosing size type between fraction of bbox and absolute.
//
//   Hank Childs, Fri Oct  1 21:13:56 PDT 2010
//   Add size type option for absTol.
//
//   Hank Childs, Wed Oct  6 20:27:09 PDT 2010
//   Add options for different termination types.
//
//   Hank Childs, Sun Dec  5 04:59:00 PST 2010
//   Add new data members for warnings for stiffness and critical points.
//
// ****************************************************************************

class QvisStreamlinePlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
  public:
    QvisStreamlinePlotWindow(const int type,
                         StreamlineAttributes *subj,
                         const QString &caption = QString::null,
                         const QString &shortName = QString::null,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisStreamlinePlotWindow();
    virtual void CreateWindowContents();

    virtual void ProcessOldVersions(DataNode *node, const char *ver);
  public slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();
  protected:
    void CreateAppearanceTab(QWidget *);
    void CreateAdvancedTab(QWidget *);
    
    void UpdateWindow(bool doAll);
    void GetCurrentValues(int which_widget);
    void Apply(bool ignore = false);
    void UpdateSourceAttributes();
    void TurnOffSourceAttributes();
    void UpdateAlgorithmAttributes();
    void UpdateIntegrationAttributes();
  private slots:
    void sourceTypeChanged(int val);
    void integrationTypeChanged(int val);
    void streamlineAlgorithmChanged(int val);
    void directionTypeChanged(int val);
    void maxStepLengthProcessText();
    void maxTimeStepProcessText();
    void maxStepsProcessText();
    void limitMaxTimeChanged(bool);
    void maxTimeProcessText();
    void limitMaxDistanceChanged(bool);
    void maxDistanceProcessText();
    void relTolProcessText();
    void absTolProcessText();
    void absTolSizeTypeChanged(int);
    void pointSourceProcessText();
    void lineStartProcessText();
    void lineEndProcessText();
    void pointListProcessText();
    void planeOriginProcessText();
    void planeNormalProcessText();
    void planeUpAxisProcessText();
    void radiusProcessText();
    void sphereOriginProcessText();
    void sampleDensity0Changed(int val);
    void sampleDensity1Changed(int val);
    void sampleDensity2Changed(int val);
    void sampleDistance0ProcessText();
    void sampleDistance1ProcessText();
    void sampleDistance2ProcessText();
    void randomSamplesChanged(bool);
    void randomSeedChanged(int);
    void numberOfRandomSamplesChanged(int);
    void fillChanged(int);
    void maxSLCountChanged(int val);
    void maxDomainCacheChanged(int val);
    void workGroupSizeChanged(int val);
    void displayMethodChanged(int val);
    void geomDisplayQualityChanged(int val);
    void showSeedsChanged(bool val);
    void showHeadsChanged(bool val);
    void tubeRadiusProcessText();
    void ribbonWidthProcessText();
    void seedRadiusProcessText();
    void headRadiusProcessText();
    void headHeightRatioProcessText();
    void lineWidthChanged(int style);
    void coloringMethodChanged(int val);
    void colorTableNameChanged(bool useDefault, const QString &ctName);
    void singleColorChanged(const QColor &color);
    void legendFlagChanged(bool val);
    void lightingFlagChanged(bool val);
    void icButtonGroupChanged(int val);
    void coordinateButtonGroupChanged(int val);
    void phiFactorProcessText();
    void pathlineOverrideStartingTimeFlagChanged(bool val);
    void pathlineOverrideStartingTimeProcessText();
    void pathlineCMFEButtonGroupChanged(int val);
    void useWholeBoxChanged(bool val);
    void coloringVariableChanged(const QString &var);
    void opacityTypeChanged(int val);
    void opacityVariableChanged(const QString &var);
    void opacityChanged(int opacity, const void*);
    void boxExtentsProcessText();
    void legendMaxToggled(bool);
    void legendMinToggled(bool);
    void processMaxLimitText();
    void processMinLimitText();
    void processDisplayBeginText();
    void processDisplayEndText();
    void opacityMinToggled(bool);
    void opacityMaxToggled(bool);
    void processOpacityVarMin();
    void processOpacityVarMax();
    void displayReferenceTypeChanged(int val);
    void displayBeginToggled(bool);
    void displayEndToggled(bool);
    void tubeDisplayDensityChanged(int);
    void seedDisplayQualityChanged(int);
    void headDisplayTypeChanged(int);
    void pointListClicked(QListWidgetItem*);
    void pointListDoubleClicked(QListWidgetItem*);
    void addPoint();
    void deletePoint();
    void deletePoints();
    void readPoints();
    void textChanged(const QString &currentText);
    void forceNodalChanged(bool);
    void limitMaxTimeStepChanged(bool);
    void headSizeTypeChanged(int);
    void seedSizeTypeChanged(int);
    void tubeSizeTypeChanged(int);
    void ribbonSizeTypeChanged(int);
    void issueWarningForMaxStepsChanged(bool);
    void issueWarningForStiffnessChanged(bool);
    void issueWarningForCriticalPointsChanged(bool);
    void criticalPointThresholdProcessText();

  private:
    int plotType;
    QComboBox *sourceType;
    QComboBox *directionType;
    QLabel    *limitMaxTimeStepLabel;
    QCheckBox *limitMaxTimeStep;
    QLineEdit *maxStepLength;
    QLabel    *maxStepLengthLabel;
    QLineEdit *maxTimeStep;
    QLabel    *maxTimeStepLabel;
    QLineEdit *maxSteps;
    QCheckBox *limitMaxTime;
    QLineEdit *maxTime;
    QCheckBox *limitMaxDistance;
    QLineEdit *maxDistance;
    QLineEdit *relTol;
    QLabel    *relTolLabel;
    QLineEdit *absTol;
    QComboBox *absTolSizeType;
    QLabel    *absTolLabel;
    QLabel    *forceNodalLabel;
    QCheckBox *forceNodal;
    QLineEdit *pointSource;
    QLabel    *pointSourceLabel;
    QLineEdit *lineStart;
    QLabel    *lineStartLabel;
    QLineEdit *lineEnd;
    QLabel    *lineEndLabel;
    QListWidget *pointList;
    QPushButton *pointListDelPoint, *pointListDelAllPoints, *pointListAddPoint, *pointListReadPoints;
    QLineEdit *planeOrigin;
    QLabel    *planeOriginLabel;
    QLineEdit *planeNormal;
    QLabel    *planeNormalLabel;
    QLineEdit *planeUpAxis;
    QLabel    *planeUpAxisLabel;
    QLineEdit *radius;
    QLabel    *radiusLabel;
    QLineEdit *sphereOrigin;
    QLabel    *sphereOriginLabel;
    QCheckBox *useWholeBox;
    QLineEdit *boxExtents[3];
    QLabel    *boxExtentsLabel[3];
    QLabel    *sampleDensityLabel[3], *sampleDistanceLabel[3];
    QSpinBox  *sampleDensity[3];
    QLineEdit *sampleDistance[3];

    QGroupBox *samplingGroup;
    QCheckBox *randomSamples;
    QSpinBox  *numberOfRandomSamples;
    QLabel    *numberOfRandomSamplesLabel;
    QSpinBox  *randomSeed;
    QLabel    *randomSeedLabel;
    QLabel    *fillLabel;
    QButtonGroup *fillButtonGroup;
    QRadioButton *fillButtons[2];

    QComboBox *displayMethod;
    QCheckBox *showSeeds, *showHeads;
    QLabel    *seedRadiusLabel, *headRadiusLabel, *headHeightLabel;
    QComboBox *headDisplayType;
    QComboBox *headSizeType;
    QLabel    *headDisplayTypeLabel;
    QLineEdit *seedRadius, *headRadius, *headHeight;
    QComboBox *seedSizeType;
    QLineEdit *tubeRadius, *ribbonWidth;
    QComboBox *tubeSizeType;
    QComboBox *ribbonSizeType;
    QLabel    *geomRadiusLabel;
    QvisLineWidthWidget *lineWidth;
    QLabel    *lineWidthLabel;
    QLabel    *lineWidthDummy;
    QComboBox *dataValueComboBox;
    QvisColorTableButton *colorTableName;
    QLabel    *colorTableNameLabel;
    QvisColorButton *singleColor;
    QLabel    *singleColorLabel;
    QCheckBox *legendFlag;
    QCheckBox *lightingFlag;
    QButtonGroup *icButtonGroup;
    QButtonGroup *coordinateButtonGroup;
    QLineEdit *phiFactor;
    QLabel    *phiFactorLabel;
    QCheckBox *pathlineOverrideStartingTimeFlag;
    QLineEdit *pathlineOverrideStartingTime;
    QButtonGroup *pathlineCMFEButtonGroup;
    QComboBox *integrationType;
    QLabel    *slAlgoLabel;
    QComboBox *slAlgo;
    QLabel    *maxSLCountLabel;
    QSpinBox  *maxSLCount;
    QLabel    *maxDomainCacheLabel;
    QSpinBox  *maxDomainCache;
    QLabel    *workGroupSizeLabel;
    QSpinBox  *workGroupSize;
    QvisVariableButton *coloringVar;
    QLabel    *limitsLabel;
    QCheckBox *legendMaxToggle;
    QCheckBox *legendMinToggle;
    QLineEdit *legendMaxEdit;
    QLineEdit *legendMinEdit;
    QComboBox *displayReferenceType;
    QLabel    *displayLabel;
    QCheckBox *displayBeginToggle;
    QCheckBox *displayEndToggle;
    QLineEdit *displayBeginEdit;
    QLineEdit *displayEndEdit;
    QComboBox *opacityType;
    QLabel    *opacityVarLabel;
    QvisVariableButton *opacityVar;
    QvisOpacitySlider *opacitySlider;
    QCheckBox *opacityMinToggle;
    QCheckBox *opacityMaxToggle;
    QLineEdit *opacityVarMin;
    QLineEdit *opacityVarMax;
    QLabel    *tubeDisplayDensityLabel;
    QSpinBox  *tubeDisplayDensity;
    QLabel    *geomDisplayQualityLabel;
    QComboBox *geomDisplayQuality;

    QCheckBox *issueWarningForMaxSteps;
    QCheckBox *issueWarningForStiffness;
    QCheckBox *issueWarningForCriticalPoints;
    QLineEdit *criticalPointThreshold;
    QLabel    *criticalPointThresholdLabel;

    StreamlineAttributes *streamAtts;
};


#endif


