// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_SUBSET_PANEL_ITEM_H
#define QVIS_SUBSET_PANEL_ITEM_H
#include <gui_exports.h>

#include <QTreeWidget>


typedef enum {NotChecked, PartiallyChecked, CompletelyChecked, Unset} CheckedState;


// ****************************************************************************
// Class: QvisSubsetPanelItem
//
// Purpose:
//   This class is a tree viewitem that has a tri-state checkbox and 
//   contains an integer that represents a set or collection id.
//
// Notes:
//  Adapted from the QvisSubsetListViewItem class.
//
// Programmer: Cyrus Harrison
// Creation:   June 26, 2008
//
// Modifications:
//  Cyrus Harrison, Wed Dec  1 16:24:41 PST 2010
//  Change critera for isOn to FullyChecked.
//
// ****************************************************************************

class QvisSubsetPanelItem: public QTreeWidgetItem
{
public:
    QvisSubsetPanelItem(QTreeWidget *parent,
                        const QString &text,
                        CheckedState s = NotChecked,
                        int id = 0);

    QvisSubsetPanelItem(QTreeWidgetItem *parent,
                        const QString &text,
                        int id = 0);

    virtual ~QvisSubsetPanelItem();

    void         setCheckable(bool val);
    bool         getCheckable() const;

    void         setState(CheckedState s);
    CheckedState getState() const;

    void         toggleState();

    int          id() const;
    void         setId(int id);
    bool         isOn() const { return state == CompletelyChecked; }

private:
    static void  InitIcons();
    static bool  iconsReady;
    static QIcon iconChecked;
    static QIcon iconMixed;
    static QIcon iconUnchecked;


    CheckedState state;
    int          itemId;
    bool         checkable;
};

#endif
