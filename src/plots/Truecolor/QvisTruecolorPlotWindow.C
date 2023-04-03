// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "QvisTruecolorPlotWindow.h"

#include <TruecolorAttributes.h>
#include <ViewerProxy.h>

#include <QCheckBox>
#include <QLabel>
#include <QGroupBox>
#include <QLayout>
#include <QvisOpacitySlider.h>


// ****************************************************************************
// Method: QvisTruecolorPlotWindow::QvisTruecolorPlotWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Tue Jun 15 11:10:32 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

QvisTruecolorPlotWindow::QvisTruecolorPlotWindow(const int type,
                         TruecolorAttributes *subj,
                         const QString &caption,
                         const QString &shortName,
                         QvisNotepadArea *notepad)
    : QvisPostableWindowObserver(subj, caption, shortName, notepad)
{
    plotType = type;
    atts = subj;
}


// ****************************************************************************
// Method: QvisTruecolorPlotWindow::~QvisTruecolorPlotWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Tue Jun 15 11:10:32 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

QvisTruecolorPlotWindow::~QvisTruecolorPlotWindow()
{
}


// ****************************************************************************
// Method: QvisTruecolorPlotWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Tue Jun 15 11:10:32 PDT 2004
//
// Modifications:
//   Kathleen Bonnell, Mon Jul 25 15:27:06 PDT 2005
//   Added lighting checkbox.
//
//   Brad Whitlock, Wed Apr 23 12:09:16 PDT 2008
//   Added tr()'s
//
//   Brad Whitlock, Tue Aug 5 20:03:23 PST 2008
//   Qt 4.
//
//   Allen Sanderson, Sun Mar  7 12:49:56 PST 2010
//   Change layout of window for 2.0 interface changes.
//
// ****************************************************************************

void
QvisTruecolorPlotWindow::CreateWindowContents()
{
    //
    // Create the color stuff
    //
    QGroupBox * colorGroup = new QGroupBox(central);
    colorGroup->setTitle(tr("Color"));
    topLayout->addWidget(colorGroup);

    QGridLayout *colorLayout = new QGridLayout(colorGroup);
    colorLayout->setContentsMargins(5,5,5,5);
    colorLayout->setSpacing(10);
 
    opacityLabel = new QLabel(tr("Opacity"), central);
    colorLayout->addWidget(opacityLabel,0,0);
    opacity = new QvisOpacitySlider(0,255,25,255, central, NULL);
    opacity->setTickInterval(64);
    opacity->setGradientColor(QColor(0, 0, 0));
    connect(opacity, SIGNAL(valueChanged(int, const void*)),
            this, SLOT(opacityChanged(int, const void*)));
    colorLayout->addWidget(opacity, 0, 1, 1, 3);

    //
    // Create the misc stuff
    //
    QGroupBox * miscGroup = new QGroupBox(central);
    miscGroup->setTitle(tr("Misc"));
    topLayout->addWidget(miscGroup);

    QGridLayout *miscLayout = new QGridLayout(miscGroup);
    miscLayout->setContentsMargins(5,5,5,5);
    miscLayout->setSpacing(10);
 
    // Create the lighting toggle
    lightingToggle = new QCheckBox(tr("Lighting"), central);
    connect(lightingToggle, SIGNAL(toggled(bool)),
            this, SLOT(lightingToggled(bool)));
    miscLayout->addWidget(lightingToggle, 0, 1);
}


// ****************************************************************************
// Method: QvisTruecolorPlotWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: Chris Wojtan
// Creation:   Tue Jun 15 11:10:32 PDT 2004
//
// Modifications:
//   Kathleen Bonnell, Mon Jul 25 15:27:06 PDT 2005
//   Added lighting.
//   
// ****************************************************************************

void
QvisTruecolorPlotWindow::UpdateWindow(bool doAll)
{
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
          case 0: //opacity
            opacity->blockSignals(true);
            opacity->setValue(int((float)atts->GetOpacity()*255.f));
            opacity->blockSignals(false);
            break;
          case 1: //lighting
            lightingToggle->blockSignals(true);
            lightingToggle->setChecked(atts->GetLightingFlag());
            lightingToggle->blockSignals(false);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisTruecolorPlotWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Tue Jun 15 11:10:32 PDT 2004
//
// Modifications:
//   Kathleen Bonnell, Wed Jun 4 07:57:23 PDT 2008
//   Removed unused variable.
//   
// ****************************************************************************

void
QvisTruecolorPlotWindow::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);
    QString msg, temp;

    // Do opacity
    if(which_widget == 0 || doAll)
    {
        // Nothing for opacity
    }

}


// ****************************************************************************
// Method: QvisTruecolorPlotWindow::Apply
//
// Purpose: 
//   Called to apply changes in the subject.
//
// Programmer: xml2window
// Creation:   Tue Jun 15 11:10:32 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisTruecolorPlotWindow::Apply(bool ignore)
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


//
// Qt Slot functions
//


// ****************************************************************************
// Method: QvisTruecolorPlotWindow::apply
//
// Purpose: 
//   Qt slot function called when apply button is clicked.
//
// Programmer: xml2window
// Creation:   Tue Jun 15 11:10:32 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisTruecolorPlotWindow::apply()
{
    Apply(true);
}


// ****************************************************************************
// Method: QvisTruecolorPlotWindow::makeDefault
//
// Purpose: 
//   Qt slot function called when "Make default" button is clicked.
//
// Programmer: xml2window
// Creation:   Tue Jun 15 11:10:32 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisTruecolorPlotWindow::makeDefault()
{
    GetCurrentValues(-1);
    atts->Notify();
    GetViewerMethods()->SetDefaultPlotOptions(plotType);
}


// ****************************************************************************
// Method: QvisTruecolorPlotWindow::reset
//
// Purpose: 
//   Qt slot function called when reset button is clicked.
//
// Programmer: xml2window
// Creation:   Tue Jun 15 11:10:32 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisTruecolorPlotWindow::reset()
{
    GetViewerMethods()->ResetPlotOptions(plotType);
}


void
QvisTruecolorPlotWindow::opacityChanged(int opacity, const void*)
{
    atts->SetOpacity((float)opacity/255.);
    Apply();
}

void
QvisTruecolorPlotWindow::lightingToggled(bool val)
{
    atts->SetLightingFlag(val);
    Apply();
}


