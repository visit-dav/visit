#ifndef QVISBOXWINDOW_H
#define QVISBOXWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class BoxAttributes;
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
// Class: QvisBoxWindow
//
// Purpose: 
//   Defines QvisBoxWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Fri Apr 12 14:20:13 PST 2002
//
// Modifications:
//   
// ****************************************************************************

class QvisBoxWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisBoxWindow(const int type,
                         BoxAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisBoxWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void amountChanged(int val);
    void minxProcessText();
    void maxxProcessText();
    void minyProcessText();
    void maxyProcessText();
    void minzProcessText();
    void maxzProcessText();
  private:
    QButtonGroup *amount;
    QLineEdit *minx;
    QLineEdit *maxx;
    QLineEdit *miny;
    QLineEdit *maxy;
    QLineEdit *minz;
    QLineEdit *maxz;

    BoxAttributes *atts;
};



#endif
