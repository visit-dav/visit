// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_SUBSET_WINDOW_H
#define QVIS_SUBSET_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowSimpleObserver.h>
#include <vector>

// Forward declarations.
class QComboBox;
class QFrame;
class QGridLayout;
class QLabel;
class QTreeView;
class QTreeViewItem;
class QPushButton;
class QMenu;
class QScrollArea;
class QSplitter;
class QvisSubsetPanelWidget;
class QvisSubsetPanelItem;

class avtSIL;
class SILRestrictionAttributes;
class SelectionList;
class PlotList;

// ****************************************************************************
// Class: QvisSubsetWindow
//
// Purpose:
//   This class contains the code for the subset window.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:06:29 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Aug 21 16:00:21 PST 2001
//   Made the window postable.
//
//   Brad Whitlock, Tue Sep 25 14:30:35 PST 2001
//   Added post(), unpost(), specialResize().
//
//   Brad Whitlock, Fri Feb 8 15:52:11 PST 2002
//   Modified to use a splitter.
//
//   Brad Whitlock, Mon Mar 4 15:12:38 PST 2002
//   Added autoUpdate support.
//
//   Brad Whitlock, Wed Apr 23 14:14:33 PST 2003
//   I changed the interface of ItemClicked.
//
//   Brad Whitlock, Fri Aug 6 13:56:01 PST 2004
//   Made changes that allow multiple sets to be selected.
//
//   Brad Whitlock, Wed Apr  9 11:04:31 PDT 2008
//   QString for caption, shortName.
//
//   Cyrus Harrison, Fri Jul 18 09:02:29 PDT 2008
//   Refactored for Qt4.
//
//   Brad Whitlock, Tue Aug 10 11:04:31 PDT 2010
//   I made it inherit QvisPostableWindowSimpleObserver.
//
// ****************************************************************************

class GUI_API QvisSubsetWindow : public QvisPostableWindowSimpleObserver
{
    Q_OBJECT
public:
    QvisSubsetWindow(const QString &caption = QString(),
                     const QString &shortName = QString(),
                     QvisNotepadArea *notepad = 0);
    virtual ~QvisSubsetWindow();
    virtual void CreateWindowContents();

    void ConnectSILRestrictionAttributes(SILRestrictionAttributes *);
    void ConnectSelectionList(SelectionList *);
    void ConnectPlotList(PlotList *);
    virtual void SubjectRemoved(Subject *);
protected:
    virtual void UpdateWindow(bool doAll);
    
    void Apply(bool ignore = false);
    int  AddPanel(bool visible = true);
    void UpdatePanels(int index = -1, bool panels_after = false);
    void ClearPanelsToTheRight(int index);
    void UpdateSILControls();
    void UpdateSelectionControls();
private slots:
    void apply();
    void onPanelItemSelected(int id,bool parent);
    void onPanelStateChanged();
    void selectionChanged(const QString &);

private:
    int FindPanelIndex(QObject *obj);

    SILRestrictionAttributes       *silrAtts;
    SelectionList                  *selectionList;
    PlotList                       *plotList;

    QScrollArea                    *scrollView;
    QSplitter                      *panelSplitter;
    QList<QvisSubsetPanelWidget *>  panels;

    QLabel                         *selectionLabel;
    QComboBox                      *selections;

    // A few attributes of the current SIL restriction that we use to
    // determine how the window must be redrawn.
    int                  sil_TopSet;
    int                  sil_NumSets;
    int                  sil_NumCollections;
    int                  sil_dirty;

    // We use this to help determine when we must apply a new selection.
    QString              sel_selectionName;
    bool                 sel_dirty;
};

#endif
