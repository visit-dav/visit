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
#include <QButtonGroup>
#include <QFrame>
#include <QLabel>
#include <QLayout>
#include <QRadioButton>
#include <QSpinBox>

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
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//   
//   Brad Whitlock, Tue Oct  7 09:28:24 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QvisSequenceTransition::QvisSequenceTransition(const QPixmap &pix,
    QWidget *parent) : QGroupBox(parent)
{
    QVBoxLayout *innerLayout = new QVBoxLayout(this);
    innerLayout->setMargin(10);
    innerLayout->addSpacing(15);
    QHBoxLayout *hLayout = new QHBoxLayout(0);
    innerLayout->addLayout(hLayout);
    hLayout->setSpacing(15);

    // Left controls.
    QGridLayout *leftLayout = new QGridLayout(0);
    hLayout->addLayout(leftLayout);
    leftLayout->setSpacing(5);
    leftLayout->addWidget(
        new QLabel(tr("Transition from"),this), 0, 0, 1, 2);
    leftLayout->addWidget(new QLabel("    ", this), 1, 0);

    bg1 = new QButtonGroup(this);
    connect(bg1, SIGNAL(buttonClicked(int)),
            this, SLOT(bg1Clicked(int)));
    QRadioButton *rb = new QRadioButton(tr("Frames"), this);
    bg1->addButton(rb, 0);
    leftLayout->addWidget(rb, 1, 1);
    rb = new QRadioButton(tr("Color"), this);
    bg1->addButton(rb, 1);
    leftLayout->addWidget(rb, 2, 1);
    colorButton1 = new QvisColorButton(this);
    leftLayout->addWidget(colorButton1, 2, 2);

    // Add the pixmap here.
    QFrame *vf1 = new QFrame(this);
    vf1->setFrameStyle(QFrame::VLine | QFrame::Sunken);
    hLayout->addWidget(vf1);
    hLayout->addSpacing(10);
    QLabel *pixLabel = new QLabel(this);
    pixLabel->setPixmap(pix);
    hLayout->addWidget(pixLabel);
    hLayout->addSpacing(10);
    QFrame *vf2 = new QFrame(this);
    vf2->setFrameStyle(QFrame::VLine | QFrame::Sunken);
    hLayout->addWidget(vf2);

    // Right controls
    QGridLayout *rightLayout = new QGridLayout(0);
    hLayout->addLayout(rightLayout);
    rightLayout->setSpacing(5);
    rightLayout->addWidget(
        new QLabel(tr("Transition to"), this), 0, 0, 1, 2);
    rightLayout->addWidget(new QLabel("    ", this), 1, 0);

    bg2 = new QButtonGroup(this);
    connect(bg2, SIGNAL(buttonClicked(int)),
            this, SLOT(bg2Clicked(int)));
    rb = new QRadioButton(tr("Frames"), this);
    bg2->addButton(rb, 0);
    rightLayout->addWidget(rb, 1, 1);
    rb = new QRadioButton(tr("Color"), this);
    bg2->addButton(rb, 1);
    rightLayout->addWidget(rb, 2, 1);
    colorButton2 = new QvisColorButton(this);
    rightLayout->addWidget(colorButton2, 2, 2);
    hLayout->addStretch(5);

    // Number of frames controls.
    innerLayout->addSpacing(15);
    QHBoxLayout *frameLayout = new QHBoxLayout(0);
    innerLayout->addLayout(frameLayout);
    frameLayout->setSpacing(5);
    nFrames = new QSpinBox(this);
    nFrames->setMinimum(1);
    nFrames->setValue(10);
    frameLayout->addWidget(
        new QLabel(tr("Number of transition frames"), this));
    frameLayout->addWidget(nFrames);
    frameLayout->addStretch(10);

    innerLayout->addStretch(10);

    // Set some default values.
    bg1->button(0)->setChecked(true);
    colorButton1->setEnabled(false);
    bg2->button(0)->setChecked(true);
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
}

//
// Set/get methods.
//

void
QvisSequenceTransition::getFromTransition(bool &frames, QColor &color) const
{
    frames = (bg1->checkedId() == 0);
    color = colorButton1->buttonColor();
}

void
QvisSequenceTransition::setFromTransition(bool frames,  const QColor &color)
{
    bg1->blockSignals(true);
    bg1->button(frames?0:1)->setChecked(true);
    bg1->blockSignals(false);

    colorButton1->setEnabled(!frames);
    colorButton1->setButtonColor(color);
}

void
QvisSequenceTransition::getToTransition(bool &frames, QColor &color) const
{
    frames = (bg2->checkedId() == 0);
    color = colorButton2->buttonColor();
}

void
QvisSequenceTransition::setToTransition(bool frames,  const QColor &color)
{
    bg2->blockSignals(true);
    bg2->button(frames?0:1)->setChecked(true);
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

