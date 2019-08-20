// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PLUGIN_MANAGER_ATTRIBUTES_DATA_MODEL_H
#define PLUGIN_MANAGER_ATTRIBUTES_DATA_MODEL_H

#include <QAbstractItemModel>
#include <Observer.h>

class PluginManagerAttributes;

// ****************************************************************************
// Class: QvisPluginManagerAttributesDataModel
//
// Purpose:
//   This class exposes PluginManagerAttributes as a tree-like data model that
//   we can use with QTreeView.
//
// Notes:      This class exists primarily so we can easily edit the operator 
//             category in the plugin atts directly and still control which
//             fields in the tree are editable.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  9 11:39:20 PST 2010
//
// Modifications:
//   
// ****************************************************************************

class QvisPluginManagerAttributesDataModel : public QAbstractItemModel, public Observer
{
    Q_OBJECT
public:
    typedef enum
    {
        PlotPlugin,
        OperatorPlugin
    } PluginType;

    QvisPluginManagerAttributesDataModel(PluginManagerAttributes *p, PluginType pt, 
        QObject *parent);
    virtual ~QvisPluginManagerAttributesDataModel();

    virtual void Update(Subject *);

    // Implement QAbstractDataModel
    virtual int rowCount(const QModelIndex &index) const;
    virtual int columnCount(const QModelIndex &) const;
    virtual QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    virtual Qt::ItemFlags flags (const QModelIndex &index) const;
private:
    int numPlugins() const;
    int getPluginI(int index) const;

    PluginType               pluginType;
    PluginManagerAttributes *pluginAtts;
};

#endif
