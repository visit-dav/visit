#ifndef QVISDISPLACEWINDOW_H
#define QVISDISPLACEWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class DisplaceAttributes;
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
// Class: QvisDisplaceWindow
//
// Purpose: 
//   Defines QvisDisplaceWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Thu Dec 9 15:24:08 PST 2004
//
// Modifications:
//   Brad Whitlock, Thu Dec 9 15:26:42 PST 2004
//   Changed it so it uses a variable button.
//
// ****************************************************************************

class QvisDisplaceWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisDisplaceWindow(const int type,
                         DisplaceAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisDisplaceWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void factorProcessText();
    void variableChanged(const QString &varName);
  private:
    QLineEdit *factor;
    QvisVariableButton *variable;

    DisplaceAttributes *atts;
};



#endif
