#ifndef QVIS_GLOBALLINEOUT_WINDOW_H
#define QVIS_GLOBALLINEOUT_WINDOW_H

#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>

class GlobalLineoutAttributes;
class QCheckBox;
class QLabel;
class QLineEdit;


// ****************************************************************************
// Class: QvisGlobalLineoutWindow
//
// Purpose: 
//   Defines QvisGlobalLineoutWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Fri Nov 19 10:46:23 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

class GUI_API QvisGlobalLineoutWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
  public:
    QvisGlobalLineoutWindow(
                         GlobalLineoutAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisGlobalLineoutWindow();
    virtual void CreateWindowContents();
  public slots:
    virtual void apply();
  protected:
    void UpdateWindow(bool doAll);
    void GetCurrentValues(int which_widget);
    void Apply(bool ignore = false);
  private slots:
    void DynamicChanged(bool val);
    void createWindowChanged(bool val);
    void windowIdProcessText();
    void samplingOnChanged(bool val);
    void numSamplesProcessText();
    void createReflineLabelsChanged(bool val);
  private:
    QCheckBox *Dynamic;
    QCheckBox *createWindow;
    QLineEdit *windowId;
    QCheckBox *samplingOn;
    QLineEdit *numSamples;
    QCheckBox *createReflineLabels;
    QLabel *createWindowLabel;
    QLabel *windowIdLabel;
    QLabel *numSamplesLabel;

    GlobalLineoutAttributes *atts;
};



#endif
