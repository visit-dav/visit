#include "QvisTopologyPlotWindow.h"

#include <TopologyAttributes.h>
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
// Method: QvisTopologyPlotWindow::QvisTopologyPlotWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Tue Jul 1 08:50:16 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

QvisTopologyPlotWindow::QvisTopologyPlotWindow(const int type,
                         TopologyAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisPostableWindowObserver(subj, caption, shortName, notepad)
{
    plotType = type;
    atts = subj;
}


// ****************************************************************************
// Method: QvisTopologyPlotWindow::~QvisTopologyPlotWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Tue Jul 1 08:50:16 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

QvisTopologyPlotWindow::~QvisTopologyPlotWindow()
{
}


// ****************************************************************************
// Method: QvisTopologyPlotWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Tue Jul 1 08:50:16 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisTopologyPlotWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 11,2,  10, "mainLayout");


    mainLayout->addWidget(new QLabel("Line width", central, "lineWidthLabel"),0,0);
    lineWidth = new QvisLineWidthWidget(0, central, "lineWidth");
    connect(lineWidth, SIGNAL(lineWidthChanged(int)),
            this, SLOT(lineWidthChanged(int)));
    mainLayout->addWidget(lineWidth, 0,1);

    mainLayout->addWidget(new QLabel("Line style", central, "lineStyleLabel"),1,0);
    lineStyle = new QvisLineStyleWidget(0, central, "lineStyle");
    connect(lineStyle, SIGNAL(lineStyleChanged(int)),
            this, SLOT(lineStyleChanged(int)));
    mainLayout->addWidget(lineStyle, 1,1);


    mainLayout->addWidget(new QLabel("Min Color", central, "minColor"), 2, 0);
    minColorButton = new QvisColorButton(central, "minColorButton");
    connect(minColorButton, SIGNAL(selectedColor(const QColor &)),
                                this, SLOT(ColorChanged(const QColor &)));
    mainLayout->addWidget(minColorButton, 2, 1); 
    
    mainLayout->addWidget(new QLabel("Opacity", central, "minOpacityLabel"),3,0);
    minOpacity = new QvisOpacitySlider(0,255,25,255, central, "minOpacity", NULL);
    minOpacity->setTickInterval(64);
    minOpacity->setGradientColor(QColor(0, 0, 0));
    connect(minOpacity, SIGNAL(valueChanged(int, const void*)),
            this, SLOT(minOpacityChanged(int, const void*)));
    mainLayout->addWidget(minOpacity, 3,1);

    mainLayout->addWidget(new QLabel("Min Plateau Color", central, "minPlateauColor"), 4, 0);
    minPlateauColorButton = new QvisColorButton(central, "minPlateauColorButton");
    connect(minPlateauColorButton, SIGNAL(selectedColor(const QColor &)),
                                this, SLOT(ColorChanged(const QColor &)));
    mainLayout->addWidget(minPlateauColorButton, 4, 1); 

    mainLayout->addWidget(new QLabel("Opacity", central, "minPlateauOpacityLabel"),5,0);
    minPlateauOpacity = new QvisOpacitySlider(0,255,25,255, central, "minPlateauOpacity", NULL);
    minPlateauOpacity->setTickInterval(64);
    minPlateauOpacity->setGradientColor(QColor(0, 0, 0));
    connect(minPlateauOpacity, SIGNAL(valueChanged(int, const void*)),
            this, SLOT(minPlateauOpacityChanged(int, const void*)));
    mainLayout->addWidget(minPlateauOpacity, 5,1);


    mainLayout->addWidget(new QLabel("Max Plateau Color", central, "maxPlateauColor"), 6, 0);
    maxPlateauColorButton = new QvisColorButton(central, "maxPlateauColorButton");
    connect(maxPlateauColorButton, SIGNAL(selectedColor(const QColor &)),
                                this, SLOT(ColorChanged(const QColor &)));
    mainLayout->addWidget(maxPlateauColorButton, 6, 1); 

    mainLayout->addWidget(new QLabel("Opacity", central, "maxPlateauOpacityLabel"),7,0);
    maxPlateauOpacity = new QvisOpacitySlider(0,255,25,255, central, "maxPlateauOpacity", NULL);
    maxPlateauOpacity->setTickInterval(64);
    maxPlateauOpacity->setGradientColor(QColor(0, 0, 0));
    connect(maxPlateauOpacity, SIGNAL(valueChanged(int, const void*)),
            this, SLOT(maxPlateauOpacityChanged(int, const void*)));
    mainLayout->addWidget(maxPlateauOpacity, 7,1);

    mainLayout->addWidget(new QLabel("Max Color", central, "maxColor"), 8, 0);
    maxColorButton = new QvisColorButton(central, "maxColorButton");
    connect(maxColorButton, SIGNAL(selectedColor(const QColor &)),
                                this, SLOT(ColorChanged(const QColor &)));
    mainLayout->addWidget(maxColorButton, 8, 1); 

    mainLayout->addWidget(new QLabel("Opacity", central, "maxOpacityLabel"),9,0);
    maxOpacity = new QvisOpacitySlider(0,255,25,255, central, "maxOpacity", NULL);
    maxOpacity->setTickInterval(64);
    maxOpacity->setGradientColor(QColor(0, 0, 0));
    connect(maxOpacity, SIGNAL(valueChanged(int, const void*)),
            this, SLOT(maxOpacityChanged(int, const void*)));
    mainLayout->addWidget(maxOpacity, 9,1);

    mainLayout->addWidget(new QLabel("tolerance", central, "toleranceLabel"),10,0);
    tolerance = new QLineEdit(central, "tolerance");
    connect(tolerance, SIGNAL(returnPressed()),
            this, SLOT(toleranceProcessText()));
    mainLayout->addWidget(tolerance, 10,1);

    mainLayout->addWidget(new QLabel("hitpercent", central, "hitpercentLabel"),11,0);
    hitpercent = new QLineEdit(central, "hitpercent");
    connect(hitpercent, SIGNAL(returnPressed()),
            this, SLOT(hitpercentProcessText()));
    mainLayout->addWidget(hitpercent, 11,1);

}


// ****************************************************************************
// Method: QvisTopologyPlotWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Tue Jul 1 08:50:16 PDT 2003
//
// Modifications:
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
// ****************************************************************************

void
QvisTopologyPlotWindow::UpdateWindow(bool doAll)
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
        int                   r,g,b;
        ColorAttributeList    mc;
        switch(i)
        {
          case 0: //lineWidth
            lineWidth->blockSignals(true);
            lineWidth->SetLineWidth(atts->GetLineWidth());
            lineWidth->blockSignals(false);
            break;
          case 1: //lineStyle
            lineStyle->blockSignals(true);
            lineStyle->SetLineStyle(atts->GetLineStyle());
            lineStyle->blockSignals(false);
            break;
          case 2: //multiColor
            mc = atts->GetMultiColor();
            r = mc[0].Red(); g = mc[0].Green(); b = mc[0].Blue();
            minColorButton->setButtonColor(QColor(r,g,b));
            r = mc[1].Red(); g = mc[1].Green(); b = mc[1].Blue();
            minPlateauColorButton->setButtonColor(QColor(r,g,b));
            r = mc[2].Red(); g = mc[2].Green(); b = mc[2].Blue();
            maxPlateauColorButton->setButtonColor(QColor(r,g,b));
            r = mc[3].Red(); g = mc[3].Green(); b = mc[3].Blue();
            maxColorButton->setButtonColor(QColor(r,g,b));
            break;
          case 3: //minOpacity
            minOpacity->setValue(int(atts->GetMinOpacity()*255.));
            break;
          case 4: //minPlateauOpacity
            minPlateauOpacity->setValue(int(atts->GetMinPlateauOpacity()*255.));
            break;
          case 5: //maxPlateauOpacity
            maxPlateauOpacity->setValue(int(atts->GetMaxPlateauOpacity()*255.));
            break;
          case 6: //maxOpacity
            maxOpacity->setValue(int(atts->GetMaxOpacity()*255.));
            break;
          case 7: //tolerance
            temp.setNum(atts->GetTolerance());
            tolerance->setText(temp);
            break;
          case 8: //hitpercent
            temp.setNum(atts->GetHitpercent());
            hitpercent->setText(temp);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisTopologyPlotWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Tue Jul 1 08:50:16 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisTopologyPlotWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do lineWidth
    if(which_widget == 0 || doAll)
    {
        // Nothing for lineWidth
    }

    // Do lineStyle
    if(which_widget == 1 || doAll)
    {
        // Nothing for lineStyle
    }

    // Do multiColor
    if(which_widget == 2 || doAll)
    {
        ColorAttributeList mc;
        int r,g,b;
        minColorButton->buttonColor().rgb(&r,&g,&b);
        mc.AddColorAttribute(ColorAttribute(r,g,b));
        minPlateauColorButton->buttonColor().rgb(&r,&g,&b);
        mc.AddColorAttribute(ColorAttribute(r,g,b));
        maxPlateauColorButton->buttonColor().rgb(&r,&g,&b);
        mc.AddColorAttribute(ColorAttribute(r,g,b));
        maxColorButton->buttonColor().rgb(&r,&g,&b);
        mc.AddColorAttribute(ColorAttribute(r,g,b));

        atts->SetMultiColor(mc);
    }

    // Do minOpacity
    if(which_widget == 3 || doAll)
    {
        // Nothing for minOpacity
    }

    // Do minPlateauOpacity
    if(which_widget == 4 || doAll)
    {
        // Nothing for minPlateauOpacity
    }

    // Do maxPlateauOpacity
    if(which_widget == 5 || doAll)
    {
        // Nothing for maxPlateauOpacity
    }

    // Do maxOpacity
    if(which_widget == 6 || doAll)
    {
        // Nothing for maxOpacity
    }

    // Do tolerance
    if(which_widget == 7 || doAll)
    {
        temp = tolerance->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetTolerance(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of tolerance was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetTolerance());
            Message(msg);
            atts->SetTolerance(atts->GetTolerance());
        }
    }

    // Do hitpercent
    if(which_widget == 8 || doAll)
    {
        temp = hitpercent->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetHitpercent(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of hitpercent was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetHitpercent());
            Message(msg);
            atts->SetHitpercent(atts->GetHitpercent());
        }
    }

}


// ****************************************************************************
// Method: QvisTopologyPlotWindow::Apply
//
// Purpose: 
//   Called to apply changes in the subject.
//
// Programmer: xml2window
// Creation:   Tue Jul 1 08:50:16 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisTopologyPlotWindow::Apply(bool ignore)
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
// Method: QvisTopologyPlotWindow::apply
//
// Purpose: 
//   Qt slot function called when apply button is clicked.
//
// Programmer: xml2window
// Creation:   Tue Jul 1 08:50:16 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisTopologyPlotWindow::apply()
{
    Apply(true);
}


// ****************************************************************************
// Method: QvisTopologyPlotWindow::makeDefault
//
// Purpose: 
//   Qt slot function called when "Make default" button is clicked.
//
// Programmer: xml2window
// Creation:   Tue Jul 1 08:50:16 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisTopologyPlotWindow::makeDefault()
{
    GetCurrentValues(-1);
    atts->Notify();
    viewer->SetDefaultPlotOptions(plotType);
}


// ****************************************************************************
// Method: QvisTopologyPlotWindow::reset
//
// Purpose: 
//   Qt slot function called when reset button is clicked.
//
// Programmer: xml2window
// Creation:   Tue Jul 1 08:50:16 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisTopologyPlotWindow::reset()
{
    viewer->ResetPlotOptions(plotType);
}


void
QvisTopologyPlotWindow::lineWidthChanged(int style)
{
    atts->SetLineWidth(style);
    Apply();
}


void
QvisTopologyPlotWindow::lineStyleChanged(int style)
{
    atts->SetLineStyle(style);
    Apply();
}


void
QvisTopologyPlotWindow::ColorChanged(const QColor &color)
{
    GetCurrentValues(2);
    Apply();
}


void
QvisTopologyPlotWindow::minOpacityChanged(int opacity, const void*)
{
    atts->SetMinOpacity((float)opacity/255.);
    Apply();
}


void
QvisTopologyPlotWindow::minPlateauOpacityChanged(int opacity, const void*)
{
    atts->SetMinPlateauOpacity((float)opacity/255.);
    Apply();
}


void
QvisTopologyPlotWindow::maxPlateauOpacityChanged(int opacity, const void*)
{
    atts->SetMaxPlateauOpacity((float)opacity/255.);
    Apply();
}


void
QvisTopologyPlotWindow::maxOpacityChanged(int opacity, const void*)
{
    atts->SetMaxOpacity((float)opacity/255.);
    Apply();
}


void
QvisTopologyPlotWindow::toleranceProcessText()
{
    GetCurrentValues(7);
    Apply();
}


void
QvisTopologyPlotWindow::hitpercentProcessText()
{
    GetCurrentValues(8);
    Apply();
}


