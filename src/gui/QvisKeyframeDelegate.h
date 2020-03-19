// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_KEYFRAME_DELEGATE_H
#define QVIS_KEYFRAME_DELEGATE_H
#include <QAbstractItemDelegate>

class QvisKeyframeDrawer;

// ****************************************************************************
// Class: QvisKeyframeDelegate
//
// Purpose:
//   Delegate that lets users interact with various keyframing data in the
//   Keyframing window's treeview. The delegate is responsible for repsenting
//   the keyframe data to the user graphically and for instantiating interaction
//   widgets that let the user modify the data.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 11:43:00 PST 2008
//
// Modifications:
//   
// ****************************************************************************

class QvisKeyframeDelegate : public QAbstractItemDelegate
{
    QvisKeyframeDrawer *drawer;
public:
    QvisKeyframeDelegate(QObject *parent);
    virtual ~QvisKeyframeDelegate();

    virtual void paint(QPainter *p, 
                       const QStyleOptionViewItem &option, 
                       const QModelIndex &index) const;

    virtual QSize sizeHint(const QStyleOptionViewItem &option, 
                           const QModelIndex &index) const;

    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;
    virtual void updateEditorGeometry(QWidget *editor, 
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
private:
    int delegateType(const QModelIndex &index) const;
};

#endif
