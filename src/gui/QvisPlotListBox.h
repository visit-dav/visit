#ifndef QVIS_PLOT_LIST_BOX_H
#define QVIS_PLOT_LIST_BOX_H
#include <gui_exports.h>
#include <qlistbox.h>

class PlotList;

// ****************************************************************************
// Class: QvisPlotListBox
//
// Purpose:
//   This has the same functionality as QListBox except that its
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
// ****************************************************************************

class GUI_API QvisPlotListBox : public QListBox
{
    Q_OBJECT
public:
    QvisPlotListBox(QWidget *parent = 0, const char *name = 0, WFlags f=0);
    virtual ~QvisPlotListBox();

    bool isExpanded(int) const;
    int  activeOperatorIndex(int) const;

    bool NeedsUpdated(const PlotList *) const;
signals:
    void activateSubsetWindow();
    void activatePlotWindow(int plotType);
    void activateOperatorWindow(int operatorType);
    void promoteOperator(int operatorIndex);
    void demoteOperator(int operatorIndex);
    void removeOperator(int operatorIndex);
protected:
    virtual void viewportMousePressEvent(QMouseEvent *e);
    virtual void viewportMouseDoubleClickEvent(QMouseEvent *e);
    void clickHandler(const QPoint &p, bool);
    void itemClicked(QListBoxItem *item, const QPoint &point, bool);
};

#endif
