#include "QvisTruecolorPlotWindow.h"

#include <TruecolorAttributes.h>
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
                         const char *caption,
                         const char *shortName,
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
//   
// ****************************************************************************

void
QvisTruecolorPlotWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 1,2,  10, "mainLayout");


    opacityLabel = new QLabel("opacity", central, "opacityLabel");
    mainLayout->addWidget(opacityLabel,0,0);
    opacity = new QvisOpacitySlider(0,255,25,255, central, "opacity", NULL);
    opacity->setTickInterval(64);
    opacity->setGradientColor(QColor(0, 0, 0));
    connect(opacity, SIGNAL(valueChanged(int, const void*)),
            this, SLOT(opacityChanged(int, const void*)));
    mainLayout->addWidget(opacity, 0,1);

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
//   
// ****************************************************************************

void
QvisTruecolorPlotWindow::UpdateWindow(bool doAll)
{
    QString temp;
    double r;

    for(int i = 0; i < atts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!atts->IsSelected(i))
            {
                continue;
            }
        }

        const double         *dptr;
        const float          *fptr;
        const int            *iptr;
        const char           *cptr;
        const unsigned char  *uptr;
        const string         *sptr;
        QColor                tempcolor;
        switch(i)
        {
          case 0: //opacity
            opacity->blockSignals(true);
            opacity->setValue(int(atts->GetOpacity()*255.));
            opacity->blockSignals(false);
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
//   
// ****************************************************************************

void
QvisTruecolorPlotWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
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

        viewer->SetPlotOptions(plotType);
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
    viewer->SetDefaultPlotOptions(plotType);
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
    viewer->ResetPlotOptions(plotType);
}


void
QvisTruecolorPlotWindow::opacityChanged(int opacity, const void*)
{
    atts->SetOpacity((float)opacity/255.);
    Apply();
}


