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
#include <vector>

#include <qbuttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qtabwidget.h>
#include <qvbox.h>

#include <snprintf.h>
#include <visit-config.h>

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
//   Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//   Added AllExtraButtons
//
//   Brad Whitlock, Wed Apr  9 11:34:45 PDT 2008
//   QString for caption, shortName.
//
// ****************************************************************************

QvisMeshManagementWindow::QvisMeshManagementWindow(MeshManagementAttributes *subj,
    const QString &caption, const QString &shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(subj, caption, shortName, notepad,
                               QvisPostableWindowObserver::AllExtraButtons)
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
//   Mark C. Miller, Tue Dec  5 18:14:58 PST 2006
//   Changed initialization based on existence of FI library 
//
//   Mark C. Miller, Wed Dec 19 11:32:58 PST 2007
//   Made Qt objects for tolerances a little more both in code and in the
//   running GUI. Changed discretizationTolerances to smallestZone and
//   flatEnough.
//
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
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
    tabs->addTab(pageCSG, tr("CSG Meshing"));

    pageCSGGroup = new QGroupBox(pageCSG, "pageCSGGroup");
    pageCSGGroup->setFrameStyle(QFrame::NoFrame);

    QVBoxLayout *internalLayoutCSGGroup = new QVBoxLayout(pageCSGGroup);
    internalLayoutCSGGroup->addSpacing(10);
    QGridLayout *layoutCSGGroup = new QGridLayout(internalLayoutCSGGroup, 5, 3);
    layoutCSGGroup->setSpacing(5);

    renderCSGDirect = new QCheckBox(tr("Don't discretize. Pass native CSG down pipeline"),
                                    pageCSGGroup, "renderCSGDirect");
    connect(renderCSGDirect, SIGNAL(toggled(bool)),
            this, SLOT(renderCSGDirectChanged(bool)));
    layoutCSGGroup->addMultiCellWidget(renderCSGDirect, 0, 0, 0, 3);
    renderCSGDirect->setEnabled(false);

    discretizeBoundaryOnly = new QCheckBox(tr("Discretize boundary only"),
                                    pageCSGGroup, "discretizeBoundaryOnly");
    connect(discretizeBoundaryOnly, SIGNAL(toggled(bool)),
            this, SLOT(discretizeBoundaryOnlyChanged(bool)));
    layoutCSGGroup->addMultiCellWidget(discretizeBoundaryOnly, 1, 1, 0, 3);

    discretizeModeLabel = new QLabel(tr("Discretization Mode"), pageCSGGroup,
                                             "discretizeModeLabel");
    layoutCSGGroup->addWidget(discretizeModeLabel, 2, 0);
    discretizationMode = new QButtonGroup(0, "discretizationMode");
    connect(discretizationMode, SIGNAL(clicked(int)),
            this, SLOT(discretizationModeChanged(int)));
    QRadioButton *discretizeUniform = new QRadioButton(tr("Uniform"), pageCSGGroup, "Uniform");
    discretizationMode->insert(discretizeUniform);
    layoutCSGGroup->addWidget(discretizeUniform, 2, 1);
    QRadioButton *discretizeAdaptive = new QRadioButton(tr("Adaptive"), pageCSGGroup, "Adaptive");
    discretizationMode->insert(discretizeAdaptive);
#if !HAVE_BILIB
    discretizeAdaptive->setEnabled(false);
#endif
    layoutCSGGroup->addWidget(discretizeAdaptive, 2, 2);

    smallestZoneLabel = new QLabel(tr("Smallest Zone (% bbox diag)"), pageCSGGroup,
                                       "smallestZone");
    layoutCSGGroup->addWidget(smallestZoneLabel, 3, 0);
    smallestZoneLineEdit = new QLineEdit(pageCSGGroup, "smallestZoneLineEdit");
    connect(smallestZoneLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processSmallestZoneText()));
    connect(smallestZoneLineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(processSmallestZoneText(const QString &)));
    layoutCSGGroup->addMultiCellWidget(smallestZoneLineEdit, 3, 3, 1, 3);

    flatEnoughLabel = new QLabel(tr("Flat Enough (recip. curvature)"), pageCSGGroup,
                                       "flatEnough");
    layoutCSGGroup->addWidget(flatEnoughLabel, 4, 0);
    flatEnoughLineEdit = new QLineEdit(pageCSGGroup, "flatEnoughLineEdit");
    connect(flatEnoughLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processFlatEnoughText()));
    connect(flatEnoughLineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(processFlatEnoughText(const QString &)));
    layoutCSGGroup->addMultiCellWidget(flatEnoughLineEdit, 4, 4, 1, 3);
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
//   Mark C. Miller, Tue Dec  5 18:14:58 PST 2006
//   Changed behavior based on existence of FI library 
//
//   Mark C. Miller, Wed Dec 19 11:32:58 PST 2007
//   Made Qt objects for tolerances a little more both in code and in the
//   running GUI. Changed discretizationTolerances to smallestZone and
//   flatEnough.
//
//   Brad Whitlock, Wed Dec 19 15:01:30 PST 2007
//   Made it use ids.
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
        case MeshManagementAttributes::ID_discretizationTolerance:
            {   const vector<double> tols = atts->GetDiscretizationTolerance();
                char tmp[32];
                SNPRINTF(tmp, sizeof(tmp), "%g ", tols[0]);
                temp += tmp;
                smallestZoneLineEdit->setText(temp);
		temp = "";
                SNPRINTF(tmp, sizeof(tmp), "%g ", tols[1]);
                temp += tmp;
                flatEnoughLineEdit->setText(temp);
            }
            break;
        case MeshManagementAttributes::ID_discretizationToleranceX:
            break;
        case MeshManagementAttributes::ID_discretizationToleranceY:
            break;
        case MeshManagementAttributes::ID_discretizationToleranceZ:
            break;
        case MeshManagementAttributes::ID_discretizationMode:
            {
                MeshManagementAttributes::DiscretizationModes dMode;
                dMode = atts->GetDiscretizationMode();
                discretizationMode->blockSignals(true);
                if (dMode == MeshManagementAttributes::Uniform)
                    discretizationMode->setButton(0);
                else if (dMode == MeshManagementAttributes::Adaptive)
                {
#if HAVE_BILIB
                    discretizationMode->setButton(1);
#else
                    GUIBase::Warning(tr("Adaptive not available. "
                                     "Missing boost interval template library. "
                                     "Overriding to Uniform."));
                    discretizationMode->setButton(0);
#endif
                }
                discretizationMode->blockSignals(false);
            }
            break;
        case MeshManagementAttributes::ID_discretizeBoundaryOnly:
            discretizeBoundaryOnly->blockSignals(true);
            discretizeBoundaryOnly->setChecked(atts->GetDiscretizeBoundaryOnly());
            discretizeBoundaryOnly->blockSignals(false);
            break;
        case MeshManagementAttributes::ID_passNativeCSG:
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

    double temp;
    if (doAll || widget == smallestZoneLineEdit)
    {
        bool okay = sscanf(smallestZoneLineEdit->displayText().latin1(), "%lg", &temp) == 1;
        if (okay && mmAtts->GetDiscretizationTolerance()[0] != temp)
	{
	    vector<double> temp1 = mmAtts->GetDiscretizationTolerance();
	    temp1[0] = temp;
            mmAtts->SetDiscretizationTolerance(temp1);
        }
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

        GetViewerMethods()->SetMeshManagementAttributes();
        GUIBase::Warning(tr("Note:  These settings only apply to new plots.  "
                            "To apply them to current plots, re-open the file."));

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

// ****************************************************************************
// Method: QvisMeshManagementWindow::apply
//
// Purpose:
//   Qt slot function called when apply button is clicked.
//
// ****************************************************************************

void
QvisMeshManagementWindow::apply()
{
    Apply(true);
}

// ****************************************************************************
// Method: QvisMeshManagementWindow::makeDefault
//
// Purpose:
//   Qt slot function called when "Make default" button is clicked.
//
// ****************************************************************************

void
QvisMeshManagementWindow::makeDefault()
{
    GetCurrentValues();
    mmAtts->Notify();
    GetViewerMethods()->SetDefaultMeshManagementAttributes();
}

// ****************************************************************************
// Method: QvisMeshManagementWindow::makeDefault
//
// Purpose:
//   Qt slot function called when "Make default" button is clicked.
//
// ****************************************************************************

void
QvisMeshManagementWindow::reset()
{
    GetViewerMethods()->ResetMeshManagementAttributes();
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
    {
#if HAVE_BILIB
        mmAtts->SetDiscretizationMode(MeshManagementAttributes::Adaptive);
#else
        GUIBase::Warning(tr("Adaptive not available. "
                         "Missing boost interval template library. "
                         "Overriding to Uniform."));
        mmAtts->SetDiscretizationMode(MeshManagementAttributes::Uniform);
#endif
    }
    SetUpdate(false);
    Apply();
}

void
QvisMeshManagementWindow::processSmallestZoneText()
{
    double temp = -1.0;
    bool okay = sscanf(smallestZoneLineEdit->displayText().latin1(), "%lg", &temp) == 1;

    if (okay && temp >= 0.0)
    {
        vector<double> temp1 = mmAtts->GetDiscretizationTolerance();
	temp1[0] = temp;
        mmAtts->SetDiscretizationTolerance(temp1);
    }
}

void
QvisMeshManagementWindow::processSmallestZoneText(const QString &tols)
{
    double temp = -1.0;
    bool okay = sscanf(smallestZoneLineEdit->displayText().latin1(), "%lg", &temp) == 1;

    if (okay && temp >= 0.0)
    {
        vector<double> temp1 = mmAtts->GetDiscretizationTolerance();
	temp1[0] = temp;
        mmAtts->SetDiscretizationTolerance(temp1);
    }
}

void
QvisMeshManagementWindow::processFlatEnoughText()
{
    double temp = -1.0;
    bool okay = sscanf(flatEnoughLineEdit->displayText().latin1(), "%lg", &temp) == 1;

    if (okay && temp >= 0.0)
    {
        vector<double> temp1 = mmAtts->GetDiscretizationTolerance();
	temp1[1] = temp;
        mmAtts->SetDiscretizationTolerance(temp1);
    }
}

void
QvisMeshManagementWindow::processFlatEnoughText(const QString &tols)
{
    double temp = -1.0;
    bool okay = sscanf(flatEnoughLineEdit->displayText().latin1(), "%lg", &temp) == 1;

    if (okay && temp >= 0.0)
    {
        vector<double> temp1 = mmAtts->GetDiscretizationTolerance();
	temp1[1] = temp;
        mmAtts->SetDiscretizationTolerance(temp1);
    }
}
