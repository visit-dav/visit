#ifndef QVISSILODUMPWINDOW_H
#define QVISSILODUMPWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class SiloDumpAttributes;
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
// Class: QvisSiloDumpWindow
//
// Purpose: 
//   Defines QvisSiloDumpWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Fri Apr 12 14:40:45 PST 2002
//
// Modifications:
//   
// ****************************************************************************

class QvisSiloDumpWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisSiloDumpWindow(const int type,
                         SiloDumpAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisSiloDumpWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void filenameProcessText();
    void displayChanged(bool val);
  private:
    QLineEdit *filename;
    QCheckBox *display;

    SiloDumpAttributes *atts;
};



#endif
