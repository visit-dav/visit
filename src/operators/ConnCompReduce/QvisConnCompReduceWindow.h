#ifndef QVISCONNCOMPREDUCEWINDOW_H
#define QVISCONNCOMPREDUCEWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class ConnCompReduceAttributes;
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
// Class: QvisConnCompReduceWindow
//
// Purpose: 
//   Defines QvisConnCompReduceWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Tue Oct 29 10:16:44 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

class QvisConnCompReduceWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisConnCompReduceWindow(const int type,
                         ConnCompReduceAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisConnCompReduceWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void targetProcessText();
  private:
    QLineEdit *target;

    ConnCompReduceAttributes *atts;
};



#endif
