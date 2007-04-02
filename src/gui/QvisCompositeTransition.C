/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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
#include <QvisCompositeTransition.h>
#include <QvisColorButton.h>
#include <qcheckbox.h>
#include <qframe.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qspinbox.h>

// ****************************************************************************
// Method: QvisCompositeTransition::QvisCompositeTransition
//
// Purpose: 
//   Constructor.
//
// Arguments:
//   pix    : The widget's pixmap.
//   parent : The widget's parent.
//   name   : The widget's name.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 13:21:32 PST 2006
//
// Modifications:
//   
// ****************************************************************************

QvisCompositeTransition::QvisCompositeTransition(const QPixmap &pix,
    QWidget *parent, const char *name) : QGroupBox(parent, name)
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
    QGridLayout *gLayout = new QGridLayout(hLayout, 2, 2);
    gLayout->setSpacing(15);
    hLayout->addStretch(10);
    innerLayout->addStretch(10);

    // Number of frames controls.
    nFrames = new QSpinBox(this, "nFrames");
    nFrames->setMinValue(1);
    nFrames->setValue(10);
    gLayout->addWidget(
        new QLabel("Number of transition frames", this, "nFramesLabel"), 0, 0);
    gLayout->addWidget(nFrames, 0, 1);

    // Reverse
    reverseCheckbox = new QCheckBox("Reverse", this, "reverse");
    gLayout->addWidget(reverseCheckbox, 1, 0);
}

// ****************************************************************************
// Method: QvisCompositeTransition::~QvisCompositeTransition
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 13:22:16 PST 2006
//
// Modifications:
//   
// ****************************************************************************

QvisCompositeTransition::~QvisCompositeTransition()
{
}

//
// Set/Get methods.
//

void
QvisCompositeTransition::setNFrames(int val)
{
    nFrames->setValue(val);
}

int
QvisCompositeTransition::getNFrames() const
{
    return nFrames->value();
}

void
QvisCompositeTransition::setReverse(bool val)
{
    reverseCheckbox->setChecked(val);
}

bool
QvisCompositeTransition::getReverse() const
{
    return reverseCheckbox->isChecked();
}
