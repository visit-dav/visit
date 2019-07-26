// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_SUBSET_PANEL_WIDGET_H
#define QVIS_SUBSET_PANEL_WIDGET_H
#include <gui_exports.h>
#include <QWidget>


class QTreeWidget;
class QTreeWidgetItem;
class QLabel;
class QPushButton;
class QToolButton;
class QMenu;
class ViewerProxy;
class QvisSubsetPanelItem;

#include <avtSILRestrictionTraverser.h>


// ****************************************************************************
// Class: QvisSubsetPanelWidget
//
// Purpose:
//   Provide a selection panel widget or the subset window.
//
// Notes:
//
// Programmer: Cyrus Harrison
// Creation:   Mon Jun 30 08:45:28 PDT 2008
//
// Notes: The guts of this class were pulled out of QvisSubsetWindow.
//
// Modifications:
//   Cyrus Harrison, Mon Mar  8 09:19:12 PST 2010
//   Switch from using two QPushButtons to a single QToolButton.
//
// ****************************************************************************

class GUI_API QvisSubsetPanelWidget : public QWidget
{
    Q_OBJECT
public:
    QvisSubsetPanelWidget(QWidget *parent, ViewerProxy *viewer_proxy);
    virtual ~QvisSubsetPanelWidget();

public slots:
    void SetTitle(const QString &title);
    void ViewSet(int id);
    void ViewCollection(int id);
    void UpdateView();
    void SetSelectedItemState(int state);
    void Clear();

signals:
    void itemSelected(int id, bool parent);
    void parentStateChanged(int value);
    void stateChanged();

protected slots:
    void onAllSetsButton();
    void onAllSetsActionReverse();
    void onAllSetsActionOn();
    void onAllSetsActionOff();

    void onSelectedSetsButton();
    void onSelectedSetsActionReverse();
    void onSelectedSetsActionOn();
    void onSelectedSetsActionOff();

    void onItemSelectionChanged();
    void onItemClicked(QTreeWidgetItem *,int);

private:
    void UpdateParentState();
    void EnableButtons(bool);
    void Reverse(bool);
    void TurnOnOff(bool,bool);

    ViewerProxy         *viewerProxy;

    QTreeWidget         *tree;
    QvisSubsetPanelItem *activeChild;
    int                  numCheckable;
    int                  numChecked;

    QLabel              *allSetsLabel;
    QToolButton         *allSetsButton;

    QLabel              *selectedSetsLabel;
    QToolButton         *selectedSetsButton;

};

#endif
