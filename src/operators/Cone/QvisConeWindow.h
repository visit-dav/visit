#ifndef QVISCONEWINDOW_H
#define QVISCONEWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class ConeAttributes;
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
// Class: QvisConeWindow
//
// Purpose: 
//   Defines QvisConeWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Mon Jun 3 15:59:57 PST 2002
//
// Modifications:
//   
// ****************************************************************************

class QvisConeWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisConeWindow(const int type,
                         ConeAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisConeWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void angleProcessText();
    void originProcessText();
    void normalProcessText();
    void representationChanged(int val);
    void upAxisProcessText();
    void cutByLengthChanged(bool val);
    void lengthProcessText();
  private:
    QLineEdit *angle;
    QLineEdit *origin;
    QLineEdit *normal;
    QButtonGroup *representation;
    QLineEdit *upAxis;
    QCheckBox *cutByLength;
    QLineEdit *length;

    ConeAttributes *atts;
};



#endif
