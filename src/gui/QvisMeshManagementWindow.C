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

#include <vector>
#include <snprintf.h>

#include <qbuttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qtabwidget.h>
#include <qvbox.h>

#include <QvisMeshManagementWindow.h>
#include <MeshManagementAttributes.h>
#include <ViewerProxy.h>

using std::vector;

// ****************************************************************************
// Method: QvisMeshManagementWindow::QvisMeshManagementWindow
//
// Purpose: 
//   This is the constructor for the QvisMeshManagementWindow class.
//
// Programmer: Eric Brugger
// Creation:   Mon Nov 19 14:15:03 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue Oct 7 09:37:42 PDT 2003
//   Added playbackModeButtonGroup.
//
// ****************************************************************************

QvisMeshManagementWindow::QvisMeshManagementWindow(MeshManagementAttributes *subj,
    const char *caption, const char *shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(subj, caption, shortName, notepad,
                               QvisPostableWindowObserver::ApplyButton)
{
    mmAtts = subj;
}

// ****************************************************************************
// Method: QvisMeshManagementWindow::~QvisMeshManagementWindow
//
// Purpose: 
//   This is the destructor for the QvisMeshManagementWindow class.
//
// Programmer: Eric Brugger
// Creation:   Mon Nov 19 14:15:03 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue Oct 7 09:38:31 PDT 2003
//   Deleted playbackModeButtonGroup since it has no parent.
//
// ****************************************************************************

QvisMeshManagementWindow::~QvisMeshManagementWindow()
{
}

// ****************************************************************************
// Method: QvisMeshManagementWindow::CreateWindowContents
//
// Purpose: 
//   This method creates the widgets for the window.
//
// Programmer: Eric Brugger
// Creation:   Mon Nov 19 14:15:03 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue May 14 11:39:42 PDT 2002
//   Added a slider for the animation playback speed.
//
//   Brad Whitlock, Mon Oct 6 16:21:12 PST 2003
//   Added radio buttons that allow the user to set the animation style.
//
// ****************************************************************************

void
QvisMeshManagementWindow::CreateWindowContents()
{

    tabs = new QTabWidget(central, "tabs");
    connect(tabs, SIGNAL(selected(const QString &)),
            this, SLOT(tabSelected(const QString &)));
    topLayout->setSpacing(5);
    topLayout->addWidget(tabs);

    pageCSG = new QVBox(central, "pageCSG");
    pageCSG->setSpacing(5);
    pageCSG->setMargin(10);
    tabs->addTab(pageCSG, "CSG Meshing");

    pageCSGGroup = new QGroupBox(pageCSG, "pageCSGGroup");
    pageCSGGroup->setFrameStyle(QFrame::NoFrame);

    QVBoxLayout *internalLayoutCSGGroup = new QVBoxLayout(pageCSGGroup);
    internalLayoutCSGGroup->addSpacing(10);
    QGridLayout *layoutCSGGroup = new QGridLayout(internalLayoutCSGGroup, 5, 3);
    layoutCSGGroup->setSpacing(5);

    renderCSGDirect = new QCheckBox("Don't discretize. Pass native CSG down pipeline",
                                    pageCSGGroup, "renderCSGDirect");
    connect(renderCSGDirect, SIGNAL(toggled(bool)),
            this, SLOT(renderCSGDirectChanged(bool)));
    layoutCSGGroup->addMultiCellWidget(renderCSGDirect, 0, 0, 0, 3);
    renderCSGDirect->setEnabled(false);

    discretizeBoundaryOnly = new QCheckBox("Discretize boundary only",
                                    pageCSGGroup, "discretizeBoundaryOnly");
    connect(discretizeBoundaryOnly, SIGNAL(toggled(bool)),
            this, SLOT(discretizeBoundaryOnlyChanged(bool)));
    layoutCSGGroup->addMultiCellWidget(discretizeBoundaryOnly, 1, 1, 0, 3);

    discretizeModeLabel = new QLabel("Discretization Mode", pageCSGGroup,
                                             "discretizeModeLabel");
    layoutCSGGroup->addWidget(discretizeModeLabel, 2, 0);
    discretizationMode = new QButtonGroup(0, "discretizationMode");
    connect(discretizationMode, SIGNAL(clicked(int)),
            this, SLOT(discretizationModeChanged(int)));
    QRadioButton *discretizeUniform = new QRadioButton("Uniform", pageCSGGroup, "Uniform");
    discretizationMode->insert(discretizeUniform);
    layoutCSGGroup->addWidget(discretizeUniform, 2, 1);
    QRadioButton *discretizeAdaptive = new QRadioButton("Adaptive", pageCSGGroup, "Adaptive");
    discretizationMode->insert(discretizeAdaptive);
    layoutCSGGroup->addWidget(discretizeAdaptive, 2, 2);

    discretizationToleranceLabel = new QLabel("Tolerance(s)", pageCSGGroup,
                                              "discretizationTolerance");
    layoutCSGGroup->addWidget(discretizationToleranceLabel, 3, 0);
    discretizationToleranceLineEdit = new QLineEdit(pageCSGGroup, "discretizationToleranceLineEdit");
    connect(discretizationToleranceLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processDiscretizationToleranceText()));
    connect(discretizationToleranceLineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(processDiscretizationToleranceText(const QString &)));
    layoutCSGGroup->addMultiCellWidget(discretizationToleranceLineEdit, 3, 3, 1, 3);
}

// ****************************************************************************
// Method: QvisMeshManagementWindow::UpdateWindow
//
// Purpose: 
//   This method is called when the animation attributes are updated.
//
// Arguments:
//   doAll : Whether or not to update all widgets.
//
// Programmer: Eric Brugger
// Creation:   Mon Nov 19 14:15:03 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue May 14 11:49:44 PDT 2002
//   Added animation timeout slider.
//
//   Brad Whitlock, Mon Oct 6 16:24:09 PST 2003
//   Added the animation style button group.
//
// ****************************************************************************

void
QvisMeshManagementWindow::UpdateWindow(bool doAll)
{
    MeshManagementAttributes *atts = (MeshManagementAttributes *)subject;

    for(int i = 0; i < atts->NumAttributes(); ++i)
    {
        if(!atts->IsSelected(i) && !doAll)
            continue;

        QString temp;

        switch(i)
        {
        case 0: // overall discretization tolerance
            {   const vector<double> tols = atts->GetDiscretizationTolerance();
                for (int i = 0; i < tols.size(); i++)
                {
                    char tmp[32];
                    SNPRINTF(tmp, sizeof(tmp), "%g ", tols[i]);
                    temp += tmp;
                }
                discretizationToleranceLineEdit->setText(temp);
            }
            break;
        case 1: // discretization tolerance in X
            break;
        case 2: // discretization tolerance in Y
            break;
        case 3: // discretization tolerance in Z
            break;
        case 4: // discretization mode 
            {
                MeshManagementAttributes::DiscretizationModes dMode;
                dMode = atts->GetDiscretizationMode();
                discretizationMode->blockSignals(true);
                if (dMode == MeshManagementAttributes::Uniform)
                    discretizationMode->setButton(0);
                else if (dMode == MeshManagementAttributes::Adaptive)
                    discretizationMode->setButton(1);
                discretizationMode->blockSignals(false);
            }
            break;
        case 5: // discretize boundary only
            discretizeBoundaryOnly->blockSignals(true);
            discretizeBoundaryOnly->setChecked(atts->GetDiscretizeBoundaryOnly());
            discretizeBoundaryOnly->blockSignals(false);
            break;
        case 6: // pass native CSG 
            renderCSGDirect->blockSignals(true);
            renderCSGDirect->setChecked(atts->GetPassNativeCSG());
            renderCSGDirect->blockSignals(false);
            break;
        }
    }
}

// ****************************************************************************
// Method: QvisMeshManagementWindow::GetCurrentValues
//
// Purpose:
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: Mark C. Miller
// Creation:   Thu Oct 24 10:03:40 PDT 2002
//
// Modifications:
//    Brad Whitlock, Wed Nov 23 11:42:26 PDT 2005
//    Fixed for Qt 3.0.2.
//
// ****************************************************************************
void
QvisMeshManagementWindow::GetCurrentValues(const QWidget *widget)
{
    const bool doAll = widget == 0;

    vector<double> temp;
    if (doAll || widget == discretizationToleranceLineEdit)
    {
        StringToDoubleList(discretizationToleranceLineEdit->
                           displayText().latin1(), temp);
        if (mmAtts->GetDiscretizationTolerance() != temp)
            mmAtts->SetDiscretizationTolerance(temp);
    }

    if (doAll || widget == discretizationMode)
    {
        int selectedId = discretizationMode->id(discretizationMode->selected());
        if (selectedId == 0 &&
            mmAtts->GetDiscretizationMode() != MeshManagementAttributes::Uniform)
            mmAtts->SetDiscretizationMode(MeshManagementAttributes::Uniform);
        else if (selectedId == 1 &&
            mmAtts->GetDiscretizationMode() != MeshManagementAttributes::Adaptive)
            mmAtts->SetDiscretizationMode(MeshManagementAttributes::Adaptive);
    }

    if (doAll || widget == discretizeBoundaryOnly)
    {
        if (discretizeBoundaryOnly->isChecked() != mmAtts->GetDiscretizeBoundaryOnly())
            mmAtts->SetDiscretizeBoundaryOnly(discretizeBoundaryOnly->isChecked());
    }

    if (doAll || widget == renderCSGDirect)
    {
        if (renderCSGDirect->isChecked() != mmAtts->GetPassNativeCSG())
            mmAtts->SetPassNativeCSG(renderCSGDirect->isChecked());
    }
}

// ****************************************************************************
// Method: QvisMeshManagementWindow::Apply
//
// Purpose: 
//   This method is called when the animation attributes are updated.
//
// Arguments:
//   ignore : Whether or not to ignore the apply.
//
// Programmer: Eric Brugger
// Creation:   Mon Nov 19 14:15:03 PST 2001
//
// ****************************************************************************

void
QvisMeshManagementWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        // Tell the viewer to set the mesh management attributes.
        if(mmAtts->NumAttributesSelected() > 0 || ignore)
        {
            GetCurrentValues();
            mmAtts->Notify();
        }

        viewer->SetMeshManagementAttributes();
        GUIBase::Warning("Note:  These settings only apply to new plots.  "
                         "To apply them to current plots, re-open the file.");

    }
    else
    {
        // Send the new state to the viewer.
        mmAtts->Notify();
    }
}
 
//
// Qt Slot functions...
//

void
QvisMeshManagementWindow::apply()
{
    Apply(true);
}

void
QvisMeshManagementWindow::tabSelected(const QString &tabLabel)
{
}

void
QvisMeshManagementWindow::renderCSGDirectChanged(bool val)
{
    mmAtts->SetPassNativeCSG(val);
    SetUpdate(false);
    Apply();
}

void
QvisMeshManagementWindow::discretizeBoundaryOnlyChanged(bool val)
{
    mmAtts->SetDiscretizeBoundaryOnly(val);
    SetUpdate(false);
    Apply();
}

void
QvisMeshManagementWindow::discretizationModeChanged(int val)
{
    if (val == 0)
        mmAtts->SetDiscretizationMode(MeshManagementAttributes::Uniform);
    else if (val == 1)
        mmAtts->SetDiscretizationMode(MeshManagementAttributes::Adaptive);
    SetUpdate(false);
    Apply();
}

void
QvisMeshManagementWindow::processDiscretizationToleranceText()
{
    doubleVector temp;
    StringToDoubleList(discretizationToleranceLineEdit->displayText().latin1(), temp);
    if (temp.size())
        mmAtts->SetDiscretizationTolerance(temp);
    Apply();
}

void
QvisMeshManagementWindow::processDiscretizationToleranceText(const QString &tols)
{
    doubleVector temp;
    StringToDoubleList(tols.latin1(), temp);
    if (temp.size())
        mmAtts->SetDiscretizationTolerance(temp);
}
