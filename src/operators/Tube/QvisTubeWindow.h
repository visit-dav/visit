#ifndef QVISTUBEWINDOW_H
#define QVISTUBEWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class TubeAttributes;
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
// Class: QvisTubeWindow
//
// Purpose: 
//   Defines QvisTubeWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Wed Oct 30 10:58:01 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

class QvisTubeWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisTubeWindow(const int type,
                         TubeAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisTubeWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void widthProcessText();
    void finenessProcessText();
    void cappingChanged(bool val);
  private:
    QLineEdit *width;
    QLineEdit *fineness;
    QCheckBox *capping;

    TubeAttributes *atts;
};



#endif
