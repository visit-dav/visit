// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_INTEGRALCURVE_WINDOW_H
#define QVIS_INTEGRALCURVE_WINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

// Forward declarations
class IntegralCurveAttributes;

class QLabel;
class QCheckBox;
class QLineEdit;
class QSpinBox;
class QButtonGroup;
class QRadioButton;
class QComboBox;
class QGroupBox;
class QPushButton;
class QListWidget;
class QListWidgetItem;

class QvisVariableButton;

class SelectionList;

// ****************************************************************************
// Class: QvisIntegralCurvePlotWindow
//
// Purpose:
//   Defines QvisIntegralCurvePlotWindow class.
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
//   Add integral curve direction option.
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
//   Add lots of appearance options to the integral curves plots.
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
//   Dave Pugmire, Fri Jan 28 14:49:50 EST 2011
//   Add vary tube radius by variable.
//
//   Dave Pugmire, Mon Feb 21 08:17:42 EST 2011
//   Add color by correlation distance.
//
//   Dave Pugmire, Thu Mar 15 11:23:18 EDT 2012
//   Add named selections as a seed source.
//
//   Kathleen Biagas, Fri Nov  8 09:13:02 PST 2019
//   Added CreateSourceTab, a new tab to reduce window height.
//
// ****************************************************************************

class QvisIntegralCurveWindow : public QvisOperatorWindow
{
    Q_OBJECT

  public:
    QvisIntegralCurveWindow(const int type,
                            IntegralCurveAttributes *subj,
                            const QString &caption = QString::null,
                            const QString &shortName = QString::null,
                            QvisNotepadArea *notepad = 0);
    virtual ~QvisIntegralCurveWindow();
    virtual void CreateWindowContents();

  protected:
    void CreateSourceTab(QWidget *);
    void CreateIntegrationTab(QWidget *);
    void CreateAppearanceTab(QWidget *);
    void CreateAdvancedTab(QWidget *);

    void UpdateWindow(bool doAll);
    void GetCurrentValues(int which_widget);
    void UpdateSourceAttributes();
    void TurnOffSourceAttributes();
    void UpdateAlgorithmAttributes();
    void UpdateFieldAttributes();
    void UpdateIntegrationAttributes();

  private slots:
  // Integral Curves
    void sourceTypeChanged(int val);
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
    void useWholeBoxChanged(bool val);
    void boxExtentsProcessText();
    void sampleDistance0ProcessText();
    void sampleDistance1ProcessText();
    void sampleDistance2ProcessText();
    void selectionsChanged(int val);

    void pointListClicked(QListWidgetItem*);
    void pointListDoubleClicked(QListWidgetItem*);
    void addPoint();
    void deletePoint();
    void deletePoints();
    void readPoints();
    void copyPoints();
    void textChanged(const QString &currentText);

    void samplingTypeChanged(int);
    void randomSeedChanged(int);
    void numberOfRandomSamplesChanged(int);
    void fillChanged(int);

    void fieldTypeChanged(int val);
    void fieldConstantProccessText();
    void velocitySourceProcessText();
//    void forceNodalChanged(bool);

    void directionTypeChanged(int val);
    void integrationTypeChanged(int val);
    void maxStepLengthProcessText();
    void limitMaxTimeStepChanged(bool);
    void maxTimeStepProcessText();

    void relTolProcessText();
    void absTolProcessText();
    void absTolSizeTypeChanged(int);

    void limitMaxTimeChanged(bool);
    void limitMaxDistanceChanged(bool);

    void maxStepsProcessText();
    void maxTimeProcessText();
    void maxDistanceProcessText();

  // Appearance
    void dataValueChanged(int val);
    void dataVariableChanged(const QString &var);

    void correlationDistanceMinDistTypeChanged(int);
    void processCorrelationDistanceAngTolEditText();
    void processCorrelationDistanceMinDistEditText();

    void cleanupMethodChanged(int val);
    void cleanupThresholdProcessText();

    void cropBeginFlagChanged(bool val);
    void cropBeginProcessText();
    void cropEndFlagChanged(bool val);
    void cropEndProcessText();
    void cropValueChanged(int val);

  // Advanced
    void parallelAlgorithmChanged(int val);
    void maxSLCountChanged(int val);
    void maxDomainCacheChanged(int val);
    void workGroupSizeChanged(int val);

    void icButtonGroupChanged(int val);
    void pathlineOverrideStartingTimeFlagChanged(bool val);
    void pathlineOverrideStartingTimeProcessText();
    void pathlinePeriodProcessText();
    void pathlineCMFEButtonGroupChanged(int val);

    void issueWarningForAdvectionChanged(bool);
    void issueWarningForBoundaryChanged(bool);
    void issueWarningForMaxStepsChanged(bool);
    void issueWarningForStepsizeChanged(bool);
    void issueWarningForStiffnessChanged(bool);
    void issueWarningForCriticalPointsChanged(bool);
    void criticalPointThresholdProcessText();

  private:
    int plotType;

  // Integral Curves
    QComboBox *sourceType;

    QLineEdit *pointSource;
    QLabel    *pointSourceLabel;
    QLineEdit *lineStart;
    QLabel    *lineStartLabel;
    QLineEdit *lineEnd;
    QLabel    *lineEndLabel;
    QListWidget *pointList;
    QPushButton *pointListDelPoint, *pointListDelAllPoints, *pointListAddPoint, *pointListReadPoints;
    QListWidget *fieldData;
    QPushButton *fieldDataCopyPoints;
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
    QLabel    *selectionsLabel;
    QComboBox *selections;


    QGroupBox *samplingGroup;
    QLabel    *fillLabel;
    QButtonGroup *fillButtonGroup;
    QRadioButton *fillButtons[2];
    QLabel    *samplingTypeLabel;
    QButtonGroup *samplingTypeButtonGroup;
    QRadioButton *samplingTypeButtons[2];
    QSpinBox  *numberOfRandomSamples;
    QLabel    *numberOfRandomSamplesLabel;
    QSpinBox  *randomSeed;
    QLabel    *randomSeedLabel;

    QComboBox *fieldType;
    QLabel    *fieldConstantLabel;
    QLineEdit *fieldConstant;
    QLineEdit *velocitySource;
    QLabel    *velocitySourceLabel;
//    QCheckBox *forceNodal;

    QComboBox *directionType;
    QComboBox *integrationType;

    QCheckBox *limitMaxTimeStep;
    QLineEdit *maxTimeStep;
    QLineEdit *relTol;
    QLabel    *relTolLabel;
    QLineEdit *absTol;
    QComboBox *absTolSizeType;
    QLabel    *absTolLabel;
    QLineEdit *maxStepLength;
    QLabel    *maxStepLengthLabel;

    QLineEdit *maxSteps;
    QCheckBox *limitMaxTime;
    QLineEdit *maxTime;
    QCheckBox *limitMaxDistance;
    QLineEdit *maxDistance;

  // Appearance
    QComboBox *dataValueComboBox;
    QvisVariableButton *dataVariable;
    QLabel    *correlationDistanceAngTolLabel, *correlationDistanceMinDistLabel;
    QLineEdit *correlationDistanceAngTolEdit, *correlationDistanceMinDistEdit;
    QComboBox *correlationDistanceMinDistType;

    QLabel    *cleanupThresholdLabel;
    QLineEdit *cleanupThreshold;
    QComboBox *cleanupMethodComboBox;

    QCheckBox *cropBeginFlag;
    QLineEdit *cropBegin;
    QCheckBox *cropEndFlag;
    QLineEdit *cropEnd;
    QComboBox *cropValueComboBox;

  // Advanced
    QLabel    *parallelAlgoLabel;
    QComboBox *parallelAlgo;
    QLabel    *maxSLCountLabel;
    QSpinBox  *maxSLCount;
    QLabel    *maxDomainCacheLabel;
    QSpinBox  *maxDomainCache;
    QLabel    *workGroupSizeLabel;
    QSpinBox  *workGroupSize;

    QButtonGroup *icButtonGroup;
    QCheckBox *pathlineOverrideStartingTimeFlag;
    QLineEdit *pathlineOverrideStartingTime;
    QLineEdit *pathlinePeriod;
    QButtonGroup *pathlineCMFEButtonGroup;

    QCheckBox *issueWarningForAdvection;
    QCheckBox *issueWarningForBoundary;
    QCheckBox *issueWarningForMaxSteps;
    QCheckBox *issueWarningForStepsize;
    QCheckBox *issueWarningForStiffness;
    QCheckBox *issueWarningForCriticalPoints;
    QLineEdit *criticalPointThreshold;
    QLabel    *criticalPointThresholdLabel;

    IntegralCurveAttributes *atts;
    SelectionList *selectionList;
};

#endif
