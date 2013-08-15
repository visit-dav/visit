/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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

#ifndef QVIS_FTLE_WINDOW_H
#define QVIS_FTLE_WINDOW_H
#include <QvisOperatorWindow.h>
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
class FTLEAttributes;
class SelectionList;

// ****************************************************************************
// Class: QvisFTLEWindow
//
// Purpose: 
//   Defines QvisFTLEWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 21 14:12:49 PST 2002
//
// Modifications:
//
// ****************************************************************************

class QvisFTLEWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisFTLEWindow(const int type,
                         FTLEAttributes *subj,
                         const QString &caption = QString::null,
                         const QString &shortName = QString::null,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisFTLEWindow();
    virtual void CreateWindowContents();
  protected:
    void CreateAdvancedTab(QWidget *);    
    void UpdateWindow(bool doAll);
    void GetCurrentValues(int which_widget);
    void UpdateAlgorithmAttributes();
    void UpdateFieldAttributes();
    void UpdateIntegrationAttributes();
  private slots:
    void sourceTypeChanged(int val);
    void ResolutionProcessText();
    void UseDataSetStartChanged(bool val);
    void StartPositionProcessText();
    void UseDatasetEndChanged(bool val);
    void EndPositionProcessText();
    void fieldTypeChanged(int val);
    void fieldConstantProccessText();
    void integrationTypeChanged(int val);
    void streamlineAlgorithmChanged(int val);
    void directionTypeChanged(int val);
    void maxStepLengthProcessText();
    void maxTimeStepProcessText();
    void maxStepsProcessText();
    void limitButtonGroupChanged(int);
//    void limitMaxTimeChanged(bool);
    void maxTimeProcessText();
//    void limitMaxDistanceChanged(bool);
    void maxDistanceProcessText();
    void relTolProcessText();
    void absTolProcessText();
    void absTolSizeTypeChanged(int);
    void velocitySourceProcessText();

    void maxSLCountChanged(int val);
    void maxDomainCacheChanged(int val);
    void workGroupSizeChanged(int val);

    void icButtonGroupChanged(int val);
    void pathlineOverrideStartingTimeFlagChanged(bool val);
    void pathlineOverrideStartingTimeProcessText();
    void pathlineCMFEButtonGroupChanged(int val);


    void forceNodalChanged(bool);
    void limitMaxTimeStepChanged(bool);
    void issueWarningForMaxStepsChanged(bool);
    void issueWarningForStiffnessChanged(bool);
    void issueWarningForCriticalPointsChanged(bool);
    void criticalPointThresholdProcessText();

  private:
    int plotType;
    QComboBox *sourceType;
    QLabel *ResolutionLabel;
    QLineEdit *Resolution;
    QCheckBox *UseDataSetStart;
    QLabel *StartPositionLabel;
    QLineEdit *StartPosition;
    QLabel *EndPositionLabel;
    QCheckBox *UseDatasetEnd;
    QLineEdit *EndPosition;
    QComboBox *directionType;
    QCheckBox *limitMaxTimeStep;
    QLineEdit *maxStepLength;
    QLabel    *maxStepLengthLabel;
    QLineEdit *maxTimeStep;
    QLineEdit *maxSteps;
    QButtonGroup *limitButtonGroup;
    QRadioButton *limitButtons[2];
//    QCheckBox *limitMaxTime;
    QLineEdit *maxTime;
//    QCheckBox *limitMaxDistance;
    QLineEdit *maxDistance;
    QLineEdit *relTol;
    QLabel    *relTolLabel;
    QLineEdit *absTol;
    QComboBox *absTolSizeType;
    QLabel    *absTolLabel;
    QCheckBox *forceNodal;
    QLineEdit *velocitySource;
    QLabel    *velocitySourceLabel;

    QButtonGroup *icButtonGroup;
    QCheckBox *pathlineOverrideStartingTimeFlag;
    QLineEdit *pathlineOverrideStartingTime;
    QButtonGroup *pathlineCMFEButtonGroup;
    QComboBox *fieldType;
    QLabel    *fieldConstantLabel;
    QLineEdit *fieldConstant;
    QComboBox *integrationType;
    QLabel    *slAlgoLabel;
    QComboBox *slAlgo;
    QLabel    *maxSLCountLabel;
    QSpinBox  *maxSLCount;
    QLabel    *maxDomainCacheLabel;
    QSpinBox  *maxDomainCache;
    QLabel    *workGroupSizeLabel;
    QSpinBox  *workGroupSize;

    QComboBox *displayReferenceType;
    QLabel    *displayLabel;
    QCheckBox *displayBeginToggle;
    QCheckBox *displayEndToggle;
    QLineEdit *displayBeginEdit;
    QLineEdit *displayEndEdit;


    QCheckBox *issueWarningForMaxSteps;
    QCheckBox *issueWarningForStiffness;
    QCheckBox *issueWarningForCriticalPoints;
    QLineEdit *criticalPointThreshold;
    QLabel    *criticalPointThresholdLabel;

    FTLEAttributes *FTLEAtts;
    SelectionList *selectionList;
};

#endif
