// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisRotationTransition.h>
#include <QvisColorButton.h>
#include <QButtonGroup>
#include <QFrame>
#include <QLabel>
#include <QLayout>
#include <QRadioButton>
#include <QSpinBox>

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
//   Brad Whitlock, Tue Apr  8 15:26:49 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Tue Oct  7 09:37:43 PDT 2008
//   Qt 4.
//
//   Kathleen Biagas, Thu Jan 21, 2021
//   Fixed use of QString.asprintf to correct form.
//
// ****************************************************************************

QvisRotationTransition::QvisRotationTransition(const QPixmap &pix,
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
    gLayout->setSpacing(10);
    hLayout->addStretch(10);

    // Angle controls.
    startAngle = new QSpinBox(this);
    startAngle->setKeyboardTracking(false);
    startAngle->setMinimum(-360 * 100);
    startAngle->setMaximum(360 * 100);
    startAngle->setValue(0);
    QString deg = QString::asprintf("%c", 176);
    QString startLabel(tr("Start angle") + deg);
    gLayout->addWidget(
        new QLabel(startLabel, this), 0, 0);
    gLayout->addWidget(startAngle, 0, 1);

    endAngle = new QSpinBox(this);
    endAngle->setKeyboardTracking(false);
    endAngle->setMinimum(-360 * 100);
    endAngle->setMaximum(360 * 100);
    endAngle->setValue(360);
    QString endLabel(tr("End angle") +deg);
    gLayout->addWidget(
        new QLabel(endLabel, this), 1, 0);
    gLayout->addWidget(endAngle, 1, 1);

    // Number of steps controls.
    nSteps = new QSpinBox(this);
    nSteps->setKeyboardTracking(false);
    nSteps->setMinimum(1);
    nSteps->setValue(30);
    gLayout->addWidget(
        new QLabel(tr("Number of steps"), this), 2, 0);
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

