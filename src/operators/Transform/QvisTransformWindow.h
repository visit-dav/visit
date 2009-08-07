/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#ifndef QVISTRANSFORMWINDOW_H
#define QVISTRANSFORMWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>
#include <QLineEdit>

class TransformAttributes;
class QButtonGroup;
class QCheckBox;
class QLabel;
class QTabWidget;
class QComboBox;

// ****************************************************************************
//  Class:  QNarrowLineEdit
//
//  Purpose:
//    A QLineEdit that has a narrower default size.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 25, 2001
//
//  Modifications:
//    Brad Whitlock, Wed Aug 13 20:10:25 PDT 2008
//    Qt 4.
//
// ****************************************************************************
class QNarrowLineEdit : public QLineEdit
{
  public:
    QNarrowLineEdit(QWidget *p) : QLineEdit(p)
    {
    }
    QSize sizeHint() const
    {
        QSize size = QLineEdit::sizeHint();
        QFontMetrics fm(font());
        int w = fm.width('x') * 4; // 4 characters
        size.setWidth(w);
        return size;
    }
};

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
// ****************************************************************************

class QvisTransformWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisTransformWindow(const int type,
                         TransformAttributes *subj,
                         const QString &caption = QString::null,
                         const QString &shortName = QString::null,
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
    void ltElementtChanged();
    void linearInvertChanged(bool val);
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
    QNarrowLineEdit *m10;
    QNarrowLineEdit *m11;
    QNarrowLineEdit *m12;
    QNarrowLineEdit *m20;
    QNarrowLineEdit *m21;
    QNarrowLineEdit *m22;
    QCheckBox       *linearInvert;

    QButtonGroup    *inputCoord;
    QButtonGroup    *outputCoord;
    QComboBox       *vectorMethodCombo;

    TransformAttributes *atts;
    QWidget *firstPage;
    QWidget *secondPage;
    QWidget *thirdPage;
};



#endif
