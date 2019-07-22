// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_SEQUENCE_TRANSITION_H
#define QVIS_SEQUENCE_TRANSITION_H

#include <QGroupBox>

class QvisColorButton;
class QButtonGroup;
class QSpinBox;

// ****************************************************************************
// Class: QvisSequenceTransition
//
// Purpose:
//   Sequence transition widget used in the movie wizard.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 12 15:11:36 PST 2006
//
// Modifications:
//   Brad Whitlock, Tue Oct  7 09:27:48 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class QvisSequenceTransition : public QGroupBox
{
    Q_OBJECT
public:
    QvisSequenceTransition(const QPixmap &, QWidget *parent);
    virtual ~QvisSequenceTransition();

    void getFromTransition(bool &frames, QColor &color) const;
    void setFromTransition(bool frames,  const QColor &color);

    void getToTransition(bool &frames, QColor &color) const;
    void setToTransition(bool frames,  const QColor &color);

    void setNFrames(int);
    int getNFrames() const;

private slots:
    void bg1Clicked(int);
    void bg2Clicked(int);
private:
    QButtonGroup    *bg1;
    QButtonGroup    *bg2;
    QvisColorButton *colorButton1;
    QvisColorButton *colorButton2;
    QSpinBox        *nFrames;
};

#endif
