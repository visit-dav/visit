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

// ****************************************************************************
// Class: QvisDisplaceWindow
//
// Purpose: 
//   Defines QvisDisplaceWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Fri Apr 12 14:40:27 PST 2002
//
// Modifications:
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
    void variableProcessText();
  private:
    QLineEdit *factor;
    QLineEdit *variable;

    DisplaceAttributes *atts;
};



#endif
