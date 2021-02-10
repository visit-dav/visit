// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISTRUECOLORPLOTWINDOW_H
#define QVISTRUECOLORPLOTWINDOW_H

#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

class TruecolorAttributes;
class QLabel;
class QCheckBox;
class QvisOpacitySlider;

// ****************************************************************************
// Class: QvisTruecolorPlotWindow
//
// Purpose: 
//   Defines QvisTruecolorPlotWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Tue Jun 15 11:10:32 PDT 2004
//
// Modifications:
//   Kathleen Bonnell, Mon Jul 25 15:27:06 PDT 2005
//   Add lighting toggle.
//   
//   Allen Sanderson, Sun Mar  7 12:49:56 PST 2010
//   Rename lighting to lightingToggle.
//
// ****************************************************************************

class QvisTruecolorPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
  public:
    QvisTruecolorPlotWindow(const int type,
                         TruecolorAttributes *subj,
                         const QString &caption = QString(),
                         const QString &shortName = QString(),
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisTruecolorPlotWindow();
    virtual void CreateWindowContents();
  public slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();
  protected:
    void UpdateWindow(bool doAll);
    void GetCurrentValues(int which_widget);
    void Apply(bool ignore = false);
  private slots:
    void opacityChanged(int opacity, const void*);
    void lightingToggled(bool on);
  private:
    int plotType;
    QvisOpacitySlider *opacity;
    QLabel *opacityLabel;
    QCheckBox *lightingToggle;

    TruecolorAttributes *atts;
};



#endif
