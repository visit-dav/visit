#ifndef QVISMERGEWINDOW_H
#define QVISMERGEWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class MergeOperatorAttributes;
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
// Class: QvisMergeWindow
//
// Purpose: 
//   Defines QvisMergeWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Sun Aug 14 17:22:31 PST 2005
//
// Modifications:
//   
// ****************************************************************************

class QvisMergeWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisMergeWindow(const int type,
                         MergeOperatorAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisMergeWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void dummyChanged(bool val);
  private:
    QCheckBox *dummy;
    QLabel *dummyLabel;

    MergeOperatorAttributes *atts;
};



#endif
