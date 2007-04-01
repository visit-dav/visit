#ifndef QVISINDEXSELECTWINDOW_H
#define QVISINDEXSELECTWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class IndexSelectAttributes;
class QButtonGroup;
class QGroupBox;
class QLabel;
class QLineEdit;
class QSpinBox;

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
//   Kathleen Bonnell, Thu Aug 26 16:55:59 PDT 2004
//   Changed Min/Max/Incr from LineEdit to SpinBox for usability, added
//   labels and group boxes for each dim.
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
    void oneDMinChanged(int min);
    void oneDMaxChanged(int max);
    void oneDIncrChanged(int incr);
    void twoDMinChanged(int min);
    void twoDMaxChanged(int max);
    void twoDIncrChanged(int incr);
    void threeDMinChanged(int min);
    void threeDMaxChanged(int max);
    void threeDIncrChanged(int incr);
    void whichDataChanged(int val);
    void domainIndexProcessText();
    void groupIndexProcessText();
  private:
    QButtonGroup *dim;

    QGroupBox    *oneDWidgetGroup;
    QLabel       *oneDLabel;
    QSpinBox     *oneDMin;
    QSpinBox     *oneDMax;
    QSpinBox     *oneDIncr;

    QGroupBox    *twoDWidgetGroup;
    QLabel       *twoDLabel;
    QSpinBox     *twoDMin;
    QSpinBox     *twoDMax;
    QSpinBox     *twoDIncr;

    QGroupBox    *threeDWidgetGroup;
    QLabel       *threeDLabel;
    QSpinBox     *threeDMin;
    QSpinBox     *threeDMax;
    QSpinBox     *threeDIncr;

    QButtonGroup *whichData;
    QLineEdit *domainIndex;
    QLineEdit *groupIndex;

    IndexSelectAttributes *atts;
};



#endif
