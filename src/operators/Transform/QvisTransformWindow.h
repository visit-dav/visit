/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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

#ifndef QVISTRANSFORMWINDOW_H
#define QVISTRANSFORMWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>
#include <qlineedit.h>

class TransformAttributes;
class QLabel;
class QCheckBox;
class QSpinBox;
class QVBox;
class QButtonGroup;
class QvisColorTableButton;
class QvisOpacitySlider;
class QvisColorButton;
class QvisLineStyleWidget;
class QvisLineWidthWidget;
class QTabWidget;

// ****************************************************************************
//  Class:  QNarrowLineEdit
//
//  Purpose:
//    A QLineEdit that has a narrower default size.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 25, 2001
//
// ****************************************************************************
class QNarrowLineEdit : public QLineEdit
{
  public:
    QNarrowLineEdit(QWidget *p, const char *n=0)
        : QLineEdit(p, n)
    {
    }
    QNarrowLineEdit(const QString &s, QWidget *p, const char *n=0)
        : QLineEdit(s, p, n)
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
// ****************************************************************************

class QvisTransformWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisTransformWindow(const int type,
                         TransformAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
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
    void pageTurned(QWidget*);
    void inputCoordChanged(int);
    void outputCoordChanged(int);
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

    QButtonGroup    *inputCoord;
    QButtonGroup    *outputCoord;

    TransformAttributes *atts;
    QFrame *firstPage;
    QFrame *secondPage;
};



#endif
