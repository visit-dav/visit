// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisCompositeTransition.h>
#include <QvisColorButton.h>
#include <QCheckBox>
#include <QFrame>
#include <QLabel>
#include <QLayout>
#include <QSpinBox>

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
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Tue Oct  7 09:24:21 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QvisCompositeTransition::QvisCompositeTransition(const QPixmap &pix,
    QWidget *parent) : QGroupBox(parent)
{
    QVBoxLayout *innerLayout = new QVBoxLayout(this);
    innerLayout->setContentsMargins(10,10,10,10);
    innerLayout->addSpacing(15);

    // Add the pixmap here.
    QLabel *pixLabel = new QLabel(this);
    pixLabel->setPixmap(pix);
    innerLayout->addWidget(pixLabel);
    innerLayout->addSpacing(15);

    QHBoxLayout *hLayout = new QHBoxLayout(0);
    innerLayout->addLayout(hLayout);
    QGridLayout *gLayout = new QGridLayout(0);
    hLayout->addLayout(gLayout);
    gLayout->setSpacing(15);
    hLayout->addStretch(10);
    innerLayout->addStretch(10);

    // Number of frames controls.
    nFrames = new QSpinBox(this);
    nFrames->setMinimum(1);
    nFrames->setValue(10);
    gLayout->addWidget(
        new QLabel(tr("Number of transition frames"), this), 0, 0);
    gLayout->addWidget(nFrames, 0, 1);

    // Reverse
    reverseCheckbox = new QCheckBox(tr("Reverse"), this);
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
