/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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

#include "QvisScatterPlotWindow.h"

#include <ScatterAttributes.h>
#include <ViewerProxy.h>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qtabwidget.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qtooltip.h>
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
    "X coordinate", "Y coordinate", "Z coordinate", "Color", "None"};

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
//   
// ****************************************************************************

QvisScatterPlotWindow::QvisScatterPlotWindow(const int type,
                         ScatterAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisPostableWindowObserver(subj, caption, shortName, notepad)
{
    plotType = type;
    atts = subj;
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
// ****************************************************************************

void
QvisScatterPlotWindow::CreateWindowContents()
{
    int i;

    //
    // Create a tab widget so we can split up inputs and appearance.
    // 
    QTabWidget *topTab = new QTabWidget(central, "topTab");
    topLayout->addWidget(topTab);

    //
    // Create a tab widget so we can split up the different inputs.
    //
    QGroupBox *inputGroup = new QGroupBox(central, "inputGroup");
    inputGroup->setFrameStyle(QFrame::NoFrame);
    topTab->addTab(inputGroup, "Inputs");
    QVBoxLayout *inputVeryTopLayout = new QVBoxLayout(inputGroup);
    inputVeryTopLayout->setMargin(10);
    inputVeryTopLayout->setSpacing(5);
    QTabWidget *tabs = new QTabWidget(inputGroup, "tabs");
    inputVeryTopLayout->addWidget(tabs);

    //
    // Create a tab for the var1
    //
    QGroupBox *var1Top = new QGroupBox(inputGroup, "var1Top");
    var1Top->setFrameStyle(QFrame::NoFrame);
    tabs->addTab(var1Top, "Input 1");
    QVBoxLayout *var1VeryTopLayout = new QVBoxLayout(var1Top);
    var1VeryTopLayout->setMargin(10);
    var1VeryTopLayout->setSpacing(5);
    QGridLayout *var1TopLayout = new QGridLayout(var1VeryTopLayout,
        6, 2, 10, "var1TopLayout");

    QLabel *var1Label = new QLabel("Variable", var1Top, "var1Label");
    var1Label->setEnabled(false);
    var1TopLayout->addWidget(var1Label,0,0);
    QLineEdit *var1 = new QLineEdit(var1Top, "var1");
    var1->setText("default");
    var1->setEnabled(false);
//    connect(var1, SIGNAL(returnPressed()),
//            this, SLOT(var1ProcessText()));
    var1TopLayout->addWidget(var1, 0,1);

    var1Role = new QComboBox(var1Top, "var1Role");
    for(i = 0; i < 5; ++i)
        var1Role->insertItem(roleNames[i]);
    connect(var1Role, SIGNAL(activated(int)),
            this, SLOT(var1RoleChanged(int)));
    var1TopLayout->addWidget(var1Role, 1,1);
    QLabel *var1RoleLabel = new QLabel(var1Role, "Role", var1Top, "var1RoleLabel");
    var1TopLayout->addWidget(var1RoleLabel,1,0);

    // var1 Min
    var1MinFlag = new QCheckBox("Min", var1Top, "var1MinFlag");
    connect(var1MinFlag, SIGNAL(toggled(bool)),
            this, SLOT(var1MinFlagChanged(bool)));
    var1TopLayout->addWidget(var1MinFlag, 2,0);
    var1Min = new QLineEdit(var1Top, "var1Min");
    connect(var1Min, SIGNAL(returnPressed()),
            this, SLOT(var1MinProcessText()));
    var1TopLayout->addWidget(var1Min, 2,1);

    // var1 Max
    var1MaxFlag = new QCheckBox("Max", var1Top, "var1MaxFlag");
    connect(var1MaxFlag, SIGNAL(toggled(bool)),
            this, SLOT(var1MaxFlagChanged(bool)));
    var1TopLayout->addWidget(var1MaxFlag, 3,0);
    var1Max = new QLineEdit(var1Top, "var1Max");
    connect(var1Max, SIGNAL(returnPressed()),
            this, SLOT(var1MaxProcessText()));
    var1TopLayout->addWidget(var1Max, 3,1);

    var1ScalingLabel = new QLabel("Scale", var1Top, "var1ScalingLabel");
    var1TopLayout->addWidget(var1ScalingLabel,4,0);
    var1Scaling = new QButtonGroup(var1Top, "var1Scaling");
    var1Scaling->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *var1ScalingLayout = new QHBoxLayout(var1Scaling);
    var1ScalingLayout->setSpacing(10);
    QRadioButton *var1ScalingScalingLinear = new QRadioButton("Linear", var1Scaling);
    var1ScalingLayout->addWidget(var1ScalingScalingLinear);
    QRadioButton *var1ScalingScalingLog = new QRadioButton("Log", var1Scaling);
    var1ScalingLayout->addWidget(var1ScalingScalingLog);
    QRadioButton *var1ScalingScalingSkew = new QRadioButton("Skew", var1Scaling);
    var1ScalingLayout->addWidget(var1ScalingScalingSkew);
    connect(var1Scaling, SIGNAL(clicked(int)),
            this, SLOT(var1ScalingChanged(int)));
    var1TopLayout->addWidget(var1Scaling, 4,1);

    var1SkewFactorLabel = new QLabel("Skew factor", var1Top, "var1SkewFactorLabel");
    var1TopLayout->addWidget(var1SkewFactorLabel,5,0);
    var1SkewFactor = new QLineEdit(var1Top, "var1SkewFactor");
    connect(var1SkewFactor, SIGNAL(returnPressed()),
            this, SLOT(var1SkewFactorProcessText()));
    var1TopLayout->addWidget(var1SkewFactor, 5,1);


    //
    // Create a tab for the var2
    //
    QGroupBox *var2Top = new QGroupBox(inputGroup, "var2Top");
    var2Top->setFrameStyle(QFrame::NoFrame);
    tabs->addTab(var2Top, "Input 2");
    QVBoxLayout *var2VeryTopLayout = new QVBoxLayout(var2Top);
    var2VeryTopLayout->setMargin(10);
    var2VeryTopLayout->setSpacing(5);
    QGridLayout *var2TopLayout = new QGridLayout(var2VeryTopLayout,
        6, 2, 10, "var2TopLayout");

    var2Label = new QLabel("Variable", var2Top, "var2Label");
    var2TopLayout->addWidget(var2Label,0,0);

    // Create a variable button
    var2 = new QvisVariableButton(true, true, true, QvisVariableButton::Scalars,
        var2Top, "var2");
    connect(var2, SIGNAL(activated(const QString &)),
            this, SLOT(var2Selected(const QString &)));
    var2TopLayout->addWidget(var2, 0,1);

    var2Role = new QComboBox(var2Top, "var2Role");
    for(i = 0; i < 5; ++i)
        var2Role->insertItem(roleNames[i]);
    connect(var2Role, SIGNAL(activated(int)),
            this, SLOT(var2RoleChanged(int)));
    var2TopLayout->addWidget(var2Role, 1,1);
    QLabel *var2RoleLabel = new QLabel(var2Role, "Role", var2Top, "var2RoleLabel");
    var2TopLayout->addWidget(var2RoleLabel,1,0);

    // var2 Min
    var2MinFlag = new QCheckBox("Min", var2Top, "var2MinFlag");
    connect(var2MinFlag, SIGNAL(toggled(bool)),
            this, SLOT(var2MinFlagChanged(bool)));
    var2TopLayout->addWidget(var2MinFlag, 2,0);
    var2Min = new QLineEdit(var2Top, "var2Min");
    connect(var2Min, SIGNAL(returnPressed()),
            this, SLOT(var2MinProcessText()));
    var2TopLayout->addWidget(var2Min, 2,1);

    // var2 Max
    var2MaxFlag = new QCheckBox("Max", var2Top, "var2MaxFlag");
    connect(var2MaxFlag, SIGNAL(toggled(bool)),
            this, SLOT(var2MaxFlagChanged(bool)));
    var2TopLayout->addWidget(var2MaxFlag, 3,0);
    var2Max = new QLineEdit(var2Top, "var2Max");
    connect(var2Max, SIGNAL(returnPressed()),
            this, SLOT(var2MaxProcessText()));
    var2TopLayout->addWidget(var2Max, 3,1);

    var2ScalingLabel = new QLabel("Scale", var2Top, "var2ScalingLabel");
    var2TopLayout->addWidget(var2ScalingLabel,4,0);
    var2Scaling = new QButtonGroup(var2Top, "var2Scaling");
    var2Scaling->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *var2ScalingLayout = new QHBoxLayout(var2Scaling);
    var2ScalingLayout->setSpacing(10);
    QRadioButton *var2ScalingScalingLinear = new QRadioButton("Linear", var2Scaling);
    var2ScalingLayout->addWidget(var2ScalingScalingLinear);
    QRadioButton *var2ScalingScalingLog = new QRadioButton("Log", var2Scaling);
    var2ScalingLayout->addWidget(var2ScalingScalingLog);
    QRadioButton *var2ScalingScalingSkew = new QRadioButton("Skew", var2Scaling);
    var2ScalingLayout->addWidget(var2ScalingScalingSkew);
    connect(var2Scaling, SIGNAL(clicked(int)),
            this, SLOT(var2ScalingChanged(int)));
    var2TopLayout->addWidget(var2Scaling, 4,1);

    var2SkewFactorLabel = new QLabel("Skew factor", var2Top, "var2SkewFactorLabel");
    var2TopLayout->addWidget(var2SkewFactorLabel,5,0);
    var2SkewFactor = new QLineEdit(var2Top, "var2SkewFactor");
    connect(var2SkewFactor, SIGNAL(returnPressed()),
            this, SLOT(var2SkewFactorProcessText()));
    var2TopLayout->addWidget(var2SkewFactor, 5,1);

    //
    // Create a tab for the var3
    //
    QGroupBox *var3Top = new QGroupBox(inputGroup, "var3Top");
    var3Top->setFrameStyle(QFrame::NoFrame);
    tabs->addTab(var3Top, "Input 3");
    QVBoxLayout *var3VeryTopLayout = new QVBoxLayout(var3Top);
    var3VeryTopLayout->setMargin(10);
    var3VeryTopLayout->setSpacing(5);
    QGridLayout *var3TopLayout = new QGridLayout(var3VeryTopLayout,
        6, 2, 10, "var3TopLayout");

    var3Label = new QLabel("Variable", var3Top, "var3Label");
    var3TopLayout->addWidget(var3Label,0,0);

    // Create a variable button.
    var3 = new QvisVariableButton(true, true, true, QvisVariableButton::Scalars,
        var3Top, "var3");
    connect(var3, SIGNAL(activated(const QString &)),
            this, SLOT(var3Selected(const QString &)));
    var3TopLayout->addWidget(var3, 0,1);

    var3Role = new QComboBox(var3Top, "var3Role");
    for(i = 0; i < 5; ++i)
        var3Role->insertItem(roleNames[i]);
    connect(var3Role, SIGNAL(activated(int)),
            this, SLOT(var3RoleChanged(int)));
    var3TopLayout->addWidget(var3Role, 1,1);
    QLabel *var3RoleLabel = new QLabel(var3Role, "Role", var3Top, "var3RoleLabel");
    var3TopLayout->addWidget(var3RoleLabel,1,0);

    // var3 Min
    var3MinFlag = new QCheckBox("Min", var3Top, "var3MinFlag");
    connect(var3MinFlag, SIGNAL(toggled(bool)),
            this, SLOT(var3MinFlagChanged(bool)));
    var3TopLayout->addWidget(var3MinFlag, 2,0);
    var3Min = new QLineEdit(var3Top, "var3Min");
    connect(var3Min, SIGNAL(returnPressed()),
            this, SLOT(var3MinProcessText()));
    var3TopLayout->addWidget(var3Min, 2,1);

    // var3 Max
    var3MaxFlag = new QCheckBox("Max", var3Top, "var3MaxFlag");
    connect(var3MaxFlag, SIGNAL(toggled(bool)),
            this, SLOT(var3MaxFlagChanged(bool)));
    var3TopLayout->addWidget(var3MaxFlag, 3,0);
    var3Max = new QLineEdit(var3Top, "var3Max");
    connect(var3Max, SIGNAL(returnPressed()),
            this, SLOT(var3MaxProcessText()));
    var3TopLayout->addWidget(var3Max, 3,1);

    var3ScalingLabel = new QLabel("Scale", var3Top, "var3ScalingLabel");
    var3TopLayout->addWidget(var3ScalingLabel,4,0);
    var3Scaling = new QButtonGroup(var3Top, "var3Scaling");
    var3Scaling->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *var3ScalingLayout = new QHBoxLayout(var3Scaling);
    var3ScalingLayout->setSpacing(10);
    QRadioButton *var3ScalingScalingLinear = new QRadioButton("Linear", var3Scaling);
    var3ScalingLayout->addWidget(var3ScalingScalingLinear);
    QRadioButton *var3ScalingScalingLog = new QRadioButton("Log", var3Scaling);
    var3ScalingLayout->addWidget(var3ScalingScalingLog);
    QRadioButton *var3ScalingScalingSkew = new QRadioButton("Skew", var3Scaling);
    var3ScalingLayout->addWidget(var3ScalingScalingSkew);
    connect(var3Scaling, SIGNAL(clicked(int)),
            this, SLOT(var3ScalingChanged(int)));
    var3TopLayout->addWidget(var3Scaling, 4,1);

    var3SkewFactorLabel = new QLabel("Skew factor", var3Top, "var3SkewFactorLabel");
    var3TopLayout->addWidget(var3SkewFactorLabel,5,0);
    var3SkewFactor = new QLineEdit(var3Top, "var3SkewFactor");
    connect(var3SkewFactor, SIGNAL(returnPressed()),
            this, SLOT(var3SkewFactorProcessText()));
    var3TopLayout->addWidget(var3SkewFactor, 5,1);


    //
    // Create a tab for the var4
    //
    QGroupBox *var4Top = new QGroupBox(inputGroup, "var4Top");
    var4Top->setFrameStyle(QFrame::NoFrame);
    tabs->addTab(var4Top, "Input 4");
    QVBoxLayout *var4VeryTopLayout = new QVBoxLayout(var4Top);
    var4VeryTopLayout->setMargin(10);
    var4VeryTopLayout->setSpacing(5);
    QGridLayout *var4TopLayout = new QGridLayout(var4VeryTopLayout,
        6, 2, 10, "var4TopLayout");

    var4Label = new QLabel("Variable", var4Top, "var4Label");
    var4TopLayout->addWidget(var4Label,0,0);

    // Create a variable button.
    var4 = new QvisVariableButton(true, true, true, QvisVariableButton::Scalars,
        var4Top, "var4");
    connect(var4, SIGNAL(activated(const QString &)),
            this, SLOT(var4Selected(const QString &)));
    var4TopLayout->addWidget(var4, 0,1);

    var4Role = new QComboBox(var4Top, "var4Role");
    for(i = 0; i < 5; ++i)
        var4Role->insertItem(roleNames[i]);
    connect(var4Role, SIGNAL(activated(int)),
            this, SLOT(var4RoleChanged(int)));
    var4TopLayout->addWidget(var4Role, 1,1);
    QLabel *var4RoleLabel = new QLabel(var4Role, "Role", var4Top, "var4RoleLabel");
    var4TopLayout->addWidget(var4RoleLabel,1,0);

    // var4 Min
    var4MinFlag = new QCheckBox("Min", var4Top, "var4MinFlag");
    connect(var4MinFlag, SIGNAL(toggled(bool)),
            this, SLOT(var4MinFlagChanged(bool)));
    var4TopLayout->addWidget(var4MinFlag, 2,0);
    var4Min = new QLineEdit(var4Top, "var4Min");
    connect(var4Min, SIGNAL(returnPressed()),
            this, SLOT(var4MinProcessText()));
    var4TopLayout->addWidget(var4Min, 2,1);

    // var4 Max
    var4MaxFlag = new QCheckBox("Max", var4Top, "var4MaxFlag");
    connect(var4MaxFlag, SIGNAL(toggled(bool)),
            this, SLOT(var4MaxFlagChanged(bool)));
    var4TopLayout->addWidget(var4MaxFlag, 3,0);
    var4Max = new QLineEdit(var4Top, "var4Max");
    connect(var4Max, SIGNAL(returnPressed()),
            this, SLOT(var4MaxProcessText()));
    var4TopLayout->addWidget(var4Max, 3,1);

    var4ScalingLabel = new QLabel("Scale", var4Top, "var4ScalingLabel");
    var4TopLayout->addWidget(var4ScalingLabel,4,0);
    var4Scaling = new QButtonGroup(var4Top, "var4Scaling");
    var4Scaling->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *var4ScalingLayout = new QHBoxLayout(var4Scaling);
    var4ScalingLayout->setSpacing(10);
    QRadioButton *var4ScalingScalingLinear = new QRadioButton("Linear", var4Scaling);
    var4ScalingLayout->addWidget(var4ScalingScalingLinear);
    QRadioButton *var4ScalingScalingLog = new QRadioButton("Log", var4Scaling);
    var4ScalingLayout->addWidget(var4ScalingScalingLog);
    QRadioButton *var4ScalingScalingSkew = new QRadioButton("Skew", var4Scaling);
    var4ScalingLayout->addWidget(var4ScalingScalingSkew);
    connect(var4Scaling, SIGNAL(clicked(int)),
            this, SLOT(var4ScalingChanged(int)));
    var4TopLayout->addWidget(var4Scaling, 4,1);

    var4SkewFactorLabel = new QLabel("Skew factor", var4Top, "var4SkewFactorLabel");
    var4TopLayout->addWidget(var4SkewFactorLabel,5,0);
    var4SkewFactor = new QLineEdit(var4Top, "var4SkewFactor");
    connect(var4SkewFactor, SIGNAL(returnPressed()),
            this, SLOT(var4SkewFactorProcessText()));
    var4TopLayout->addWidget(var4SkewFactor, 5,1);

    //
    // Appearance widgets
    //
    QGroupBox *appearanceGroup = new QGroupBox(central, "appearanceGroup");
    appearanceGroup->setFrameStyle(QFrame::NoFrame);
    topTab->addTab(appearanceGroup, "Appearance");
    QVBoxLayout *aTopLayout = new QVBoxLayout(appearanceGroup, 10, 2);
    aTopLayout->addSpacing(10);
    QGridLayout *aLayout = new QGridLayout(aTopLayout, 5, 2);
    aLayout->setSpacing(10);
    aTopLayout->addStretch(10);

    pointSize = new QLineEdit(appearanceGroup, "pointSize");
    connect(pointSize, SIGNAL(returnPressed()),
            this, SLOT(pointSizeProcessText()));
    aLayout->addWidget(pointSize, 0, 1);
    pointSizeLabel = new QLabel(pointSize, "Point size",
        appearanceGroup, "pointSizeLabel");
    aLayout->addWidget(pointSizeLabel, 0, 0);

    pointType = new QComboBox(appearanceGroup, "pointType");
    pointType->insertItem("Box");
    pointType->insertItem("Axis");
    pointType->insertItem("Icosahedron");
    pointType->insertItem("Point");
    pointType->insertItem("Sphere");
    connect(pointType, SIGNAL(activated(int)),
            this, SLOT(pointTypeChanged(int)));
    aLayout->addWidget(pointType, 1, 1);
    QLabel *pointTypeLabel = new QLabel(pointType, "Point Type",
        appearanceGroup, "pointTypeLabel");
    aLayout->addWidget(pointTypeLabel, 1, 0);

    colorTableName = new QvisColorTableButton(appearanceGroup, "colorTableName");
    connect(colorTableName, SIGNAL(selectedColorTable(bool, const QString&)),
            this, SLOT(colorTableNameChanged(bool, const QString&)));
    aLayout->addWidget(colorTableName, 2, 1);
    QLabel *colorTableNameLabel = new QLabel(colorTableName, "Color table",
        appearanceGroup, "colorTableNameLabel");
    aLayout->addWidget(colorTableNameLabel, 2, 0);

    QHBox *scBox = new QHBox(appearanceGroup, "scBox");
    singleColor = new QvisColorButton(scBox, "singleColor");
    connect(singleColor, SIGNAL(selectedColor(const QColor&)),
            this, SLOT(singleColorChanged(const QColor&)));

    singleColorLabel = new QLabel(singleColor, "Single color",
        appearanceGroup, "singleColorLabel");

    scBox->setSpacing(10);
    foregroundFlag = new QCheckBox("Use foreground ", scBox, "foregroundFlag");
    connect(foregroundFlag, SIGNAL(toggled(bool)),
            this, SLOT(foregroundFlagChanged(bool)));
    aLayout->addWidget(scBox, 3, 1);
    aLayout->addWidget(singleColorLabel, 3, 0);
//    aLayout->addWidget(foregroundFlag);

    scaleCube = new QCheckBox("Scale to cube", appearanceGroup, "scaleCube");
    connect(scaleCube, SIGNAL(toggled(bool)),
            this, SLOT(scaleCubeChanged(bool)));
    aLayout->addMultiCellWidget(scaleCube, 4, 4, 0, 1);

    //
    // Role labels.
    //
    QGroupBox *roleGroup = new QGroupBox(central, "roleGroup");
    roleGroup->setTitle("Roles");
    topLayout->addWidget(roleGroup);
    QVBoxLayout *roleTopLayout = new QVBoxLayout(roleGroup, 10, 2);
    roleTopLayout->addSpacing(10);
    QGridLayout *roleLayout = new QGridLayout(roleTopLayout, 2, 2);
    roleLayout->setSpacing(5);
    xCoordRoleLabel = new QLabel("X coordinate:    ", roleGroup, "xCoordRoleLabel");
    roleLayout->addWidget(xCoordRoleLabel, 0, 0);
    yCoordRoleLabel = new QLabel("Y coordinate:    ", roleGroup, "yCoordRoleLabel");
    roleLayout->addWidget(yCoordRoleLabel, 1, 0);
    zCoordRoleLabel = new QLabel("Z coordinate:    ", roleGroup, "zCoordRoleLabel");
    roleLayout->addWidget(zCoordRoleLabel, 0, 1);
    colorRoleLabel = new QLabel("Color:    ", roleGroup, "colorRoleLabel");
    roleLayout->addWidget(colorRoleLabel, 1, 1);



    legendFlag = new QCheckBox("Legend", central, "legendFlag");
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
            var1Role->setCurrentItem(atts->GetVar1Role());
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
            temp.sprintf("%g", atts->GetVar1Min());
            var1Min->setText(temp);
            break;
        case ScatterAttributes::ID_var1Max:
            temp.sprintf("%g", atts->GetVar1Max());
            var1Max->setText(temp);
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
            var1Scaling->setButton(atts->GetVar1Scaling());
            var1Scaling->blockSignals(false);
            break;
        case ScatterAttributes::ID_var1SkewFactor:
            temp.sprintf("%g", atts->GetVar1SkewFactor());
            var1SkewFactor->setText(temp);
            break;
        case ScatterAttributes::ID_var2Role:
            var2Role->blockSignals(true);
            var2Role->setCurrentItem(atts->GetVar2Role());
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
            temp.sprintf("%g", atts->GetVar2Min());
            var2Min->setText(temp);
            break;
        case ScatterAttributes::ID_var2Max:
            temp.sprintf("%g", atts->GetVar2Max());
            var2Max->setText(temp);
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
            var2Scaling->setButton(atts->GetVar2Scaling());
            var2Scaling->blockSignals(false);
            break;
        case ScatterAttributes::ID_var2SkewFactor:
            temp.sprintf("%g", atts->GetVar2SkewFactor());
            var2SkewFactor->setText(temp);
            break;
        case ScatterAttributes::ID_var3Role:
            var3Role->blockSignals(true);
            var3Role->setCurrentItem(atts->GetVar3Role());
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
            temp.sprintf("%g", atts->GetVar3Min());
            var3Min->setText(temp);
            break;
        case ScatterAttributes::ID_var3Max:
            temp.sprintf("%g", atts->GetVar3Max());
            var3Max->setText(temp);
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
            var3Scaling->setButton(atts->GetVar3Scaling());
            var3Scaling->blockSignals(false);
            break;
        case ScatterAttributes::ID_var3SkewFactor:
            temp.sprintf("%g", atts->GetVar3SkewFactor());
            var3SkewFactor->setText(temp);
            break;
        case ScatterAttributes::ID_var4Role:
            var4Role->blockSignals(true);
            var4Role->setCurrentItem(atts->GetVar4Role());
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
            temp.sprintf("%g", atts->GetVar4Min());
            var4Min->setText(temp);
            break;
        case ScatterAttributes::ID_var4Max:
            temp.sprintf("%g", atts->GetVar4Max());
            var4Max->setText(temp);
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
            var4Scaling->setButton(atts->GetVar4Scaling());
            var4Scaling->blockSignals(false);
            break;
        case ScatterAttributes::ID_var4SkewFactor:
            temp.sprintf("%g", atts->GetVar4SkewFactor());
            var4SkewFactor->setText(temp);
            break;
        case ScatterAttributes::ID_pointSize:
            updatePointSize = true;
            break;
        case ScatterAttributes::ID_pointSizePixels:
            updatePointSize = true;
            break;
        case ScatterAttributes::ID_pointType:
            pointType->blockSignals(true);
            pointType->setCurrentItem(atts->GetPointType());
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
            temp.sprintf("%g", atts->GetPointSize());
            pointSize->setText(temp);
            pointSizeLabel->setText("Point size");
        }
        else
        {
            temp.sprintf("%d", atts->GetPointSizePixels());
            pointSize->setText(temp);
            pointSizeLabel->setText("Point size (pixels)");
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
                int index = var.findRev("/");
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
                    QToolTip::add(roleLabels[role], roleVar);
                else
                    QToolTip::remove(roleLabels[role]);

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
// ****************************************************************************

void
QvisScatterPlotWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do var1Min
    if(which_widget == ScatterAttributes::ID_var1Min || doAll)
    {
        temp = var1Min->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetVar1Min(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of var1Min was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetVar1Min());
            Message(msg);
            atts->SetVar1Min(atts->GetVar1Min());
        }
    }

    // Do var1Max
    if(which_widget == ScatterAttributes::ID_var1Max || doAll)
    {
        temp = var1Max->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetVar1Max(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of var1Max was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetVar1Max());
            Message(msg);
            atts->SetVar1Max(atts->GetVar1Max());
        }
    }

    // Do var1SkewFactor
    if(which_widget == ScatterAttributes::ID_var1SkewFactor || doAll)
    {
        temp = var1SkewFactor->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetVar1SkewFactor(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of var1SkewFactor was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetVar1SkewFactor());
            Message(msg);
            atts->SetVar1SkewFactor(atts->GetVar1SkewFactor());
        }
    }

    // Do var2Min
    if(which_widget == ScatterAttributes::ID_var2Min || doAll)
    {
        temp = var2Min->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetVar2Min(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of var2Min was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetVar2Min());
            Message(msg);
            atts->SetVar2Min(atts->GetVar2Min());
        }
    }

    // Do var2Max
    if(which_widget == ScatterAttributes::ID_var2Max || doAll)
    {
        temp = var2Max->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetVar2Max(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of var2Max was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetVar2Max());
            Message(msg);
            atts->SetVar2Max(atts->GetVar2Max());
        }
    }

    // Do var2SkewFactor
    if(which_widget == ScatterAttributes::ID_var2SkewFactor|| doAll)
    {
        temp = var2SkewFactor->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetVar2SkewFactor(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of var2SkewFactor was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetVar2SkewFactor());
            Message(msg);
            atts->SetVar2SkewFactor(atts->GetVar2SkewFactor());
        }
    }

    // Do var3Min
    if(which_widget == ScatterAttributes::ID_var2Min || doAll)
    {
        temp = var3Min->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetVar3Min(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of var3Min was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetVar3Min());
            Message(msg);
            atts->SetVar3Min(atts->GetVar3Min());
        }
    }

    // Do var3Max
    if(which_widget == ScatterAttributes::ID_var3Max || doAll)
    {
        temp = var3Max->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetVar3Max(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of var3Max was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetVar3Max());
            Message(msg);
            atts->SetVar3Max(atts->GetVar3Max());
        }
    }

    // Do var3SkewFactor
    if(which_widget == ScatterAttributes::ID_var2SkewFactor || doAll)
    {
        temp = var3SkewFactor->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetVar3SkewFactor(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of var3SkewFactor was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetVar3SkewFactor());
            Message(msg);
            atts->SetVar3SkewFactor(atts->GetVar3SkewFactor());
        }
    }

    // Do var4Min
    if(which_widget == ScatterAttributes::ID_var4Min || doAll)
    {
        temp = var4Min->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetVar4Min(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of var4Min was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetVar4Min());
            Message(msg);
            atts->SetVar4Min(atts->GetVar4Min());
        }
    }

    // Do var4Max
    if(which_widget == ScatterAttributes::ID_var4Max || doAll)
    {
        temp = var4Max->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetVar4Max(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of var4Max was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetVar4Max());
            Message(msg);
            atts->SetVar4Max(atts->GetVar4Max());
        }
    }

    // Do var4SkewFactor
    if(which_widget == ScatterAttributes::ID_var4SkewFactor || doAll)
    {
        temp = var4SkewFactor->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetVar4SkewFactor(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of var4SkewFactor was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetVar4SkewFactor());
            Message(msg);
            atts->SetVar4SkewFactor(atts->GetVar4SkewFactor());
        }
    }

    // Do pointSize
    if(which_widget == ScatterAttributes::ID_pointSize || doAll)
    {
        temp = pointSize->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(atts->GetPointType() == ScatterAttributes::Point ||
           atts->GetPointType() == ScatterAttributes::Sphere)
        {
            if(okay)
            {
                int val = temp.toInt(&okay);
                atts->SetPointSizePixels(val);
            }

            if(!okay)
            {
                msg.sprintf("The value of pointSizePixels was invalid. "
                    "Resetting to the last good value of %d.",
                    atts->GetPointSizePixels());
                Message(msg);
                atts->SetPointSizePixels(atts->GetPointSizePixels());
            }
        }
        else
        {
            if(okay)
            {
                double val = temp.toDouble(&okay);
                atts->SetPointSize(val);
            }

            if(!okay)
            {
                msg.sprintf("The value of pointSize was invalid. "
                    "Resetting to the last good value of %g.",
                    atts->GetPointSize());
                Message(msg);
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
        msg.sprintf("The %s role was being fulfilled by another input "
                    "variable. To ensure that the %s role is fulfilled by "
                    "the %s variable, the other variable's role has been "
                    "set to None.", roleNames[val], roleNames[val],
                    var);
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
    atts->SetVar2(var.latin1());
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
    atts->SetVar3(var.latin1());
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
    atts->SetVar4(var.latin1());
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
    atts->SetColorTableName(ctName.latin1());
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


