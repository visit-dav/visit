/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
#include <KeyframeDataModel.h>
#include <KeyframeData.h>

#include <KeyframeAttributes.h>
#include <Plot.h>
#include <PlotList.h>
#include <PlotPluginInfo.h>
#include <PlotPluginManager.h>
#include <QualifiedFilename.h>
#include <ViewerProxy.h>
#include <WindowInformation.h>

// ****************************************************************************
// Class: RowInfo
//
// Purpose:
//   Contains data for a row in the data model's tree.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 13:38:46 PST 2008
//
// Modifications:
//   
// ****************************************************************************

class RowInfo
{
public:
    RowInfo(const QString &n, int p=-1, int dt=KeyframeDataModel::NothingDelegate, int i=-1) :
        _name(n), _parent(p), _id(i), _delegateType(dt)
    {
    }

    QString name() const             { return _name;   }
    int     parent() const           { return _parent; }
    virtual QIcon icon() const       { return QIcon(); }
    virtual int delegateType() const { return _delegateType; }
    virtual int id() const           { return _id; }
private:
    QString _name; 
    int     _parent;
    int     _id;
    int     _delegateType;
};

// ****************************************************************************
// Class: PlotRowInfo
//
// Purpose:
//   Derived class that also has a plot's icon.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 13:38:52 PST 2008
//
// Modifications:
//   Cyrus Harrison, Tue Nov 11 10:43:31 PST 2008
//   Removed default value for arugment p, b/c having it was making gcc cry
//   about not having default arugments for plotIndex & i.
//
// ****************************************************************************

class PlotRowInfo : public RowInfo
{
public:
    PlotRowInfo(const QString &n, int p, int plotIndex, const QIcon &i) :
        RowInfo(n,p,KeyframeDataModel::PlotDelegate,plotIndex), _icon(i)
    { }

    virtual QIcon icon() const { return _icon; }
private:
    QIcon _icon;
};


const char *KeyframeDataModel::KF_TIME_SLIDER = "Keyframe animation";

// ****************************************************************************
// Method: KeyframeDataModel::KeyframeDataModel
//
// Purpose: 
//   Constructor.
//
// Arguments:
//   parent : The object's parent.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:33:37 PST 2008
//
// Modifications:
//   
// ****************************************************************************

KeyframeDataModel::KeyframeDataModel(QObject *parent) : QAbstractItemModel(parent),
    rowInfo()
{
    kfAtts = 0;
    plotList = 0;
    windowInfo = 0;
}

// ****************************************************************************
// Method: KeyframeDataModel::~KeyframeDataModel
//
// Purpose: 
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:33:59 PST 2008
//
// Modifications:
//   
// ****************************************************************************

KeyframeDataModel::~KeyframeDataModel()
{
    for(int i = 0; i < rowInfo.size(); ++i)
        delete rowInfo[i];

    if(kfAtts != 0)
        kfAtts->Detach(this);

    if(plotList != 0)
        plotList->Detach(this);

    if(windowInfo != 0)
        windowInfo->Detach(this);
}

// ****************************************************************************
// Method: Connect*Attributes
//
// Purpose: 
//   Connect attributes that we observe to the model.
//
// Arguments:
//   atts : The state object to observe.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:34:13 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
KeyframeDataModel::ConnectKeyframeAttributes(KeyframeAttributes *atts)
{
    kfAtts = atts;
    kfAtts->Attach(this);
    Update(kfAtts);
}

void
KeyframeDataModel::ConnectPlotList(PlotList *atts)
{
    plotList = atts;
    plotList->Attach(this);
    Update(plotList);
}

void
KeyframeDataModel::ConnectWindowInformation(WindowInformation *atts)
{
    windowInfo = atts;
    windowInfo->Attach(this);
    Update(windowInfo);
}

// ****************************************************************************
// Method: KeyframeDataModel::SubjectRemoved
//
// Purpose: 
//   Called to detach a state object when it is deleted.
//
// Arguments:
//   subj : The state object that was deleted.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:34:50 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
KeyframeDataModel::SubjectRemoved(Subject *subj)
{
    if(subj == kfAtts)
        kfAtts = 0;
    if(subj == plotList)
        plotList = 0;
    if(subj == windowInfo)
        windowInfo = 0;
}

// ****************************************************************************
// Method: KeyframeDataModel::Update
//
// Purpose: 
//   This method updates the model when a state object changes.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:35:29 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
KeyframeDataModel::Update(Subject *)
{
    RebuildRowInfo();
    reset();
}

// ****************************************************************************
// Method: KeyframeDataModel::GetUniqueDatabases
//
// Purpose: 
//   Gets the list of unique databases in the plot list.
//
// Returns:    The list of databases in the plot list.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:35:56 PST 2008
//
// Modifications:
//   
// ****************************************************************************

QStringList
KeyframeDataModel::GetUniqueDatabases() const
{
    QStringList dbs;
    if(plotList != 0)
    {
        for(int i = 0; i < plotList->GetNumPlots(); ++i)
        {
            const Plot &current = plotList->GetPlots(i);
            QString db(current.GetDatabaseName().c_str());
            if(dbs.indexOf(db) == -1)
                dbs.append(db);
        }
    }
    return dbs;
}

// ****************************************************************************
// Method: KeyframeDataModel::GetPlotsForDatabase
//
// Purpose: 
//   Gets the plot indices that use the specified database.
//
// Arguments:
//   db : The database whose plots we want to obtain.
//
// Returns:    The list of plot indices that use the db.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:36:24 PST 2008
//
// Modifications:
//   
// ****************************************************************************

QList<int>
KeyframeDataModel::GetPlotsForDatabase(const QString &db) const
{
    QList<int> plots;
    if(plotList != 0)
    {
        for(int i = 0; i < plotList->GetNumPlots(); ++i)
        {
            const Plot &current = plotList->GetPlots(i);
            QString plotdb(current.GetDatabaseName().c_str());
            if(db == plotdb)
                plots.append(i);
        }
    }
    return plots;
}

// ****************************************************************************
// Method: KeyframeDataModel::RebuildRowInfo
//
// Purpose: 
//   Constructs the tree that makes up the data model.
//
// Note:       "State" turned off for now.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:37:20 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
KeyframeDataModel::RebuildRowInfo()
{
    for(int i = 0; i < rowInfo.size(); ++i)
        delete rowInfo[i];
    rowInfo.clear();

    rowInfo << new RowInfo(tr("Current frame"),-1, CurrentFrameDelegate);
    rowInfo << new RowInfo(tr("View"),-1, ViewDelegate);
    QStringList uniqueDBs(GetUniqueDatabases());
    for(int i = 0; i < uniqueDBs.size(); ++i)
    {
        int dbRow = rowInfo.size();
        QualifiedFilename f(uniqueDBs[i].toStdString());
        rowInfo << new RowInfo(f.filename.c_str(),-1);
//        rowInfo << new RowInfo(tr("State"), dbRow, StateDelegate, i);

        QList<int> plotsForDatabase(GetPlotsForDatabase(uniqueDBs[i]));
        for(int p = 0; p < plotsForDatabase.size(); ++p)
        {
            int plotRow = rowInfo.size();
            const Plot &current = plotList->GetPlots(plotsForDatabase[p]);

            QString plotName(PlotName(current.GetPlotType()));
            plotName += QString("(");
            plotName += current.GetPlotVar().c_str();
            plotName += QString(")");

            rowInfo << new PlotRowInfo(plotName, dbRow, plotsForDatabase[p],
                                       PlotIcon(current.GetPlotType()));
            rowInfo << new RowInfo(tr("State"), plotRow, PlotStateDelegate, plotsForDatabase[p]);
            rowInfo << new RowInfo(tr("Attributes"), plotRow, PlotAttsDelegate, plotsForDatabase[p]);
        }
    }
}

// ****************************************************************************
// Method: KeyframeDataModel::PlotName
//
// Purpose: 
//   Return the name of the plot type.
//
// Arguments:
//   plotType : A plot type.
//
// Returns:    The name of the plot type.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:37:54 PST 2008
//
// Modifications:
//   
// ****************************************************************************

QString
KeyframeDataModel::PlotName(int plotType) const
{
    QString retval;
    PlotPluginManager *plotPluginManager = GetViewerProxy()->GetPlotPluginManager();
    GUIPlotPluginInfo *GUIInfo = plotPluginManager->GetGUIPluginInfo(
        plotPluginManager->GetEnabledID(plotType));
    if(GUIInfo != 0)
    {
        QString *menuName = GUIInfo->GetMenuName();       
        retval = *menuName;
        delete menuName;
    }
    return retval;
}

// ****************************************************************************
// Method: KeyframeDataModel::PlotIcon
//
// Purpose: 
//   Returns the plot icon for the plot type, if there is one.
//
// Arguments:
//   plotType : A plot type.
//
// Returns:    The plot's icon.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:38:25 PST 2008
//
// Modifications:
//   
// ****************************************************************************

QIcon
KeyframeDataModel::PlotIcon(int plotType) const
{
    QIcon retval;
    PlotPluginManager *plotPluginManager = GetViewerProxy()->GetPlotPluginManager();
    GUIPlotPluginInfo *GUIInfo = plotPluginManager->GetGUIPluginInfo(
        plotPluginManager->GetEnabledID(plotType));
    if(GUIInfo != 0 && GUIInfo->XPMIconData() != 0)
        retval = QIcon(GUIInfo->XPMIconData());
    return retval;
}

// ****************************************************************************
// Method: KeyframeDataModel::rowCount
//
// Purpose: 
//   Returns the row count for the model index.
//
// Arguments:
//   index : A model index.
//
// Returns:    The row count for the model index.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:39:00 PST 2008
//
// Modifications:
//   
// ****************************************************************************

int
KeyframeDataModel::rowCount(const QModelIndex &index) const
{
    int parentIndex = -1;
    if(index.isValid())
        parentIndex = index.internalId();
    int c = 0;
    for(int i = 0; i < rowInfo.size(); ++i)
    {
        if(rowInfo[i]->parent() == parentIndex)
        {
            ++c;
        }
    }
    return c;
}

// ****************************************************************************
// Method: KeyframeDataModel::columnCount
//
// Purpose: 
//   Returns the column count for the model index.
//
// Arguments:
//
// Returns:    The column count.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:39:31 PST 2008
//
// Modifications:
//   
// ****************************************************************************

int
KeyframeDataModel::columnCount(const QModelIndex &) const
{
    return 2;
}

// ****************************************************************************
// Method: KeyframeDataModel::headerData
//
// Purpose: 
//   Returns the names of the header columns.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:40:04 PST 2008
//
// Modifications:
//   
// ****************************************************************************

QVariant
KeyframeDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole)
    {
        if(orientation == Qt::Horizontal)
            return (section==0) ? tr("Attributes") : tr("Keyframes");
    }

    return QAbstractItemModel::headerData(section, orientation, role);
}

// ****************************************************************************
// Method: KeyframeDataModel::currentIndex
//
// Purpose: 
//   Returns the time state of the keyframe time slider.
//
// Returns:    The time state of the keyframe time slider.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:40:36 PST 2008
//
// Modifications:
//   
// ****************************************************************************

int
KeyframeDataModel::currentIndex() const
{
    int curIndex = -1;
    for(int i = 0; i < windowInfo->GetTimeSliders().size(); ++i)
    {
        if(windowInfo->GetTimeSliders()[i] == KF_TIME_SLIDER)
        {
            curIndex = windowInfo->GetTimeSliderCurrentStates()[i];
            break;
        }
    }
    return curIndex;
}

// ****************************************************************************
// Method: KeyframeDataModel::data
//
// Purpose: 
//   Returns data for the specified model index.
//
// Arguments:
//   index : The model index whose data we want.
//   role  : The role of the data for the model index.
//
// Returns:    The model index's data.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:41:20 PST 2008
//
// Modifications:
//   
// ****************************************************************************

QVariant
KeyframeDataModel::data(const QModelIndex &index, int role) const
{
    QVariant retval;

    if(role == Qt::DisplayRole)
    {
        int id = (int)index.internalId();
        if(index.column() == 0)
        {
            if(id >= 0 && id < rowInfo.size())
                retval = rowInfo[id]->name();
        }
        else if(index.column() == 1)
        {
            if(rowInfo[id]->delegateType() == PlotDelegate)
            {
                PlotRangeData s;
                s.id = rowInfo[id]->id();
                s.numFrames = kfAtts->GetNFrames();
                s.currentIndex = currentIndex();

                if(s.id >= 0 && s.id < plotList->GetNumPlots())
                {
                    s.start = plotList->GetPlots(s.id).GetBeginFrame();
                    s.end = plotList->GetPlots(s.id).GetEndFrame();
                    retval = qVariantFromValue(s);
                }
            }
            else
            {
                KeyframePoints s;
                s.id = rowInfo[id]->id();
                s.numFrames = kfAtts->GetNFrames();
                s.currentIndex = currentIndex();

                if(rowInfo[id]->delegateType() == StateDelegate)
                {
                    // What's this one again?
                }
                else if(rowInfo[id]->delegateType() == ViewDelegate)
                {
                    const intVector &kf = windowInfo->GetViewKeyframes();
                    for(size_t i = 0; i < kf.size(); ++i)
                        s.idToIndex[kf[i]] = kf[i];
                    retval = qVariantFromValue(s);
                }
                else if(rowInfo[id]->delegateType() == PlotAttsDelegate)
                {
                    if(s.id >= 0 && s.id < plotList->GetNumPlots())
                    {
                        const intVector &kf = plotList->GetPlots(s.id).GetKeyframes();
                        for(size_t i = 0; i < kf.size(); ++i)
                            s.idToIndex[kf[i]] = kf[i];
                        retval = qVariantFromValue(s);
                    }
                }
                else if(rowInfo[id]->delegateType() == PlotStateDelegate)
                {
                    if(s.id >= 0 && s.id < plotList->GetNumPlots())
                    {
                        const intVector &kf = plotList->GetPlots(s.id).GetDatabaseKeyframes();
                        for(size_t i = 0; i < kf.size(); ++i)
                            s.idToIndex[kf[i]] = kf[i];
                        retval = qVariantFromValue(s);
                    }
                }
                else if(rowInfo[id]->delegateType() == CurrentFrameDelegate)
                {
                    s.idToIndex[0] = currentIndex();
                    retval = qVariantFromValue(s);
                }
            }
        }
    }
    else if(role == Qt::DecorationRole)
    {
        int id = (int)index.internalId();
        if(id >= 0 && id < rowInfo.size())
            retval = rowInfo[id]->icon();
    }
    else if(role == NumFramesRole)
    {
        retval = kfAtts->GetNFrames();
    }
    else if(role == CurrentIndexRole)
    {
        retval = currentIndex();
    }
    else if(role == DelegateTypeRole)
    {
        int id = (int)index.internalId();
        if(id >= 0 && id < rowInfo.size())
            retval = rowInfo[id]->delegateType();
    }
    else if(role == GetIdRole)
    {
        int id = (int)index.internalId();
        retval = rowInfo[id]->id();
    }

    return retval;
}

// ****************************************************************************
// Method: KeyframeDataModel::setData
//
// Purpose: 
//   Sets data for the mode index into the data model.
//
// Arguments:
//   index : The model index that will get new data.
//   value : The variant value of the new data.
//   role  : The role of the new data we're replacing.
//
// Returns:    True on success; False on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:42:14 PST 2008
//
// Modifications:
//   
// ****************************************************************************

bool
KeyframeDataModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    bool retval = false;
    if (qVariantCanConvert<PlotRangeData>(value))
    {
        PlotRangeData s = value.value<PlotRangeData>();
        GetViewerMethods()->SetPlotFrameRange(s.id,
                                              qMin(s.start, s.end),
                                              qMax(s.start, s.end));
        retval = true;
    }
    else if (qVariantCanConvert<KeyframePoints>(value))
    {
        KeyframePoints s = value.value<KeyframePoints>();
        int id = (int)index.internalId();
        if(rowInfo[id]->delegateType() == StateDelegate)
        {
            // What's this one again?
        }
        else if(rowInfo[id]->delegateType() == ViewDelegate)
        {
            for(QMap<int,int>::const_iterator it = s.idToIndex.begin();
                it != s.idToIndex.end(); ++it)
            {
                if(it.value() == -1)
                    GetViewerMethods()->DeleteViewKeyframe(it.key());
                else if(it.key() != it.value())
                    GetViewerMethods()->MoveViewKeyframe(it.key(), it.value());
            }
            retval = true;
        }
        else if(rowInfo[id]->delegateType() == PlotAttsDelegate)
        {
            for(QMap<int,int>::const_iterator it = s.idToIndex.begin();
                it != s.idToIndex.end(); ++it)
            {
                if(it.value() == -1)
                    GetViewerMethods()->DeletePlotKeyframe(s.id, it.key());
                else if(it.key() != it.value())
                    GetViewerMethods()->MovePlotKeyframe(s.id, it.key(), it.value());
            }
            retval = true;
        }
        else if(rowInfo[id]->delegateType() == PlotStateDelegate)
        {
            for(QMap<int,int>::const_iterator it = s.idToIndex.begin();
                it != s.idToIndex.end(); ++it)
            {
                if(it.value() == -1)
                    GetViewerMethods()->DeletePlotDatabaseKeyframe(s.id, it.key());
                else if(it.key() != it.value())
                    GetViewerMethods()->MovePlotDatabaseKeyframe(s.id, it.key(), it.value());
            }
            retval = true;
        }
        else if(rowInfo[id]->delegateType() == CurrentFrameDelegate)
        {
            for(QMap<int,int>::const_iterator it = s.idToIndex.begin();
                it != s.idToIndex.end(); ++it)
            {
                //
                // Set the active time slider to be the keyframing time slider 
                // if that's not the currently active time slider.
                //
                if((windowInfo->GetActiveTimeSlider() >= 0) &&
                   (windowInfo->GetTimeSliders()[windowInfo->GetActiveTimeSlider()] !=
                    KF_TIME_SLIDER))
                {
                    GetViewerMethods()->SetActiveTimeSlider(KF_TIME_SLIDER);
                }

                // Set the active time slider to the kf time slider???
                GetViewerMethods()->SetTimeSliderState(it.value());
                break;
            }
            retval = true;
        }
    }

    return retval;
}

// ****************************************************************************
// Method: KeyframeDataModel::GetIthChild
//
// Purpose: 
//   Get the i'th child of the specified parent node.
//
// Arguments:
//   parentIndex : The parent index.
//   child       : The i'th child of the parent index.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:43:30 PST 2008
//
// Modifications:
//   
// ****************************************************************************

int
KeyframeDataModel::GetIthChild(int parentIndex, int child) const
{
    // Look for children in the list that have a parent of parentIndex.
    int c = 0;
    for(int i = 0; i < rowInfo.size(); ++i)
    {
        if(rowInfo[i]->parent() == parentIndex)
        {
            if(c == child)
                return i;
            ++c;
        }
    }
    return -1;
}

// ****************************************************************************
// Method: KeyframeDataModel::GetChild
//
// Purpose: 
//   Return the parent's child
//
// Arguments:
//   parentIndex : The parent index.
//   child       : The i'th child of the parent index.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:44:23 PST 2008
//
// Modifications:
//   
// ****************************************************************************

int
KeyframeDataModel::GetChild(int parentIndex, int child) const
{
    // Look for children in the list that have a parent of parentIndex.
    int c = 0;
    for(int i = 0; i < rowInfo.size(); ++i)
    {
        if(rowInfo[i]->parent() == parentIndex)
        {
            if(i == child)
                return c;
            ++c;
        }
    }
    return -1;
}

// ****************************************************************************
// Method: KeyframeDataModel::GetParent
//
// Purpose: 
//   Returns a child's parent.
//
// Arguments:
//   childIndex : The index of a child.
//
// Returns:    The child's parent.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:45:08 PST 2008
//
// Modifications:
//   
// ****************************************************************************

int
KeyframeDataModel::GetParent(int childIndex) const
{
    if(childIndex >= 0 && childIndex < rowInfo.size())
       return rowInfo[childIndex]->parent();
    return -1;
}

// ****************************************************************************
// Method: KeyframeDataModel::index
//
// Purpose: 
//   Creates an index for the row,col having the specified parent.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:45:44 PST 2008
//
// Modifications:
//   
// ****************************************************************************

QModelIndex
KeyframeDataModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    int parentIndex = -1;
    if (parent.isValid())
        parentIndex = (int)parent.internalId(); // index into rowInfo

    int childIndex = GetIthChild(parentIndex, row);
    if (childIndex != -1)
        return createIndex(row, column, (quint32)childIndex);

    return QModelIndex();
}

// ****************************************************************************
// Method: KeyframeDataModel::parent
//
// Purpose: 
//   Returns the model index of the specified model index.
//
// Arguments:
//   index : The model index whose parent we want.
//
// Returns:    The model index of the parent.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:46:09 PST 2008
//
// Modifications:
//   
// ****************************************************************************

QModelIndex
KeyframeDataModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    int childIndex = (int)index.internalId();
    int parentIndex = GetParent(childIndex);

    if (parentIndex == -1)
        return QModelIndex();

    // Determine the parent's row within its parent.
    int pp = GetParent(parentIndex);
    int row = GetChild(pp, parentIndex);

    return createIndex(row, 0, parentIndex);
}

// ****************************************************************************
// Method: KeyframeDataModel::flags
//
// Purpose: 
//   The edit flags for the model index.
//
// Arguments:
//   index : The model index.
//
// Returns:    The edit flags.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:46:48 PST 2008
//
// Modifications:
//   
// ****************************************************************************

Qt::ItemFlags
KeyframeDataModel::flags(const QModelIndex & index) const
{
    Qt::ItemFlags f = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if(index.column()==1)
        f |= Qt::ItemIsEditable;
    return f;
}

