/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/
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
