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
  private:
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

    TransformAttributes *atts;
};



#endif
