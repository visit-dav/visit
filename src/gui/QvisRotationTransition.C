/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/
#include <QvisRotationTransition.h>
#include <QvisColorButton.h>
#include <qbuttongroup.h>
#include <qframe.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qspinbox.h>

// ****************************************************************************
// Method: QvisRotationTransition::QvisRotationTransition
//
// Purpose: 
//   Constructor
//
// Arguments:
//   pix    : The pixmap to use in the widget.
//   parent : The widhet's parent.
//   name   : The widget's name.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 8 10:12:57 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

QvisRotationTransition::QvisRotationTransition(const QPixmap &pix,
    QWidget *parent, const char *name)
{
    QVBoxLayout *innerLayout = new QVBoxLayout(this);
    innerLayout->setMargin(10);
    innerLayout->addSpacing(15);

    // Add the pixmap here.
    QLabel *pixLabel = new QLabel(this, "pixLabel");
    pixLabel->setPixmap(pix);
    innerLayout->addWidget(pixLabel);
    innerLayout->addSpacing(15);

    QHBoxLayout *hLayout = new QHBoxLayout(innerLayout);
    QGridLayout *gLayout = new QGridLayout(hLayout, 3, 2);
    gLayout->setSpacing(10);
    hLayout->addStretch(10);

    // Angle controls.
    startAngle = new QSpinBox(this, "startAngle");
    startAngle->setMinValue(-360 * 100);
    startAngle->setMaxValue(360 * 100);
    startAngle->setValue(0);
    QString startLabel; startLabel.sprintf("Start angle%c", 176);
    gLayout->addWidget(
        new QLabel(startLabel, this, "startAngle"), 0, 0);
    gLayout->addWidget(startAngle, 0, 1);
    
    endAngle = new QSpinBox(this, "endAngle");
    endAngle->setMinValue(-360 * 100);
    endAngle->setMaxValue(360 * 100);
    endAngle->setValue(360);
    QString endLabel; endLabel.sprintf("End angle%c", 176);
    gLayout->addWidget(
        new QLabel(endLabel, this, "endAngle"), 1, 0);
    gLayout->addWidget(endAngle, 1, 1);

    // Number of steps controls.
    nSteps = new QSpinBox(this, "nSteps");
    nSteps->setMinValue(1);
    nSteps->setValue(30);
    gLayout->addWidget(
        new QLabel("Number of steps", this, "nStepsLabel"), 2, 0);
    gLayout->addWidget(nSteps, 2, 1);

    innerLayout->addStretch(10);
}

// ****************************************************************************
// Method: QvisRotationTransition::~QvisRotationTransition
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 13:26:39 PST 2006
//
// Modifications:
//   
// ****************************************************************************

QvisRotationTransition::~QvisRotationTransition()
{
}

//
// Set/Get methods.
//

void
QvisRotationTransition::setStartAngle(float sa)
{
    startAngle->setValue(int(sa));
}

void
QvisRotationTransition::setEndAngle(float ea)
{
    endAngle->setValue(int(ea));
}

void
QvisRotationTransition::getAngles(float &sa, float &ea) const
{
    sa = float(startAngle->value());
    ea = float(endAngle->value());
}

void
QvisRotationTransition::setNSteps(int val)
{
    nSteps->setValue(val);
}

int
QvisRotationTransition::getNSteps() const
{
    return nSteps->value();
}

