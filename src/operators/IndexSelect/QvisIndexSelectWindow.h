#ifndef QVISINDEXSELECTWINDOW_H
#define QVISINDEXSELECTWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class IndexSelectAttributes;
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
// Class: QvisIndexSelectWindow
//
// Purpose: 
//   Defines QvisIndexSelectWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Thu Jun 6 17:02:08 PST 2002
//
// Modifications:
//   
// ****************************************************************************

class QvisIndexSelectWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisIndexSelectWindow(const int type,
                         IndexSelectAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisIndexSelectWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void dimChanged(int val);
    void oneDProcessText();
    void twoDProcessText();
    void threeDProcessText();
    void whichDataChanged(int val);
    void domainIndexProcessText();
    void groupIndexProcessText();
  private:
    QButtonGroup *dim;
    QLineEdit *oneD;
    QLineEdit *twoD;
    QLineEdit *threeD;
    QButtonGroup *whichData;
    QLineEdit *domainIndex;
    QLineEdit *groupIndex;

    IndexSelectAttributes *atts;
};



#endif
