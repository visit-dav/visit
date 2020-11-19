// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISMATERIALWINDOW_H
#define QVISMATERIALWINDOW_H

#include <AttributeSubject.h>
#include <QvisPostableWindowObserver.h>

class MaterialAttributes;
class QLabel;
class QCheckBox;
class QNarrowLineEdit;
class QButtonGroup;
class QComboBox;

// ****************************************************************************
// Class: QvisMaterialWindow
//
// Purpose: 
//   Defines QvisMaterialWindow class.
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
//    Jeremy Meredith, Thu Aug 18 16:14:59 PDT 2005
//    Changed algorithm selection to a multiple-choice.
//    Added VF for isovolume method.
//
//    Brad Whitlock, Wed Apr  9 11:36:37 PDT 2008
//    QString for caption, shortName.
//
//    Jeremy Meredith, Fri Feb 13 12:11:07 EST 2009
//    Added material iteration capability.
//
// ****************************************************************************

class QvisMaterialWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
  public:
    QvisMaterialWindow(MaterialAttributes *subj,
                       const QString &caption = QString(),
                       const QString &shortName = QString(),
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
    void algorithmChanged(int val);
    void simplifyHeavilyMixedZonesChanged(bool val);
    void maxMatsPerZoneProcessText(void);
    void isoVolumeFractionProcessText(void);
    void numIterationsProcessText(void);
    void iterationDampingProcessText(void);
    void enableIterationChanged(bool val);
    void annealingTimeProcessText(void);
  private:
    MaterialAttributes *atts;

    QCheckBox *smoothing;
    QCheckBox *forceFullConnectivity;
    QCheckBox *forceMIR;
    QCheckBox *cleanZonesOnly;
    QComboBox *algorithm;
    QCheckBox *simplifyHeavilyMixedZones;
    QNarrowLineEdit *maxMatsPerZone;
    QNarrowLineEdit *isoVolumeFraction;
    QCheckBox *enableIteration;
    QNarrowLineEdit *numIterations;
    QNarrowLineEdit *iterationDamping;
    QNarrowLineEdit *annealingTime;
    QLabel *maxMatsPerZoneLabel;
    QLabel *algorithmLabel;
    QLabel *isoVolumeFractionLabel;
    QLabel *numIterationsLabel;
    QLabel *iterationDampingLabel;
    QLabel *annealingTimeLabel;
};

#endif
