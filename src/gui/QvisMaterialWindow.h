#ifndef QVISMATERIALWINDOW_H
#define QVISMATERIALWINDOW_H

#include <AttributeSubject.h>
#include <QvisPostableWindowObserver.h>

class MaterialAttributes;
class QLabel;
class QCheckBox;

// ****************************************************************************
// Class: QvisMaterialWindow
//
// Purpose: 
//   Defines QvisMaterialWindow class.
//
// Notes:      This class was automatically generated!
//
// Programmer: Jeremy Meredith
// Creation:   October 24, 2002
//
// Modifications:
//    Jeremy Meredith, Wed Jul 30 10:46:04 PDT 2003
//    Added the toggle for forcing full connectivity.
//
// ****************************************************************************

class QvisMaterialWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
  public:
    QvisMaterialWindow(MaterialAttributes *subj,
                       const char *caption = 0,
                       const char *shortName = 0,
                       QvisNotepadArea *notepad = 0);
    virtual ~QvisMaterialWindow();
    virtual void CreateWindowContents();

  public slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();
  protected:
    void UpdateWindow(bool doAll);
    void Apply(bool iIgnore = false);
    void GetCurrentValues(int which_widget);
  private slots:
    void smoothingChanged(bool val);
    void forceFullConnectivityChanged(bool val);
    void forceMIRChanged(bool val);
    void cleanZonesOnlyChanged(bool val);
  private:
    MaterialAttributes *atts;

    QCheckBox *smoothing;
    QCheckBox *forceFullConnectivity;
    QCheckBox *forceMIR;
    QCheckBox *cleanZonesOnly;
};

#endif
