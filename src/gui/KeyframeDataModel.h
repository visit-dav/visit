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
        PlotStateDelegate
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
