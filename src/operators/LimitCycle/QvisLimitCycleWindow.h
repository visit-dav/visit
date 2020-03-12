// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_INTEGRALCURVE_WINDOW_H
#define QVIS_INTEGRALCURVE_WINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

// Forward declarations
class LimitCycleAttributes;

class QLabel;
class QCheckBox;
class QLineEdit;
class QSpinBox;
class QButtonGroup;
class QRadioButton;
class QComboBox;
class QGroupBox;
class QListWidget;

class QvisVariableButton;

// ****************************************************************************
// Class: QvisLimitCyclePlotWindow
//
// Purpose:
//   Defines QvisLimitCyclePlotWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 21 14:12:49 PST 2002
//
// ****************************************************************************

class QvisLimitCycleWindow : public QvisOperatorWindow
{
    Q_OBJECT

  public:
    QvisLimitCycleWindow(const int type,
                            LimitCycleAttributes *subj,
                            const QString &caption = QString::null,
                            const QString &shortName = QString::null,
                            QvisNotepadArea *notepad = 0);
    virtual ~QvisLimitCycleWindow();
    virtual void CreateWindowContents();

  protected:
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
    void lineStartProcessText();
    void lineEndProcessText();
    void planeOriginProcessText();
    void planeNormalProcessText();
    void planeUpAxisProcessText();
    void sampleDensity0Changed(int val);
    void sampleDensity1Changed(int val);
    void sampleDistance0ProcessText();
    void sampleDistance1ProcessText();

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
    void cycleToleranceProcessText();
    void maxIterationsProcessText();
    void showPartialResultsChanged(bool);
    void showReturnDistancesChanged(bool);

    void dataValueChanged(int val);
    void dataVariableChanged(const QString &var);

    void correlationDistanceMinDistTypeChanged(int);
    void processCorrelationDistanceAngTolEditText();
    void processCorrelationDistanceMinDistEditText();

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
    QLineEdit *planeOrigin;
    QLabel    *planeOriginLabel;
    QLineEdit *planeNormal;
    QLabel    *planeNormalLabel;
    QLineEdit *planeUpAxis;
    QLabel    *planeUpAxisLabel;
    QLabel    *sampleDensityLabel[2], *sampleDistanceLabel[2];
    QSpinBox  *sampleDensity[2];
    QLineEdit *sampleDistance[2];

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
    QLineEdit *cycleTolerance;
    QLineEdit *maxIterations;
    QCheckBox *showPartialResults;
    QCheckBox *showReturnDistances;

    QComboBox *dataValueComboBox;
    QvisVariableButton *dataVariable;
    QLabel    *correlationDistanceAngTolLabel, *correlationDistanceMinDistLabel;
    QLineEdit *correlationDistanceAngTolEdit, *correlationDistanceMinDistEdit;
    QComboBox *correlationDistanceMinDistType;

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

    QCheckBox *issueWarningForMaxSteps;
    QCheckBox *issueWarningForStepsize;
    QCheckBox *issueWarningForStiffness;
    QCheckBox *issueWarningForCriticalPoints;
    QLineEdit *criticalPointThreshold;
    QLabel    *criticalPointThresholdLabel;

    LimitCycleAttributes *atts;
};

#endif
