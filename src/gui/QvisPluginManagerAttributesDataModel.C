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
#include <QvisPluginManagerAttributesDataModel.h>
#include <PluginManagerAttributes.h>

//
// This class lets an integer represent a 1 level tree structure and it is used 
// so we can easily adapt Qt's example code for the index() and parent() methods
// in our data model.
// 
class TreeItem
{
public:
    TreeItem() : index(-1) { }
    TreeItem(int i) : index(i) { }
    TreeItem(const TreeItem &obj) : index(obj.index) { }
    TreeItem parent() const { return TreeItem(-1); }
    TreeItem child(int row) const { return TreeItem(row); }
    int row() const { return (index >= 0) ? index : 0; }
    bool exists() const { return index >= 0; }
    bool operator == (const TreeItem &obj) const { return index == obj.index; }

    int index;
};

// ****************************************************************************
// Method: QvisPluginManagerAttributesDataModel::QvisPluginManagerAttributesDataModel
//
// Purpose: 
//   Constructor
//
// Arguments:
//   p      : The plugin atts to observe.
//   pt     : The type of plugin to expose in the model.
//   parent : The parent of this object.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  9 14:29:40 PST 2010
//
// Modifications:
//   
// ****************************************************************************

QvisPluginManagerAttributesDataModel::QvisPluginManagerAttributesDataModel(
    PluginManagerAttributes *p, QvisPluginManagerAttributesDataModel::PluginType pt,
    QObject *parent) : 
    QAbstractItemModel(parent), Observer(p), pluginType(pt), pluginAtts(p)
{
}

// ****************************************************************************
// Method: QvisPluginManagerAttributesDataModel::~QvisPluginManagerAttributesDataModel
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  9 14:30:38 PST 2010
//
// Modifications:
//   
// ****************************************************************************

QvisPluginManagerAttributesDataModel::~QvisPluginManagerAttributesDataModel()
{
}

// ****************************************************************************
// Method: QvisPluginManagerAttributesDataModel::Update
//
// Purpose: 
//   This method is called when the pluginAtts are modified. We use it to reset
//   the model so the views that use it will display the new data.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  9 14:30:50 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
QvisPluginManagerAttributesDataModel::Update(Subject *)
{
    reset();
}

// ****************************************************************************
// Method: QvisPluginManagerAttributesDataModel::rowCount
//
// Purpose: 
//   Returns the number of rows for the index in the data model.
//
// Arguments:
//   index : The data model index.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  9 14:31:32 PST 2010
//
// Modifications:
//   
// ****************************************************************************

int
QvisPluginManagerAttributesDataModel::rowCount(const QModelIndex &index) const
{
    if(index.column() > 0)
        return 0;

    int rc = 0;
    if(!index.isValid())
        rc = numPlugins(); // row has numOperator rows

    return rc;
}

// ****************************************************************************
// Method: QvisPluginManagerAttributesDataModel::columnCount
//
// Purpose: 
//   Returns the number of columns that the model provides.
//
// Arguments:
//   
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  9 14:32:06 PST 2010
//
// Modifications:
//   
// ****************************************************************************

int
QvisPluginManagerAttributesDataModel::columnCount(const QModelIndex &) const
{
    return (pluginType == OperatorPlugin) ? 4 : 3;
}

// ****************************************************************************
// Method: QvisPluginManagerAttributesDataModel::index
//
// Purpose: 
//   Creates a data model index that helps represent the tree form of the data model.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  9 14:32:25 PST 2010
//
// Modifications:
//   
// ****************************************************************************

QModelIndex
QvisPluginManagerAttributesDataModel::index(int row, int column,
    const QModelIndex & parent) const
{
    if(!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem parentItem;
    if(!parent.isValid())
        parentItem = TreeItem(-1); // root
    else
        parentItem = TreeItem((int)parent.internalId());

    TreeItem childItem(parentItem.child(row));
    if(childItem.exists())
        return createIndex(row, column, (quint32)childItem.index);
    else
        return QModelIndex();
}

// ****************************************************************************
// Method: QvisPluginManagerAttributesDataModel::parent
//
// Purpose: 
//   Returns the model index of the input index's parent.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  9 14:32:57 PST 2010
//
// Modifications:
//   
// ****************************************************************************

QModelIndex
QvisPluginManagerAttributesDataModel::parent(const QModelIndex &index) const
{
    if(!index.isValid())
         return QModelIndex();

    TreeItem childItem((int)index.internalId());
    TreeItem parentItem(childItem.parent());

    if(parentItem == TreeItem(-1))
         return QModelIndex();

    return createIndex(parentItem.row(), 0, (int)parentItem.index);
}

// ****************************************************************************
// Method: QvisPluginManagerAttributesDataModel::headerData
//
// Purpose: 
//   Returns the header data for the model.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  9 14:33:21 PST 2010
//
// Modifications:
//   
// ****************************************************************************

QVariant
QvisPluginManagerAttributesDataModel::headerData(int section, 
    Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole)
    {
        if(orientation == Qt::Horizontal)
        {
            if(section==0)
                return tr("Enabled");
            else if(section==1)
                return tr("Version");
            else if(section==2)
                return tr("Name");
            else
                return tr("Category");
        }
    }  
    return QAbstractItemModel::headerData(section, orientation, role);
}

// ****************************************************************************
// Method: QvisPluginManagerAttributesDataModel::data
//
// Purpose: 
//   Returns the data for the model. We get it via the pluginAtts.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  9 14:33:38 PST 2010
//
// Modifications:
//   
// ****************************************************************************

QVariant
QvisPluginManagerAttributesDataModel::data(const QModelIndex &index, int role) const
{
    QVariant retval;

    int op = getPluginI(index.row());
    if(op > -1)
    {
        if(role == Qt::DisplayRole || role == Qt::EditRole)
        {
            if(index.column() == 1)
                retval = QVariant(QString(pluginAtts->GetVersion()[op].c_str()));
            else if(index.column() == 2)
                retval = QVariant(QString(pluginAtts->GetName()[op].c_str()));
            else if(index.column() == 3 && op < pluginAtts->GetCategory().size())
                retval = QVariant(QString(pluginAtts->GetCategory()[op].c_str()));
        }
        else if(role == Qt::CheckStateRole)
        {
            if(index.column() == 0)
                return QVariant((pluginAtts->GetEnabled()[op] > 0) ? Qt::Checked : Qt::Unchecked);
        }
    }

    return retval;
}

// ****************************************************************************
// Method: QvisPluginManagerAttributesDataModel::setData
//
// Purpose: 
//   This method sets new data from the delegate into the data model, allowing
//   us to edit the data in the model.
//
// Arguments:
//   index : The model index being edited.
//   value : The new value.
//   role  : The type of data being provided.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  9 14:34:13 PST 2010
//
// Modifications:
//   
// ****************************************************************************

bool
QvisPluginManagerAttributesDataModel::setData(const QModelIndex &index,
    const QVariant &value, int role)
{
    bool retval = false;

    if(index.isValid())
    {
        int pIndex = getPluginI(index.row());
        if(pIndex >= 0 && pIndex < pluginAtts->GetCategory().size())
        {
            // If we're editing the 3rd column, poke a new category name
            // into the pluginAtts.
            if(index.column() == 3)
            {
                pluginAtts->GetCategory()[pIndex] = value.toString().toStdString();
                pluginAtts->SelectCategory();
                emit dataChanged(index, index);
                retval = true;
            }
            else if(index.column() == 0)
            {
                pluginAtts->GetEnabled()[pIndex] = value.toBool() ? 1 : 0;
                pluginAtts->SelectEnabled();
                emit dataChanged(index, index);
                retval = true;
            }
        } 
    }

    return retval;
}

// ****************************************************************************
// Method: QvisPluginManagerAttributesDataModel::flags
//
// Purpose: 
//   Return the flags for the specified model index.
//
// Arguments:
//   index : The model index.
//
// Returns:    The index's flags.
//
// Note:       We make column 0 a check box, and column 3 is editable text.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  9 14:36:00 PST 2010
//
// Modifications:
//   
// ****************************************************************************

Qt::ItemFlags
QvisPluginManagerAttributesDataModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags f = QAbstractItemModel::flags(index);
    if(index.column() == 0)
        f |= Qt::ItemIsUserCheckable;
    else if(index.column() == 3)
        f |= Qt::ItemIsEditable;
    return f;
}

// ****************************************************************************
// Method: QvisPluginManagerAttributesDataModel::numPlugins
//
// Purpose: 
//   Return the number of plugins of the desired type.
//
// Arguments:
//
// Returns:    The number of plugins.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  9 14:36:45 PST 2010
//
// Modifications:
//   
// ****************************************************************************

int
QvisPluginManagerAttributesDataModel::numPlugins() const
{
    int c = 0;
    std::string pt((pluginType == OperatorPlugin) ? "operator" : "plot");
    const stringVector &types = pluginAtts->GetType();
    for(size_t i = 0; i < types.size(); ++i)
        if(types[i] == pt)
            c++;
    return c;
}

// ****************************************************************************
// Method: QvisPluginManagerAttributesDataModel::getPluginI
//
// Purpose: 
//   Get the i'th plugin of the desired type and return its global index in
//   the pluginAtts lists.
//
// Arguments:
//   index : The index of the desired X plugin.
//
// Returns:    The index of that plugin in the pluginAtts' global lists.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  9 14:37:12 PST 2010
//
// Modifications:
//   
// ****************************************************************************

int
QvisPluginManagerAttributesDataModel::getPluginI(int index) const
{
    int c = 0;
    std::string pt((pluginType == OperatorPlugin) ? "operator" : "plot");
    const stringVector &types = pluginAtts->GetType();
    for(int i = 0; i < types.size(); ++i)
    {
        if(types[i] == pt)
        {
            if(c == index)
                return i;
            else
                c++;
        }
    }
    return -1;
}
