#ifndef QVIS_LISTVIEW_FILE_ITEM_H
#define QVIS_LISTVIEW_FILE_ITEM_H
#include <gui_exports.h>
#include <qlistview.h>
#include <QualifiedFilename.h>

// ****************************************************************************
// Class: QvisListViewFileItem
//
// Purpose:
//   This is a listviewitem that contains a QualifiedFilename object.
//   It is used in the file panel's selected file list.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 13 12:15:46 PDT 2000
//
// Modifications:
//   Brad Whitlock, Wed Mar 21 00:36:23 PDT 2001
//   Added an argument to the constructor and added some convenience
//   functions.
//
// ****************************************************************************

class GUI_API QvisListViewFileItem : public QListViewItem
{
public:
    static const int ROOT_NODE;
    static const int HOST_NODE;
    static const int DIRECTORY_NODE;
    static const int FILE_NODE;

    QvisListViewFileItem(QListView *parent, const QString &str,
        const QualifiedFilename &qf, int nodeType = 3, int state = -1);
    QvisListViewFileItem(QListViewItem *parent, const QString &str,
        const QualifiedFilename &qf, int nodeType = 3, int state = -1);
    virtual ~QvisListViewFileItem();

    virtual void paintCell(QPainter *p, const QColorGroup &cg,
                           int column, int width, int align);

    virtual QString key(int column, bool ascending) const;

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
private:
    static int        globalNodeNumber;
    int               nodeNumber;
};

#endif
