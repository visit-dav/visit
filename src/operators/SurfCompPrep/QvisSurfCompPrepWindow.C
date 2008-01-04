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

#include "QvisSurfCompPrepWindow.h"

#include <SurfCompPrepAttributes.h>
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
// Method: QvisSurfCompPrepWindow::QvisSurfCompPrepWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Tue Jul 1 11:03:04 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

QvisSurfCompPrepWindow::QvisSurfCompPrepWindow(const int type,
                         SurfCompPrepAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisSurfCompPrepWindow::~QvisSurfCompPrepWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Tue Jul 1 11:03:04 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

QvisSurfCompPrepWindow::~QvisSurfCompPrepWindow()
{
}


// ****************************************************************************
// Method: QvisSurfCompPrepWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Tue Jul 1 11:03:04 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisSurfCompPrepWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 20,2,  10, "mainLayout");


    mainLayout->addWidget(new QLabel("Method for inferring surface", central, "surfaceTypeLabel"),0,0);
    surfaceType = new QButtonGroup(central, "surfaceType");
    surfaceType->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *surfaceTypeLayout = new QHBoxLayout(surfaceType);
    surfaceTypeLayout->setSpacing(10);
    QRadioButton *surfaceTypeSurfaceTypeClosest = new QRadioButton("Closest", surfaceType);
    surfaceTypeLayout->addWidget(surfaceTypeSurfaceTypeClosest);
    QRadioButton *surfaceTypeSurfaceTypeFarthest = new QRadioButton("Farthest", surfaceType);
    surfaceTypeLayout->addWidget(surfaceTypeSurfaceTypeFarthest);
    QRadioButton *surfaceTypeSurfaceTypeAverage = new QRadioButton("Average", surfaceType);
    surfaceTypeLayout->addWidget(surfaceTypeSurfaceTypeAverage);
    connect(surfaceType, SIGNAL(clicked(int)),
            this, SLOT(surfaceTypeChanged(int)));
    mainLayout->addWidget(surfaceType, 0,1);

    mainLayout->addWidget(new QLabel("Coordinate System", central, "coordSystemLabel"),1,0);
    coordSystem = new QButtonGroup(central, "coordSystem");
    coordSystem->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *coordSystemLayout = new QHBoxLayout(coordSystem);
    coordSystemLayout->setSpacing(10);
    QRadioButton *coordSystemCoordinateSystemCartesian = new QRadioButton("Cartesian", coordSystem);
    coordSystemLayout->addWidget(coordSystemCoordinateSystemCartesian);
    QRadioButton *coordSystemCoordinateSystemCylindrical = new QRadioButton("Cylindrical", coordSystem);
    coordSystemLayout->addWidget(coordSystemCoordinateSystemCylindrical);
    QRadioButton *coordSystemCoordinateSystemSpherical = new QRadioButton("Spherical", coordSystem);
    coordSystemLayout->addWidget(coordSystemCoordinateSystemSpherical);
    connect(coordSystem, SIGNAL(clicked(int)),
            this, SLOT(coordSystemChanged(int)));
    mainLayout->addWidget(coordSystem, 1,1);

    mainLayout->addWidget(new QLabel("Start for Theta (degrees)", central, "thetaStartLabel"),2,0);
    thetaStart = new QLineEdit(central, "thetaStart");
    connect(thetaStart, SIGNAL(returnPressed()),
            this, SLOT(thetaStartProcessText()));
    mainLayout->addWidget(thetaStart, 2,1);

    mainLayout->addWidget(new QLabel("Stop for Theta (degrees)", central, "thetaStopLabel"),3,0);
    thetaStop = new QLineEdit(central, "thetaStop");
    connect(thetaStop, SIGNAL(returnPressed()),
            this, SLOT(thetaStopProcessText()));
    mainLayout->addWidget(thetaStop, 3,1);

    mainLayout->addWidget(new QLabel("Number of samples in Theta", central, "thetaStepsLabel"),4,0);
    thetaSteps = new QLineEdit(central, "thetaSteps");
    connect(thetaSteps, SIGNAL(returnPressed()),
            this, SLOT(thetaStepsProcessText()));
    mainLayout->addWidget(thetaSteps, 4,1);

    mainLayout->addWidget(new QLabel("Start for Phi (degrees)", central, "phiStartLabel"),5,0);
    phiStart = new QLineEdit(central, "phiStart");
    connect(phiStart, SIGNAL(returnPressed()),
            this, SLOT(phiStartProcessText()));
    mainLayout->addWidget(phiStart, 5,1);

    mainLayout->addWidget(new QLabel("Stop for Phi (degrees)", central, "phiStopLabel"),6,0);
    phiStop = new QLineEdit(central, "phiStop");
    connect(phiStop, SIGNAL(returnPressed()),
            this, SLOT(phiStopProcessText()));
    mainLayout->addWidget(phiStop, 6,1);

    mainLayout->addWidget(new QLabel("Number of samples in Phi", central, "phiStepsLabel"),7,0);
    phiSteps = new QLineEdit(central, "phiSteps");
    connect(phiSteps, SIGNAL(returnPressed()),
            this, SLOT(phiStepsProcessText()));
    mainLayout->addWidget(phiSteps, 7,1);

    mainLayout->addWidget(new QLabel("First radius to sample", central, "startRadiusLabel"),8,0);
    startRadius = new QLineEdit(central, "startRadius");
    connect(startRadius, SIGNAL(returnPressed()),
            this, SLOT(startRadiusProcessText()));
    mainLayout->addWidget(startRadius, 8,1);

    mainLayout->addWidget(new QLabel("Last radius to sample", central, "endRadiusLabel"),9,0);
    endRadius = new QLineEdit(central, "endRadius");
    connect(endRadius, SIGNAL(returnPressed()),
            this, SLOT(endRadiusProcessText()));
    mainLayout->addWidget(endRadius, 9,1);

    mainLayout->addWidget(new QLabel("Number of samples in radius", central, "radiusStepsLabel"),10,0);
    radiusSteps = new QLineEdit(central, "radiusSteps");
    connect(radiusSteps, SIGNAL(returnPressed()),
            this, SLOT(radiusStepsProcessText()));
    mainLayout->addWidget(radiusSteps, 10,1);

    mainLayout->addWidget(new QLabel("Starting point for X", central, "xStartLabel"),11,0);
    xStart = new QLineEdit(central, "xStart");
    connect(xStart, SIGNAL(returnPressed()),
            this, SLOT(xStartProcessText()));
    mainLayout->addWidget(xStart, 11,1);

    mainLayout->addWidget(new QLabel("Ending point for X", central, "xStopLabel"),12,0);
    xStop = new QLineEdit(central, "xStop");
    connect(xStop, SIGNAL(returnPressed()),
            this, SLOT(xStopProcessText()));
    mainLayout->addWidget(xStop, 12,1);

    mainLayout->addWidget(new QLabel("Number of steps in X", central, "xStepsLabel"),13,0);
    xSteps = new QLineEdit(central, "xSteps");
    connect(xSteps, SIGNAL(returnPressed()),
            this, SLOT(xStepsProcessText()));
    mainLayout->addWidget(xSteps, 13,1);

    mainLayout->addWidget(new QLabel("Starting point for Y", central, "yStartLabel"),14,0);
    yStart = new QLineEdit(central, "yStart");
    connect(yStart, SIGNAL(returnPressed()),
            this, SLOT(yStartProcessText()));
    mainLayout->addWidget(yStart, 14,1);

    mainLayout->addWidget(new QLabel("Ending point for Y", central, "yStopLabel"),15,0);
    yStop = new QLineEdit(central, "yStop");
    connect(yStop, SIGNAL(returnPressed()),
            this, SLOT(yStopProcessText()));
    mainLayout->addWidget(yStop, 15,1);

    mainLayout->addWidget(new QLabel("Number of steps in Y", central, "yStepsLabel"),16,0);
    ySteps = new QLineEdit(central, "ySteps");
    connect(ySteps, SIGNAL(returnPressed()),
            this, SLOT(yStepsProcessText()));
    mainLayout->addWidget(ySteps, 16,1);

    mainLayout->addWidget(new QLabel("Starting point for Z", central, "zStartLabel"),17,0);
    zStart = new QLineEdit(central, "zStart");
    connect(zStart, SIGNAL(returnPressed()),
            this, SLOT(zStartProcessText()));
    mainLayout->addWidget(zStart, 17,1);

    mainLayout->addWidget(new QLabel("Ending point for Z", central, "zStopLabel"),18,0);
    zStop = new QLineEdit(central, "zStop");
    connect(zStop, SIGNAL(returnPressed()),
            this, SLOT(zStopProcessText()));
    mainLayout->addWidget(zStop, 18,1);

    mainLayout->addWidget(new QLabel("Number of steps in Z", central, "zStepsLabel"),19,0);
    zSteps = new QLineEdit(central, "zSteps");
    connect(zSteps, SIGNAL(returnPressed()),
            this, SLOT(zStepsProcessText()));
    mainLayout->addWidget(zSteps, 19,1);

}


// ****************************************************************************
// Method: QvisSurfCompPrepWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Tue Jul 1 11:03:04 PDT 2003
//
// Modifications:
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
// ****************************************************************************

void
QvisSurfCompPrepWindow::UpdateWindow(bool doAll)
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
          case 0: //surfaceType
            surfaceType->setButton(atts->GetSurfaceType());
            break;
          case 1: //coordSystem
            if (atts->GetCoordSystem() == SurfCompPrepAttributes::Spherical || atts->GetCoordSystem() == SurfCompPrepAttributes::Cylindrical)
                thetaStart->setEnabled(true);
            else
                thetaStart->setEnabled(false);
            if (atts->GetCoordSystem() == SurfCompPrepAttributes::Spherical || atts->GetCoordSystem() == SurfCompPrepAttributes::Cylindrical)
                thetaStop->setEnabled(true);
            else
                thetaStop->setEnabled(false);
            if (atts->GetCoordSystem() == SurfCompPrepAttributes::Spherical || atts->GetCoordSystem() == SurfCompPrepAttributes::Cylindrical)
                thetaSteps->setEnabled(true);
            else
                thetaSteps->setEnabled(false);
            if (atts->GetCoordSystem() == SurfCompPrepAttributes::Spherical)
                phiStart->setEnabled(true);
            else
                phiStart->setEnabled(false);
            if (atts->GetCoordSystem() == SurfCompPrepAttributes::Spherical)
                phiStop->setEnabled(true);
            else
                phiStop->setEnabled(false);
            if (atts->GetCoordSystem() == SurfCompPrepAttributes::Spherical)
                phiSteps->setEnabled(true);
            else
                phiSteps->setEnabled(false);
            if (atts->GetCoordSystem() == SurfCompPrepAttributes::Cylindrical || atts->GetCoordSystem() == SurfCompPrepAttributes::Spherical)
                startRadius->setEnabled(true);
            else
                startRadius->setEnabled(false);
            if (atts->GetCoordSystem() == SurfCompPrepAttributes::Cylindrical || atts->GetCoordSystem() == SurfCompPrepAttributes::Spherical)
                endRadius->setEnabled(true);
            else
                endRadius->setEnabled(false);
            if (atts->GetCoordSystem() == SurfCompPrepAttributes::Cylindrical || atts->GetCoordSystem() == SurfCompPrepAttributes::Spherical)
                radiusSteps->setEnabled(true);
            else
                radiusSteps->setEnabled(false);
            if (atts->GetCoordSystem() == SurfCompPrepAttributes::Cartesian)
                xStart->setEnabled(true);
            else
                xStart->setEnabled(false);
            if (atts->GetCoordSystem() == SurfCompPrepAttributes::Cartesian)
                xStop->setEnabled(true);
            else
                xStop->setEnabled(false);
            if (atts->GetCoordSystem() == SurfCompPrepAttributes::Cartesian)
                xSteps->setEnabled(true);
            else
                xSteps->setEnabled(false);
            if (atts->GetCoordSystem() == SurfCompPrepAttributes::Cartesian)
                yStart->setEnabled(true);
            else
                yStart->setEnabled(false);
            if (atts->GetCoordSystem() == SurfCompPrepAttributes::Cartesian)
                yStop->setEnabled(true);
            else
                yStop->setEnabled(false);
            if (atts->GetCoordSystem() == SurfCompPrepAttributes::Cartesian)
                ySteps->setEnabled(true);
            else
                ySteps->setEnabled(false);
            if (atts->GetCoordSystem() == SurfCompPrepAttributes::Cartesian || atts->GetCoordSystem() == SurfCompPrepAttributes::Cylindrical)
                zStart->setEnabled(true);
            else
                zStart->setEnabled(false);
            if (atts->GetCoordSystem() == SurfCompPrepAttributes::Cartesian || atts->GetCoordSystem() == SurfCompPrepAttributes::Cylindrical)
                zStop->setEnabled(true);
            else
                zStop->setEnabled(false);
            if (atts->GetCoordSystem() == SurfCompPrepAttributes::Cartesian || atts->GetCoordSystem() == SurfCompPrepAttributes::Cylindrical)
                zSteps->setEnabled(true);
            else
                zSteps->setEnabled(false);
            coordSystem->setButton(atts->GetCoordSystem());
            break;
          case 2: //thetaStart
            temp.setNum(atts->GetThetaStart());
            thetaStart->setText(temp);
            break;
          case 3: //thetaStop
            temp.setNum(atts->GetThetaStop());
            thetaStop->setText(temp);
            break;
          case 4: //thetaSteps
            temp.sprintf("%d", atts->GetThetaSteps());
            thetaSteps->setText(temp);
            break;
          case 5: //phiStart
            temp.setNum(atts->GetPhiStart());
            phiStart->setText(temp);
            break;
          case 6: //phiStop
            temp.setNum(atts->GetPhiStop());
            phiStop->setText(temp);
            break;
          case 7: //phiSteps
            temp.sprintf("%d", atts->GetPhiSteps());
            phiSteps->setText(temp);
            break;
          case 8: //startRadius
            temp.setNum(atts->GetStartRadius());
            startRadius->setText(temp);
            break;
          case 9: //endRadius
            temp.setNum(atts->GetEndRadius());
            endRadius->setText(temp);
            break;
          case 10: //radiusSteps
            temp.sprintf("%d", atts->GetRadiusSteps());
            radiusSteps->setText(temp);
            break;
          case 11: //xStart
            temp.setNum(atts->GetXStart());
            xStart->setText(temp);
            break;
          case 12: //xStop
            temp.setNum(atts->GetXStop());
            xStop->setText(temp);
            break;
          case 13: //xSteps
            temp.sprintf("%d", atts->GetXSteps());
            xSteps->setText(temp);
            break;
          case 14: //yStart
            temp.setNum(atts->GetYStart());
            yStart->setText(temp);
            break;
          case 15: //yStop
            temp.setNum(atts->GetYStop());
            yStop->setText(temp);
            break;
          case 16: //ySteps
            temp.sprintf("%d", atts->GetYSteps());
            ySteps->setText(temp);
            break;
          case 17: //zStart
            temp.setNum(atts->GetZStart());
            zStart->setText(temp);
            break;
          case 18: //zStop
            temp.setNum(atts->GetZStop());
            zStop->setText(temp);
            break;
          case 19: //zSteps
            temp.sprintf("%d", atts->GetZSteps());
            zSteps->setText(temp);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisSurfCompPrepWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Tue Jul 1 11:03:04 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisSurfCompPrepWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do surfaceType
    if(which_widget == 0 || doAll)
    {
        // Nothing for surfaceType
    }

    // Do coordSystem
    if(which_widget == 1 || doAll)
    {
        // Nothing for coordSystem
    }

    // Do thetaStart
    if(which_widget == 2 || doAll)
    {
        temp = thetaStart->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetThetaStart(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of thetaStart was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetThetaStart());
            Message(msg);
            atts->SetThetaStart(atts->GetThetaStart());
        }
    }

    // Do thetaStop
    if(which_widget == 3 || doAll)
    {
        temp = thetaStop->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetThetaStop(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of thetaStop was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetThetaStop());
            Message(msg);
            atts->SetThetaStop(atts->GetThetaStop());
        }
    }

    // Do thetaSteps
    if(which_widget == 4 || doAll)
    {
        temp = thetaSteps->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetThetaSteps(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of thetaSteps was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetThetaSteps());
            Message(msg);
            atts->SetThetaSteps(atts->GetThetaSteps());
        }
    }

    // Do phiStart
    if(which_widget == 5 || doAll)
    {
        temp = phiStart->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetPhiStart(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of phiStart was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetPhiStart());
            Message(msg);
            atts->SetPhiStart(atts->GetPhiStart());
        }
    }

    // Do phiStop
    if(which_widget == 6 || doAll)
    {
        temp = phiStop->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetPhiStop(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of phiStop was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetPhiStop());
            Message(msg);
            atts->SetPhiStop(atts->GetPhiStop());
        }
    }

    // Do phiSteps
    if(which_widget == 7 || doAll)
    {
        temp = phiSteps->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetPhiSteps(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of phiSteps was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetPhiSteps());
            Message(msg);
            atts->SetPhiSteps(atts->GetPhiSteps());
        }
    }

    // Do startRadius
    if(which_widget == 8 || doAll)
    {
        temp = startRadius->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetStartRadius(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of startRadius was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetStartRadius());
            Message(msg);
            atts->SetStartRadius(atts->GetStartRadius());
        }
    }

    // Do endRadius
    if(which_widget == 9 || doAll)
    {
        temp = endRadius->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetEndRadius(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of endRadius was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetEndRadius());
            Message(msg);
            atts->SetEndRadius(atts->GetEndRadius());
        }
    }

    // Do radiusSteps
    if(which_widget == 10 || doAll)
    {
        temp = radiusSteps->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetRadiusSteps(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of radiusSteps was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetRadiusSteps());
            Message(msg);
            atts->SetRadiusSteps(atts->GetRadiusSteps());
        }
    }

    // Do xStart
    if(which_widget == 11 || doAll)
    {
        temp = xStart->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetXStart(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of xStart was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetXStart());
            Message(msg);
            atts->SetXStart(atts->GetXStart());
        }
    }

    // Do xStop
    if(which_widget == 12 || doAll)
    {
        temp = xStop->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetXStop(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of xStop was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetXStop());
            Message(msg);
            atts->SetXStop(atts->GetXStop());
        }
    }

    // Do xSteps
    if(which_widget == 13 || doAll)
    {
        temp = xSteps->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetXSteps(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of xSteps was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetXSteps());
            Message(msg);
            atts->SetXSteps(atts->GetXSteps());
        }
    }

    // Do yStart
    if(which_widget == 14 || doAll)
    {
        temp = yStart->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetYStart(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of yStart was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetYStart());
            Message(msg);
            atts->SetYStart(atts->GetYStart());
        }
    }

    // Do yStop
    if(which_widget == 15 || doAll)
    {
        temp = yStop->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetYStop(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of yStop was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetYStop());
            Message(msg);
            atts->SetYStop(atts->GetYStop());
        }
    }

    // Do ySteps
    if(which_widget == 16 || doAll)
    {
        temp = ySteps->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetYSteps(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of ySteps was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetYSteps());
            Message(msg);
            atts->SetYSteps(atts->GetYSteps());
        }
    }

    // Do zStart
    if(which_widget == 17 || doAll)
    {
        temp = zStart->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetZStart(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of zStart was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetZStart());
            Message(msg);
            atts->SetZStart(atts->GetZStart());
        }
    }

    // Do zStop
    if(which_widget == 18 || doAll)
    {
        temp = zStop->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetZStop(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of zStop was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetZStop());
            Message(msg);
            atts->SetZStop(atts->GetZStop());
        }
    }

    // Do zSteps
    if(which_widget == 19 || doAll)
    {
        temp = zSteps->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetZSteps(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of zSteps was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetZSteps());
            Message(msg);
            atts->SetZSteps(atts->GetZSteps());
        }
    }

}


//
// Qt Slot functions
//


void
QvisSurfCompPrepWindow::surfaceTypeChanged(int val)
{
    if(val != atts->GetSurfaceType())
    {
        atts->SetSurfaceType(SurfCompPrepAttributes::SurfaceType(val));
        Apply();
    }
}


void
QvisSurfCompPrepWindow::coordSystemChanged(int val)
{
    if(val != atts->GetCoordSystem())
    {
        atts->SetCoordSystem(SurfCompPrepAttributes::CoordinateSystem(val));
        Apply();
    }
}


void
QvisSurfCompPrepWindow::thetaStartProcessText()
{
    GetCurrentValues(2);
    Apply();
}


void
QvisSurfCompPrepWindow::thetaStopProcessText()
{
    GetCurrentValues(3);
    Apply();
}


void
QvisSurfCompPrepWindow::thetaStepsProcessText()
{
    GetCurrentValues(4);
    Apply();
}


void
QvisSurfCompPrepWindow::phiStartProcessText()
{
    GetCurrentValues(5);
    Apply();
}


void
QvisSurfCompPrepWindow::phiStopProcessText()
{
    GetCurrentValues(6);
    Apply();
}


void
QvisSurfCompPrepWindow::phiStepsProcessText()
{
    GetCurrentValues(7);
    Apply();
}


void
QvisSurfCompPrepWindow::startRadiusProcessText()
{
    GetCurrentValues(8);
    Apply();
}


void
QvisSurfCompPrepWindow::endRadiusProcessText()
{
    GetCurrentValues(9);
    Apply();
}


void
QvisSurfCompPrepWindow::radiusStepsProcessText()
{
    GetCurrentValues(10);
    Apply();
}


void
QvisSurfCompPrepWindow::xStartProcessText()
{
    GetCurrentValues(11);
    Apply();
}


void
QvisSurfCompPrepWindow::xStopProcessText()
{
    GetCurrentValues(12);
    Apply();
}


void
QvisSurfCompPrepWindow::xStepsProcessText()
{
    GetCurrentValues(13);
    Apply();
}


void
QvisSurfCompPrepWindow::yStartProcessText()
{
    GetCurrentValues(14);
    Apply();
}


void
QvisSurfCompPrepWindow::yStopProcessText()
{
    GetCurrentValues(15);
    Apply();
}


void
QvisSurfCompPrepWindow::yStepsProcessText()
{
    GetCurrentValues(16);
    Apply();
}


void
QvisSurfCompPrepWindow::zStartProcessText()
{
    GetCurrentValues(17);
    Apply();
}


void
QvisSurfCompPrepWindow::zStopProcessText()
{
    GetCurrentValues(18);
    Apply();
}


void
QvisSurfCompPrepWindow::zStepsProcessText()
{
    GetCurrentValues(19);
    Apply();
}


