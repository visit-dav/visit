/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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

#ifndef QVIS_LCS_WINDOW_H
#define QVIS_LCS_WINDOW_H
#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

// Forward declarations
class LCSAttributes;

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

    void operationTypeChanged(int);
    void operatorTypeChanged(int);
    void terminationTypeButtonGroupChanged(int);
    void clampLogValuesChanged(bool);

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

    void issueWarningForMaxStepsChanged(bool);
    void issueWarningForStiffnessChanged(bool);
    void issueWarningForCriticalPointsChanged(bool);
    void criticalPointThresholdProcessText();

  private:
    int plotType;

    // Integration
    QComboBox *sourceType;
    QLabel *ResolutionLabel;
    QLineEdit *Resolution;
    QButtonGroup *UseDataSetStart;
    QLineEdit *StartPosition;
    QButtonGroup *UseDataSetEnd;
    QLineEdit *EndPosition;

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
    QComboBox *operatorType;
    QCheckBox *clampLogValues;

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

    QCheckBox *issueWarningForMaxSteps;
    QCheckBox *issueWarningForStiffness;
    QCheckBox *issueWarningForCriticalPoints;
    QLineEdit *criticalPointThreshold;
    QLabel    *criticalPointThresholdLabel;

    LCSAttributes *atts;
};

#endif
