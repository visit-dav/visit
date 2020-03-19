// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_PLOT_LIST_BOX_H
#define QVIS_PLOT_LIST_BOX_H
#include <gui_exports.h>
#include <QAction>
#include <QListWidget>
#include <QMenu>
#include <QMouseEvent>
#include <vectortypes.h>
#include <GUIBase.h>

class PlotList;

// ****************************************************************************
// Class: QvisPlotListBox
//
// Purpose:
//   This has the same functionality as QListWidget except that its
//   doubleClicked signal also sends the position of where the mouse
//   was clicked.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 11 11:25:17 PDT 2000
//
// Notes:
//   This class can only contain QvisPlotListBoxItem objects because it
//   needs that class's extanded functionality in order to function correctly.
//
// Modifications:
//   Brad Whitlock, Tue Apr 8 11:09:55 PDT 2003
//   I renamed the class to QvisPlotListBox and made enhancements that
//   enable operators to be moved within a plot.
//
//   Brad Whitlock, Fri Dec 5 16:22:48 PST 2003
//   I added NeedToUpdateSelection.
//
//   Brad Whitlock, Wed Jul 28 17:33:21 PST 2004
//   I added prefixes to arguments for NeedsToBeRegenerated.
//
//   Ellen Tarwate, Fri, May 25
//   adding Context Menu to the Active Plots list
//
//   Brad Whitlock, Tue Nov 20 14:37:05 PST 2007
//   Removed itemClicked.
//
//   Brad Whitlock, Fri May 30 15:43:50 PDT 2008
//   Qt 4.
//
//   Brad Whitlock, Tue Oct 20 15:16:39 PDT 2009
//   I added signals for manipulating the plot list.
//
//   Brad Whitlock, Fri Jul 23 15:34:25 PDT 2010
//   I added support for selections.
//
//   Brad Whitlock, Tue Mar 29 11:29:00 PDT 2011
//   I renamed disconnectThisPlot to followTimeSliderThisPlot.
//
//   Brad Whitlock, Thu Mar 14 16:22:08 PDT 2013
//   I added setApplyOperators.
//
//   Brad Whitlock, Fri Sep 13 12:17:40 PDT 2013
//   I added animateAct.
//
// ****************************************************************************

class GUI_API QvisPlotListBox : public QListWidget
{
    Q_OBJECT
public:
    QvisPlotListBox(QWidget *parent = 0);
    virtual ~QvisPlotListBox();

    bool isExpanded(int) const;
    int  activeOperatorIndex(int) const;
    void setApplyOperators(bool);

    bool NeedsToBeRegenerated(const PlotList *, 
                              const stringVector &prefixes, 
                              const stringVector &createdSelections) const;
    bool NeedToUpdateSelection(const PlotList *) const;

    void triggerPlotRename(int, const QString &);
    bool IsSelecting() const;

signals:
    void itemExpansionChanged();
    void activateSubsetWindow();
    void activateSelectionsWindow(const QString &);
    void activatePlotWindow(int plotType);
    void activateOperatorWindow(int operatorType);
    void promoteOperator(int operatorIndex);
    void demoteOperator(int operatorIndex);
    void removeOperator(int operatorIndex);

    void hideThisPlot();
    void deleteThisPlot();
    void drawThisPlot();
    void clearThisPlot();
    void cloneThisPlot();
    void copyToWinThisPlot();
    void redrawThisPlot();
    void followTimeSliderThisPlot(bool);
    void animateThisPlot(bool);
    void setActivePlot();

    void renamePlot(int row, const QString &);
    void makeThisPlotFirst();
    void moveThisPlotTowardFirst();
    void moveThisPlotTowardLast();
    void makeThisPlotLast();
private slots:
    void setPlotDescription();
protected:
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseDoubleClickEvent(QMouseEvent *e);
    void clickHandler(const QPoint &p, bool, bool, Qt::KeyboardModifiers);
    void contextMenuEvent( QContextMenuEvent *event );
private:
    void contextMenuCreateActions();
public:
    void contextMenuCreate();
    
private:
    QMenu      *plotContextMenu;
    QAction    *hideShowAct;
    QAction    *deleteAct;
    QAction    *drawAct;
    QAction    *clearAct;
    QAction    *cloneAct;
    QAction    *copyToWinAct;
    QAction    *redrawAct;
    QAction    *followTimeSliderAct;
    QAction    *setPlotDescriptionAct;
    QAction    *moveThisPlotTowardFirstAct;
    QAction    *moveThisPlotTowardLastAct;
    QAction    *makeThisPlotFirstAct;
    QAction    *makeThisPlotLastAct;
    QAction    *animateAct;

    QMenu      *copyWinSubMenu;
    QAction    *win1Act;
    QAction    *win2Act;
};

#endif
