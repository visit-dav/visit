#ifndef QVISSUBDIVIDEQUADSWINDOW_H
#define QVISSUBDIVIDEQUADSWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class SubdivideQuadsAttributes;
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
// Class: QvisSubdivideQuadsWindow
//
// Purpose: 
//   Defines QvisSubdivideQuadsWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Tue Nov 2 06:28:41 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

class QvisSubdivideQuadsWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisSubdivideQuadsWindow(const int type,
                         SubdivideQuadsAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisSubdivideQuadsWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void thresholdProcessText();
    void maxSubdivsProcessText();
    void fanOutPointsChanged(bool val);
    void doTrianglesChanged(bool val);
    void variableProcessText();
  private:
    QLineEdit *threshold;
    QLineEdit *maxSubdivs;
    QCheckBox *fanOutPoints;
    QCheckBox *doTriangles;
    QLineEdit *variable;
    QLabel *thresholdLabel;
    QLabel *maxSubdivsLabel;
    QLabel *fanOutPointsLabel;
    QLabel *doTrianglesLabel;
    QLabel *variableLabel;

    SubdivideQuadsAttributes *atts;
};



#endif
