// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_FILE_PANEL_ITEM_H
#define QVIS_FILE_PANEL_ITEM_H
#include <gui_exports.h>
#include <QTreeWidget>
#include <QualifiedFilename.h>

// ****************************************************************************
// Class: QvisFilePanelItem
//
// Purpose:
//   This is a tree view item that contains a QualifiedFilename object.
//   It is used in the file panel's selected file list.
//
// Notes:      
//
// Programmer: Cyrus Harrison
// Creation:   Tue Jul  1 14:05:53 PDT 2008
//
// Notes: Qt4 port - adapated from Brad Whitlock's QvisListViewFileItem class.
//
// Modifications:
//
// ****************************************************************************

class GUI_API QvisFilePanelItem : public QTreeWidgetItem
{
public:
    static const int ROOT_NODE;
    static const int HOST_NODE;
    static const int DIRECTORY_NODE;
    static const int FILE_NODE;

    QvisFilePanelItem(QTreeWidget *parent, const QString &str,
                      const QualifiedFilename &qf,
                      int nodeType = 3, int state = -1,
                      bool tsForced = false);
                         
    QvisFilePanelItem(QTreeWidgetItem *parent, const QString &str,
                      const QualifiedFilename &qf, 
                      int nodeType = 3, int state = -1,
                      bool tsForced = false);

    virtual ~QvisFilePanelItem();

    /*
    TODO
    virtual void paintCell(QPainter *p, const QColorGroup &cg,
                           int column, int width, int align);

    virtual QString key(int column, bool ascending) const;
    */

    // Convenience functions
    bool isRoot() const { return (nodeType == ROOT_NODE); };
    bool isHost() const { return (nodeType == HOST_NODE); };
    bool isDirectory() const { return (nodeType == DIRECTORY_NODE); };
    bool isFile() const { return (nodeType == FILE_NODE); };
    
    static void resetNodeNumber();

    // public data members
    QualifiedFilename file;
    int               nodeType;
    int               timeState;
    bool              timeStateHasBeenForced;

private:
    static int        globalNodeNumber;
    int               nodeNumber;
};

#endif
