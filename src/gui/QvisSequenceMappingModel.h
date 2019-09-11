// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_SEQUENCE_MAPPING_MODEL_H
#define QVIS_SEQUENCE_MAPPING_MODEL_H
#include <QAbstractItemModel>
#include <QList>
#include <QMap>
#include <QPixmap>

// ****************************************************************************
// Class: QvisSequenceMappingModel
//
// Purpose:
//   This data model contains the sequence to viewport mapping and provides
//   accessor methods so the table view can display the data.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 17 16:38:06 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

class QvisSequenceMappingModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    struct QSequenceData
    {
        QSequenceData() : name(), pixmap() { seqType=-1; }

        QString name;
        QPixmap pixmap;
        int     seqType;
    };

    QvisSequenceMappingModel(QObject *parent=0);
    virtual ~QvisSequenceMappingModel();

    //
    // Methods for manipulating the model.
    //
    Qt::DropActions supportedDragActions() const;
    void clear();
    void addViewport(const QString &viewportName);
    void addSequenceToViewport(const QString &vpt, 
                               const QString &seqName, const QPixmap &pix, int seqType);
    void insertSequenceInViewport(const QString &vpt, int index,
                                  const QSequenceData &seqInfo);
    void removeSequence(const QString &seqName);
    QString viewportContainingSequence(const QString &seqName) const;
    QModelIndex indexForSequence(const QString &seqName) const;

    //
    // Methods that implement QAbstractItemModel
    //
    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;
    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;

    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex & index) const;

    virtual Qt::ItemFlags flags(const QModelIndex &) const;
    virtual QStringList mimeTypes() const;
    virtual Qt::DropActions supportedDropActions() const;

    virtual QMimeData *mimeData(const QModelIndexList & indexes) const;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);

    static const int   SequenceTypeRole;
    static const char *SequenceMimeType;
signals:
    void updatedMapping(const QString &vp1,
                        const QStringList &seqList1);
    void updatedMapping(const QString &vp1,
                        const QStringList &seqList1,
                        const QString &vp2,
                        const QStringList &seqList2);
private:
    QSequenceData sequence(int row, int col) const;
    QStringList   sequenceNames(const QString &vpt) const;
    void printContents() const;

    typedef QList<QSequenceData>             QSequenceDataList;
    typedef QMap<QString, QSequenceDataList> QStringQSequenceDataListMap;

    QStringQSequenceDataListMap sequencesPerViewport;
};

#endif
