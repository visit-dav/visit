#ifndef QVISTHREESLICEWINDOW_H
#define QVISTHREESLICEWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class ThreeSliceAttributes;
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
// Class: QvisThreeSliceWindow
//
// Purpose: 
//   Defines QvisThreeSliceWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Mon Jun 9 13:18:31 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class QvisThreeSliceWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisThreeSliceWindow(const int type,
                         ThreeSliceAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisThreeSliceWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void xProcessText();
    void yProcessText();
    void zProcessText();
  private:
    QLineEdit *x;
    QLineEdit *y;
    QLineEdit *z;

    ThreeSliceAttributes *atts;
};



#endif
