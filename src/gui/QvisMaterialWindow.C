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

#include "QvisMaterialWindow.h"

#include <MaterialAttributes.h>
#include <ViewerProxy.h>

#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <QNarrowLineEdit.h>
#include <qspinbox.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <QvisColorTableButton.h>
#include <QvisOpacitySlider.h>
#include <QvisColorButton.h>
#include <QvisLineStyleWidget.h>
#include <QvisLineWidthWidget.h>
#include <stdio.h>
#include <string>

using std::string;

// ****************************************************************************
// Method: QvisMaterialWindow::QvisMaterialWindow
//
// Purpose: 
//   Constructor
//
// Programmer: Jeremy Meredith
// Creation:   October 24, 2002
//
// Modifications:
//   
// ****************************************************************************

QvisMaterialWindow::QvisMaterialWindow(MaterialAttributes *subj,
                                       const char *caption,
                                       const char *shortName,
                                       QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(subj, caption, shortName, notepad,
                               QvisPostableWindowObserver::AllExtraButtons,
                               false)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisMaterialWindow::~QvisMaterialWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Thu Oct 24 10:03:40 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

QvisMaterialWindow::~QvisMaterialWindow()
{
}


// ****************************************************************************
// Method: QvisMaterialWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: Jeremy Meredith
// Creation:   October 25, 2002
//
// Modifications:
//    Jeremy Meredith, Wed Jul 30 10:46:51 PDT 2003
//    Added a toggle for forcing full connectivity.
//   
//    Jeremy Meredith, Mon Sep 15 17:16:55 PDT 2003
//    Added a toggle for the new material algorithm.
//
//    Hank Childs, Tue Aug 16 15:36:43 PDT 2005
//    Add a toggle for "simplify heavily mixed zones".
//
//    Jeremy Meredith, Thu Aug 18 16:14:59 PDT 2005
//    Changed algorithm selection to a multiple-choice.
//    Added VF for isovolume method.
//
// ****************************************************************************

void
QvisMaterialWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 8,2,  10, "mainLayout");

    QHBox *algBox = new QHBox(central);
    algorithmLabel = new QLabel("Algorithm:", algBox, "algorithmLabel");
    algBox->setSpacing(10);

    algorithm = new QComboBox(false, algBox, "algorithm");
    algorithm->insertItem("Tetrahedral (obsolete)");
    algorithm->insertItem("Zoo-based (default)");
    algorithm->insertItem("Isovolume (special-purpose)");
    connect(algorithm, SIGNAL(activated(int)),
            this, SLOT(algorithmChanged(int)));
    mainLayout->addMultiCellWidget(algBox, 0,0, 0,1);

    smoothing = new QCheckBox("Enable interface smoothing", central, "smoothing");
    connect(smoothing, SIGNAL(toggled(bool)),
            this, SLOT(smoothingChanged(bool)));
    mainLayout->addWidget(smoothing, 1,0);

    forceFullConnectivity = new QCheckBox("Force full connectivity", central, "forceFullConnectivity");
    connect(forceFullConnectivity, SIGNAL(toggled(bool)),
            this, SLOT(forceFullConnectivityChanged(bool)));
    mainLayout->addWidget(forceFullConnectivity, 2,0);

    forceMIR = new QCheckBox("Force interface reconstruction", central, "forceMIR");
    connect(forceMIR, SIGNAL(toggled(bool)),
            this, SLOT(forceMIRChanged(bool)));
    mainLayout->addWidget(forceMIR, 3,0);

    cleanZonesOnly = new QCheckBox("Clean zones only", central, "cleanZonesOnly");
    cleanZonesOnly->setEnabled(false);
    connect(cleanZonesOnly, SIGNAL(toggled(bool)),
            this, SLOT(cleanZonesOnlyChanged(bool)));
    mainLayout->addWidget(cleanZonesOnly, 4,0);

    simplifyHeavilyMixedZones = new QCheckBox("Simplify heavily mixed zones", 
                 central, "simplifyHeavilyMixedZones");
    connect(simplifyHeavilyMixedZones, SIGNAL(toggled(bool)),
            this, SLOT(simplifyHeavilyMixedZonesChanged(bool)));
    mainLayout->addWidget(simplifyHeavilyMixedZones, 5,0);

    maxMatsPerZoneLabel = new QLabel("Maximum materials per zone",
                                     central, "maxMatsPerZoneLabel");
    mainLayout->addWidget(maxMatsPerZoneLabel, 6, 0);

    maxMatsPerZone = new QNarrowLineEdit(central, "maxMatsPerZone");
    connect(maxMatsPerZone, SIGNAL(returnPressed()), this,
            SLOT(maxMatsPerZoneProcessText()));
    mainLayout->addWidget(maxMatsPerZone, 6, 1);

    isoVolumeFractionLabel = new QLabel("Volume Fraction for Isovolume", central, "isoVolumeFractionLabel");
    mainLayout->addWidget(isoVolumeFractionLabel,7,0);
    isoVolumeFraction = new QNarrowLineEdit(central, "isoVolumeFraction");
    connect(isoVolumeFraction, SIGNAL(returnPressed()),
            this, SLOT(isoVolumeFractionProcessText()));
    mainLayout->addWidget(isoVolumeFraction, 7,1);
}


// ****************************************************************************
// Method: QvisMaterialWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: Jeremy Meredith
// Creation:   October 24, 2002
//
// Modifications:
//    Jeremy Meredith, Wed Jul 30 10:46:51 PDT 2003
//    Added a toggle for forcing full connectivity.
//   
//    Jeremy Meredith, Mon Sep 15 17:20:57 PDT 2003
//    Added a toggle for the new MIR algorithm.
//
//    Hank Childs, Tue Aug 16 15:36:43 PDT 2005
//    Add a toggle for "simplify heavily mixed zones".
//
//    Jeremy Meredith, Thu Aug 18 16:14:59 PDT 2005
//    Changed algorithm selection to a multiple-choice.
//    Added VF for isovolume method.
//
//    Brad Whitlock, Fri Dec 14 17:38:49 PST 2007
//    Made it use ids.
//
// ****************************************************************************

void
QvisMaterialWindow::UpdateWindow(bool doAll)
{
    QString temp;

    smoothing->blockSignals(true);
    forceMIR->blockSignals(true);
    cleanZonesOnly->blockSignals(true);
    forceFullConnectivity->blockSignals(true);
    algorithm->blockSignals(true);
    simplifyHeavilyMixedZones->blockSignals(true);
    maxMatsPerZone->blockSignals(true);
    isoVolumeFraction->blockSignals(true);

    for(int i = 0; i < atts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!atts->IsSelected(i))
            {
                continue;
            }
        }

        switch(i)
        {
          case MaterialAttributes::ID_smoothing:
            smoothing->setChecked(atts->GetSmoothing());
            break;
          case MaterialAttributes::ID_forceMIR:
            forceMIR->setChecked(atts->GetForceMIR());
            break;
          case MaterialAttributes::ID_cleanZonesOnly:
            cleanZonesOnly->setChecked(atts->GetCleanZonesOnly());
            break;
          case MaterialAttributes::ID_needValidConnectivity:
            forceFullConnectivity->setChecked(atts->GetNeedValidConnectivity());
            break;
          case MaterialAttributes::ID_algorithm:
            if (atts->GetAlgorithm() == MaterialAttributes::Isovolume)
            {
                isoVolumeFraction->setEnabled(true);
                isoVolumeFractionLabel->setEnabled(true);
            }
            else
            {
                isoVolumeFraction->setEnabled(false);
                isoVolumeFractionLabel->setEnabled(false);
            }
            algorithm->setCurrentItem(atts->GetAlgorithm());
            break;
          case MaterialAttributes::ID_simplifyHeavilyMixedZones:
            simplifyHeavilyMixedZones->setChecked(
                                         atts->GetSimplifyHeavilyMixedZones());
            maxMatsPerZone->setEnabled(atts->GetSimplifyHeavilyMixedZones());
            maxMatsPerZoneLabel->setEnabled(atts->GetSimplifyHeavilyMixedZones());
            break;
          case MaterialAttributes::ID_maxMaterialsPerZone:
            temp.sprintf("%d", atts->GetMaxMaterialsPerZone());
            maxMatsPerZone->setText(temp);
            break;
          case MaterialAttributes::ID_isoVolumeFraction:
            temp.setNum(atts->GetIsoVolumeFraction());
            isoVolumeFraction->setText(temp);
            break;
        }
    }

    smoothing->blockSignals(false);
    forceMIR->blockSignals(false);
    cleanZonesOnly->blockSignals(false);
    forceFullConnectivity->blockSignals(false);
    algorithm->blockSignals(false);
    simplifyHeavilyMixedZones->blockSignals(false);
    maxMatsPerZone->blockSignals(false);
    isoVolumeFraction->blockSignals(false);
}


// ****************************************************************************
// Method: QvisMaterialWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Thu Oct 24 10:03:40 PDT 2002
//
// Modifications:
//    Jeremy Meredith, Wed Jul 30 10:46:51 PDT 2003
//    Added a toggle for forcing full connectivity.
//   
//    Jeremy Meredith, Mon Sep 15 17:17:12 PDT 2003
//    Added the toggle for the new MIR algorithm.
//
//    Hank Childs, Tue Aug 16 15:36:43 PDT 2005
//    Add a toggle for "simplify heavily mixed zones".
//
//    Jeremy Meredith, Thu Aug 18 16:14:59 PDT 2005
//    Changed algorithm selection to a multiple-choice.
//    Added VF for isovolume method.
//
//    Brad Whitlock, Fri Dec 14 17:40:16 PST 2007
//    Made it use ids.
//
// ****************************************************************************

void
QvisMaterialWindow::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);
    QString msg, temp;

    // Do smoothing
    if(which_widget == MaterialAttributes::ID_smoothing || doAll)
    {
        // Nothing for smoothing
    }

    // Do forceMIR
    if(which_widget == MaterialAttributes::ID_forceMIR || doAll)
    {
        // Nothing for forceMIR
    }

    // Do cleanZonesOnly
    if(which_widget == MaterialAttributes::ID_cleanZonesOnly || doAll)
    {
        // Nothing for cleanZonesOnly
    }

    // Do needValidConnectivity
    if(which_widget == MaterialAttributes::ID_needValidConnectivity || doAll)
    {
        // Nothing for needValidConnectivity
    }

    // Do algorithm
    if(which_widget == MaterialAttributes::ID_algorithm || doAll)
    {
        // Nothing for algorithm
    }

    // Do simplifyHeavilyMixedZones
    if(which_widget == MaterialAttributes::ID_simplifyHeavilyMixedZones || doAll)
    {
        // Nothing for simplifyHeavilyMixedZones
    }

    // Do maxMatsPerZone
    if(which_widget == MaterialAttributes::ID_maxMaterialsPerZone || doAll)
    {
        temp = maxMatsPerZone->displayText().simplifyWhiteSpace();
        bool okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            if (val < 1)
                okay = false;
            else
                atts->SetMaxMaterialsPerZone(val);
        }

        if(!okay)
        {
            msg.sprintf("Max mats per zone must be at least 1."
                "Resetting to the last good value of %d.",
                atts->GetMaxMaterialsPerZone());
            Message(msg);
            atts->SetMaxMaterialsPerZone(atts->GetMaxMaterialsPerZone());
        }
    }

    // Do isoVolumeFraction
    if(which_widget == MaterialAttributes::ID_isoVolumeFraction || doAll)
    {
        temp = isoVolumeFraction->displayText().simplifyWhiteSpace();
        bool okay = !temp.isEmpty();
        if (okay)
        {
            float val = temp.toFloat(&okay);
            if (val < 0 || val > 1)
                okay = false;
            else
                atts->SetIsoVolumeFraction(val);
        }

        if (!okay)
        {
            msg.sprintf("The value of isoVolumeFraction was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetIsoVolumeFraction());
            Message(msg);
            atts->SetIsoVolumeFraction(atts->GetIsoVolumeFraction());
        }
    }
}


// ****************************************************************************
// Method: QvisMaterialWindow::Apply
//
// Purpose: 
//   Called to apply changes in the subject.
//
// Programmer: xml2window
// Creation:   Thu Oct 24 10:03:40 PDT 2002
//
// Modifications:
//    Jeremy Meredith, Fri Jan 24 11:05:28 PST 2003
//    Added a message so as not to confuse users.
//
//    Hank Childs, Mon Jun 11 21:33:42 PDT 2007
//    Only issue the warning a few times.
//
// ****************************************************************************

void
QvisMaterialWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        GetCurrentValues(-1);
        atts->Notify();

        GetViewerMethods()->SetMaterialAttributes();
        static int timesIssued = 0;
        if (timesIssued == 0)
            GUIBase::Warning("Note:  These settings only apply to new plots.  "
                         "To apply them to current plots, re-open the file.");
        else if (timesIssued == 1)
            GUIBase::Warning("Note:  These settings only apply to new plots.  "
                      "To apply them to current plots, re-open the file.  "
                      "VisIt will NOT issue this message any further times "
                      "for this session, but keep in mind that you must "
                      "re-open each time you change the material attributes.");
        timesIssued++;
    }
    else
        atts->Notify();
}


//
// Qt Slot functions
//


// ****************************************************************************
// Method: QvisMaterialWindow::apply
//
// Purpose: 
//   Qt slot function called when apply button is clicked.
//
// Programmer: xml2window
// Creation:   Thu Oct 24 10:03:40 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisMaterialWindow::apply()
{
    Apply(true);
}


// ****************************************************************************
// Method: QvisMaterialWindow::makeDefault
//
// Purpose: 
//   Qt slot function called when "Make default" button is clicked.
//
// Programmer: xml2window
// Creation:   Thu Oct 24 10:03:40 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisMaterialWindow::makeDefault()
{
    GetCurrentValues(-1);
    atts->Notify();
    GetViewerMethods()->SetDefaultMaterialAttributes();
}


// ****************************************************************************
// Method: QvisMaterialWindow::reset
//
// Purpose: 
//   Qt slot function called when reset button is clicked.
//
// Programmer: Jeremy Meredith
// Creation:   October 24, 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisMaterialWindow::reset()
{
    GetViewerMethods()->ResetMaterialAttributes();
}


void
QvisMaterialWindow::smoothingChanged(bool val)
{
    atts->SetSmoothing(val);
    Apply();
}


void
QvisMaterialWindow::forceMIRChanged(bool val)
{
    atts->SetForceMIR(val);
    Apply();
}


void
QvisMaterialWindow::forceFullConnectivityChanged(bool val)
{
    atts->SetNeedValidConnectivity(val);
    Apply();
}


void
QvisMaterialWindow::cleanZonesOnlyChanged(bool val)
{
    atts->SetCleanZonesOnly(val);
    Apply();
}


void
QvisMaterialWindow::simplifyHeavilyMixedZonesChanged(bool val)
{
    atts->SetSimplifyHeavilyMixedZones(val);
    Apply();
}


void
QvisMaterialWindow::maxMatsPerZoneProcessText()
{
    GetCurrentValues(MaterialAttributes::ID_maxMaterialsPerZone);
    Apply();
}


void
QvisMaterialWindow::algorithmChanged(int val)
{
    if(val != atts->GetAlgorithm())
    {
        atts->SetAlgorithm(MaterialAttributes::Algorithm(val));
        Apply();
    }
}

void
QvisMaterialWindow::isoVolumeFractionProcessText()
{
    GetCurrentValues(MaterialAttributes::ID_isoVolumeFraction);
    Apply();
}

