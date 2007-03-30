#ifndef QVISCONTEXTWINDOW_H
#define QVISCONTEXTWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class ContextAttributes;
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

// ****************************************************************************
// Class: QvisContextWindow
//
// Purpose: 
//   Defines QvisContextWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Fri Apr 12 14:40:17 PST 2002
//
// Modifications:
//   
// ****************************************************************************

class QvisContextWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisContextWindow(const int type,
                         ContextAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisContextWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void offsetProcessText();
    void lowProcessText();
    void hiProcessText();
    void contextProcessText();
    void cutoffProcessText();
    void belowProcessText();
    void aboveProcessText();
  private:
    QLineEdit *offset;
    QLineEdit *low;
    QLineEdit *hi;
    QLineEdit *context;
    QLineEdit *cutoff;
    QLineEdit *below;
    QLineEdit *above;

    ContextAttributes *atts;
};



#endif
