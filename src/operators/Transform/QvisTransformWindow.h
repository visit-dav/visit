// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISTRANSFORMWINDOW_H
#define QVISTRANSFORMWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>
#include <QNarrowLineEdit.h>

class TransformAttributes;
class QButtonGroup;
class QCheckBox;
class QLabel;
class QTabWidget;
class QComboBox;


// ****************************************************************************
//  Class:  QvisTransformWindow
//
//  Purpose:
//    Sets the attributes for the transform operator.
//
//  Note:   
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 24, 2001
//
//  Modifications:
//    Jeremy Meredith, Tue Sep 25 11:43:01 PDT 2001
//    Made some of the LineEdits use my new QNarrowLineEdit so that the
//    window would default to a smaller width.
//
//    Brad Whitlock, Fri Apr 12 13:16:24 PST 2002
//    Made it inherit from QvisOperatorWindow.
//
//    Jeremy Meredith, Fri Feb  4 17:48:04 PST 2005
//    Added support for coordinate transforms.
//
//    Jeremy Meredith, Tue Apr 15 13:26:09 EDT 2008
//    Added support for linear transforms.
//
//    Jeremy Meredith, Fri Aug  7 16:12:05 EDT 2009
//    Added choice for multiple ways one might want to transform a vector
//    in coordinate system conversions.
//
//    Dave Pugmire, Fri May 14 08:04:43 EDT 2010
//    Flag for vector transformations.
//
//    Tom Fogal, Tue Jul 27 11:08:13 MDT 2010
//    Add widgets for 4x4 matrices.
//
// ****************************************************************************

class QvisTransformWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisTransformWindow(const int type,
                         TransformAttributes *subj,
                         const QString &caption = QString(),
                         const QString &shortName = QString(),
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisTransformWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void doRotateChanged(bool val);
    void rotateOriginProcessText();
    void rotateAxisProcessText();
    void rotateAmountProcessText();
    void rotateTypeChanged(int val);
    void doScaleChanged(bool val);
    void scaleOriginProcessText();
    void scaleXProcessText();
    void scaleYProcessText();
    void scaleZProcessText();
    void doTranslateChanged(bool val);
    void translateXProcessText();
    void translateYProcessText();
    void translateZProcessText();
    void pageTurned(int);
    void inputCoordChanged(int);
    void outputCoordChanged(int);
    void continuousPhiChanged(bool val);
    void ltElementtChanged();
    void linearInvertChanged(bool val);
    void transformVectorsChanged(bool val);
    void vectorMethodChanged(int);
  private:
    QTabWidget      *transformTypeTabs;

    QCheckBox       *doRotate;
    QLineEdit       *rotateOrigin;
    QLabel          *rotateOriginLabel;
    QLineEdit       *rotateAxis;
    QLabel          *rotateAxisLabel;
    QNarrowLineEdit *rotateAmount;
    QLabel          *rotateAmountLabel;
    QButtonGroup    *rotateType;
    QWidget         *rotateTypeWidget;

    QCheckBox       *doScale;
    QLineEdit       *scaleOrigin;
    QLabel          *scaleOriginLabel;
    QNarrowLineEdit *scaleX;
    QLabel          *scaleXLabel;
    QNarrowLineEdit *scaleY;
    QLabel          *scaleYLabel;
    QNarrowLineEdit *scaleZ;
    QLabel          *scaleZLabel;

    QCheckBox       *doTranslate;
    QNarrowLineEdit *translateX;
    QLabel          *translateXLabel;
    QNarrowLineEdit *translateY;
    QLabel          *translateYLabel;
    QNarrowLineEdit *translateZ;
    QLabel          *translateZLabel;

    QNarrowLineEdit *m00;
    QNarrowLineEdit *m01;
    QNarrowLineEdit *m02;
    QNarrowLineEdit *m03;
    QNarrowLineEdit *m10;
    QNarrowLineEdit *m11;
    QNarrowLineEdit *m12;
    QNarrowLineEdit *m13;
    QNarrowLineEdit *m20;
    QNarrowLineEdit *m21;
    QNarrowLineEdit *m22;
    QNarrowLineEdit *m23;
    QNarrowLineEdit *m30;
    QNarrowLineEdit *m31;
    QNarrowLineEdit *m32;
    QNarrowLineEdit *m33;
    QCheckBox       *linearInvert;
    QCheckBox       *transformVectors1, *transformVectors3;

    QButtonGroup    *inputCoord;
    QButtonGroup    *outputCoord;
    QCheckBox       *continuousPhi;
    QLabel          *continuousPhiLabel;
    QComboBox       *vectorMethodCombo;

    TransformAttributes *atts;
    QWidget *firstPage;
    QWidget *secondPage;
    QWidget *thirdPage;
};
#endif
