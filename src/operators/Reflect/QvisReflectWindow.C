#include "QvisReflectWindow.h"

#include <ReflectAttributes.h>
#include <ViewerProxy.h>

#include <QvisReflectWidget.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <stdio.h>
#include <string>
using std::string;

static const int widget2OperatorOctants[] = {0, 1, 3, 2, 4, 5, 7, 6};
static const int operator2WidgetOctants[] = {0, 1, 3, 2, 4, 5, 7, 6};


// ****************************************************************************
//  Constructor:  QvisReflectWindow::QvisReflectWindow
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 11, 2002
//
//  Modifications:
//    Brad Whitlock, Fri Apr 12 13:13:11 PST 2002
//    Made it inherit from QvisOperatorWindow.
//
//    Brad Whitlock, Thu Mar 6 11:31:12 PDT 2003
//    I added stretch and initialized some widgets that have no parents.
//
// ****************************************************************************

QvisReflectWindow::QvisReflectWindow(const int type,
                         ReflectAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type, subj, caption, shortName, notepad, false)
{
    atts = subj;

    xBound = 0;
    yBound = 0;
    zBound = 0;
}

// ****************************************************************************
//  Destructor:  QvisReflectWindow::~QvisReflectWindow
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 11, 2002
//
//  Modifications:
//    Brad Whitlock, Tue Mar 11 11:56:34 PDT 2003
//    I added code to delete certain widgets that have no parents.
//
// ****************************************************************************

QvisReflectWindow::~QvisReflectWindow()
{
    delete xBound;
    delete yBound;
    delete zBound;
}

// ****************************************************************************
//  Method:  QvisReflectWindow::CreateWindowContents
//
//  Purpose:
//    Sets up widgets and signal connections.
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 11, 2002
//
//  Modifications:
//    Brad Whitlock, Mon Mar 3 11:42:37 PDT 2003
//    I made it use a QvisReflectWidget.
//
// ****************************************************************************
void
QvisReflectWindow::CreateWindowContents()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(topLayout, 10, "mainLayout");

    // Octant
    QHBoxLayout *octantLayout = new QHBoxLayout(mainLayout);
    octantLayout->addWidget(new QLabel("Original data is in octant", central, "octantLayout"));
    octant = new QComboBox(false, central, "octant");
    octant->insertItem("+X  +Y  +Z");
    octant->insertItem("-X  +Y  +Z");
    octant->insertItem("+X  -Y  +Z");
    octant->insertItem("-X  -Y  +Z");
    octant->insertItem("+X  +Y  -Z");
    octant->insertItem("-X  +Y  -Z");
    octant->insertItem("+X  -Y  -Z");
    octant->insertItem("-X  -Y  -Z");
    octantLayout->addWidget(octant);

    reflect = new QvisReflectWidget(central, "reflect");
    connect(reflect, SIGNAL(valueChanged(bool*)),
            this, SLOT(selectOctants(bool*)));
    mainLayout->addWidget(new QLabel(reflect, "Reflection octants", central));
    mainLayout->addWidget(reflect, 100);

    // Limits
    mainLayout->addWidget(new QLabel("Reflection Limits:", central, "reflectionLabel"));
    QGridLayout *limitsLayout = new QGridLayout(mainLayout, 3,4, 10, "limitsLayout");
    limitsLayout->addColSpacing(0, 20);

    xBound = new QButtonGroup(0, "xBound");
    xUseData = new QRadioButton("Use X Min",   central, "xUseData");
    xSpecify = new QRadioButton("Specify X =", central, "xSpecify");
    xBound->insert(xUseData);
    xBound->insert(xSpecify);
    specifiedX = new QLineEdit("0", central);
    limitsLayout->addWidget(xUseData,   0, 1);
    limitsLayout->addWidget(xSpecify,   0, 2);
    limitsLayout->addWidget(specifiedX, 0, 3);

    yBound = new QButtonGroup(0, "yBound");
    yUseData = new QRadioButton("Use Y Min",   central, "yUseData");
    ySpecify = new QRadioButton("Specify Y =", central, "ySpecify");
    yBound->insert(yUseData);
    yBound->insert(ySpecify);
    specifiedY = new QLineEdit("0", central);
    limitsLayout->addWidget(yUseData,   1, 1);
    limitsLayout->addWidget(ySpecify,   1, 2);
    limitsLayout->addWidget(specifiedY, 1, 3);

    zBound = new QButtonGroup(0, "zBound");
    zUseData = new QRadioButton("Use Z Min",   central, "zUseData");
    zSpecify = new QRadioButton("Specify Z =", central, "zSpecify");
    zBound->insert(zUseData);
    zBound->insert(zSpecify);
    specifiedZ = new QLineEdit("0", central);
    limitsLayout->addWidget(zUseData,   2, 1);
    limitsLayout->addWidget(zSpecify,   2, 2);
    limitsLayout->addWidget(specifiedZ, 2, 3);

    connect(octant,        SIGNAL(activated(int)),
            this,          SLOT(octantChanged(int)));
    connect(xBound,        SIGNAL(clicked(int)),
            this,          SLOT(xBoundaryChanged(int)));
    connect(specifiedX,    SIGNAL(returnPressed()),
            this,          SLOT(specifiedXProcessText()));
    connect(yBound,        SIGNAL(clicked(int)),
            this,          SLOT(yBoundaryChanged(int)));
    connect(specifiedY,    SIGNAL(returnPressed()),
            this,          SLOT(specifiedYProcessText()));
    connect(zBound,        SIGNAL(clicked(int)),
            this,          SLOT(zBoundaryChanged(int)));
    connect(specifiedZ,    SIGNAL(returnPressed()),
            this,          SLOT(specifiedZProcessText()));
}

// ****************************************************************************
//  Method:  QvisReflectWindow::UpdateWindow
//
//  Purpose:
//    Updates portions of the window based on the current attributes.
//
//  Arguments:
//    doAll      update the whole window
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 11, 2002
//
//  Modifications:
//    Brad Whitlock, Wed Mar 5 15:49:22 PST 2003
//    I made it use a QvisReflectWidget widget.
//
// ****************************************************************************

void
QvisReflectWindow::UpdateWindow(bool doAll)
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
          case 0: //octant
            octant->blockSignals(true);
            octant->setCurrentItem(atts->GetOctant());
            octant->blockSignals(false);
            reflect->blockSignals(true);
            reflect->setOriginalOctant(operator2WidgetOctants[atts->GetOctant()]);
            reflect->blockSignals(false);
            break;
          case 1: //xBound
            xUseData->setText((atts->GetOctant()&0x01) ? "Use dataset max" : "Use dataset min");
            xBound->setButton(atts->GetUseXBoundary() ? 0 : 1);
            specifiedX->setEnabled(atts->GetUseXBoundary() ? false : true);
            break;
          case 2: //specifiedX
            temp.sprintf("%g", atts->GetSpecifiedX());
            specifiedX->setText(temp);
            break;
          case 3: //yBound
            yUseData->setText((atts->GetOctant()&0x02) ? "Use dataset max" : "Use dataset min");
            yBound->setButton(atts->GetUseYBoundary() ? 0 : 1);
            specifiedY->setEnabled(atts->GetUseYBoundary() ? false : true);
            break;
          case 4: //specifiedY
            temp.sprintf("%g", atts->GetSpecifiedY());
            specifiedY->setText(temp);
            break;
          case 5: //zBound
            zUseData->setText((atts->GetOctant()&0x04) ? "Use dataset max" : "Use dataset min");
            zBound->setButton(atts->GetUseZBoundary() ? 0 : 1);
            specifiedZ->setEnabled(atts->GetUseZBoundary() ? false : true);
            break;
          case 6: //specifiedZ
            temp.sprintf("%g", atts->GetSpecifiedZ());
            specifiedZ->setText(temp);
            break;
          case 7: //reflections
            {
                int *r = atts->GetReflections();
                bool octants[8];
                for (int i=0; i<8; i++)
                {
                    int b = i ^ atts->GetOctant();
                    octants[operator2WidgetOctants[i]] = (r[b] > 0);
                }

                reflect->blockSignals(true);
                reflect->setValues(octants);
                reflect->blockSignals(false);
            }
            break;
        }
    }
}

// ****************************************************************************
//  Method:  QvisReflectWindow::GetCurrentValues
//
//  Purpose:
//    Gets the values from the window text fields for the attributes.
//
//  Arguments:
//    which_field  an integer corresponding to the field to retrieve
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 11, 2002
//
// ****************************************************************************
void
QvisReflectWindow::GetCurrentValues(int which_field)
{
    bool okay, doAll = (which_field == -1);
    QString msg, temp;

    // Do specifiedX
    if(which_field == 2 || doAll)
    {
        temp = specifiedX->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetSpecifiedX(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of specifiedX was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetSpecifiedX());
            Message(msg);
            atts->SetSpecifiedX(atts->GetSpecifiedX());
        }
    }

    // Do specifiedY
    if(which_field == 4 || doAll)
    {
        temp = specifiedY->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetSpecifiedY(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of specifiedY was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetSpecifiedY());
            Message(msg);
            atts->SetSpecifiedY(atts->GetSpecifiedY());
        }
    }

    // Do specifiedZ
    if(which_field == 6 || doAll)
    {
        temp = specifiedZ->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetSpecifiedZ(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of specifiedZ was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetSpecifiedZ());
            Message(msg);
            atts->SetSpecifiedZ(atts->GetSpecifiedZ());
        }
    }
}

//
// Qt Slot functions.
//

// ****************************************************************************
// Method: QvisReflectWindow::selectOctants
//
// Purpose: 
//   Slot for changing selected octants.
//
// Arguments:
//   octants : The selected octants.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 11 11:23:25 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisReflectWindow::selectOctants(bool *octants)
{
    int r[8];
    for(int i = 0; i < 8; ++i)
    {
        int b = i ^ atts->GetOctant();
        r[b] = octants[widget2OperatorOctants[i]] ? 1  : 0;
    }

    atts->SetReflections(r);
    Apply();    
}

// ****************************************************************************
//  Method:  QvisReflectWindow::octantChanged
//
//  Purpose:
//    Slot for the original octant changing.
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 11, 2002
//
//  Modifications:
//    Brad Whitlock, Tue Mar 11 11:05:03 PDT 2003
//    I made it work with the new widget.
//
// ****************************************************************************

void
QvisReflectWindow::octantChanged(int val)
{
    ReflectAttributes::Octant Old = atts->GetOctant();
    ReflectAttributes::Octant New = ReflectAttributes::Octant(val);

    if(Old != New)
    {
        // must swap reflections to new match
        int r[8];
        bool b[8];
        reflect->blockSignals(true);
        int i;
        for (i=0; i<8; i++)
        {
            r[i ^ Old] = atts->GetReflections()[i];
        }
        for (i=0; i<8; i++)
        {
            b[widget2OperatorOctants[i]] = (r[i ^ New] != 0);
        }
        reflect->setValues(b);
        reflect->blockSignals(false);

        xUseData->setText((New&0x01) ? "Use dataset max" : "Use dataset min");
        yUseData->setText((New&0x02) ? "Use dataset max" : "Use dataset min");
        zUseData->setText((New&0x04) ? "Use dataset max" : "Use dataset min");

        atts->SetOctant(New);
        Apply();
    }
}

// ****************************************************************************
//  Method:  QvisReflectWindow::?BoundaryChanged
//
//  Purpose:
//    Slot function to set the attributes for the type of boundary limits.
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 11, 2002
//
// ****************************************************************************
void
QvisReflectWindow::xBoundaryChanged(int val)
{
    atts->SetUseXBoundary(val==0 ? true : false);
    Apply();
}

void
QvisReflectWindow::yBoundaryChanged(int val)
{
    atts->SetUseYBoundary(val==0 ? true : false);
    Apply();
}

void
QvisReflectWindow::zBoundaryChanged(int val)
{
    atts->SetUseZBoundary(val==0 ? true : false);
    Apply();
}

// ****************************************************************************
//  Method:  QvisReflectWindow::specified?ProcessText
//
//  Purpose:
//    Slot that gets the current values for the specified reflection limits.
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 11, 2002
//
// ****************************************************************************
void
QvisReflectWindow::specifiedXProcessText()
{
    GetCurrentValues(2);
    Apply();
}

void
QvisReflectWindow::specifiedYProcessText()
{
    GetCurrentValues(4);
    Apply();
}

void
QvisReflectWindow::specifiedZProcessText()
{
    GetCurrentValues(6);
    Apply();
}
