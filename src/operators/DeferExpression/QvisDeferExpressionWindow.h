#ifndef QVISDEFEREXPRESSIONWINDOW_H
#define QVISDEFEREXPRESSIONWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class DeferExpressionAttributes;
class QLabel;
class QCheckBox;
class QLineEdit;
class QSpinBox;
class QVBox;
class QButtonGroup;
class QvisColorTableButton;
class QvisOpacitySlider;
class QvisColorButton;
class QvisLineStyleWidget;
class QvisLineWidthWidget;
class QvisVariableButton;

// ****************************************************************************
// Class: QvisDeferExpressionWindow
//
// Purpose: 
//   Defines QvisDeferExpressionWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Tue Sep 20 13:25:13 PST 2005
//
// Modifications:
//   
// ****************************************************************************

class QvisDeferExpressionWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisDeferExpressionWindow(const int type,
                         DeferExpressionAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisDeferExpressionWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void addVariable(const QString &);
    void variableProcessText();
  private:

    DeferExpressionAttributes *atts;
    QvisVariableButton  *varsButton;
    QLineEdit           *varsLineEdit;
};



#endif
