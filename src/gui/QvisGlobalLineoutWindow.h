// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_GLOBALLINEOUT_WINDOW_H
#define QVIS_GLOBALLINEOUT_WINDOW_H

#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>

class GlobalLineoutAttributes;
class QCheckBox;
class QComboBox;
class QGroupBox;
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
//   Kathleen Bonnell, Fri Feb  4 07:10:27 PST 2005
//   Added widgets for new atts (colorOption, curveOption).  Modified
//   'dynamic' widget. 
//   
//   Kathleen Bonnell, Fri Feb  4 07:10:27 PST 2005
//   Added freezeInTime widget.
//
//   Brad Whitlock, Wed Apr  9 11:47:09 PDT 2008
//   QString for caption, shortName.
//
// ****************************************************************************

class GUI_API QvisGlobalLineoutWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
  public:
    QvisGlobalLineoutWindow(
                         GlobalLineoutAttributes *subj,
                         const QString &caption = QString(),
                         const QString &shortName = QString(),
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisGlobalLineoutWindow();
    virtual void CreateWindowContents();
  public slots:
    virtual void apply();
  protected:
    void UpdateWindow(bool);
    void GetCurrentValues(int);
    void Apply(bool ignore = false);
  private slots:
    void dynamicChanged(bool);
    void freezeInTimeChanged(bool);
    void createWindowChanged(bool);
    void windowIdProcessText();
    void samplingOnChanged(bool);
    void numSamplesProcessText();
    void createReflineLabelsChanged(bool);
    void curveOptionsChanged(int);
    void colorOptionsChanged(int);
  private:
    QCheckBox *createWindow;
    QLineEdit *windowId;
    QCheckBox *samplingOn;
    QLineEdit *numSamples;
    QCheckBox *createReflineLabels;
    QLabel *windowIdLabel;
    QLabel *numSamplesLabel;

    QGroupBox *dynamic;
    QComboBox    *curveOptions;
    QLabel       *curveLabel;
    QComboBox    *colorOptions;
    QLabel       *colorLabel;
    QCheckBox    *freezeInTime;

    GlobalLineoutAttributes *atts;
};



#endif
