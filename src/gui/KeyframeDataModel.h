// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef KEYFRAME_DATA_MODEL_H
#define KEYFRAME_DATA_MODEL_H
#include <QAbstractItemModel>
#include <SimpleObserver.h>
#include <GUIBase.h>

#include <QIcon>
#include <QList>

class KeyframeAttributes;
class PlotList;
class WindowInformation;

class RowInfo; // for internal use only

// ****************************************************************************
// Class: KeyframeDataModel
//
// Purpose:
//   This is a special observer that exposes various VisIt state objects 
//   as tree-type model that can be viewed in a QTreeView using special 
//   delegates.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 11:54:43 PST 2008
//
// Modifications:
//   Eric Brugger, Wed Mar 22 16:23:12 PDT 2023
//   Added operator keyframing.
//   
// ****************************************************************************

class KeyframeDataModel : public QAbstractItemModel, public SimpleObserver, public GUIBase
{
public:
    KeyframeDataModel(QObject *parent);
    virtual ~KeyframeDataModel();

    void ConnectKeyframeAttributes(KeyframeAttributes *atts);
    void ConnectPlotList(PlotList *atts);
    void ConnectWindowInformation(WindowInformation *atts);

    // SimpleObserver methods
    virtual void SubjectRemoved(Subject *);
    virtual void Update(Subject *);

    // QAbstractItemModel methods
    virtual int rowCount(const QModelIndex &) const;
    virtual int columnCount(const QModelIndex &) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    virtual QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual Qt::ItemFlags flags (const QModelIndex & index) const;

    enum {
        NumFramesRole = 1000,
        GetIdRole,
        DelegateTypeRole,
        CurrentIndexRole
    };

    enum {
        NothingDelegate, 
        CurrentFrameDelegate,
        ViewDelegate,
        StateDelegate,
        PlotDelegate,
        PlotAttsDelegate,
        PlotStateDelegate,
        PlotOperatorDelegate
    };

    static const char *KF_TIME_SLIDER;
private:
    void RebuildRowInfo();
    QStringList GetUniqueDatabases() const;
    QList<int>  GetPlotsForDatabase(const QString &db) const;
    int GetChild(int parentIndex, int child) const;
    int GetIthChild(int parentIndex, int child) const;
    int GetParent(int childIndex) const;
    QString PlotName(int plotType) const;
    QIcon   PlotIcon(int plotType) const;

    int currentIndex() const;

    QList<RowInfo*>     rowInfo;

    KeyframeAttributes *kfAtts;
    PlotList           *plotList;
    WindowInformation  *windowInfo;
};

#endif
