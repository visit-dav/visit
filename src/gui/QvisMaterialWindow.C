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
          case 0: //smoothing
            smoothing->setChecked(atts->GetSmoothing());
            break;
          case 1: //forceMIR
            forceMIR->setChecked(atts->GetForceMIR());
            break;
          case 2: //cleanZonesOnly
            cleanZonesOnly->setChecked(atts->GetCleanZonesOnly());
            break;
          case 3: //needValidConnectivity
            forceFullConnectivity->setChecked(atts->GetNeedValidConnectivity());
            break;
          case 4: //algorithm
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
          case 5: //simplifyHeavilyMixedZones
            simplifyHeavilyMixedZones->setChecked(
                                         atts->GetSimplifyHeavilyMixedZones());
            maxMatsPerZone->setEnabled(atts->GetSimplifyHeavilyMixedZones());
            maxMatsPerZoneLabel->setEnabled(atts->GetSimplifyHeavilyMixedZones());
            break;
          case 6: //maxMatsPerZone
            temp.sprintf("%d", atts->GetMaxMaterialsPerZone());
            maxMatsPerZone->setText(temp);
            break;
          case 7: //isoVolumeFraction
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
// ****************************************************************************

void
QvisMaterialWindow::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);
    QString msg, temp;

    // Do smoothing
    if(which_widget == 0 || doAll)
    {
        // Nothing for smoothing
    }

    // Do forceMIR
    if(which_widget == 1 || doAll)
    {
        // Nothing for forceMIR
    }

    // Do cleanZonesOnly
    if(which_widget == 2 || doAll)
    {
        // Nothing for cleanZonesOnly
    }

    // Do forceFullConnectivity
    if(which_widget == 3 || doAll)
    {
        // Nothing for forceFullConnectivity
    }

    // Do algorithm
    if(which_widget == 4 || doAll)
    {
        // Nothing for algorithm
    }

    // Do simplifyHeavilyMixedZones
    if(which_widget == 5 || doAll)
    {
        // Nothing for simplifyHeavilyMixedZone
    }

    // Do maxMatsPerZone
    if(which_widget == 6 || doAll)
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
    if(which_widget == 7 || doAll)
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
// ****************************************************************************

void
QvisMaterialWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        GetCurrentValues(-1);
        atts->Notify();

        viewer->SetMaterialAttributes();
        GUIBase::Warning("Note:  These settings only apply to new plots.  "
                         "To apply them to current plots, re-open the file.");
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
    viewer->SetDefaultMaterialAttributes();
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
    viewer->ResetMaterialAttributes();
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
    GetCurrentValues(6);
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
    GetCurrentValues(7);
    Apply();
}

