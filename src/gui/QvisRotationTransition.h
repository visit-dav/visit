// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_ROTATION_TRANSITION_H
#define QVIS_ROTATION_TRANSITION_H

#include <QGroupBox>

class QSpinBox;

// ****************************************************************************
// Class: QvisRotationTransition
//
// Purpose:
//   Sequence transition widget used in the movie wizard.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 8 08:51:03 PDT 2006
//
// Modifications:
//   Brad Whitlock, Tue Oct  7 09:37:17 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class QvisRotationTransition : public QGroupBox
{
public:
    QvisRotationTransition(const QPixmap &, QWidget *parent);
    virtual ~QvisRotationTransition();

    void setStartAngle(float sa);
    void setEndAngle(float ea);
    void getAngles(float &sa, float &ea) const;

    void setNSteps(int);
    int getNSteps() const;

private:
    QSpinBox *startAngle;
    QSpinBox *endAngle;
    QSpinBox *nSteps;
};

#endif
