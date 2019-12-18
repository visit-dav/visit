// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_LCS_WINDOW_H
#define QVIS_LCS_WINDOW_H
#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

// Forward declarations
class LCSAttributes;

class QLabel;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QSlider;
class QSpinBox;
class QButtonGroup;

// ****************************************************************************
// Class: QvisLCSWindow
//
// Purpose:
//   Defines QvisLCSWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 21 14:12:49 PST 2002
//
// Modifications:
//   Kathleen Biagas, Fri Nov  8 08:07:40 PST 2019
//   Added CreateSourceTab.
//
// ****************************************************************************

class QvisLCSWindow : public QvisOperatorWindow
{
    Q_OBJECT

  public:
    QvisLCSWindow(const int type,
                         LCSAttributes *subj,
                         const QString &caption = QString::null,
                         const QString &shortName = QString::null,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisLCSWindow();
    virtual void CreateWindowContents();

  protected:
    void CreateSourceTab(QWidget *);
    void CreateIntegrationTab(QWidget *);
    void CreateAppearanceTab(QWidget *);
    void CreateAdvancedTab(QWidget *);

    void UpdateWindow(bool doAll);
    void GetCurrentValues(int which_widget);
    void UpdateAlgorithmAttributes();
    void UpdateFieldAttributes();
    void UpdateIntegrationAttributes();

  private slots:
    // Integration
    void sourceTypeChanged(int val);
    void ResolutionProcessText();
    void UseDataSetStartChanged(int);
    void StartPositionProcessText();
    void UseDataSetEndChanged(int);
    void EndPositionProcessText();

    void auxiliaryGridChanged(int val);
    void auxiliaryGridSpacingProcessText();

    void thresholdLimitProcessText();
    void radialLimitProcessText();
    void boundaryLimitProcessText();
    void seedLimitChanged(int val);

    void fieldTypeChanged(int val);
    void fieldConstantProcessText();
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

    void operationTypeChanged(int);
    void cauchyGreenTensorChanged(int);
    void eigenComponentChanged(int);
    void operatorTypeChanged(int);

    void terminationTypeButtonGroupChanged(int);
    void clampLogValuesChanged(bool);
    void eigenWeightEditProcessText();

    void eigenWeightSliderPressed();
    void eigenWeightSliderReleased();
    void eigenWeightSliderChanged(int val);

    void maxTimeProcessText();
    void maxDistanceProcessText();
    void maxSizeProcessText();
    void maxStepsProcessText();

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

    // Integration
    QComboBox *sourceType;
    QLabel       *ResolutionLabel;
    QLineEdit    *Resolution;
    QButtonGroup *UseDataSetStart;
    QLineEdit    *StartPosition;
    QButtonGroup *UseDataSetEnd;
    QLineEdit    *EndPosition;

    QComboBox *auxiliaryGrid;
    QLabel    *auxiliaryGridSpacingLabel;
    QLineEdit *auxiliaryGridSpacing;

    QLineEdit *thresholdLimit;
    QLineEdit *radialLimit;
    QLineEdit *boundaryLimit;
    QSpinBox  *seedLimit;

    QComboBox *fieldType;
    QLabel    *fieldConstantLabel;
    QLineEdit *fieldConstant;
    QLineEdit *velocitySource;
    QLabel    *velocitySourceLabel;
//    QCheckBox *forceNodal;

    QComboBox *directionType;
    QComboBox *integrationType;
    QCheckBox *limitMaxTimeStep;
    QLineEdit *maxStepLength;
    QLabel    *maxStepLengthLabel;
    QLineEdit *maxTimeStep;
    QLineEdit *maxSteps;
    QLineEdit *relTol;
    QLabel    *relTolLabel;
    QLineEdit *absTol;
    QComboBox *absTolSizeType;
    QLabel    *absTolLabel;

    QCheckBox *limitMaxTime;
    QCheckBox *limitMaxDistance;

    QComboBox *operationType;
    QLabel    *cauchyGreenTensorLabel;
    QComboBox *cauchyGreenTensor;
    QLabel    *eigenComponentLabel;
    QComboBox *eigenComponent;
    QComboBox *operatorType;
    QCheckBox *clampLogValues;
    QLineEdit *eigenWeightEdit;
    QSlider   *eigenWeightSlider;
    QLabel    *eigenWeightLabel;

    bool      sliderDragging;

    QButtonGroup *terminationTypeButtonGroup;
    QLineEdit *maxTime;
    QLineEdit *maxDistance;
    QLineEdit *maxSize;

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

    LCSAttributes *atts;
};

#endif
