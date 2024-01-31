// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISINTERACTORWINDOW_H
#define QVISINTERACTORWINDOW_H

#include <AttributeSubject.h>
#include <QvisPostableWindowObserver.h>

class InteractorAttributes;
class QCheckBox;
class QButtonGroup;

// ****************************************************************************
// Class: QvisInteractorWindow
//
// Purpose: 
//   Defines QvisInteractorWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Mon Aug 16 15:29:28 PST 2004
//
// Modifications:
//   Eric Brugger, Thu Nov 11 11:47:28 PST 2004
//   I added the navigation mode toggle buttons.
//   
//   Eric Brugger, Thu Nov 18 13:01:35 PST 2004
//   I added the fill viewport on zoom toggle button.
//   
//   Jeremy Meredith, Thu Feb  7 17:51:32 EST 2008
//   Added snap-to-horizontal grid support for axis array mode navigation.
//
//   Brad Whitlock, Wed Apr  9 11:31:21 PDT 2008
//   QString for caption, shortName.
//
//   Hank Childs, Sat Mar 13 19:03:03 PST 2010
//   Add bounding box mode.
//
// ****************************************************************************

class QvisInteractorWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
  public:
    QvisInteractorWindow(
                         InteractorAttributes *subj,
                         const QString &caption = QString(),
                         const QString &shortName = QString(),
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisInteractorWindow();
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
    void showGuidelinesChanged(bool val);
    void clampSquareChanged(bool val);
    void fillViewportOnZoomChanged(bool val);
    void navigationModeChanged(int val);
    void boundingBoxModeChanged(int val);
    void axisSnapChanged(bool val);
  private:
    QCheckBox *showGuidelines;
    QCheckBox *clampSquare;
    QCheckBox *fillViewportOnZoom;
    QButtonGroup *navigationMode;
    QButtonGroup *boundingBoxMode;
    QCheckBox *axisSnap;

    InteractorAttributes *atts;
};



#endif
