#include "QvisMaterialWindow.h"

#include <MaterialAttributes.h>
#include <ViewerProxy.h>

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
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
// ****************************************************************************

void
QvisMaterialWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 5,2,  10, "mainLayout");


    smoothing = new QCheckBox("Enable interface smoothing", central, "smoothing");
    connect(smoothing, SIGNAL(toggled(bool)),
            this, SLOT(smoothingChanged(bool)));
    mainLayout->addWidget(smoothing, 0,0);

    forceFullConnectivity = new QCheckBox("Force full connectivity", central, "forceFullConnectivity");
    connect(forceFullConnectivity, SIGNAL(toggled(bool)),
            this, SLOT(forceFullConnectivityChanged(bool)));
    mainLayout->addWidget(forceFullConnectivity, 1,0);

    forceMIR = new QCheckBox("Force interface reconstruction", central, "forceMIR");
    connect(forceMIR, SIGNAL(toggled(bool)),
            this, SLOT(forceMIRChanged(bool)));
    mainLayout->addWidget(forceMIR, 2,0);

    useNewMIR = new QCheckBox("Use new MIR algorithm", central, "useNewMIR");
    connect(useNewMIR, SIGNAL(toggled(bool)),
            this, SLOT(useNewMIRChanged(bool)));
    mainLayout->addWidget(useNewMIR, 3,0);

    cleanZonesOnly = new QCheckBox("Clean zones only", central, "cleanZonesOnly");
    cleanZonesOnly->setEnabled(false);
    connect(cleanZonesOnly, SIGNAL(toggled(bool)),
            this, SLOT(cleanZonesOnlyChanged(bool)));
    mainLayout->addWidget(cleanZonesOnly, 4,0);
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
// ****************************************************************************

void
QvisMaterialWindow::UpdateWindow(bool doAll)
{
    QString temp;

    smoothing->blockSignals(true);
    forceMIR->blockSignals(true);
    cleanZonesOnly->blockSignals(true);
    forceFullConnectivity->blockSignals(true);
    useNewMIR->blockSignals(true);

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
          case 4: //useNewMIR
            useNewMIR->setChecked(atts->GetUseNewMIRAlgorithm());
            break;
        }
    }

    smoothing->blockSignals(false);
    forceMIR->blockSignals(false);
    cleanZonesOnly->blockSignals(false);
    forceFullConnectivity->blockSignals(false);
    useNewMIR->blockSignals(false);
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
// ****************************************************************************

void
QvisMaterialWindow::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);

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

    // Do useNewMIR
    if(which_widget == 4 || doAll)
    {
        // Nothing for useNewMIR
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
QvisMaterialWindow::useNewMIRChanged(bool val)
{
    atts->SetUseNewMIRAlgorithm(val);
    Apply();
}


