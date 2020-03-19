// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_SEQUENCE_VIEW_H
#define QVIS_SEQUENCE_VIEW_H

#include <QTableView>

// ****************************************************************************
// Class: QvisSequenceView
//
// Purpose:
//   A widget for viewing sequences.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 4 12:33:24 PDT 2006
//
// Modifications:
//   Brad Whitlock, Fri Oct 17 14:27:20 PDT 2008
//   Total rewrite for Qt 4.
//
// ****************************************************************************

class QvisSequenceView : public QTableView
{
    Q_OBJECT
public:
    QvisSequenceView(QWidget *parent);
    virtual ~QvisSequenceView();

    void clear();

    void addViewport(const QString &);
    void addSequenceToViewport(const QString &vpt, const QString &seqName, int seqType = 0);
    void addSequenceToViewport(const QString &vpt, const QString &seqName,
                               const QPixmap &pix, int seqType = 0);
    void removeSequence(const QString &name);

    bool getSequenceInViewport(const QString &vpName, int index,
                               QString &, int &) const;
    void selectSequence(const QString &name);

signals:
    void updatedMapping(const QString &vp1,
                        const QStringList &seqList1);
    void updatedMapping(const QString &vp1,
                        const QStringList &seqList1,
                        const QString &vp2,
                        const QStringList &seqList2);
};

#endif
