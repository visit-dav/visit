#include <QvisHelpListViewItem.h>

QvisHelpListViewItem::QvisHelpListViewItem(QListView *parent, QListViewItem *after) :
    QListViewItem(parent, after), doc()
{
}

QvisHelpListViewItem::QvisHelpListViewItem(QListViewItem *parent, QListViewItem *after) :
    QListViewItem(parent, after), doc()
{
}

QvisHelpListViewItem::~QvisHelpListViewItem()
{
}

const QString &
QvisHelpListViewItem::document() const
{
    return doc;
}

void
QvisHelpListViewItem::setDocument(const QString &d)
{
    doc = d;
}
