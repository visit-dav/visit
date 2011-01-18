/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
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

#include <QvisColorTableWidget.h>
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

    haveColorRole = (atts->GetVar1Role() == 3 || atts->GetVar2Role() == 3 || 
                     atts->GetVar3Role() == 3 || atts->GetVar4Role() == 3);
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
//   Allen Sanderson, Sun Mar  7 12:49:56 PST 2010
//   Change layout of window for 2.0 interface changes.
//
//   Brad Whitlock, Fri Jul 16 14:34:23 PDT 2010
//   Make curves an allowable variable type.
//
//   Cyrus Harrison, Thu Aug 19 13:19:11 PDT 2010
//   Widget changes to capture when var1 is changed.
//
//   Kathleen Bonnell, Mon Jan 17 18:10:28 MST 2011
//   Change colorTableButton to colorTableWidget to gain invert toggle.
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

    // Create a role button
    var1Role = new QComboBox(var1Top);
    for(i = 0; i < 4; ++i)
        var1Role->addItem(roleNames[i]);
    connect(var1Role, SIGNAL(activated(int)),
            this, SLOT(var1RoleChanged(int)));

    QLabel *var1RoleLabel = new QLabel(tr("Role"), var1Top);
    var1RoleLabel->setBuddy(var1Role);
    var1TopLayout->addWidget(var1RoleLabel, 0, 0, Qt::AlignRight);
    var1TopLayout->addWidget(var1Role, 0, 1, Qt::AlignLeft);

    // Create a variable button
    var1 = new QvisVariableButton(true, true, true,
        QvisVariableButton::Scalars | QvisVariableButton::Curves,
        var1Top);
    connect(var1, SIGNAL(activated(const QString &)),
            this, SLOT(var1Selected(const QString &)));

    QLabel *var1Label = new QLabel(tr("Variable"), var1Top);
    var1Label->setBuddy(var1);
    var1TopLayout->addWidget(var1Label, 0, 2, Qt::AlignRight);
    var1TopLayout->addWidget(var1, 0, 3, Qt::AlignLeft);

    //
    // Create the scale group
    //
    QGroupBox * dataGroup = new QGroupBox(central);
    dataGroup->setTitle(tr("Data"));
    var1TopLayout->addWidget(dataGroup, 1, 0, 2, 4);

    QGridLayout * dataLayout = new QGridLayout(dataGroup);
    dataLayout->setMargin(5);
    dataLayout->setSpacing(10);

    //
    // Create the scale radio buttons
    //
    dataLayout->addWidget( new QLabel(tr("Scale"), central), 0, 0);
    
    // Create the radio buttons
    var1Scaling = new QButtonGroup(central);

    QRadioButton * rb = new QRadioButton(tr("Linear"), central);
    rb->setChecked(true);
    var1Scaling->addButton(rb, 0);
    dataLayout->addWidget(rb, 0, 1);
    rb = new QRadioButton(tr("Log"), central);
    var1Scaling->addButton(rb, 1);
    dataLayout->addWidget(rb, 0, 2);
    rb = new QRadioButton(tr("Skew"), central);
    var1Scaling->addButton(rb, 2);
    dataLayout->addWidget(rb, 0, 3);

    // Each time a radio button is clicked, call the scale clicked slot.
    connect(var1Scaling, SIGNAL(buttonClicked(int)),
            this, SLOT(var1ScalingChanged(int)));

    // Create the skew factor line edit    
    var1SkewFactor = new QLineEdit(central);
    dataLayout->addWidget(var1SkewFactor, 0, 4);
    connect(var1SkewFactor, SIGNAL(returnPressed()),
            this, SLOT(var1SkewFactorProcessText()));

    //
    // Create the Limits stuff
    //
    QGroupBox *limitsGroup = new QGroupBox(central);
    dataLayout->addWidget(limitsGroup, 1, 0, 2, 5);

    QGridLayout *limitsLayout = new QGridLayout(limitsGroup);
    limitsLayout->setMargin(5);
    limitsLayout->setSpacing(10);

//     limitsLayout->addWidget( new QLabel(tr("Limits"), central), 0, 0);

//     limitsSelect = new QComboBox(central);
//     limitsSelect->addItem(tr("Use Original Data"));
//     limitsSelect->addItem(tr("Use Current Plot"));
//     connect(limitsSelect, SIGNAL(activated(int)),
//             this, SLOT(limitsSelectChanged(int))); 
//     limitsLayout->addWidget(limitsSelect, 0, 1, 1, 2, Qt::AlignLeft);

    // Create the min toggle and line edit
    var1MinFlag = new QCheckBox(tr("Minimum"), central);
    limitsLayout->addWidget(var1MinFlag, 1, 0);
    connect(var1MinFlag, SIGNAL(toggled(bool)),
            this, SLOT(var1MinFlagChanged(bool)));
    var1Min = new QLineEdit(central);
    connect(var1Min, SIGNAL(returnPressed()),
            this, SLOT(var1MinProcessText())); 
    limitsLayout->addWidget(var1Min, 1, 1);

    // Create the max toggle and line edit
    var1MaxFlag = new QCheckBox(tr("Maximum"), central);
    limitsLayout->addWidget(var1MaxFlag, 1, 2);
    connect(var1MaxFlag, SIGNAL(toggled(bool)),
            this, SLOT(var1MaxFlagChanged(bool)));
    var1Max = new QLineEdit(central);
    connect(var1Max, SIGNAL(returnPressed()),
            this, SLOT(var1MaxProcessText())); 
    limitsLayout->addWidget(var1Max, 1, 3);


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

    // Create a role button
    var2Role = new QComboBox(var2Top);
    for(i = 0; i < 5; ++i)
        var2Role->addItem(roleNames[i]);
    connect(var2Role, SIGNAL(activated(int)),
            this, SLOT(var2RoleChanged(int)));

    QLabel *var2RoleLabel = new QLabel(tr("Role"), var2Top);
    var2RoleLabel->setBuddy(var2Role);
    var2TopLayout->addWidget(var2RoleLabel, 0, 0, Qt::AlignRight);
    var2TopLayout->addWidget(var2Role, 0, 1, Qt::AlignLeft);

    // Create a variable button
    var2 = new QvisVariableButton(true, true, true,
        QvisVariableButton::Scalars | QvisVariableButton::Curves,
        var2Top);
    connect(var2, SIGNAL(activated(const QString &)),
            this, SLOT(var2Selected(const QString &)));

    QLabel *var2Label = new QLabel(tr("Variable"), var2Top);
    var2Label->setBuddy(var2);
    var2TopLayout->addWidget(var2Label, 0, 2, Qt::AlignRight);
    var2TopLayout->addWidget(var2, 0, 3, Qt::AlignLeft);

    //
    // Create the scale group
    //
    dataGroup = new QGroupBox(central);
    dataGroup->setTitle(tr("Data"));
    var2TopLayout->addWidget(dataGroup, 1, 0, 2, 4);

    dataLayout = new QGridLayout(dataGroup);
    dataLayout->setMargin(5);
    dataLayout->setSpacing(10);

    //
    // Create the scale radio buttons
    //
    dataLayout->addWidget( new QLabel(tr("Scale"), central), 0, 0);
    
    // Create the radio buttons
    var2Scaling = new QButtonGroup(central);

    rb = new QRadioButton(tr("Linear"), central);
    rb->setChecked(true);
    var2Scaling->addButton(rb, 0);
    dataLayout->addWidget(rb, 0, 1);
    rb = new QRadioButton(tr("Log"), central);
    var2Scaling->addButton(rb, 1);
    dataLayout->addWidget(rb, 0, 2);
    rb = new QRadioButton(tr("Skew"), central);
    var2Scaling->addButton(rb, 2);
    dataLayout->addWidget(rb, 0, 3);

    // Each time a radio button is clicked, call the scale clicked slot.
    connect(var2Scaling, SIGNAL(buttonClicked(int)),
            this, SLOT(var2ScalingChanged(int)));

    // Create the skew factor line edit    
    var2SkewFactor = new QLineEdit(central);
    dataLayout->addWidget(var2SkewFactor, 0, 4);
    connect(var2SkewFactor, SIGNAL(returnPressed()),
            this, SLOT(var2SkewFactorProcessText()));

    //
    // Create the Limits stuff
    //
    limitsGroup = new QGroupBox(central);
    dataLayout->addWidget(limitsGroup, 1, 0, 2, 5);

    limitsLayout = new QGridLayout(limitsGroup);
    limitsLayout->setMargin(5);
    limitsLayout->setSpacing(10);

//     limitsLayout->addWidget( new QLabel(tr("Limits"), central), 0, 0);

//     limitsSelect = new QComboBox(central);
//     limitsSelect->addItem(tr("Use Original Data"));
//     limitsSelect->addItem(tr("Use Current Plot"));
//     connect(limitsSelect, SIGNAL(activated(int)),
//             this, SLOT(limitsSelectChanged(int))); 
//     limitsLayout->addWidget(limitsSelect, 0, 1, 1, 2, Qt::AlignLeft);

    // Create the min toggle and line edit
    var2MinFlag = new QCheckBox(tr("Minimum"), central);
    limitsLayout->addWidget(var2MinFlag, 1, 0);
    connect(var2MinFlag, SIGNAL(toggled(bool)),
            this, SLOT(var2MinFlagChanged(bool)));
    var2Min = new QLineEdit(central);
    connect(var2Min, SIGNAL(returnPressed()),
            this, SLOT(var2MinProcessText())); 
    limitsLayout->addWidget(var2Min, 1, 1);

    // Create the max toggle and line edit
    var2MaxFlag = new QCheckBox(tr("Maximum"), central);
    limitsLayout->addWidget(var2MaxFlag, 1, 2);
    connect(var2MaxFlag, SIGNAL(toggled(bool)),
            this, SLOT(var2MaxFlagChanged(bool)));
    var2Max = new QLineEdit(central);
    connect(var2Max, SIGNAL(returnPressed()),
            this, SLOT(var2MaxProcessText())); 
    limitsLayout->addWidget(var2Max, 1, 3);


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


    // Create a role button
    var3Role = new QComboBox(var3Top);
    for(i = 0; i < 5; ++i)
        var3Role->addItem(roleNames[i]);
    connect(var3Role, SIGNAL(activated(int)),
            this, SLOT(var3RoleChanged(int)));

    QLabel *var3RoleLabel = new QLabel(tr("Role"), var3Top);
    var3RoleLabel->setBuddy(var3Role);
    var3TopLayout->addWidget(var3RoleLabel, 0, 0, Qt::AlignRight);
    var3TopLayout->addWidget(var3Role, 0, 1, Qt::AlignLeft);

    // Create a variable button
    var3 = new QvisVariableButton(true, true, true,
        QvisVariableButton::Scalars | QvisVariableButton::Curves,
        var3Top);
    connect(var3, SIGNAL(activated(const QString &)),
            this, SLOT(var3Selected(const QString &)));

    QLabel *var3Label = new QLabel(tr("Variable"), var3Top);
    var3Label->setBuddy(var3);
    var3TopLayout->addWidget(var3Label, 0, 2, Qt::AlignRight);
    var3TopLayout->addWidget(var3, 0, 3, Qt::AlignLeft);

    //
    // Create the scale group
    //
    dataGroup = new QGroupBox(central);
    dataGroup->setTitle(tr("Data"));
    var3TopLayout->addWidget(dataGroup, 1, 0, 2, 4);

    dataLayout = new QGridLayout(dataGroup);
    dataLayout->setMargin(5);
    dataLayout->setSpacing(10);

    //
    // Create the scale radio buttons
    //
    dataLayout->addWidget( new QLabel(tr("Scale"), central), 0, 0);
    
    // Create the radio buttons
    var3Scaling = new QButtonGroup(central);

    rb = new QRadioButton(tr("Linear"), central);
    rb->setChecked(true);
    var3Scaling->addButton(rb, 0);
    dataLayout->addWidget(rb, 0, 1);
    rb = new QRadioButton(tr("Log"), central);
    var3Scaling->addButton(rb, 1);
    dataLayout->addWidget(rb, 0, 2);
    rb = new QRadioButton(tr("Skew"), central);
    var3Scaling->addButton(rb, 2);
    dataLayout->addWidget(rb, 0, 3);

    // Each time a radio button is clicked, call the scale clicked slot.
    connect(var3Scaling, SIGNAL(buttonClicked(int)),
            this, SLOT(var3ScalingChanged(int)));

    // Create the skew factor line edit    
    var3SkewFactor = new QLineEdit(central);
    dataLayout->addWidget(var3SkewFactor, 0, 4);
    connect(var3SkewFactor, SIGNAL(returnPressed()),
            this, SLOT(var3SkewFactorProcessText()));

    //
    // Create the Limits stuff
    //
    limitsGroup = new QGroupBox(central);
    dataLayout->addWidget(limitsGroup, 1, 0, 2, 5);

    limitsLayout = new QGridLayout(limitsGroup);
    limitsLayout->setMargin(5);
    limitsLayout->setSpacing(10);

//     limitsLayout->addWidget( new QLabel(tr("Limits"), central), 0, 0);

//     limitsSelect = new QComboBox(central);
//     limitsSelect->addItem(tr("Use Original Data"));
//     limitsSelect->addItem(tr("Use Current Plot"));
//     connect(limitsSelect, SIGNAL(activated(int)),
//             this, SLOT(limitsSelectChanged(int))); 
//     limitsLayout->addWidget(limitsSelect, 0, 1, 1, 2, Qt::AlignLeft);

    // Create the min toggle and line edit
    var3MinFlag = new QCheckBox(tr("Minimum"), central);
    limitsLayout->addWidget(var3MinFlag, 1, 0);
    connect(var3MinFlag, SIGNAL(toggled(bool)),
            this, SLOT(var3MinFlagChanged(bool)));
    var3Min = new QLineEdit(central);
    connect(var3Min, SIGNAL(returnPressed()),
            this, SLOT(var3MinProcessText())); 
    limitsLayout->addWidget(var3Min, 1, 1);

    // Create the max toggle and line edit
    var3MaxFlag = new QCheckBox(tr("Maximum"), central);
    limitsLayout->addWidget(var3MaxFlag, 1, 2);
    connect(var3MaxFlag, SIGNAL(toggled(bool)),
            this, SLOT(var3MaxFlagChanged(bool)));
    var3Max = new QLineEdit(central);
    connect(var3Max, SIGNAL(returnPressed()),
            this, SLOT(var3MaxProcessText())); 
    limitsLayout->addWidget(var3Max, 1, 3);


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



    // Create a role button
    var4Role = new QComboBox(var4Top);
    for(i = 0; i < 5; ++i)
        var4Role->addItem(roleNames[i]);
    connect(var4Role, SIGNAL(activated(int)),
            this, SLOT(var4RoleChanged(int)));

    QLabel *var4RoleLabel = new QLabel(tr("Role"), var4Top);
    var4RoleLabel->setBuddy(var4Role);
    var4TopLayout->addWidget(var4RoleLabel, 0, 0, Qt::AlignRight);
    var4TopLayout->addWidget(var4Role, 0, 1, Qt::AlignLeft);

    // Create a variable button
    var4 = new QvisVariableButton(true, true, true,
        QvisVariableButton::Scalars | QvisVariableButton::Curves,
        var4Top);
    connect(var4, SIGNAL(activated(const QString &)),
            this, SLOT(var4Selected(const QString &)));

    QLabel *var4Label = new QLabel(tr("Variable"), var4Top);
    var4Label->setBuddy(var4);
    var4TopLayout->addWidget(var4Label, 0, 2, Qt::AlignRight);
    var4TopLayout->addWidget(var4, 0, 3, Qt::AlignLeft);

    //
    // Create the scale group
    //
    dataGroup = new QGroupBox(central);
    dataGroup->setTitle(tr("Data"));
    var4TopLayout->addWidget(dataGroup, 1, 0, 2, 4);

    dataLayout = new QGridLayout(dataGroup);
    dataLayout->setMargin(5);
    dataLayout->setSpacing(10);

    //
    // Create the scale radio buttons
    //
    dataLayout->addWidget( new QLabel(tr("Scale"), central), 0, 0);
    
    // Create the radio buttons
    var4Scaling = new QButtonGroup(central);

    rb = new QRadioButton(tr("Linear"), central);
    rb->setChecked(true);
    var4Scaling->addButton(rb, 0);
    dataLayout->addWidget(rb, 0, 1);
    rb = new QRadioButton(tr("Log"), central);
    var4Scaling->addButton(rb, 1);
    dataLayout->addWidget(rb, 0, 2);
    rb = new QRadioButton(tr("Skew"), central);
    var4Scaling->addButton(rb, 2);
    dataLayout->addWidget(rb, 0, 3);

    // Each time a radio button is clicked, call the scale clicked slot.
    connect(var4Scaling, SIGNAL(buttonClicked(int)),
            this, SLOT(var4ScalingChanged(int)));

    // Create the skew factor line edit    
    var4SkewFactor = new QLineEdit(central);
    dataLayout->addWidget(var4SkewFactor, 0, 4);
    connect(var4SkewFactor, SIGNAL(returnPressed()),
            this, SLOT(var4SkewFactorProcessText()));

    //
    // Create the Limits stuff
    //
    limitsGroup = new QGroupBox(central);
    dataLayout->addWidget(limitsGroup, 1, 0, 2, 5);

    limitsLayout = new QGridLayout(limitsGroup);
    limitsLayout->setMargin(5);
    limitsLayout->setSpacing(10);

//     limitsLayout->addWidget( new QLabel(tr("Limits"), central), 0, 0);

//     limitsSelect = new QComboBox(central);
//     limitsSelect->addItem(tr("Use Original Data"));
//     limitsSelect->addItem(tr("Use Current Plot"));
//     connect(limitsSelect, SIGNAL(activated(int)),
//             this, SLOT(limitsSelectChanged(int))); 
//     limitsLayout->addWidget(limitsSelect, 0, 1, 1, 2, Qt::AlignLeft);

    // Create the min toggle and line edit
    var4MinFlag = new QCheckBox(tr("Minimum"), central);
    limitsLayout->addWidget(var4MinFlag, 1, 0);
    connect(var4MinFlag, SIGNAL(toggled(bool)),
            this, SLOT(var4MinFlagChanged(bool)));
    var4Min = new QLineEdit(central);
    connect(var4Min, SIGNAL(returnPressed()),
            this, SLOT(var4MinProcessText())); 
    limitsLayout->addWidget(var4Min, 1, 1);

    // Create the max toggle and line edit
    var4MaxFlag = new QCheckBox(tr("Maximum"), central);
    limitsLayout->addWidget(var4MaxFlag, 1, 2);
    connect(var4MaxFlag, SIGNAL(toggled(bool)),
            this, SLOT(var4MaxFlagChanged(bool)));
    var4Max = new QLineEdit(central);
    connect(var4Max, SIGNAL(returnPressed()),
            this, SLOT(var4MaxProcessText())); 
    limitsLayout->addWidget(var4Max, 1, 3);


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

    // Scaling
    QGroupBox * scalingGroup = new QGroupBox(central);
    scalingGroup->setTitle(tr("Scaling"));
    aTopLayout->addWidget(scalingGroup);

    QGridLayout *scalingLayout = new QGridLayout(scalingGroup);
    scalingLayout->setMargin(5);
    scalingLayout->setSpacing(10);
 
    scaleCube = new QCheckBox(tr("Normlize the axis to a cube"), appearanceGroup);
    connect(scaleCube, SIGNAL(toggled(bool)),
            this, SLOT(scaleCubeChanged(bool)));
    scalingLayout->addWidget(scaleCube, 0, 0 );

    // Color
    QGroupBox * colorGroup = new QGroupBox(central);
    colorGroup->setTitle(tr("Color"));
    aTopLayout->addWidget(colorGroup);

    QGridLayout *colorLayout = new QGridLayout(colorGroup);
    colorLayout->setMargin(5);
    colorLayout->setSpacing(10);


    // Create the mode buttons that determine if the window is in single
    // or multiple color mode.
    colorModeButtons = new QButtonGroup(colorGroup);
    connect(colorModeButtons, SIGNAL(buttonClicked(int)),
            this, SLOT(colorModeChanged(int)));

    rb = new QRadioButton(tr("Foreground Color"), colorGroup);
    colorModeButtons->addButton(rb, 0);
    colorLayout->addWidget(rb, 0, 0);
    rb = new QRadioButton(tr("Single color"), colorGroup);
    colorModeButtons->addButton(rb, 1);
    colorLayout->addWidget(rb, 1, 0);
    colorTableRadioButton =
      new QRadioButton(tr("Color table"), colorGroup);
    colorModeButtons->addButton(colorTableRadioButton, 2);
    colorLayout->addWidget(colorTableRadioButton, 2, 0);

    // Create the single color button.
    singleColor = new QvisColorButton(colorGroup);
    singleColor->setButtonColor(QColor(255, 0, 0));
    connect(singleColor, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(singleColorChanged(const QColor &)));
    colorLayout->addWidget(singleColor, 1, 1);

    // Create the color table selection.
    colorTableWidget = new QvisColorTableWidget(colorGroup, true);
    connect(colorTableWidget, SIGNAL(selectedColorTable(bool, const QString&)),
            this, SLOT(colorTableNameChanged(bool, const QString&)));
    connect(colorTableWidget,
            SIGNAL(invertColorTableToggled(bool)),
            this,
            SLOT(invertColorTableToggled(bool)));
    colorLayout->addWidget(colorTableWidget, 2, 1);


    // Rendering
    QGroupBox * styleGroup = new QGroupBox(central);
    styleGroup->setTitle(tr("Point Style"));
    aTopLayout->addWidget(styleGroup);

    QGridLayout *styleLayout = new QGridLayout(styleGroup);
    styleLayout->setMargin(5);
    styleLayout->setSpacing(10);
 
    pointType = new QComboBox(appearanceGroup);
    pointType->addItem(tr("Box"));
    pointType->addItem(tr("Axis"));
    pointType->addItem(tr("Icosahedron"));
    pointType->addItem(tr("Point"));
    pointType->addItem(tr("Sphere"));
    connect(pointType, SIGNAL(activated(int)),
            this, SLOT(pointTypeChanged(int)));
    styleLayout->addWidget(pointType, 0, 1);
    QLabel *pointTypeLabel = new QLabel(tr("Point Type"), appearanceGroup);
    pointTypeLabel->setBuddy(pointType);
    styleLayout->addWidget(pointTypeLabel, 0, 0);

    pointSize = new QLineEdit(appearanceGroup);
    connect(pointSize, SIGNAL(returnPressed()),
            this, SLOT(pointSizeProcessText()));
    styleLayout->addWidget(pointSize, 0, 3);
    pointSizeLabel = new QLabel(tr("Point size"), appearanceGroup);
    pointSizeLabel->setBuddy(pointSize);
    styleLayout->addWidget(pointSizeLabel, 0, 2);

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
    roleLayout->addWidget(zCoordRoleLabel, 2, 0);
    colorRoleLabel = new QLabel(tr("Color:    "), roleGroup);
    roleLayout->addWidget(colorRoleLabel, 0, 1);

    //
    // Create the misc stuff
    //
    QGroupBox * miscGroup = new QGroupBox(central);
    miscGroup->setTitle(tr("Misc"));
    topLayout->addWidget(miscGroup);

    QGridLayout *miscLayout = new QGridLayout(miscGroup);
    miscLayout->setMargin(5);
    miscLayout->setSpacing(10);
 
    // Create the legend toggle
    legendToggle = new QCheckBox(tr("Legend"), miscGroup);
    connect(legendToggle, SIGNAL(toggled(bool)),
            this, SLOT(legendToggled(bool)));
    miscLayout->addWidget(legendToggle, 0, 0);
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
//   Cyrus Harrison, Thu Aug 19 09:08:41 PDT 2010
//   Capture update of var1.
//
//   Kathleen Bonnell, Mon Jan 17 18:10:28 MST 2011
//   Change colorTableButton to colorTableWidget to gain invert toggle.
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
        case ScatterAttributes::ID_var1:
            temp = atts->GetVar1().c_str();
            var1->setText(temp);
            varsChanged = true;
            break;
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
                var1SkewFactor->setEnabled(true);
            else
                var1SkewFactor->setEnabled(false);
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
                var2SkewFactor->setEnabled(true);
            else
                var2SkewFactor->setEnabled(false);
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
                var3SkewFactor->setEnabled(true);
            else
                var3SkewFactor->setEnabled(false);
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
                var4SkewFactor->setEnabled(true);
            else
                var4SkewFactor->setEnabled(false);
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
        case ScatterAttributes::ID_colorType:
          if( atts->GetColorType() ==
              ScatterAttributes::ColorByForegroundColor) 
            colorModeButtons->button(0)->setChecked(true);
          else if( atts->GetColorType() ==
                  ScatterAttributes::ColorBySingleColor) 
            colorModeButtons->button(1)->setChecked(true);
          else if( atts->GetColorType() ==
                  ScatterAttributes::ColorByColorTable) 
            colorModeButtons->button(2)->setChecked(true);

          singleColor->setEnabled(atts->GetColorType() ==
                                  ScatterAttributes::ColorBySingleColor);

          colorTableWidget->setEnabled(atts->GetColorType() ==
                                     ScatterAttributes::ColorByColorTable);
            break;
        case ScatterAttributes::ID_colorTableName:
            colorTableWidget->setColorTable(atts->GetColorTableName().c_str());
            break;
        case ScatterAttributes::ID_invertColorTable:
            colorTableWidget->setInvertColorTable(atts->GetInvertColorTable());
            break;
        case ScatterAttributes::ID_singleColor:
            tempcolor = QColor(atts->GetSingleColor().Red(),
                               atts->GetSingleColor().Green(),
                               atts->GetSingleColor().Blue());
            singleColor->setButtonColor(tempcolor);
            break;
        case ScatterAttributes::ID_legendFlag:
            legendToggle->blockSignals(true);
            legendToggle->setChecked(atts->GetLegendFlag());
            legendToggle->blockSignals(false);
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
        QLabel *roleLabels[] = { xCoordRoleLabel, yCoordRoleLabel, 
                                 zCoordRoleLabel, colorRoleLabel, 0 };

        for(int role = 0; role < 4; ++role)
        {
            QString label(roleNames[role]);
            label = label + ": ";

            QString roleVar;
            if(role == int(atts->GetVar1Role()))
                roleVar = atts->GetVar1().c_str();
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

        // Look for a state change
        if( haveColorRole !=
            (atts->GetVar1Role() == 3 || atts->GetVar2Role() == 3 || 
             atts->GetVar3Role() == 3 || atts->GetVar4Role() == 3) )
        {
          haveColorRole = (atts->GetVar1Role() == 3 ||
                           atts->GetVar2Role() == 3 || 
                           atts->GetVar3Role() == 3 ||
                           atts->GetVar4Role() == 3);

          // If a state change and have a color var then
          // assume the user wants a color table.
          if( haveColorRole )
          {
            colorModeButtons->blockSignals(true);
            colorModeButtons->button(2)->setChecked(true);
            colorModeButtons->blockSignals(false);
            colorModeChanged(2);
          }
        
          // If no color role turn off the color by table
          else if( !haveColorRole &&
                   atts->GetColorType() == ScatterAttributes::ColorByColorTable )
          {
            colorModeButtons->blockSignals(true);
            colorModeButtons->button(0)->setChecked(true);
            colorModeButtons->blockSignals(false);
            colorModeChanged(0);
          }
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
        EnsureUniqueRole(14, val, atts->GetVar1().c_str());
        atts->SetVar1Role(ScatterAttributes::VariableRole(val));
        Apply();
    }
}

void
QvisScatterPlotWindow::var1Selected(const QString &var)
{
    atts->SetVar1(var.toStdString());
    Apply();
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
QvisScatterPlotWindow::colorModeChanged(int index)
{
    if(index == 0)
        atts->SetColorType(ScatterAttributes::ColorByForegroundColor);
    else if(index == 1)
        atts->SetColorType(ScatterAttributes::ColorBySingleColor);
    else if(index == 2)
        atts->SetColorType(ScatterAttributes::ColorByColorTable);

    Apply();
}

void
QvisScatterPlotWindow::colorTableNameChanged(bool useDefault, const QString &ctName)
{
    atts->SetColorTableName(ctName.toStdString());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisScatterPlotWindow::invertColorTableToggled
//
// Purpose: 
//   This is a Qt slot function that sets the invert color table flag into the
//   scatter plot attributes.
//
// Arguments:
//   val    :  Whether or not to invert the color table.
//
// Programmer: Kathleen Bonnell
// Creation:   January  17, 2011
//
// Modifications:
//   
// ****************************************************************************

void
QvisScatterPlotWindow::invertColorTableToggled(bool val)
{
    atts->SetInvertColorTable(val);
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
QvisScatterPlotWindow::legendToggled(bool val)
{
    atts->SetLegendFlag(val);
    SetUpdate(false);
    Apply();
}
