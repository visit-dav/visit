// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_COMPOSITE_TRANSITION_H
#define QVIS_COMPOSITE_TRANSITION_H

#include <QGroupBox>

class QCheckBox;
class QSpinBox;

// ****************************************************************************
// Class: QvisCompositeTransition
//
// Purpose:
//   Sequence transition widget used in the movie wizard.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 16 11:35:55 PDT 2006
//
// Modifications:
//   Brad Whitlock, Tue Oct  7 09:22:35 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class QvisCompositeTransition : public QGroupBox
{
    Q_OBJECT
public:
    QvisCompositeTransition(const QPixmap &, QWidget *parent);
    virtual ~QvisCompositeTransition();

    void setNFrames(int);
    int getNFrames() const;

    void setReverse(bool);
    bool getReverse() const;

private:
    QSpinBox  *nFrames;
    QCheckBox *reverseCheckbox;
};

#endif
