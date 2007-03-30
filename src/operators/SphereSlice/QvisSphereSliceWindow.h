#ifndef QVISSPHERESLICEWINDOW_H
#define QVISSPHERESLICEWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class SphereSliceAttributes;
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
// Class: QvisSphereSliceWindow
//
// Purpose: 
//   Defines QvisSphereSliceWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Fri Apr 12 14:40:57 PST 2002
//
// Modifications:
//   
// ****************************************************************************

class QvisSphereSliceWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisSphereSliceWindow(const int type,
                         SphereSliceAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisSphereSliceWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void originProcessText();
    void radiusProcessText();
  private:
    QLineEdit *origin;
    QLineEdit *radius;

    SphereSliceAttributes *atts;
};



#endif
