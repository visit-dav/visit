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

#include "QvisReflectWindow.h"

#include <ReflectAttributes.h>
#include <ViewerProxy.h>

#include <QvisReflectWidget.h>
#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QWidget>
#include <QButtonGroup>
#include <QRadioButton>
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
//    Brad Whitlock, Wed Jun 25 09:24:38 PDT 2003
//    I initialized modeButtons, userSetMode, and mode2D.
//
// ****************************************************************************

QvisReflectWindow::QvisReflectWindow(const int type,
                         ReflectAttributes *subj,
                         const QString &caption,
                         const QString &shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type, subj, caption, shortName, notepad, false)
{
    atts = subj;

    userSetMode = false;
    mode2D = true;

    modeButtons = 0;
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
//    Cyrus Harrison, Tue Aug 26 15:45:00 PDT 2008
//    Set parents for button groups, so we dont have to delete the explicitly.
//
// ****************************************************************************

QvisReflectWindow::~QvisReflectWindow()
{

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
//    Brad Whitlock, Wed Jun 25 09:25:32 PDT 2003
//    I added code that lets us have a 2D input mode.
//
//    Jeremy Meredith, Wed Mar  3 16:17:01 PST 2004
//    I had it update the octant menu since we only
//    update it when it changes now.
//
//    Brad Whitlock, Thu Apr 24 16:54:17 PDT 2008
//    Added tr()'s
//
//    Cyrus Harrison, Tue Aug 19 08:52:09 PDT 2008
//    Qt4 Port.
//
// ****************************************************************************

void
QvisReflectWindow::CreateWindowContents()
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    topLayout->addLayout(mainLayout);

    // Add the controls to select the input mode.
    QGridLayout *octantLayout = new QGridLayout();
    mainLayout->addLayout(octantLayout);
    
    modeButtons = new QButtonGroup(central);
    octantLayout->addWidget(new QLabel(tr("Input mode"), central),0, 0);
    QRadioButton *rb = new QRadioButton(tr("2D"), central);
    modeButtons->addButton(rb,0);
    octantLayout->addWidget(rb, 0, 1);
    rb = new QRadioButton(tr("3D"), central);
    modeButtons->addButton(rb,1);
    modeButtons->button(0)->setChecked(true);
    connect(modeButtons, SIGNAL(buttonClicked(int)),
            this, SLOT(selectMode(int)));
    octantLayout->addWidget(rb, 0, 2);

    // Octant
    originalDataLabel = new QLabel(tr("Original data quadrant"), central);
    octantLayout->addWidget(originalDataLabel, 1, 0);
    octant = new QComboBox(central);
    octantLayout->addWidget(octant, 1, 1, 1, 2);

    // Reflection widget
    reflect = new QvisReflectWidget(central);
    reflect->setMode2D(mode2D);
    connect(reflect, SIGNAL(valueChanged(bool*)),
            this, SLOT(selectOctants(bool*)));
    reflectionLabel = new QLabel(tr("Reflection quadrants"), central);
    mainLayout->addWidget(reflectionLabel);
    mainLayout->addWidget(reflect, 100);

    // Limits
    mainLayout->addWidget(new QLabel(tr("Reflection Limits:"), central));
    QGridLayout *limitsLayout = new QGridLayout();
    mainLayout->addLayout(limitsLayout);
    //limitsLayout->addColumnSpacing(0, 20);

    xBound = new QButtonGroup(central);
    xUseData = new QRadioButton(tr("Use X Min"),   central);
    xSpecify = new QRadioButton(tr("Specify X ="), central);
    xBound->addButton(xUseData,0);
    xBound->addButton(xSpecify,1);
    specifiedX = new QLineEdit("0", central);
    limitsLayout->addWidget(xUseData,   0, 1);
    limitsLayout->addWidget(xSpecify,   0, 2);
    limitsLayout->addWidget(specifiedX, 0, 3);

    yBound = new QButtonGroup(central);
    yUseData = new QRadioButton(tr("Use Y Min"),   central);
    ySpecify = new QRadioButton(tr("Specify Y ="), central);
    yBound->addButton(yUseData,0);
    yBound->addButton(ySpecify,1);
    specifiedY = new QLineEdit("0", central);
    limitsLayout->addWidget(yUseData,   1, 1);
    limitsLayout->addWidget(ySpecify,   1, 2);
    limitsLayout->addWidget(specifiedY, 1, 3);

    zBound = new QButtonGroup(central);
    zUseData = new QRadioButton(tr("Use Z Min"),   central);
    zSpecify = new QRadioButton(tr("Specify Z ="), central);
    zBound->addButton(zUseData,0);
    zBound->addButton(zSpecify,1);
    specifiedZ = new QLineEdit("0", central);
    limitsLayout->addWidget(zUseData,   2, 1);
    limitsLayout->addWidget(zSpecify,   2, 2);
    limitsLayout->addWidget(specifiedZ, 2, 3);

    UpdateOctantMenuContents();

    connect(octant,        SIGNAL(activated(int)),
            this,          SLOT(octantChanged(int)));
    connect(xBound,        SIGNAL(buttonClicked(int)),
            this,          SLOT(xBoundaryChanged(int)));
    connect(specifiedX,    SIGNAL(returnPressed()),
            this,          SLOT(specifiedXProcessText()));
    connect(yBound,        SIGNAL(buttonClicked(int)),
            this,          SLOT(yBoundaryChanged(int)));
    connect(specifiedY,    SIGNAL(returnPressed()),
            this,          SLOT(specifiedYProcessText()));
    connect(zBound,        SIGNAL(buttonClicked(int)),
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
//    Brad Whitlock, Wed Jun 25 10:11:14 PDT 2003
//    I added a 2D input mode.
//
//    Jeremy Meredith, Wed Mar  3 16:18:08 PST 2004
//    Only update the octant menu automatically if it has
//    changed dimensionality.
//
//    Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//    Replaced simple QString::sprintf's with a setNum because there seems
//    to be a bug causing numbers to be incremented by .00001.  See '5263.
//
//    Brad Whitlock, Thu Apr 24 16:55:18 PDT 2008
//    Added tr()'s
//
//    Cyrus Harrison, Tue Aug 19 08:52:09 PDT 2008
//    Qt4 Port.
//
// ****************************************************************************

void
QvisReflectWindow::UpdateWindow(bool doAll)
{
    QString temp;
    bool    setOctant  = false;
    bool    originIs3D = (operator2WidgetOctants[atts->GetOctant()] > 3);
    bool    reflection3D = false;

    reflect->blockSignals(true);

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
          case 0: // octant
            setOctant = true;
            break;
          case 1: //xBound
            xUseData->setText((atts->GetOctant()&0x01) ? tr("Use dataset max") : tr("Use dataset min"));
            xBound->button(atts->GetUseXBoundary() ? 0 : 1)->setChecked(true);
            specifiedX->setEnabled(atts->GetUseXBoundary() ? false : true);
            break;
          case 2: //specifiedX
            temp.setNum(atts->GetSpecifiedX());
            specifiedX->setText(temp);
            break;
          case 3: //yBound
            yUseData->setText((atts->GetOctant()&0x02) ? tr("Use dataset max") : tr("Use dataset min"));
            yBound->button(atts->GetUseYBoundary() ? 0 : 1)->setChecked(true);
            specifiedY->setEnabled(atts->GetUseYBoundary() ? false : true);
            break;
          case 4: //specifiedY
            temp.setNum(atts->GetSpecifiedY());
            specifiedY->setText(temp);
            break;
          case 5: //zBound
            zUseData->setText((atts->GetOctant()&0x04) ? tr("Use dataset max") : tr("Use dataset min"));
            zBound->button(atts->GetUseZBoundary() ? 0 : 1)->setChecked(true);
            specifiedZ->setEnabled(atts->GetUseZBoundary() ? false : true);
            break;
          case 6: //specifiedZ
            temp.setNum(atts->GetSpecifiedZ());
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
                    reflection3D |= (octants[operator2WidgetOctants[i]] && operator2WidgetOctants[i] > 3);
                }

                reflect->setValues(octants);
            }
            break;
        }
    }

    if(originIs3D || reflection3D)
    {
        bool newMode2D = !originIs3D && !reflection3D;
        if(newMode2D != mode2D)
        {
            mode2D = newMode2D;
            modeButtons->blockSignals(true);
            modeButtons->button(mode2D ? 0 : 1)->setChecked(true);
            modeButtons->blockSignals(false);
            reflect->setMode2D(mode2D);
            UpdateOctantMenuContents();
        }
    }
    else if(!userSetMode)
    {
        bool oldMode2D = mode2D;
        mode2D = true;
        modeButtons->blockSignals(true);
        modeButtons->button(0)->setChecked(true);
        modeButtons->blockSignals(false);
        reflect->setMode2D(mode2D);
        if (oldMode2D != mode2D)
            UpdateOctantMenuContents();
    }

    if(setOctant)
    {
        octant->blockSignals(true);
        octant->setCurrentIndex(atts->GetOctant());
        octant->blockSignals(false);
        reflect->setOriginalOctant(operator2WidgetOctants[atts->GetOctant()]);
    }

    reflect->blockSignals(false);
}

// ****************************************************************************
// Method: QvisReflectWindow::UpdateOctantMenuContents
//
// Purpose: 
//   Updates the octant menu's contents so it shows the appropriate choices.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 25 09:34:50 PDT 2003
//
// Modifications:
//   Jeremy Meredith, Mon Aug 18 11:38:56 PDT 2003
//   Removed Z axis from 2D labels.
//
//   Brad Whitlock, Thu Apr 24 16:55:58 PDT 2008
//   Added tr()'s
//
//    Cyrus Harrison, Tue Aug 19 08:52:09 PDT 2008
//    Qt4 Port.
//
// ****************************************************************************

void
QvisReflectWindow::UpdateOctantMenuContents()
{
    octant->clear();
    if (mode2D)
    {
        octant->addItem(tr("+X  +Y"));
        octant->addItem(tr("-X  +Y"));
        octant->addItem(tr("+X  -Y"));
        octant->addItem(tr("-X  -Y"));
        reflectionLabel->setText(tr("Reflection quadrants"));
        originalDataLabel->setText(tr("Original data quadrant"));
    }
    else
    {
        octant->addItem(tr("+X  +Y  +Z"));
        octant->addItem(tr("-X  +Y  +Z"));
        octant->addItem(tr("+X  -Y  +Z"));
        octant->addItem(tr("-X  -Y  +Z"));
        octant->addItem(tr("+X  +Y  -Z"));
        octant->addItem(tr("-X  +Y  -Z"));
        octant->addItem(tr("+X  -Y  -Z"));
        octant->addItem(tr("-X  -Y  -Z"));
        reflectionLabel->setText(tr("Reflection octants"));
        originalDataLabel->setText(tr("Original data octant"));
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
// Modifications:
//    Cyrus Harrison, Tue Aug 19 08:52:09 PDT 2008
//    Qt4 Port.
//
// ****************************************************************************
void
QvisReflectWindow::GetCurrentValues(int which_field)
{
    bool okay, doAll = (which_field == -1);
    QString msg, temp;

    // Do specifiedX
    if(which_field == ReflectAttributes::ID_specifiedX || doAll)
    {
        double val;
        if(LineEditGetDouble(specifiedX, val))
            atts->SetSpecifiedX(val);
        else
        {
            ResettingError(tr("Specified X"),
                DoubleToQString(atts->GetSpecifiedX()));
            atts->SetSpecifiedX(atts->GetSpecifiedX());
        }
    }

    // Do specifiedY
    if(which_field == ReflectAttributes::ID_specifiedY || doAll)
    {
        double val;
        if(LineEditGetDouble(specifiedY, val))
            atts->SetSpecifiedY(val);
        else
        {
            ResettingError(tr("Specified Y"),
                DoubleToQString(atts->GetSpecifiedY()));
            atts->SetSpecifiedY(atts->GetSpecifiedY());
        }
    }

    // Do specifiedZ
    if(which_field == ReflectAttributes::ID_specifiedZ || doAll)
    {
        double val;
        if(LineEditGetDouble(specifiedZ, val))
            atts->SetSpecifiedZ(val);
        else
        {
            ResettingError(tr("Specified Z"),
                DoubleToQString(atts->GetSpecifiedZ()));
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

        xUseData->setText((New&0x01) ? tr("Use dataset max") : tr("Use dataset min"));
        yUseData->setText((New&0x02) ? tr("Use dataset max") : tr("Use dataset min"));
        zUseData->setText((New&0x04) ? tr("Use dataset max") : tr("Use dataset min"));

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

// ****************************************************************************
// Method: QvisReflectWindow::selectMode
//
// Purpose: 
//   Selects the input mode.
//
// Arguments:
//   mode : The new input mode.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 25 12:44:20 PDT 2003
//
// Modifications:
//    Cyrus Harrison, Tue Aug 19 08:52:09 PDT 2008
//    Qt4 Port.
//
// ****************************************************************************

void
QvisReflectWindow::selectMode(int mode)
{
    if(mode == 0)
    {
        bool originIs3D = (operator2WidgetOctants[atts->GetOctant()] > 3);
        bool reflection3D = false;
        int *r = atts->GetReflections();
        bool octants[8];
        for (int i=0; i<8; i++)
        {
            int b = i ^ atts->GetOctant();
            octants[operator2WidgetOctants[i]] = (r[b] > 0);
            reflection3D |= (octants[operator2WidgetOctants[i]] && operator2WidgetOctants[i] > 3);
        }

        if(originIs3D || reflection3D)
        {
            Error(tr("The reflection attributes require the 3D input mode because "
                     "the original data octant or one or more of the reflection "
                     "octants has a negative Z value. The input mode will remain 3D."));
            mode2D = false;
            modeButtons->blockSignals(true);
            modeButtons->button(1)->setChecked(true);
            modeButtons->blockSignals(false);
        }
        else
        {
            if(!mode2D)
            {
                mode2D = true;
                reflect->setMode2D(mode2D);
                UpdateOctantMenuContents();
            }
        }
    }
    else
    {
        if(mode2D)
        {
            mode2D = false;
            reflect->setMode2D(mode2D);
            UpdateOctantMenuContents();
        }

        userSetMode = true;
    }
}
