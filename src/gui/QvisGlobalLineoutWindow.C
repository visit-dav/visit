#include "QvisGlobalLineoutWindow.h"

#include <GlobalLineoutAttributes.h>
#include <ViewerProxy.h>

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <QNarrowLineEdit.h>
#include <stdio.h>


// ****************************************************************************
// Method: QvisGlobalLineoutWindow::QvisGlobalLineoutWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Fri Nov 19 10:46:23 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

QvisGlobalLineoutWindow::QvisGlobalLineoutWindow(
                         GlobalLineoutAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisPostableWindowObserver(subj, caption, shortName, notepad,
                         QvisPostableWindowObserver::ApplyButton, false)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisGlobalLineoutWindow::~QvisGlobalLineoutWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Fri Nov 19 10:46:23 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

QvisGlobalLineoutWindow::~QvisGlobalLineoutWindow()
{
}


// ****************************************************************************
// Method: QvisGlobalLineoutWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Fri Nov 19 10:46:23 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisGlobalLineoutWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 3,2,  10, "mainLayout");


    //
    // CreateWindow
    //
    createWindow = new QCheckBox("Use 1st unused window\nor create new one.", 
                                  central, "createWindow");
    connect(createWindow, SIGNAL(toggled(bool)),
            this, SLOT(createWindowChanged(bool)));
    mainLayout->addMultiCellWidget(createWindow, 0,0,0,1);

    //
    // WindowId
    //
    windowIdLabel = new QLabel("Window #", central, "windowIdLabel");
    mainLayout->addWidget(windowIdLabel,1,0);
    windowId = new QNarrowLineEdit(central, "windowId");
    connect(windowId, SIGNAL(returnPressed()),
            this, SLOT(windowIdProcessText()));
    mainLayout->addWidget(windowId, 1,1);

    //
    // Dynamic
    //
    Dynamic = new QCheckBox("Dynamic", central, "Dynamic");
    connect(Dynamic, SIGNAL(toggled(bool)),
            this, SLOT(DynamicChanged(bool)));
    mainLayout->addWidget(Dynamic, 2,0);

    //
    // Want the next items grouped.
    //
    QGroupBox *gbox = new QGroupBox(central, "GBox");
    topLayout->addWidget(gbox);

    QVBoxLayout *blayout = new QVBoxLayout(gbox);
    blayout->setMargin(5);

    QGridLayout *qgrid = new QGridLayout(blayout, 5, 2);
    qgrid->setMargin(5);
    
    QLabel *msg = new QLabel(gbox); 
    msg->setText("These items can be overridden\nby Lineout Operator");
    msg->setAlignment(Qt::AlignCenter);
    qgrid->addMultiCellWidget(msg, 0,0,0,1);
    qgrid->setRowSpacing(1,10);

    //
    // SamplingOn
    //
    samplingOn = new QCheckBox("Use Sampling", gbox, "samplingOn");
    connect(samplingOn, SIGNAL(toggled(bool)),
            this, SLOT(samplingOnChanged(bool)));
    qgrid->addWidget(samplingOn, 2,0);

    //
    // NumSamples
    //
    numSamplesLabel = new QLabel("Sample Points ", 
                                  gbox, "numSamplesLabel");
    numSamplesLabel->setAlignment(Qt::AlignCenter);
    qgrid->addWidget(numSamplesLabel,3,0);

    numSamples = new QNarrowLineEdit(gbox, "numSamples");
    connect(numSamples, SIGNAL(returnPressed()),
            this, SLOT(numSamplesProcessText()));
    qgrid->addWidget(numSamples, 3,1);

    //
    // ReflineLabels
    //
    createReflineLabels = new QCheckBox("Create refline labels", 
                                        gbox, "createReflineLabels");
    connect(createReflineLabels, SIGNAL(toggled(bool)),
            this, SLOT(createReflineLabelsChanged(bool)));
    qgrid->addWidget(createReflineLabels, 4,0);
}


// ****************************************************************************
// Method: QvisGlobalLineoutWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Fri Nov 19 10:46:23 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisGlobalLineoutWindow::UpdateWindow(bool doAll)
{
    QString temp;

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
          case 0: //Dynamic
            Dynamic->setChecked(atts->GetDynamic());
            break;
          case 1: //createWindow
            if (atts->GetCreateWindow() == false)
            {
                windowId->setEnabled(true);
                windowIdLabel->setEnabled(true);
            }
            else
            {
                windowId->setEnabled(false);
                windowIdLabel->setEnabled(false);
            }
            createWindow->setChecked(atts->GetCreateWindow());
            break;
          case 2: //windowId
            temp.sprintf("%d", atts->GetWindowId());
            windowId->setText(temp);
            break;
          case 3: //samplingOn
            if (atts->GetSamplingOn() == true)
            {
                numSamples->setEnabled(true);
                numSamplesLabel->setEnabled(true);
            }
            else
            {
                numSamples->setEnabled(false);
                numSamplesLabel->setEnabled(false);
            }
            samplingOn->setChecked(atts->GetSamplingOn());
            break;
          case 4: //numSamples
            temp.sprintf("%d", atts->GetNumSamples());
            numSamples->setText(temp);
            break;
          case 5: //createReflineLabels
            createReflineLabels->setChecked(atts->GetCreateReflineLabels());
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisGlobalLineoutWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Fri Nov 19 10:46:23 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisGlobalLineoutWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do Dynamic
    if(which_widget == 0 || doAll)
    {
        // Nothing for Dynamic
    }

    // Do createWindow
    if(which_widget == 1 || doAll)
    {
        // Nothing for createWindow
    }

    // Do windowId
    if(which_widget == 2 || doAll)
    {
        temp = windowId->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetWindowId(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of windowId was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetWindowId());
            Message(msg);
            atts->SetWindowId(atts->GetWindowId());
        }
    }

    // Do samplingOn
    if(which_widget == 3 || doAll)
    {
        // Nothing for samplingOn
    }

    // Do numSamples
    if(which_widget == 4 || doAll)
    {
        temp = numSamples->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetNumSamples(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of numSamples was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetNumSamples());
            Message(msg);
            atts->SetNumSamples(atts->GetNumSamples());
        }
    }

    // Do createReflineLabels
    if(which_widget == 5 || doAll)
    {
        // Nothing for createReflineLabels
    }

}


// ****************************************************************************
// Method: QvisGlobalLineoutWindow::Apply
//
// Purpose: 
//   Called to apply changes in the subject.
//
// Programmer: xml2window
// Creation:   Fri Nov 19 10:46:23 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisGlobalLineoutWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        GetCurrentValues(-1);
        atts->Notify();

        viewer->SetGlobalLineoutAttributes();
    }
    else
        atts->Notify();
}


//
// Qt Slot functions
//


// ****************************************************************************
// Method: QvisGlobalLineoutWindow::apply
//
// Purpose: 
//   Qt slot function called when apply button is clicked.
//
// Programmer: xml2window
// Creation:   Fri Nov 19 10:46:23 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisGlobalLineoutWindow::apply()
{
    Apply(true);
}


void
QvisGlobalLineoutWindow::DynamicChanged(bool val)
{
    atts->SetDynamic(val);
    Apply();
}


void
QvisGlobalLineoutWindow::createWindowChanged(bool val)
{
    atts->SetCreateWindow(val);
    Apply();
}


void
QvisGlobalLineoutWindow::windowIdProcessText()
{
    GetCurrentValues(2);
    Apply();
}


void
QvisGlobalLineoutWindow::samplingOnChanged(bool val)
{
    atts->SetSamplingOn(val);
    Apply();
}


void
QvisGlobalLineoutWindow::numSamplesProcessText()
{
    GetCurrentValues(4);
    Apply();
}


void
QvisGlobalLineoutWindow::createReflineLabelsChanged(bool val)
{
    atts->SetCreateReflineLabels(val);
    Apply();
}


