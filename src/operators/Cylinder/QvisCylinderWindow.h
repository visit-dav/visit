#ifndef QVISCYLINDERWINDOW_H
#define QVISCYLINDERWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class CylinderAttributes;
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
// Class: QvisCylinderWindow
//
// Purpose: 
//   Defines QvisCylinderWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Tue Oct 21 13:17:13 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class QvisCylinderWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisCylinderWindow(const int type,
                         CylinderAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisCylinderWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void point1ProcessText();
    void point2ProcessText();
    void radiusProcessText();
  private:
    QLineEdit *point1;
    QLineEdit *point2;
    QLineEdit *radius;
    QLabel *point1Label;
    QLabel *point2Label;
    QLabel *radiusLabel;

    CylinderAttributes *atts;
};



#endif
