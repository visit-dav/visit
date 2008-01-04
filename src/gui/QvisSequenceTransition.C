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
#include <QvisSequenceTransition.h>
#include <QvisColorButton.h>
#include <qbuttongroup.h>
#include <qframe.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qspinbox.h>

// ****************************************************************************
// Method: QvisSequenceTransition::QvisSequenceTransition
//
// Purpose: 
//   Constructor
//
// Arguments:
//   pix    : The widget pixmap.
//   parent : The widget's parent.
//   name   : The widget's name.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 13:18:41 PST 2006
//
// Modifications:
//   
// ****************************************************************************

QvisSequenceTransition::QvisSequenceTransition(const QPixmap &pix,
    QWidget *parent, const char *name)
{
    QVBoxLayout *innerLayout = new QVBoxLayout(this);
    innerLayout->setMargin(10);
    innerLayout->addSpacing(15);
    QHBoxLayout *hLayout = new QHBoxLayout(innerLayout);
    hLayout->setSpacing(15);

    // Left controls.
    QGridLayout *leftLayout = new QGridLayout(hLayout, 3, 4);
    leftLayout->setSpacing(5);
    leftLayout->addMultiCellWidget(
        new QLabel("Transition from", this, "FromLabel"), 0, 0, 0, 2);
    leftLayout->addWidget(new QLabel("    ", this, "spacer1"), 1, 0);

    bg1 = new QButtonGroup(0, "bg1");
    connect(bg1, SIGNAL(clicked(int)),
            this, SLOT(bg1Clicked(int)));
    QRadioButton *rb = new QRadioButton("Frames", this, "bg1r0");
    bg1->insert(rb, 0);
    leftLayout->addWidget(rb, 1, 1);
    rb = new QRadioButton("Color", this, "bg1r1");
    bg1->insert(rb, 1);
    leftLayout->addWidget(rb, 2, 1);
    colorButton1 = new QvisColorButton(this, "colorButton1");
    leftLayout->addWidget(colorButton1, 2, 2);

    // Add the pixmap here.
    QFrame *vf1 = new QFrame(this, "vf1");
    vf1->setFrameStyle(QFrame::VLine | QFrame::Sunken);
    hLayout->addWidget(vf1);
    hLayout->addSpacing(10);
    QLabel *pixLabel = new QLabel(this, "pixLabel");
    pixLabel->setPixmap(pix);
    hLayout->addWidget(pixLabel);
    hLayout->addSpacing(10);
    QFrame *vf2 = new QFrame(this, "vf2");
    vf2->setFrameStyle(QFrame::VLine | QFrame::Sunken);
    hLayout->addWidget(vf2);

    // Right controls
    QGridLayout *rightLayout = new QGridLayout(hLayout, 3, 4);
    rightLayout->setSpacing(5);
    rightLayout->addMultiCellWidget(
        new QLabel("Transition to", this, "ToLabel"), 0, 0, 0, 2);
    rightLayout->addWidget(new QLabel("    ", this, "spacer2"), 1, 0);

    bg2 = new QButtonGroup(0, "bg2");
    connect(bg2, SIGNAL(clicked(int)),
            this, SLOT(bg2Clicked(int)));
    rb = new QRadioButton("Frames", this, "bg2r0");
    bg2->insert(rb, 0);
    rightLayout->addWidget(rb, 1, 1);
    rb = new QRadioButton("Color", this, "bg2r2");
    bg2->insert(rb, 1);
    rightLayout->addWidget(rb, 2, 1);
    colorButton2 = new QvisColorButton(this, "colorButton2");
    rightLayout->addWidget(colorButton2, 2, 2);
    hLayout->addStretch(5);

    // Number of frames controls.
    innerLayout->addSpacing(15);
    QHBoxLayout *frameLayout = new QHBoxLayout(innerLayout);
    frameLayout->setSpacing(5);
    nFrames = new QSpinBox(this, "nFrames");
    nFrames->setMinValue(1);
    nFrames->setValue(10);
    frameLayout->addWidget(
        new QLabel("Number of transition frames", this, "nFramesLabel"));
    frameLayout->addWidget(nFrames);
    frameLayout->addStretch(10);

    innerLayout->addStretch(10);

    // Set some default values.
    bg1->setButton(0);
    colorButton1->setEnabled(false);
    bg2->setButton(0);
    colorButton2->setEnabled(false);
}

// ****************************************************************************
// Method: QvisSequenceTransition::~QvisSequenceTransition
//
// Purpose: 
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 13:19:24 PST 2006
//
// Modifications:
//   
// ****************************************************************************

QvisSequenceTransition::~QvisSequenceTransition()
{
    delete bg1;
    delete bg2;
}

//
// Set/get methods.
//

void
QvisSequenceTransition::getFromTransition(bool &frames, QColor &color) const
{
    frames = (bg1->id(bg1->selected()) == 0);
    color = colorButton1->buttonColor();
}

void
QvisSequenceTransition::setFromTransition(bool frames,  const QColor &color)
{
    bg1->blockSignals(true);
    bg1->setButton(frames?0:1);
    bg1->blockSignals(false);

    colorButton1->setEnabled(!frames);
    colorButton1->setButtonColor(color);
}

void
QvisSequenceTransition::getToTransition(bool &frames, QColor &color) const
{
    frames = (bg2->id(bg2->selected()) == 0);
    color = colorButton2->buttonColor();
}

void
QvisSequenceTransition::setToTransition(bool frames,  const QColor &color)
{
    bg2->blockSignals(true);
    bg2->setButton(frames?0:1);
    bg2->blockSignals(false);

    colorButton2->setEnabled(!frames);
    colorButton2->setButtonColor(color);
}

void
QvisSequenceTransition::setNFrames(int val)
{
    nFrames->setValue(val);
}

int
QvisSequenceTransition::getNFrames() const
{
    return nFrames->value();
}

//
// Qt slot functions.
//

void
QvisSequenceTransition::bg1Clicked(int val)
{
    colorButton1->setEnabled(val == 1);
}

void
QvisSequenceTransition::bg2Clicked(int val)
{
    colorButton2->setEnabled(val == 1);
}

