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

#include <QvisSequenceMappingModel.h>
#include <QAbstractItemModel>
#include <QBuffer>
#include <QHeaderView>
#include <QList>
#include <QMap>
#include <QMimeData>

const int QvisSequenceMappingModel::SequenceTypeRole = 1000;
const char *QvisSequenceMappingModel::SequenceMimeType = "application/visit.sequence.item";

// ****************************************************************************
// Method: QvisSequenceMappingModel::QvisSequenceMappingModel
//
// Purpose: 
//   Constructor
//
// Arguments:
//   parent : The object's parent.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 17 16:39:06 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QvisSequenceMappingModel::QvisSequenceMappingModel(QObject *parent) : 
    QAbstractItemModel(parent), sequencesPerViewport()
{
    setSupportedDragActions(Qt::MoveAction);
}

// ****************************************************************************
// Method: QvisSequenceMappingModel::~QvisSequenceMappingModel
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 17 16:39:28 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QvisSequenceMappingModel::~QvisSequenceMappingModel()
{
}

// ****************************************************************************
// Method: QvisSequenceMappingModel::clear
//
// Purpose: 
//   Clears the data model.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 17 16:39:57 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisSequenceMappingModel::clear()
{
    sequencesPerViewport.clear();
    reset();
}

// ****************************************************************************
// Method: QvisSequenceMappingModel::addViewport
//
// Purpose: 
//   Adds a viewport to the model.
//
// Arguments:
//   viewportName : The name of the viewport to add.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 17 16:41:35 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisSequenceMappingModel::addViewport(const QString &viewportName)
{
    sequencesPerViewport[viewportName] = QSequenceDataList();
    reset();
}

// ****************************************************************************
// Method: QvisSequenceMappingModel::addSequenceToViewport
//
// Purpose: 
//   Adds a new sequence to a viewport.
//
// Arguments:
//   vpt     : The viewport name.
//   seqName : The name of the new sequence.
//   pix     : The sequence's icon.
//   seqType : The sequence's type.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 17 16:42:08 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisSequenceMappingModel::addSequenceToViewport(const QString &vpt, 
    const QString &seqName, const QPixmap &pix, int seqType)
{
    QStringQSequenceDataListMap::Iterator it;
    it = sequencesPerViewport.find(vpt);
    if(it == sequencesPerViewport.end())
    {
        sequencesPerViewport[vpt] = QSequenceDataList();
        it = sequencesPerViewport.find(vpt);
    }

    // Save info about this sequence.
    QSequenceData newSequence;
    newSequence.name = seqName;
    newSequence.pixmap = pix;
    newSequence.seqType = seqType;
    it.value().push_back(newSequence);

    reset();
}

// ****************************************************************************
// Method: QvisSequenceMappingModel::insertSequenceInViewport
//
// Purpose: 
//   Inserts a sequence into the specified viewport, which also gets added if
//   it does not exist.
//
// Arguments:
//   vpt     : The viewport.
//   index   : The location in the list where we're adding the sequence.
//   seqInfo : The sequence data that we're adding to the viewport.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 22 15:04:31 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisSequenceMappingModel::insertSequenceInViewport(const QString &vpt, int index,
    const QSequenceData &seqInfo)
{
    // Find the viewport.
    QStringQSequenceDataListMap::Iterator it;
    it = sequencesPerViewport.find(vpt);
    if(it == sequencesPerViewport.end())
    {
        sequencesPerViewport[vpt] = QSequenceDataList();
        it = sequencesPerViewport.find(vpt);
    }

    // Insert the sequence into the viewport's list at the right place.
    QSequenceDataList::Iterator pos = it.value().begin();
    for(int i = 0; i < index && i < it.value().size(); ++i)
        ++pos;
    it.value().insert(pos, seqInfo);

    reset();
}

// ****************************************************************************
// Method: QvisSequenceMappingModel::viewportContainingSequence
//
// Purpose: 
//   Return the name of the viewport containing the sequence.
//
// Arguments:
//   seqName : The sequence whose viewport we want.
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 22 15:05:49 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QString
QvisSequenceMappingModel::viewportContainingSequence(const QString &seqName) const
{
    QStringQSequenceDataListMap::const_iterator it;
    for(it = sequencesPerViewport.begin();
        it != sequencesPerViewport.end(); ++it)
    {
        QSequenceDataList::const_iterator dit;

        // Try and find the name of the viewport that
        // contains the sequence that we're looking up.
        for(dit = it.value().begin();
            dit != it.value().end(); ++dit)
        {
            if((*dit).name == seqName)
                return it.key();
        }
    }
    return QString();
}

// ****************************************************************************
// Method: QvisSequenceMappingModel::removeSequence
//
// Purpose: 
//   Removes the named sequence from the model.
//
// Arguments:
//   name : The name of the sequence to remove.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 17 16:43:42 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisSequenceMappingModel::removeSequence(const QString &name)
{
    QStringQSequenceDataListMap::Iterator it = sequencesPerViewport.begin();
    for(; it != sequencesPerViewport.end(); ++it)
    {
        // Erase the item from the list.
        QSequenceDataList::Iterator dit = it.value().begin();
        for(; dit != it.value().end(); ++dit)
        {
            if((*dit).name == name)
            {
                it.value().erase(dit);
                reset();
                return;
            }
        }
    }
}

// ****************************************************************************
// Method: QvisSequenceMappingModel::printContents
//
// Purpose: 
//   Prints the widget contents for debugging.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 12:15:13 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSequenceMappingModel::printContents() const
{
    QStringQSequenceDataListMap::const_iterator it = sequencesPerViewport.begin();
    for(; it != sequencesPerViewport.end(); ++it)
    {
        qDebug("Viewport: %s", it.key().toStdString().c_str());

        for(size_t i = 0; i < it.value().size(); ++i)
        {
            qDebug("\tsequence: %s, %d", it.value()[i].name.toStdString().c_str(),
                                         it.value()[i].seqType);
        }
    }
}

// ****************************************************************************
// Method: QvisSequenceMappingModel::indexForSequence
//
// Purpose: 
//   Returns a model index for the named sequence.
//
// Arguments:
//   seqName : The name of the sequence.
//
// Returns:    The model index that contains the sequence.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 22 11:19:53 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QModelIndex
QvisSequenceMappingModel::indexForSequence(const QString &seqName) const
{
    QList<QString> keys = sequencesPerViewport.keys();
    for(int row = 0; row < keys.size(); ++row)
    {
        const QSequenceDataList &seqs = sequencesPerViewport[keys[row]];
        for(int column = 0; column < seqs.size(); ++column)
            if(seqName == seqs[column].name)
                return createIndex(row, column);
    }
    return QModelIndex();
}

// ****************************************************************************
// Method: QvisSequenceMappingModel::QSequenceData
//
// Purpose: 
//   Returns the QSequenceData at row,column
//
// Arguments:
//   row    : The row containing the item
//   column : The column containing the item.
//
// Returns:    The sequence data.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 22 11:16:21 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QvisSequenceMappingModel::QSequenceData
QvisSequenceMappingModel::sequence(int row, int column) const
{
    QSequenceData retval;

    if(row >= 0 && row < sequencesPerViewport.size())
    {
        QList<QString> keys = sequencesPerViewport.keys();
        const QSequenceDataList &seqs = sequencesPerViewport[keys[row]];
        if(column >= 0 && column < seqs.size())
            retval = seqs[column];
    }

    return retval;
}

// ****************************************************************************
// Method: QvisSequenceMappingModel::columnCount
//
// Purpose: 
//   Returns the number of columns needed to display the model.
//
// Returns:    The number of columns.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 17 16:44:12 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

int
QvisSequenceMappingModel::columnCount(const QModelIndex &) const
{
    int cols = 1;
    QStringQSequenceDataListMap::const_iterator it;
    for(it = sequencesPerViewport.begin();
        it != sequencesPerViewport.end(); ++it)
    {
        cols = qMax(cols, it->size());
    }
    return (cols<5) ? 5 : cols;
}

// ****************************************************************************
// Method: QvisSequenceMappingModel::rowCount
//
// Purpose: 
//   Returns the number of rows needed to display the model.
//
// Returns:    Number of rows needed to display the model.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 17 16:44:51 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

int
QvisSequenceMappingModel::rowCount(const QModelIndex &) const
{
    return sequencesPerViewport.size();
}

// ****************************************************************************
// Method: QvisSequenceMappingModel::sequenceNames
//
// Purpose: 
//   Returns the sequence names in a viewport.
//
// Arguments:
//   vpt : The viewport whose names we want.
//
// Returns:    The list of names.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 22 10:17:31 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QStringList
QvisSequenceMappingModel::sequenceNames(const QString &vpt) const
{
    QStringList names;
    QStringQSequenceDataListMap::const_iterator it;
    it = sequencesPerViewport.find(vpt);
    if(it != sequencesPerViewport.end())
    {
        for(int i = 0; i < it.value().size(); ++i)
            names << it.value()[i].name;
    }
    return names;
}

// ****************************************************************************
// Method: QvisSequenceMappingModel::data
//
// Purpose: 
//   Return display data for the specified model index.
//
// Arguments:
//   index : The model index of interest.
//   role  : The type of data to be returned.
//
// Returns:    Variant data containing the data of interest.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 17 16:45:38 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QVariant
QvisSequenceMappingModel::data(const QModelIndex &index, int role) const
{
    QVariant retval;

    if(role == Qt::DisplayRole)
        retval = sequence(index.row(), index.column()).name;
    else if(role == Qt::DecorationRole)
        retval = sequence(index.row(), index.column()).pixmap;
    else if(role == SequenceTypeRole)
        retval = sequence(index.row(), index.column()).seqType;
    return retval;
}

// ****************************************************************************
// Method: QvisSequenceMappingModel::headerData
//
// Purpose: 
//   Returns the names of the viewports in the left column header.
//
// Arguments:
//   section     : The row
//   orientation : which header we're interested in.
//   role        : The type of data we want.
//
// Returns:    The header data.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 17 16:46:36 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QVariant
QvisSequenceMappingModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole)
    {
        if(orientation == Qt::Vertical)
        {
            if(section >= 0 && section < sequencesPerViewport.size())
            {
                QList<QString> keys = sequencesPerViewport.keys();
                return QVariant(keys[section]);
            }
        }
    }

    return QAbstractItemModel::headerData(section, orientation, role);
}

// ****************************************************************************
// Method: QvisSequenceMappingModel::index
//
// Purpose: 
//   Returns the model index for row,column
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 17 16:47:59 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QModelIndex
QvisSequenceMappingModel::index(int row, int column, const QModelIndex &) const
{
    return createIndex(row, column);
}

QModelIndex
QvisSequenceMappingModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

// ****************************************************************************
// Method: QvisSequenceMappingModel::flags
//
// Purpose: 
//   We use this to tell the view that the item is drag&drop capable.
//
// Returns:    The item flags.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 22 16:41:38 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

Qt::ItemFlags
QvisSequenceMappingModel::flags(const QModelIndex &) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

// ****************************************************************************
// Method: QvisSequenceMappingModel::mimeTypes
//
// Purpose: 
//   Returns the mime types that this model produces.
//
// Returns:    The mime types that this model produces.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 22 16:42:17 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QStringList
QvisSequenceMappingModel::mimeTypes() const
{
    QStringList types;
    types << SequenceMimeType;
    return types;
}

// ****************************************************************************
// Method: QvisSequenceMappingModel::supportedDropActions
//
// Purpose: 
//   Tells the view that only move actions are supported.
//
// Returns:    The supported drop actions.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 22 16:42:46 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

Qt::DropActions
QvisSequenceMappingModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

// ****************************************************************************
// Method: QvisSequenceMappingModel::mimeData
//
// Purpose: 
//   Encodes the model indices into QMimeData for the drag&drop operation.
//
// Arguments:
//   indices : The list of model indices participating in the drag.
//
// Returns:    Mime data containing model indices for dragging.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 22 14:59:53 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QMimeData *
QvisSequenceMappingModel::mimeData(const QModelIndexList &indices) const
{
    QMimeData *mimeData = new QMimeData;
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    foreach(QModelIndex index, indices)
    {
        if(index.isValid())
            stream << index.row() << index.column();
    }
    mimeData->setData(SequenceMimeType, encodedData);
    return mimeData;
}

// ****************************************************************************
// Method: QvisSequenceMappingModel::dropMimeData
//
// Purpose: 
//   This method lets the model accept drop data from itself, which lets us
//   move data between cells in the model.
//
// Arguments:
//   data       : The mime data that contains the index being moved.
//   action     : The drop action
//   row,column : invalid args
//   dest       : The destination cell in the model.
//
// Returns:    True if the data gets moved.
//
// Note:       We emit a signal here that we can use from the sequence view.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 22 14:57:24 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

bool
QvisSequenceMappingModel::dropMimeData(const QMimeData *data, Qt::DropAction action, 
    int row, int column, const QModelIndex &dest)
{
    if(action == Qt::IgnoreAction)
        return true;

    if(!data->hasFormat(SequenceMimeType))
        return false;

    // Add the sequence back in at the new location.
    bool retval = false;
    QString vpt(headerData(dest.row(), Qt::Vertical, Qt::DisplayRole).toString());
    if(sequencesPerViewport.find(vpt) != sequencesPerViewport.end())
    {
        QByteArray encodedData = data->data(SequenceMimeType);
        QDataStream stream(&encodedData, QIODevice::ReadOnly);
        int oldRow, oldColumn;
        stream >> oldRow >> oldColumn;

        // Get the data at the old index.
        QSequenceData source(sequence(oldRow, oldColumn));
        QString fromViewport(viewportContainingSequence(source.name));

        // Remove the sequence from the view and add it back in.
        removeSequence(source.name);
        insertSequenceInViewport(vpt, dest.column(), source);

        // Tell the client about the changes to the sequence lists.
        if(oldRow == dest.row())
        {
            // Only one viewport list was involved.
            emit updatedMapping(vpt, sequenceNames(vpt));
        }
        else
        {
            // Two viewport lists were involved.
            emit updatedMapping(fromViewport,
                                sequenceNames(fromViewport),
                                vpt,
                                sequenceNames(vpt));
        }
        retval = true;
    }

    return retval;
}
