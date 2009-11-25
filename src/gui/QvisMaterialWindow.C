/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#include "QvisMaterialWindow.h"

#include <MaterialAttributes.h>
#include <ViewerProxy.h>

#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QNarrowLineEdit.h>
#include <QSpinBox>
#include <QWidget>
#include <QButtonGroup>
#include <QRadioButton>
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
//   Brad Whitlock, Wed Apr  9 11:45:36 PDT 2008
//   QString for caption, shortName.
//
// ****************************************************************************

QvisMaterialWindow::QvisMaterialWindow(MaterialAttributes *subj,
                                       const QString &caption,
                                       const QString &shortName,
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
//    Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//    Support for internationalization.
//
//    Cyrus Harrison, Tue Jun 10 10:04:26 PDT 2008
//    Initial Qt4 Port.
//
//    John C. Anderson, Fri Oct 17 16:03:10 2008
//    Added "Discrete" reconstruction option.
//
//    John C. Anderson, Thu Jan 15 10:20:20 2009
//    Added annealing time for Discrete MIR.
//
//    Jeremy Meredith, Fri Feb 13 12:11:07 EST 2009
//    Added material iteration capability.
//
//    Jeremy Meredith, Tue Aug  4 10:47:50 EDT 2009
//    Added Youngs algorithm choice.
//    Added hints for algorithm specificity of some options.
//
// ****************************************************************************

void
QvisMaterialWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout();
    topLayout->addLayout(mainLayout);

    QHBoxLayout *algLayout = new QHBoxLayout();
    algorithmLabel = new QLabel(tr("Algorithm:"), central);
    algLayout->setSpacing(10);
    algLayout->addWidget(algorithmLabel);

    algorithm = new QComboBox(central);
    algorithm->addItem(tr("Tetrahedral (obsolete)"));
    algorithm->addItem(tr("Zoo-based (default)"));
    algorithm->addItem(tr("Isovolume (special-purpose)"));
    algorithm->addItem(tr("Youngs (accurate/discontinuous)"));
    algorithm->addItem(tr("Discrete (accurate/10X memory footprint)"));
    algLayout->addWidget(algorithm);
    connect(algorithm, SIGNAL(activated(int)),
            this, SLOT(algorithmChanged(int)));
    mainLayout->addLayout(algLayout, 0,0, 1,2);

    smoothing = new QCheckBox(tr("Enable interface smoothing (Zoo/Tet only)"), central);
    connect(smoothing, SIGNAL(toggled(bool)),
            this, SLOT(smoothingChanged(bool)));
    mainLayout->addWidget(smoothing, 1,0);

    forceFullConnectivity = new QCheckBox(tr("Force full connectivity (Tet only)"), central);
    connect(forceFullConnectivity, SIGNAL(toggled(bool)),
            this, SLOT(forceFullConnectivityChanged(bool)));
    mainLayout->addWidget(forceFullConnectivity, 2,0);

    forceMIR = new QCheckBox(tr("Force interface reconstruction"), central);
    connect(forceMIR, SIGNAL(toggled(bool)),
            this, SLOT(forceMIRChanged(bool)));
    mainLayout->addWidget(forceMIR, 3,0);

    cleanZonesOnly = new QCheckBox(tr("Clean zones only"), central);
    cleanZonesOnly->setEnabled(false);
    connect(cleanZonesOnly, SIGNAL(toggled(bool)),
            this, SLOT(cleanZonesOnlyChanged(bool)));
    mainLayout->addWidget(cleanZonesOnly, 4,0);

    simplifyHeavilyMixedZones = new QCheckBox(tr("Simplify heavily mixed zones"), 
                                              central);
    connect(simplifyHeavilyMixedZones, SIGNAL(toggled(bool)),
            this, SLOT(simplifyHeavilyMixedZonesChanged(bool)));
    mainLayout->addWidget(simplifyHeavilyMixedZones, 5,0);

    maxMatsPerZoneLabel = new QLabel(tr("Maximum materials per zone"),
                                     central);
    mainLayout->addWidget(maxMatsPerZoneLabel, 6, 0);

    maxMatsPerZone = new QNarrowLineEdit(central);
    connect(maxMatsPerZone, SIGNAL(returnPressed()), this,
            SLOT(maxMatsPerZoneProcessText()));
    mainLayout->addWidget(maxMatsPerZone, 6, 1);

    isoVolumeFractionLabel = new QLabel(tr("Volume Fraction for Isovolume"), 
                                        central);
    mainLayout->addWidget(isoVolumeFractionLabel,7,0);
    isoVolumeFraction = new QNarrowLineEdit(central);
    connect(isoVolumeFraction, SIGNAL(returnPressed()),
            this, SLOT(isoVolumeFractionProcessText()));
    mainLayout->addWidget(isoVolumeFraction, 7,1);

    // Iteration options
    enableIteration = new QCheckBox(tr("Enable iteration (Zoo/Isovol only)"), 
                                              central);
    connect(enableIteration, SIGNAL(toggled(bool)),
            this, SLOT(enableIterationChanged(bool)));
    mainLayout->addWidget(enableIteration, 8,0);

    numIterationsLabel = new QLabel(tr("Number of iterations"), 
                                        central);
    mainLayout->addWidget(numIterationsLabel,9,0);
    numIterations = new QNarrowLineEdit(central);
    connect(numIterations, SIGNAL(returnPressed()),
            this, SLOT(numIterationsProcessText()));
    mainLayout->addWidget(numIterations, 9,1);

    iterationDampingLabel = new QLabel(tr("Convergence rate (>0)"), 
                                        central);
    mainLayout->addWidget(iterationDampingLabel,10,0);
    iterationDamping = new QNarrowLineEdit(central);
    connect(numIterations, SIGNAL(returnPressed()),
            this, SLOT(iterationDampingProcessText()));
    mainLayout->addWidget(iterationDamping, 10,1);

    annealingTimeLabel = new QLabel(tr("Annealing time (seconds)"), central);
    mainLayout->addWidget(annealingTimeLabel,11,0);
    annealingTime = new QNarrowLineEdit(central);
    connect(annealingTime, SIGNAL(returnPressed()),
            this, SLOT(annealingTimeProcessText()));
    mainLayout->addWidget(annealingTime, 11,1);

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
//    John C. Anderson, Thu Jan 15 10:20:20 2009
//    Added annealing time for Discrete MIR.
//
//    Jeremy Meredith, Fri Feb 13 12:11:07 EST 2009
//    Added material iteration capability.
//
//    Jeremy Meredith, Tue Aug  4 13:49:49 EDT 2009
//    Added better sensitivity setting based on algorithm.
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
    enableIteration->blockSignals(true);
    numIterations->blockSignals(true);
    iterationDamping->blockSignals(true);
    annealingTime->blockSignals(true);

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
            annealingTime->setEnabled(atts->GetAlgorithm() == MaterialAttributes::Discrete);
            annealingTimeLabel->setEnabled(atts->GetAlgorithm() == MaterialAttributes::Discrete);
                
            smoothing->setEnabled(
                   atts->GetAlgorithm()==MaterialAttributes::ZooClipping ||
                   atts->GetAlgorithm()==MaterialAttributes::Tetrahedral);
            forceFullConnectivity->setEnabled(
                   atts->GetAlgorithm()==MaterialAttributes::Tetrahedral);
            enableIteration->setEnabled(
                   atts->GetAlgorithm()==MaterialAttributes::ZooClipping ||
                   atts->GetAlgorithm()==MaterialAttributes::Isovolume);
            iterationDamping->setEnabled(
                  (atts->GetAlgorithm()==MaterialAttributes::ZooClipping ||
                   atts->GetAlgorithm()==MaterialAttributes::Isovolume) &&
                  atts->GetIterationEnabled());
            iterationDampingLabel->setEnabled(
                  (atts->GetAlgorithm()==MaterialAttributes::ZooClipping ||
                   atts->GetAlgorithm()==MaterialAttributes::Isovolume) &&
                  atts->GetIterationEnabled());
            numIterations->setEnabled(
                  (atts->GetAlgorithm()==MaterialAttributes::ZooClipping ||
                   atts->GetAlgorithm()==MaterialAttributes::Isovolume) &&
                  atts->GetIterationEnabled());
            numIterationsLabel->setEnabled(
                  (atts->GetAlgorithm()==MaterialAttributes::ZooClipping ||
                   atts->GetAlgorithm()==MaterialAttributes::Isovolume) &&
                  atts->GetIterationEnabled());
            algorithm->setCurrentIndex(atts->GetAlgorithm());
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
          case MaterialAttributes::ID_iterationEnabled:
            enableIteration->setChecked(atts->GetIterationEnabled());
            iterationDamping->setEnabled(
                  (atts->GetAlgorithm()==MaterialAttributes::ZooClipping ||
                   atts->GetAlgorithm()==MaterialAttributes::Isovolume) &&
                  atts->GetIterationEnabled());
            iterationDampingLabel->setEnabled(
                  (atts->GetAlgorithm()==MaterialAttributes::ZooClipping ||
                   atts->GetAlgorithm()==MaterialAttributes::Isovolume) &&
                  atts->GetIterationEnabled());
            numIterations->setEnabled(
                  (atts->GetAlgorithm()==MaterialAttributes::ZooClipping ||
                   atts->GetAlgorithm()==MaterialAttributes::Isovolume) &&
                  atts->GetIterationEnabled());
            numIterationsLabel->setEnabled(
                  (atts->GetAlgorithm()==MaterialAttributes::ZooClipping ||
                   atts->GetAlgorithm()==MaterialAttributes::Isovolume) &&
                  atts->GetIterationEnabled());
            break;
          case MaterialAttributes::ID_numIterations:
            temp.setNum(atts->GetNumIterations());
            numIterations->setText(temp);
            break;
          case MaterialAttributes::ID_iterationDamping:
            temp.setNum(atts->GetIterationDamping());
            iterationDamping->setText(temp);
            break;
          case MaterialAttributes::ID_annealingTime:
            temp.setNum(atts->GetAnnealingTime());
            annealingTime->setText(temp);
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
    enableIteration->blockSignals(false);
    numIterations->blockSignals(false);
    iterationDamping->blockSignals(false);
    annealingTime->blockSignals(false);
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
//    Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//    Support for internationalization.
//
//    John C. Anderson, Thu Jan 15 10:20:20 2009
//    Added annealing time for Discrete MIR.
//
//    Jeremy Meredith, Fri Feb 13 12:11:07 EST 2009
//    Added material iteration capability.
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
        temp = maxMatsPerZone->displayText().simplified();
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
            msg = tr("Max mats per zone must be at least 1."
                     "Resetting to the last good value of %1.").
                  arg(atts->GetMaxMaterialsPerZone());
            Message(msg);
            atts->SetMaxMaterialsPerZone(atts->GetMaxMaterialsPerZone());
        }
    }

    // Do isoVolumeFraction
    if(which_widget == MaterialAttributes::ID_isoVolumeFraction || doAll)
    {
        temp = isoVolumeFraction->displayText().simplified();
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
            msg = tr("The value of isoVolumeFraction was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetIsoVolumeFraction());
            Message(msg);
            atts->SetIsoVolumeFraction(atts->GetIsoVolumeFraction());
        }
    }

    // Do numIterations
    if(which_widget == MaterialAttributes::ID_numIterations || doAll)
    {
        temp = numIterations->displayText().simplified();
        bool okay = !temp.isEmpty();
        if (okay)
        {
            int val = temp.toInt(&okay);
            if (val < 0 || val > 1000)
                okay = false;
            else
                atts->SetNumIterations(val);
        }

        if (!okay)
        {
            msg = tr("The value of numIterations was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetNumIterations());
            Message(msg);
            atts->SetNumIterations(atts->GetNumIterations());
        }
    }

    // Do iterationDamping
    if(which_widget == MaterialAttributes::ID_iterationDamping || doAll)
    {
        temp = iterationDamping->displayText().simplified();
        bool okay = !temp.isEmpty();
        if (okay)
        {
            float val = temp.toFloat(&okay);
            if (val <= 0 || val > 10)
                okay = false;
            else
                atts->SetIterationDamping(val);
        }

        if (!okay)
        {
            msg = tr("The value of iterationDamping was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetIterationDamping());
            Message(msg);
            atts->SetIterationDamping(atts->GetIterationDamping());
        }
    }

    // Do annealingTime
    if(which_widget == MaterialAttributes::ID_annealingTime || doAll)
    {
        temp = annealingTime->displayText().simplified();
        bool okay = !temp.isEmpty();
        if (okay)
        {
            int val = temp.toInt(&okay);
            if (val < 0)
                okay = false;
            else
                atts->SetAnnealingTime(val);
        }

        if (!okay)
        {
            msg = tr("The value of annealingTime was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetAnnealingTime());
            Message(msg);
            atts->SetAnnealingTime(atts->GetAnnealingTime());
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
//    Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//    Support for internationalization.
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
            GUIBase::Warning(tr("Note:  These settings only apply to new plots.  "
                         "To apply them to current plots, re-open the file."));
        else if (timesIssued == 1)
            GUIBase::Warning(tr("Note:  These settings only apply to new plots.  "
                      "To apply them to current plots, re-open the file.  "
                      "VisIt will NOT issue this message any further times "
                      "for this session, but keep in mind that you must "
                      "re-open each time you change the material attributes."));
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

void
QvisMaterialWindow::enableIterationChanged(bool val)
{
    atts->SetIterationEnabled(val);
    Apply();
}


void
QvisMaterialWindow::numIterationsProcessText()
{
    GetCurrentValues(MaterialAttributes::ID_numIterations);
    Apply();
}

void
QvisMaterialWindow::iterationDampingProcessText()
{
    GetCurrentValues(MaterialAttributes::ID_iterationDamping);
    Apply();
}

void
QvisMaterialWindow::annealingTimeProcessText()
{
    GetCurrentValues(MaterialAttributes::ID_annealingTime);
    Apply();
}
