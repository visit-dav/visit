#ifndef QVISISOVOLUMEWINDOW_H
#define QVISISOVOLUMEWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class IsovolumeAttributes;
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
// Class: QvisIsovolumeWindow
//
// Purpose: 
//   Defines QvisIsovolumeWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Fri Jan 30 14:50:02 PST 2004
//
// Modifications:
//   
// ****************************************************************************

class QvisIsovolumeWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisIsovolumeWindow(const int type,
                         IsovolumeAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisIsovolumeWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void lboundProcessText();
    void uboundProcessText();
    void variableChanged(const QString &);
  private:
    QLineEdit          *lbound;
    QLineEdit          *ubound;
    QvisVariableButton *variable;
    QLabel             *lboundLabel;
    QLabel             *uboundLabel;
    QLabel             *variableLabel;

    IsovolumeAttributes *atts;
};



#endif
