/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
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
