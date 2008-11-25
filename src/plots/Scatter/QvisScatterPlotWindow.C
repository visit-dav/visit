/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#include "QvisScatterPlotWindow.h"

#include <ScatterAttributes.h>
#include <ViewerProxy.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QRadioButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QToolTip>
#include <QWidget>

#include <QvisColorTableButton.h>
#include <QvisOpacitySlider.h>
#include <QvisColorButton.h>
#include <QvisLineStyleWidget.h>
#include <QvisLineWidthWidget.h>
#include <QvisVariableButton.h>

#include <stdio.h>
#include <string>

using std::string;

const char *QvisScatterPlotWindow::roleNames[5] = {
    QT_TRANSLATE_NOOP( "ScatterPlotRoleNames", "X coordinate" ),
    QT_TRANSLATE_NOOP( "ScatterPlotRoleNames", "Y coordinate" ),
    QT_TRANSLATE_NOOP( "ScatterPlotRoleNames", "Z coordinate" ),
    QT_TRANSLATE_NOOP( "ScatterPlotRoleNames", "Color" ),
    QT_TRANSLATE_NOOP( "ScatterPlotRoleNames", "None" )
};

// ****************************************************************************
// Method: QvisScatterPlotWindow::QvisScatterPlotWindow
//
// Purpose: 
//   Constructor
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 19 14:32:22 PST 2004
//
// Modifications:
//   Brad Whitlock, Fri Aug  8 09:50:37 PDT 2008
//   Added button groups.
//
// ****************************************************************************

QvisScatterPlotWindow::QvisScatterPlotWindow(const int type,
                         ScatterAttributes *subj,
                         const QString &caption,
                         const QString &shortName,
                         QvisNotepadArea *notepad)
    : QvisPostableWindowObserver(subj, caption, shortName, notepad)
{
    plotType = type;
    atts = subj;

    var1Scaling = 0;
    var2Scaling = 0;
    var3Scaling = 0;
    var4Scaling = 0;
}


// ****************************************************************************
// Method: QvisScatterPlotWindow::~QvisScatterPlotWindow
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 19 14:32:22 PST 2004
//
// Modifications:
//   Brad Whitlock, Fri Aug  8 09:50:58 PDT 2008
//   Added button groups.
//
//   Cyrus Harrison, Wed Aug 27 08:54:49 PDT 2008
//   Made sure a button groups have parents, so we don't need to explicitly
//   delete.
//
// ****************************************************************************

QvisScatterPlotWindow::~QvisScatterPlotWindow()
{
}


// ****************************************************************************
// Method: QvisScatterPlotWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 19 14:32:22 PST 2004
//
// Modifications:
//   Brad Whitlock, Thu Aug 25 10:06:06 PDT 2005
//   Added a Sphere option for the point type.
//
//   Brad Whitlock, Wed Apr 23 10:32:06 PDT 2008
//   Added tr()'s
//
//   Brad Whitlock, Fri Aug  8 09:46:31 PDT 2008
//   Qt 4.
//
//   Dave Pugmire, Wed Oct 29 16:00:48 EDT 2008
//   Swap the min/max in the gui.
//
// ****************************************************************************

void
QvisScatterPlotWindow::CreateWindowContents()
{
    int i;

    //
    // Create a tab widget so we can split up inputs and appearance.
    // 
    QTabWidget *topTab = new QTabWidget(central);
    topLayout->addWidget(topTab);

    //
    // Create a tab widget so we can split up the different inputs.
    //
    QWidget *inputGroup = new QWidget(central);
    topTab->addTab(inputGroup, tr("Inputs"));
    QVBoxLayout *inputVeryTopLayout = new QVBoxLayout(inputGroup);
    inputVeryTopLayout->setMargin(10);
    inputVeryTopLayout->setSpacing(5);
    QTabWidget *tabs = new QTabWidget(inputGroup);
    inputVeryTopLayout->addWidget(tabs);

    //
    // Create a tab for the var1
    //
    QWidget *var1Top = new QWidget(inputGroup);
    tabs->addTab(var1Top, tr("Input 1"));
    QVBoxLayout *var1VeryTopLayout = new QVBoxLayout(var1Top);
    var1VeryTopLayout->setMargin(10);
    var1VeryTopLayout->setSpacing(5);
    QGridLayout *var1TopLayout = new QGridLayout(0);
    var1TopLayout->setMargin(0);
    var1VeryTopLayout->addLayout(var1TopLayout);

    QLabel *var1Label = new QLabel(tr("Variable"), var1Top);
    var1Label->setEnabled(false);
    var1TopLayout->addWidget(var1Label,0,0);
    QLineEdit *var1 = new QLineEdit(var1Top);
    var1->setText("default");
    var1->setEnabled(false);
//    connect(var1, SIGNAL(returnPressed()),
//            this, SLOT(var1ProcessText()));
    var1TopLayout->addWidget(var1, 0,1);

    var1Role = new QComboBox(var1Top);
    for(i = 0; i < 4; ++i)
        var1Role->addItem(roleNames[i]);
    connect(var1Role, SIGNAL(activated(int)),
            this, SLOT(var1RoleChanged(int)));
    var1TopLayout->addWidget(var1Role, 1,1);
    QLabel *var1RoleLabel = new QLabel(tr("Role"), var1Top);
    var1RoleLabel->setBuddy(var1Role);
    var1TopLayout->addWidget(var1RoleLabel,1,0);

    // var1 Max
    var1MaxFlag = new QCheckBox(tr("Maximum"), var1Top);
    connect(var1MaxFlag, SIGNAL(toggled(bool)),
            this, SLOT(var1MaxFlagChanged(bool)));
    var1TopLayout->addWidget(var1MaxFlag, 2,0);
    var1Max = new QLineEdit(var1Top);
    connect(var1Max, SIGNAL(returnPressed()),
            this, SLOT(var1MaxProcessText()));
    var1TopLayout->addWidget(var1Max, 2,1);

    // var1 Min
    var1MinFlag = new QCheckBox(tr("Minimum"), var1Top);
    connect(var1MinFlag, SIGNAL(toggled(bool)),
            this, SLOT(var1MinFlagChanged(bool)));
    var1TopLayout->addWidget(var1MinFlag, 3,0);
    var1Min = new QLineEdit(var1Top);
    connect(var1Min, SIGNAL(returnPressed()),
            this, SLOT(var1MinProcessText()));
    var1TopLayout->addWidget(var1Min, 3,1);

    var1ScalingLabel = new QLabel(tr("Scale"), var1Top);
    var1TopLayout->addWidget(var1ScalingLabel,4,0);
    QWidget *var1ScalingWidget = new QWidget(var1Top);
    var1Scaling = new QButtonGroup(var1ScalingWidget);
    QHBoxLayout *var1ScalingLayout = new QHBoxLayout(var1ScalingWidget);
    var1ScalingLayout->setMargin(0);
    var1ScalingLayout->setSpacing(10);
    QRadioButton *var1ScalingScalingLinear = new QRadioButton(tr("Linear"),var1ScalingWidget);
    var1Scaling->addButton(var1ScalingScalingLinear, 0);
    var1ScalingLayout->addWidget(var1ScalingScalingLinear);
    QRadioButton *var1ScalingScalingLog = new QRadioButton(tr("Log"),var1ScalingWidget);
    var1Scaling->addButton(var1ScalingScalingLog, 1);
    var1ScalingLayout->addWidget(var1ScalingScalingLog);
    QRadioButton *var1ScalingScalingSkew = new QRadioButton(tr("Skew"),var1ScalingWidget);
    var1Scaling->addButton(var1ScalingScalingSkew, 2);
    var1ScalingLayout->addWidget(var1ScalingScalingSkew);
    connect(var1Scaling, SIGNAL(buttonClicked(int)),
            this, SLOT(var1ScalingChanged(int)));
    var1TopLayout->addWidget(var1ScalingWidget, 4,1);

    var1SkewFactorLabel = new QLabel(tr("Skew factor"), var1Top);
    var1TopLayout->addWidget(var1SkewFactorLabel,5,0);
    var1SkewFactor = new QLineEdit(var1Top);
    connect(var1SkewFactor, SIGNAL(returnPressed()),
            this, SLOT(var1SkewFactorProcessText()));
    var1TopLayout->addWidget(var1SkewFactor, 5,1);

    //
    // Create a tab for the var2
    //
    QWidget *var2Top = new QWidget(inputGroup);
    tabs->addTab(var2Top, tr("Input 2"));
    QVBoxLayout *var2VeryTopLayout = new QVBoxLayout(var2Top);
    var2VeryTopLayout->setMargin(10);
    var2VeryTopLayout->setSpacing(5);
    QGridLayout *var2TopLayout = new QGridLayout(0);
    var2TopLayout->setMargin(0);
    var2VeryTopLayout->addLayout(var2TopLayout);

    QLabel *var2Label = new QLabel(tr("Variable"), var2Top);
    var2TopLayout->addWidget(var2Label,0,0);

    // Create a variable button
    var2 = new QvisVariableButton(true, true, true, QvisVariableButton::Scalars,
        var2Top);
    connect(var2, SIGNAL(activated(const QString &)),
            this, SLOT(var2Selected(const QString &)));
    var2TopLayout->addWidget(var2, 0,1);

    var2Role = new QComboBox(var2Top);
    for(i = 0; i < 5; ++i)
        var2Role->addItem(roleNames[i]);
    connect(var2Role, SIGNAL(activated(int)),
            this, SLOT(var2RoleChanged(int)));
    var2TopLayout->addWidget(var2Role, 1,1);
    QLabel *var2RoleLabel = new QLabel(tr("Role"), var2Top);
    var2RoleLabel->setBuddy(var2Role);
    var2TopLayout->addWidget(var2RoleLabel,1,0);

    // var2 Max
    var2MaxFlag = new QCheckBox(tr("Maximum"), var2Top);
    connect(var2MaxFlag, SIGNAL(toggled(bool)),
            this, SLOT(var2MaxFlagChanged(bool)));
    var2TopLayout->addWidget(var2MaxFlag, 2,0);
    var2Max = new QLineEdit(var2Top);
    connect(var2Max, SIGNAL(returnPressed()),
            this, SLOT(var2MaxProcessText()));
    var2TopLayout->addWidget(var2Max, 2,1);

    // var2 Min
    var2MinFlag = new QCheckBox(tr("Minimum"), var2Top);
    connect(var2MinFlag, SIGNAL(toggled(bool)),
            this, SLOT(var2MinFlagChanged(bool)));
    var2TopLayout->addWidget(var2MinFlag, 3,0);
    var2Min = new QLineEdit(var2Top);
    connect(var2Min, SIGNAL(returnPressed()),
            this, SLOT(var2MinProcessText()));
    var2TopLayout->addWidget(var2Min, 3,1);

    var2ScalingLabel = new QLabel(tr("Scale"), var2Top);
    var2TopLayout->addWidget(var2ScalingLabel,4,0);
    QWidget *var2ScalingWidget = new QWidget(var2Top);
    var2Scaling = new QButtonGroup(var2ScalingWidget);
    QHBoxLayout *var2ScalingLayout = new QHBoxLayout(var2ScalingWidget);
    var2ScalingLayout->setMargin(0);
    var2ScalingLayout->setSpacing(10);
    QRadioButton *var2ScalingScalingLinear = new QRadioButton(tr("Linear"),var2ScalingWidget);
    var2Scaling->addButton(var2ScalingScalingLinear, 0);
    var2ScalingLayout->addWidget(var2ScalingScalingLinear);
    QRadioButton *var2ScalingScalingLog = new QRadioButton(tr("Log"),var2ScalingWidget);
    var2Scaling->addButton(var2ScalingScalingLog, 1);
    var2ScalingLayout->addWidget(var2ScalingScalingLog);
    QRadioButton *var2ScalingScalingSkew = new QRadioButton(tr("Skew"),var2ScalingWidget);
    var2Scaling->addButton(var2ScalingScalingSkew, 2);
    var2ScalingLayout->addWidget(var2ScalingScalingSkew);
    connect(var2Scaling, SIGNAL(buttonClicked(int)),
            this, SLOT(var2ScalingChanged(int)));
    var2TopLayout->addWidget(var2ScalingWidget, 4,1);

    var2SkewFactorLabel = new QLabel(tr("Skew factor"), var2Top);
    var2TopLayout->addWidget(var2SkewFactorLabel,5,0);
    var2SkewFactor = new QLineEdit(var2Top);
    connect(var2SkewFactor, SIGNAL(returnPressed()),
            this, SLOT(var2SkewFactorProcessText()));
    var2TopLayout->addWidget(var2SkewFactor, 5,1);

    //
    // Create a tab for the var3
    //
    QWidget *var3Top = new QWidget(inputGroup);
    tabs->addTab(var3Top, tr("Input 3"));
    QVBoxLayout *var3VeryTopLayout = new QVBoxLayout(var3Top);
    var3VeryTopLayout->setMargin(10);
    var3VeryTopLayout->setSpacing(5);
    QGridLayout *var3TopLayout = new QGridLayout(0);
    var3TopLayout->setMargin(0);
    var3VeryTopLayout->addLayout(var3TopLayout);

    QLabel *var3Label = new QLabel(tr("Variable"), var3Top);
    var3TopLayout->addWidget(var3Label,0,0);

    // Create a variable button.
    var3 = new QvisVariableButton(true, true, true, QvisVariableButton::Scalars,
        var3Top);
    connect(var3, SIGNAL(activated(const QString &)),
            this, SLOT(var3Selected(const QString &)));
    var3TopLayout->addWidget(var3, 0,1);

    var3Role = new QComboBox(var3Top);
    for(i = 0; i < 5; ++i)
        var3Role->addItem(roleNames[i]);
    connect(var3Role, SIGNAL(activated(int)),
            this, SLOT(var3RoleChanged(int)));
    var3TopLayout->addWidget(var3Role, 1,1);
    QLabel *var3RoleLabel = new QLabel(tr("Role"), var3Top);
    var3RoleLabel->setBuddy(var3Role);
    var3TopLayout->addWidget(var3RoleLabel,1,0);

    // var3 Max
    var3MaxFlag = new QCheckBox(tr("Maximum"), var3Top);
    connect(var3MaxFlag, SIGNAL(toggled(bool)),
            this, SLOT(var3MaxFlagChanged(bool)));
    var3TopLayout->addWidget(var3MaxFlag, 2,0);
    var3Max = new QLineEdit(var3Top);
    connect(var3Max, SIGNAL(returnPressed()),
            this, SLOT(var3MaxProcessText()));
    var3TopLayout->addWidget(var3Max, 2,1);

    // var3 Min
    var3MinFlag = new QCheckBox(tr("Minimum"), var3Top);
    connect(var3MinFlag, SIGNAL(toggled(bool)),
            this, SLOT(var3MinFlagChanged(bool)));
    var3TopLayout->addWidget(var3MinFlag, 3,0);
    var3Min = new QLineEdit(var3Top);
    connect(var3Min, SIGNAL(returnPressed()),
            this, SLOT(var3MinProcessText()));
    var3TopLayout->addWidget(var3Min, 3,1);

    var3ScalingLabel = new QLabel(tr("Scale"), var3Top);
    var3TopLayout->addWidget(var3ScalingLabel,4,0);
    QWidget *var3ScalingWidget = new QWidget(var3Top);
    var3Scaling = new QButtonGroup(var3ScalingWidget);
    QHBoxLayout *var3ScalingLayout = new QHBoxLayout(var3ScalingWidget);
    var3ScalingLayout->setMargin(0);
    var3ScalingLayout->setSpacing(10);
    QRadioButton *var3ScalingScalingLinear = new QRadioButton(tr("Linear"),var3ScalingWidget);
    var3Scaling->addButton(var3ScalingScalingLinear, 0);
    var3ScalingLayout->addWidget(var3ScalingScalingLinear);
    QRadioButton *var3ScalingScalingLog = new QRadioButton(tr("Log"),var3ScalingWidget);
    var3Scaling->addButton(var3ScalingScalingLog, 1);
    var3ScalingLayout->addWidget(var3ScalingScalingLog);
    QRadioButton *var3ScalingScalingSkew = new QRadioButton(tr("Skew"),var3ScalingWidget);
    var3Scaling->addButton(var3ScalingScalingSkew, 2);
    var3ScalingLayout->addWidget(var3ScalingScalingSkew);
    connect(var3Scaling, SIGNAL(buttonClicked(int)),
            this, SLOT(var3ScalingChanged(int)));
    var3TopLayout->addWidget(var3ScalingWidget, 4,1);

    var3SkewFactorLabel = new QLabel(tr("Skew factor"), var3Top);
    var3TopLayout->addWidget(var3SkewFactorLabel,5,0);
    var3SkewFactor = new QLineEdit(var3Top);
    connect(var3SkewFactor, SIGNAL(returnPressed()),
            this, SLOT(var3SkewFactorProcessText()));
    var3TopLayout->addWidget(var3SkewFactor, 5,1);

    //
    // Create a tab for the var4
    //
    QWidget *var4Top = new QWidget(inputGroup);
    tabs->addTab(var4Top, tr("Input 4"));
    QVBoxLayout *var4VeryTopLayout = new QVBoxLayout(var4Top);
    var4VeryTopLayout->setMargin(10);
    var4VeryTopLayout->setSpacing(5);
    QGridLayout *var4TopLayout = new QGridLayout(0);
    var4TopLayout->setMargin(0);
    var4VeryTopLayout->addLayout(var4TopLayout);

    QLabel *var4Label = new QLabel(tr("Variable"), var4Top);
    var4TopLayout->addWidget(var4Label,0,0);

    // Create a variable button.
    var4 = new QvisVariableButton(true, true, true, QvisVariableButton::Scalars,
        var4Top);
    connect(var4, SIGNAL(activated(const QString &)),
            this, SLOT(var4Selected(const QString &)));
    var4TopLayout->addWidget(var4, 0,1);

    var4Role = new QComboBox(var4Top);
    for(i = 0; i < 5; ++i)
        var4Role->addItem(roleNames[i]);
    connect(var4Role, SIGNAL(activated(int)),
            this, SLOT(var4RoleChanged(int)));
    var4TopLayout->addWidget(var4Role, 1,1);
    QLabel *var4RoleLabel = new QLabel(tr("Role"), var4Top);
    var4RoleLabel->setBuddy(var4Role);
    var4TopLayout->addWidget(var4RoleLabel,1,0);

    // var4 Max
    var4MaxFlag = new QCheckBox(tr("Maximum"), var4Top);
    connect(var4MaxFlag, SIGNAL(toggled(bool)),
            this, SLOT(var4MaxFlagChanged(bool)));
    var4TopLayout->addWidget(var4MaxFlag, 2,0);
    var4Max = new QLineEdit(var4Top);
    connect(var4Max, SIGNAL(returnPressed()),
            this, SLOT(var4MaxProcessText()));
    var4TopLayout->addWidget(var4Max, 2,1);

    // var4 Min
    var4MinFlag = new QCheckBox(tr("Minimum"), var4Top);
    connect(var4MinFlag, SIGNAL(toggled(bool)),
            this, SLOT(var4MinFlagChanged(bool)));
    var4TopLayout->addWidget(var4MinFlag, 3,0);
    var4Min = new QLineEdit(var4Top);
    connect(var4Min, SIGNAL(returnPressed()),
            this, SLOT(var4MinProcessText()));
    var4TopLayout->addWidget(var4Min, 3,1);

    var4ScalingLabel = new QLabel(tr("Scale"), var4Top);
    var4TopLayout->addWidget(var4ScalingLabel,4,0);
    QWidget *var4ScalingWidget = new QWidget(var4Top);
    var4Scaling = new QButtonGroup(var4ScalingWidget);
    QHBoxLayout *var4ScalingLayout = new QHBoxLayout(var4ScalingWidget);
    var4ScalingLayout->setMargin(0);
    var4ScalingLayout->setSpacing(10);
    QRadioButton *var4ScalingScalingLinear = new QRadioButton(tr("Linear"),var4ScalingWidget);
    var4Scaling->addButton(var4ScalingScalingLinear, 0);
    var4ScalingLayout->addWidget(var4ScalingScalingLinear);
    QRadioButton *var4ScalingScalingLog = new QRadioButton(tr("Log"),var4ScalingWidget);
    var4Scaling->addButton(var4ScalingScalingLog, 1);
    var4ScalingLayout->addWidget(var4ScalingScalingLog);
    QRadioButton *var4ScalingScalingSkew = new QRadioButton(tr("Skew"),var4ScalingWidget);
    var4Scaling->addButton(var4ScalingScalingSkew, 2);
    var4ScalingLayout->addWidget(var4ScalingScalingSkew);
    connect(var4Scaling, SIGNAL(buttonClicked(int)),
            this, SLOT(var4ScalingChanged(int)));
    var4TopLayout->addWidget(var4ScalingWidget, 4,1);

    var4SkewFactorLabel = new QLabel(tr("Skew factor"), var4Top);
    var4TopLayout->addWidget(var4SkewFactorLabel,5,0);
    var4SkewFactor = new QLineEdit(var4Top);
    connect(var4SkewFactor, SIGNAL(returnPressed()),
            this, SLOT(var4SkewFactorProcessText()));
    var4TopLayout->addWidget(var4SkewFactor, 5,1);

    //
    // Appearance widgets
    //
    QWidget *appearanceGroup = new QWidget(central);
    topTab->addTab(appearanceGroup, tr("Appearance"));
    QVBoxLayout *aTopLayout = new QVBoxLayout(appearanceGroup);
    aTopLayout->setMargin(0);
    aTopLayout->addSpacing(10);
    QGridLayout *aLayout = new QGridLayout(0);
    aTopLayout->addLayout(aLayout);
    aLayout->setMargin(5);
    aLayout->setSpacing(10);
    aTopLayout->addStretch(10);

    pointSize = new QLineEdit(appearanceGroup);
    connect(pointSize, SIGNAL(returnPressed()),
            this, SLOT(pointSizeProcessText()));
    aLayout->addWidget(pointSize, 0, 1);
    pointSizeLabel = new QLabel(tr("Point size"), appearanceGroup);
    pointSizeLabel->setBuddy(pointSize);
    aLayout->addWidget(pointSizeLabel, 0, 0);

    pointType = new QComboBox(appearanceGroup);
    pointType->addItem(tr("Box"));
    pointType->addItem(tr("Axis"));
    pointType->addItem(tr("Icosahedron"));
    pointType->addItem(tr("Point"));
    pointType->addItem(tr("Sphere"));
    connect(pointType, SIGNAL(activated(int)),
            this, SLOT(pointTypeChanged(int)));
    aLayout->addWidget(pointType, 1, 1);
    QLabel *pointTypeLabel = new QLabel(tr("Point Type"), appearanceGroup);
    pointTypeLabel->setBuddy(pointType);
    aLayout->addWidget(pointTypeLabel, 1, 0);

    colorTableName = new QvisColorTableButton(appearanceGroup);
    connect(colorTableName, SIGNAL(selectedColorTable(bool, const QString&)),
            this, SLOT(colorTableNameChanged(bool, const QString&)));
    aLayout->addWidget(colorTableName, 2, 1);
    QLabel *colorTableNameLabel = new QLabel(tr("Color table"), appearanceGroup);
    colorTableNameLabel->setBuddy(colorTableName);
    aLayout->addWidget(colorTableNameLabel, 2, 0);

    QWidget *scBox = new QWidget(appearanceGroup);
    QHBoxLayout *scBoxLayout = new QHBoxLayout(scBox);
    scBoxLayout->setMargin(0);
    scBoxLayout->setSpacing(10);
    aLayout->addWidget(scBox, 3, 1);

    singleColorLabel = new QLabel(tr("Single color"), appearanceGroup);
    singleColorLabel->setBuddy(singleColor);
    aLayout->addWidget(singleColorLabel, 3, 0);

    singleColor = new QvisColorButton(scBox);
    connect(singleColor, SIGNAL(selectedColor(const QColor&)),
            this, SLOT(singleColorChanged(const QColor&)));
    scBoxLayout->addWidget(singleColor);

    foregroundFlag = new QCheckBox(tr("Use foreground "), scBox);
    connect(foregroundFlag, SIGNAL(toggled(bool)),
            this, SLOT(foregroundFlagChanged(bool)));
    scBoxLayout->addWidget(foregroundFlag);

    scaleCube = new QCheckBox(tr("Scale to cube"), scBox);
    connect(scaleCube, SIGNAL(toggled(bool)),
            this, SLOT(scaleCubeChanged(bool)));
    aLayout->addWidget(scaleCube, 4, 0, 1, 2);

    //
    // Role labels.
    //
    QGroupBox *roleGroup = new QGroupBox(central);
    roleGroup->setTitle(tr("Roles"));
    topLayout->addWidget(roleGroup);
    QGridLayout *roleLayout = new QGridLayout(roleGroup);
    roleLayout->setSpacing(5);
    xCoordRoleLabel = new QLabel(tr("X coordinate:    "), roleGroup);
    roleLayout->addWidget(xCoordRoleLabel, 0, 0);
    yCoordRoleLabel = new QLabel(tr("Y coordinate:    "), roleGroup);
    roleLayout->addWidget(yCoordRoleLabel, 1, 0);
    zCoordRoleLabel = new QLabel(tr("Z coordinate:    "), roleGroup);
    roleLayout->addWidget(zCoordRoleLabel, 0, 1);
    colorRoleLabel = new QLabel(tr("Color:    "), roleGroup);
    roleLayout->addWidget(colorRoleLabel, 1, 1);



    legendFlag = new QCheckBox(tr("Legend"), central);
    connect(legendFlag, SIGNAL(toggled(bool)),
            this, SLOT(legendFlagChanged(bool)));
    topLayout->addWidget(legendFlag);
}


// ****************************************************************************
// Method: QvisScatterPlotWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 19 14:32:22 PST 2004
//
// Modifications:
//   Brad Whitlock, Wed Jul 20 15:30:52 PST 2005
//   I added pointSizePixels.
//
//   Brad Whitlock, Thu Aug 25 10:07:12 PDT 2005
//   I added the Sphere point type.
//
//   Brad Whitlock, Mon Dec 17 12:13:04 PST 2007
//   Made it use ids.
//
//   Brad Whitlock, Fri Aug  8 10:13:49 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisScatterPlotWindow::UpdateWindow(bool doAll)
{
    QString temp;
    bool rolesChanged = false;
    bool varsChanged = false;
    bool updatePointSize = false;

    for(int i = 0; i < atts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!atts->IsSelected(i))
            {
                continue;
            }
        }

        QColor tempcolor;
        switch(i)
        {
        case ScatterAttributes::ID_var1Role:
            var1Role->blockSignals(true);
            var1Role->setCurrentIndex(atts->GetVar1Role());
            var1Role->blockSignals(false);
            rolesChanged = true;
            break;
        case ScatterAttributes::ID_var1MinFlag:
            var1Min->setEnabled(atts->GetVar1MinFlag());
            var1MinFlag->blockSignals(true);
            var1MinFlag->setChecked(atts->GetVar1MinFlag());
            var1MinFlag->blockSignals(false);
            break;
        case ScatterAttributes::ID_var1MaxFlag:
            var1Max->setEnabled(atts->GetVar1MaxFlag());
            var1MaxFlag->blockSignals(true);
            var1MaxFlag->setChecked(atts->GetVar1MaxFlag());
            var1MaxFlag->blockSignals(false);
            break;
        case ScatterAttributes::ID_var1Min:
            var1Min->setText(DoubleToQString(atts->GetVar1Min()));
            break;
        case ScatterAttributes::ID_var1Max:
            var1Max->setText(DoubleToQString(atts->GetVar1Max()));
            break;
        case ScatterAttributes::ID_var1Scaling:
            if (atts->GetVar1Scaling() == ScatterAttributes::Skew)
            {
                var1SkewFactor->setEnabled(true);
                var1SkewFactorLabel->setEnabled(true);
            }
            else
            {
                var1SkewFactor->setEnabled(false);
                var1SkewFactorLabel->setEnabled(false);
            }
            var1Scaling->blockSignals(true);
            var1Scaling->button(atts->GetVar1Scaling())->setChecked(true);
            var1Scaling->blockSignals(false);
            break;
        case ScatterAttributes::ID_var1SkewFactor:
            var1SkewFactor->setText(DoubleToQString(atts->GetVar1SkewFactor()));
            break;
        case ScatterAttributes::ID_var2Role:
            var2Role->blockSignals(true);
            var2Role->setCurrentIndex(atts->GetVar2Role());
            var2Role->blockSignals(false);
            rolesChanged = true;
            break;
        case ScatterAttributes::ID_var2:
            temp = atts->GetVar2().c_str();
            var2->setText(temp);
            varsChanged = true;
            break;
        case ScatterAttributes::ID_var2MinFlag:
            var2Min->setEnabled(atts->GetVar2MinFlag());
            var2MinFlag->blockSignals(true);
            var2MinFlag->setChecked(atts->GetVar2MinFlag());
            var2MinFlag->blockSignals(false);
            break;
        case ScatterAttributes::ID_var2MaxFlag:
            var2Max->setEnabled(atts->GetVar2MaxFlag());
            var2MaxFlag->blockSignals(true);
            var2MaxFlag->setChecked(atts->GetVar2MaxFlag());
            var2MaxFlag->blockSignals(false);
            break;
        case ScatterAttributes::ID_var2Min:
            var2Min->setText(DoubleToQString(atts->GetVar2Min()));
            break;
        case ScatterAttributes::ID_var2Max:
            var2Max->setText(DoubleToQString(atts->GetVar2Max()));
            break;
        case ScatterAttributes::ID_var2Scaling:
            if (atts->GetVar2Scaling() == ScatterAttributes::Skew)
            {
                var2SkewFactor->setEnabled(true);
                var2SkewFactorLabel->setEnabled(true);
            }
            else
            {
                var2SkewFactor->setEnabled(false);
                var2SkewFactorLabel->setEnabled(false);
            }
            var2Scaling->blockSignals(true);
            var2Scaling->button(atts->GetVar2Scaling())->setChecked(true);
            var2Scaling->blockSignals(false);
            break;
        case ScatterAttributes::ID_var2SkewFactor:
            var2SkewFactor->setText(DoubleToQString(atts->GetVar2SkewFactor()));
            break;
        case ScatterAttributes::ID_var3Role:
            var3Role->blockSignals(true);
            var3Role->setCurrentIndex(atts->GetVar3Role());
            var3Role->blockSignals(false);
            rolesChanged = true;
            break;
        case ScatterAttributes::ID_var3:
            temp = atts->GetVar3().c_str();
            var3->setText(temp);
            varsChanged = true;
            break;
        case ScatterAttributes::ID_var3MinFlag:
            var3Min->setEnabled(atts->GetVar3MinFlag());
            var3MinFlag->blockSignals(true);
            var3MinFlag->setChecked(atts->GetVar3MinFlag());
            var3MinFlag->blockSignals(false);
            break;
        case ScatterAttributes::ID_var3MaxFlag:
            var3Max->setEnabled(atts->GetVar3MaxFlag());
            var3MaxFlag->blockSignals(true);
            var3MaxFlag->setChecked(atts->GetVar3MaxFlag());
            var3MaxFlag->blockSignals(false);
            break;
        case ScatterAttributes::ID_var3Min:
            var3Min->setText(DoubleToQString(atts->GetVar3Min()));
            break;
        case ScatterAttributes::ID_var3Max:
            var3Max->setText(DoubleToQString(atts->GetVar3Max()));
            break;
        case ScatterAttributes::ID_var3Scaling:
            if (atts->GetVar3Scaling() == ScatterAttributes::Skew)
            {
                var3SkewFactor->setEnabled(true);
                var3SkewFactorLabel->setEnabled(true);
            }
            else
            {
                var3SkewFactor->setEnabled(false);
                var3SkewFactorLabel->setEnabled(false);
            }
            var3Scaling->blockSignals(true);
            var3Scaling->button(atts->GetVar3Scaling())->setChecked(true);
            var3Scaling->blockSignals(false);
            break;
        case ScatterAttributes::ID_var3SkewFactor:
            var3SkewFactor->setText(DoubleToQString(atts->GetVar3SkewFactor()));
            break;
        case ScatterAttributes::ID_var4Role:
            var4Role->blockSignals(true);
            var4Role->setCurrentIndex(atts->GetVar4Role());
            var4Role->blockSignals(false);
            rolesChanged = true;
            break;
        case ScatterAttributes::ID_var4:
            temp = atts->GetVar4().c_str();
            var4->setText(temp);
            varsChanged = true;
            break;
        case ScatterAttributes::ID_var4MinFlag:
            var4Min->setEnabled(atts->GetVar4MinFlag());
            var4MinFlag->blockSignals(true);
            var4MinFlag->setChecked(atts->GetVar4MinFlag());
            var4MinFlag->blockSignals(false);
            break;
        case ScatterAttributes::ID_var4MaxFlag:
            var4Max->setEnabled(atts->GetVar4MaxFlag());
            var4MaxFlag->blockSignals(true);
            var4MaxFlag->setChecked(atts->GetVar4MaxFlag());
            var4MaxFlag->blockSignals(false);
            break;
        case ScatterAttributes::ID_var4Min:
            var4Min->setText(DoubleToQString(atts->GetVar4Min()));
            break;
        case ScatterAttributes::ID_var4Max:
            var4Max->setText(DoubleToQString(atts->GetVar4Max()));
            break;
        case ScatterAttributes::ID_var4Scaling:
            if (atts->GetVar4Scaling() == ScatterAttributes::Skew)
            {
                var4SkewFactor->setEnabled(true);
                var4SkewFactorLabel->setEnabled(true);
            }
            else
            {
                var4SkewFactor->setEnabled(false);
                var4SkewFactorLabel->setEnabled(false);
            }
            var4Scaling->blockSignals(true);
            var4Scaling->button(atts->GetVar4Scaling())->setChecked(true);
            var4Scaling->blockSignals(false);
            break;
        case ScatterAttributes::ID_var4SkewFactor:
            var4SkewFactor->setText(DoubleToQString(atts->GetVar4SkewFactor()));
            break;
        case ScatterAttributes::ID_pointSize:
            updatePointSize = true;
            break;
        case ScatterAttributes::ID_pointSizePixels:
            updatePointSize = true;
            break;
        case ScatterAttributes::ID_pointType:
            pointType->blockSignals(true);
            pointType->setCurrentIndex(atts->GetPointType());
            pointType->blockSignals(false);
            updatePointSize = true;
            break;
        case ScatterAttributes::ID_scaleCube:
            scaleCube->blockSignals(true);
            scaleCube->setChecked(atts->GetScaleCube());
            scaleCube->blockSignals(false);
            break;
        case ScatterAttributes::ID_colorTableName:
            colorTableName->setColorTable(atts->GetColorTableName().c_str());
            break;
        case ScatterAttributes::ID_singleColor:
            tempcolor = QColor(atts->GetSingleColor().Red(),
                               atts->GetSingleColor().Green(),
                               atts->GetSingleColor().Blue());
            singleColor->setButtonColor(tempcolor);
            break;
        case ScatterAttributes::ID_foregroundFlag:
            if(singleColorLabel->isEnabled() == atts->GetForegroundFlag())
            {
                singleColorLabel->setEnabled(!atts->GetForegroundFlag());
                singleColor->setEnabled(!atts->GetForegroundFlag());
            }
            foregroundFlag->blockSignals(true);
            foregroundFlag->setChecked(atts->GetForegroundFlag());
            foregroundFlag->blockSignals(false);
            break;
        case ScatterAttributes::ID_legendFlag:
            legendFlag->blockSignals(true);
            legendFlag->setChecked(atts->GetLegendFlag());
            legendFlag->blockSignals(false);
            break;
        }
    }

    if(updatePointSize)
    {
        if(atts->GetPointType() != ScatterAttributes::Point &&
           atts->GetPointType() != ScatterAttributes::Sphere)
        {
            pointSize->setText(DoubleToQString(atts->GetPointSize()));
            pointSizeLabel->setText(tr("Point size"));
        }
        else
        {
            pointSize->setText(IntToQString(atts->GetPointSizePixels()));
            pointSizeLabel->setText(tr("Point size (pixels)"));
        }
    }

    //
    // If any of the roles changed, make sure that we update the
    // role labels.
    //
    if(rolesChanged || varsChanged)
    {
        QLabel *roleLabels[] = {xCoordRoleLabel, yCoordRoleLabel, 
            zCoordRoleLabel, colorRoleLabel, 0};
        for(int role = 0; role < 4; ++role)
        {
            QString label(roleNames[role]);
            label = label + ": ";

            QString roleVar;
            if(role == int(atts->GetVar1Role()))
                roleVar = "default";
            else if(role == int(atts->GetVar2Role()))
                roleVar = atts->GetVar2().c_str();
            else if(role == int(atts->GetVar3Role()))
                roleVar = atts->GetVar3().c_str();
            else if(role == int(atts->GetVar4Role()))
                roleVar = atts->GetVar4().c_str();

            bool haveVar = !roleVar.isEmpty();
            if(haveVar)
            {
                QString var(roleVar);

                // If there's a slash, take the variable name to the 
                // right of the slash.
                int index = var.lastIndexOf("/");
                bool tip = false;
                if(index != -1)
                {
                    var = var.right(var.length() - index - 1);
                    tip = true;
                }

                // If the variable name is too long, add ...
                if(var.length() > 10)
                {
                    var = var.left(7) + "...";
                    tip = true;
                }

                // Display a tooltip if needed.
                if(tip)
                    roleLabels[role]->setToolTip(roleVar);
                else
                    roleLabels[role]->setToolTip("");

                label = label + var;
            }

            roleLabels[role]->setText(label);
            roleLabels[role]->setEnabled(haveVar);
        }
    }
}


// ****************************************************************************
// Method: QvisScatterPlotWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 19 14:32:22 PST 2004
//
// Modifications:
//   Brad Whitlock, Wed Jul 20 15:27:52 PST 2005
//   I made the point size be read differently based on its type.
//
//   Brad Whitlock, Thu Aug 25 10:08:02 PDT 2005
//   Added the Sphere point type.
//
//   Brad Whitlock, Wed Apr 23 10:52:14 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Fri Aug  8 11:03:37 PDT 2008
//   Changed to new style.
//
// ****************************************************************************

void
QvisScatterPlotWindow::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);

    // Do var1Min
    if(which_widget == ScatterAttributes::ID_var1Min || doAll)
    {
        double val;
        if(LineEditGetDouble(var1Min, val))
            atts->SetVar1Min(val);
        else
        {
            ResettingError(tr("Variable 1 minimum"),
                DoubleToQString(atts->GetVar1Min()));
            atts->SetVar1Min(atts->GetVar1Min());
        }
    }

    // Do var1Max
    if(which_widget == ScatterAttributes::ID_var1Max || doAll)
    {
        double val;
        if(LineEditGetDouble(var1Max, val))
            atts->SetVar1Max(val);
        else
        {
            ResettingError(tr("Variable 1 max"),
                DoubleToQString(atts->GetVar1Max()));
            atts->SetVar1Max(atts->GetVar1Max());
        }
    }

    // Do var1SkewFactor
    if(which_widget == ScatterAttributes::ID_var1SkewFactor || doAll)
    {
        double val;
        if(LineEditGetDouble(var1SkewFactor, val))
            atts->SetVar1SkewFactor(val);
        else
        {
            ResettingError(tr("Variable 1 skew factor"),
                DoubleToQString(atts->GetVar1SkewFactor()));
            atts->SetVar1SkewFactor(atts->GetVar1SkewFactor());
        }
    }

    // Do var2Min
    if(which_widget == ScatterAttributes::ID_var2Min || doAll)
    {
        double val;
        if(LineEditGetDouble(var2Min, val))
            atts->SetVar2Min(val);
        else
        {
            ResettingError(tr("Variable 2 minimum"),
                DoubleToQString(atts->GetVar2Min()));
            atts->SetVar2Min(atts->GetVar2Min());
        }
    }

    // Do var2Max
    if(which_widget == ScatterAttributes::ID_var2Max || doAll)
    {
        double val;
        if(LineEditGetDouble(var2Max, val))
            atts->SetVar2Max(val);
        else
        {
            ResettingError(tr("Variable 2 maximum"),
                DoubleToQString(atts->GetVar2Max()));
            atts->SetVar2Max(atts->GetVar2Max());
        }
    }

    // Do var2SkewFactor
    if(which_widget == ScatterAttributes::ID_var2SkewFactor|| doAll)
    {
        double val;
        if(LineEditGetDouble(var2SkewFactor, val))
            atts->SetVar2SkewFactor(val);
        else
        {
            ResettingError(tr("Variable 2 skew factor"),
                DoubleToQString(atts->GetVar2SkewFactor()));
            atts->SetVar2SkewFactor(atts->GetVar2SkewFactor());
        }
    }

    // Do var3Min
    if(which_widget == ScatterAttributes::ID_var2Min || doAll)
    {
        double val;
        if(LineEditGetDouble(var3Min, val))
            atts->SetVar3Min(val);
        else
        {
            ResettingError(tr("Variable 3 minimum"),
                DoubleToQString(atts->GetVar3Min()));
            atts->SetVar3Min(atts->GetVar3Min());
        }
    }

    // Do var3Max
    if(which_widget == ScatterAttributes::ID_var3Max || doAll)
    {
        double val;
        if(LineEditGetDouble(var3Max, val))
            atts->SetVar3Max(val);
        else
        {
            ResettingError(tr("Variable 3 maximum"),
                DoubleToQString(atts->GetVar3Max()));
            atts->SetVar3Max(atts->GetVar3Max());
        }
    }

    // Do var3SkewFactor
    if(which_widget == ScatterAttributes::ID_var2SkewFactor || doAll)
    {
        double val;
        if(LineEditGetDouble(var3SkewFactor, val))
            atts->SetVar3SkewFactor(val);
        else
        {
            ResettingError(tr("Variable 3 skew factor"),
                DoubleToQString(atts->GetVar3SkewFactor()));
            atts->SetVar3SkewFactor(atts->GetVar3SkewFactor());
        }
    }

    // Do var4Min
    if(which_widget == ScatterAttributes::ID_var4Min || doAll)
    {
        double val;
        if(LineEditGetDouble(var4Min, val))
            atts->SetVar4Min(val);
        else
        {
            ResettingError(tr("Variable 4 minimum"),
                DoubleToQString(atts->GetVar4Min()));
            atts->SetVar4Min(atts->GetVar4Min());
        }
    }

    // Do var4Max
    if(which_widget == ScatterAttributes::ID_var4Max || doAll)
    {
        double val;
        if(LineEditGetDouble(var4Max, val))
            atts->SetVar4Max(val);
        else
        {
            ResettingError(tr("Variable 4 maximum"),
                DoubleToQString(atts->GetVar4Max()));
            atts->SetVar4Max(atts->GetVar4Max());
        }
    }

    // Do var4SkewFactor
    if(which_widget == ScatterAttributes::ID_var4SkewFactor || doAll)
    {
        double val;
        if(LineEditGetDouble(var4SkewFactor, val))
            atts->SetVar4SkewFactor(val);
        else
        {
            ResettingError(tr("Variable 4 skew factor"),
                DoubleToQString(atts->GetVar4SkewFactor()));
            atts->SetVar4SkewFactor(atts->GetVar4SkewFactor());
        }
    }

    // Do pointSize
    if(which_widget == ScatterAttributes::ID_pointSize || doAll)
    {
        if(atts->GetPointType() == ScatterAttributes::Point ||
           atts->GetPointType() == ScatterAttributes::Sphere)
        {
            int val;
            if(LineEditGetInt(pointSize, val))
                atts->SetPointSizePixels(val);
            else
            {
                ResettingError(tr("point size (pixels)"),
                    IntToQString(atts->GetPointSizePixels()));
                atts->SetPointSizePixels(atts->GetPointSizePixels());
            }
        }
        else
        {
            double val;
            if(LineEditGetDouble(pointSize, val))
                atts->SetPointSize(val);
            else
            {
                ResettingError(tr("point size"),
                    DoubleToQString(atts->GetPointSize()));
                atts->SetPointSize(atts->GetPointSize());
            }
        }
    }
}


// ****************************************************************************
// Method: QvisScatterPlotWindow::Apply
//
// Purpose: 
//   Called to apply changes in the subject.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 19 14:32:22 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisScatterPlotWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        GetCurrentValues(-1);
        atts->Notify();

        GetViewerMethods()->SetPlotOptions(plotType);
    }
    else
        atts->Notify();
}

// ****************************************************************************
// Method: QvisScatterPlotWindow::EnsureUniqueRole
//
// Purpose: 
//   Ensures that only a single input has a given variable role.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 14 10:05:25 PDT 2004
//
// Modifications:
//   Brad Whitlock, Wed Apr 23 10:58:01 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

void
QvisScatterPlotWindow::EnsureUniqueRole(int mask, int val, const char *var)
{
    ScatterAttributes::VariableRole role = ScatterAttributes::VariableRole(val);

    // We don't need to complain if we're setting more than 1 variable to
    // the None role so exit early.
    if(role == ScatterAttributes::None)
        return;

    bool changed = false;
    if((mask & 1) && atts->GetVar1Role() == role)
    {
        atts->SetVar1Role(ScatterAttributes::None);
        changed = true;
    }
    if((mask & 2) && atts->GetVar2Role() == role)
    {
        atts->SetVar2Role(ScatterAttributes::None);
        changed = true;
    }
    if((mask & 4) && atts->GetVar3Role() == role)
    {
        atts->SetVar3Role(ScatterAttributes::None);
        changed = true;
    }
    if((mask & 8) && atts->GetVar4Role() == role)
    {
        atts->SetVar4Role(ScatterAttributes::None);
        changed = true;
    }
    if(changed)
    {
        QString msg;
        msg = tr("The %1 role was being fulfilled by another input "
                 "variable. To ensure that the %2 role is fulfilled by "
                 "the %3 variable, the other variable's role has been "
                 "set to None.").arg(roleNames[val]).arg(roleNames[val]).arg(var);

        Warning(msg);
    }
}

//
// Qt Slot functions
//


// ****************************************************************************
// Method: QvisScatterPlotWindow::apply
//
// Purpose: 
//   Qt slot function called when apply button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 19 14:32:22 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisScatterPlotWindow::apply()
{
    Apply(true);
}


// ****************************************************************************
// Method: QvisScatterPlotWindow::makeDefault
//
// Purpose: 
//   Qt slot function called when "Make default" button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 19 14:32:22 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisScatterPlotWindow::makeDefault()
{
    GetCurrentValues(-1);
    atts->Notify();
    GetViewerMethods()->SetDefaultPlotOptions(plotType);
}


// ****************************************************************************
// Method: QvisScatterPlotWindow::reset
//
// Purpose: 
//   Qt slot function called when reset button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 19 14:32:22 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisScatterPlotWindow::reset()
{
    GetViewerMethods()->ResetPlotOptions(plotType);
}


void
QvisScatterPlotWindow::var1RoleChanged(int val)
{
    if(val != atts->GetVar1Role())
    {
        EnsureUniqueRole(14, val, "default");
        atts->SetVar1Role(ScatterAttributes::VariableRole(val));
        Apply();
    }
}


void
QvisScatterPlotWindow::var1MinFlagChanged(bool val)
{
    atts->SetVar1MinFlag(val);
    Apply();
}


void
QvisScatterPlotWindow::var1MaxFlagChanged(bool val)
{
    atts->SetVar1MaxFlag(val);
    Apply();
}


void
QvisScatterPlotWindow::var1MinProcessText()
{
    GetCurrentValues(ScatterAttributes::ID_var1Min);
    Apply();
}


void
QvisScatterPlotWindow::var1MaxProcessText()
{
    GetCurrentValues(ScatterAttributes::ID_var1Max);
    Apply();
}


void
QvisScatterPlotWindow::var1ScalingChanged(int val)
{
    if(val != atts->GetVar1Scaling())
    {
        atts->SetVar1Scaling(ScatterAttributes::Scaling(val));
        Apply();
    }
}


void
QvisScatterPlotWindow::var1SkewFactorProcessText()
{
    GetCurrentValues(ScatterAttributes::ID_var1SkewFactor);
    Apply();
}


void
QvisScatterPlotWindow::var2RoleChanged(int val)
{
    if(val != atts->GetVar2Role())
    {
        EnsureUniqueRole(13, val, atts->GetVar2().c_str());
        atts->SetVar2Role(ScatterAttributes::VariableRole(val));
        Apply();
    }
}

void
QvisScatterPlotWindow::var2Selected(const QString &var)
{
    atts->SetVar2(var.toStdString());
    Apply();
}

void
QvisScatterPlotWindow::var2MinFlagChanged(bool val)
{
    atts->SetVar2MinFlag(val);
    Apply();
}


void
QvisScatterPlotWindow::var2MaxFlagChanged(bool val)
{
    atts->SetVar2MaxFlag(val);
    Apply();
}


void
QvisScatterPlotWindow::var2MinProcessText()
{
    GetCurrentValues(ScatterAttributes::ID_var2Min);
    Apply();
}


void
QvisScatterPlotWindow::var2MaxProcessText()
{
    GetCurrentValues(ScatterAttributes::ID_var2Max);
    Apply();
}


void
QvisScatterPlotWindow::var2ScalingChanged(int val)
{
    if(val != atts->GetVar2Scaling())
    {
        atts->SetVar2Scaling(ScatterAttributes::Scaling(val));
        Apply();
    }
}


void
QvisScatterPlotWindow::var2SkewFactorProcessText()
{
    GetCurrentValues(ScatterAttributes::ID_var2SkewFactor);
    Apply();
}


void
QvisScatterPlotWindow::var3RoleChanged(int val)
{
    if(val != atts->GetVar3Role())
    {
        EnsureUniqueRole(11, val, atts->GetVar3().c_str());
        atts->SetVar3Role(ScatterAttributes::VariableRole(val));
        Apply();
    }
}

void
QvisScatterPlotWindow::var3Selected(const QString &var)
{
    atts->SetVar3(var.toStdString());
    Apply();
}

void
QvisScatterPlotWindow::var3MinFlagChanged(bool val)
{
    atts->SetVar3MinFlag(val);
    Apply();
}


void
QvisScatterPlotWindow::var3MaxFlagChanged(bool val)
{
    atts->SetVar3MaxFlag(val);
    Apply();
}


void
QvisScatterPlotWindow::var3MinProcessText()
{
    GetCurrentValues(ScatterAttributes::ID_var3Min);
    Apply();
}


void
QvisScatterPlotWindow::var3MaxProcessText()
{
    GetCurrentValues(ScatterAttributes::ID_var3Max);
    Apply();
}


void
QvisScatterPlotWindow::var3ScalingChanged(int val)
{
    if(val != atts->GetVar3Scaling())
    {
        atts->SetVar3Scaling(ScatterAttributes::Scaling(val));
        Apply();
    }
}


void
QvisScatterPlotWindow::var3SkewFactorProcessText()
{
    GetCurrentValues(ScatterAttributes::ID_var3SkewFactor);
    Apply();
}


void
QvisScatterPlotWindow::var4RoleChanged(int val)
{
    if(val != atts->GetVar4Role())
    {
        EnsureUniqueRole(7, val, atts->GetVar4().c_str());
        atts->SetVar4Role(ScatterAttributes::VariableRole(val));
        Apply();
    }
}

void
QvisScatterPlotWindow::var4Selected(const QString &var)
{
    atts->SetVar4(var.toStdString());
    Apply();
}

void
QvisScatterPlotWindow::var4MinFlagChanged(bool val)
{
    atts->SetVar4MinFlag(val);
    Apply();
}


void
QvisScatterPlotWindow::var4MaxFlagChanged(bool val)
{
    atts->SetVar4MaxFlag(val);
    Apply();
}


void
QvisScatterPlotWindow::var4MinProcessText()
{
    GetCurrentValues(ScatterAttributes::ID_var4Min);
    Apply();
}


void
QvisScatterPlotWindow::var4MaxProcessText()
{
    GetCurrentValues(ScatterAttributes::ID_var4Max);
    Apply();
}


void
QvisScatterPlotWindow::var4ScalingChanged(int val)
{
    if(val != atts->GetVar4Scaling())
    {
        atts->SetVar4Scaling(ScatterAttributes::Scaling(val));
        Apply();
    }
}


void
QvisScatterPlotWindow::var4SkewFactorProcessText()
{
    GetCurrentValues(ScatterAttributes::ID_var4SkewFactor);
    Apply();
}


void
QvisScatterPlotWindow::pointSizeProcessText()
{
    GetCurrentValues(ScatterAttributes::ID_pointSize);
    Apply();
}

// ****************************************************************************
// Method: QvisScatterPlotWindow::pointTypeChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the point type widget
//   is clicked.
//
// Arguments:
//   val : The new point type.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 20 15:24:09 PST 2005
//
// Modifications:
//   Brad Whitlock, Wed Jul 20 15:24:47 PST 2005
//   I changed the method so we get the current values for the point size
//   before changing the point type. We also update the window since changing
//   the point type will change the point size widgets.
//
// ****************************************************************************

void
QvisScatterPlotWindow::pointTypeChanged(int val)
{
    if(val != atts->GetPointType())
    {
        GetCurrentValues(ScatterAttributes::ID_pointSize);
        atts->SetPointType(ScatterAttributes::PointType(val));
        Apply();
    }
}


void
QvisScatterPlotWindow::scaleCubeChanged(bool val)
{
    atts->SetScaleCube(val);
    SetUpdate(false);
    Apply();
}


void
QvisScatterPlotWindow::colorTableNameChanged(bool useDefault, const QString &ctName)
{
    atts->SetColorTableName(ctName.toStdString());
    SetUpdate(false);
    Apply();
}


void
QvisScatterPlotWindow::singleColorChanged(const QColor &color)
{
    ColorAttribute temp(color.red(), color.green(), color.blue());
    atts->SetSingleColor(temp);
    SetUpdate(false);
    Apply();
}


void
QvisScatterPlotWindow::foregroundFlagChanged(bool val)
{
    atts->SetForegroundFlag(val);
    Apply();
}


void
QvisScatterPlotWindow::legendFlagChanged(bool val)
{
    atts->SetLegendFlag(val);
    SetUpdate(false);
    Apply();
}


