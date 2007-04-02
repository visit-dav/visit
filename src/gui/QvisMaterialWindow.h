#ifndef QVISMATERIALWINDOW_H
#define QVISMATERIALWINDOW_H

#include <AttributeSubject.h>
#include <QvisPostableWindowObserver.h>

class MaterialAttributes;
class QLabel;
class QCheckBox;
class QNarrowLineEdit;

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
//    Jeremy Meredith, Fri Sep  5 15:17:20 PDT 2003
//    Added a toggle for the new MIR algorithm.
//
//    Hank Childs, Tue Aug 16 15:36:43 PDT 2005
//    Add a toggle for "simplify heavily mixed zones".
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
    void useNewMIRChanged(bool val);
    void simplifyHeavilyMixedZonesChanged(bool val);
    void maxMatsPerZoneProcessText(void);
  private:
    MaterialAttributes *atts;

    QCheckBox *smoothing;
    QCheckBox *forceFullConnectivity;
    QCheckBox *forceMIR;
    QCheckBox *cleanZonesOnly;
    QCheckBox *useNewMIR;
    QCheckBox *simplifyHeavilyMixedZones;
    QNarrowLineEdit *maxMatsPerZone;
    QLabel *maxMatsPerZoneLabel;
};

#endif
