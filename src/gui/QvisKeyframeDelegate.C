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
#include <QvisKeyframeDelegate.h>
#include <QPainter>

#include <QvisKeyframeWidget.h>
#include <QvisKeyframePlotRangeWidget.h>
#include <QvisKeyframeDrawer.h>

#include <KeyframeDataModel.h>
#include <KeyframeData.h>

// ****************************************************************************
// Method: QvisKeyframeDelegate::QvisKeyframeDelegate
//
// Purpose: 
//   Constructor.
//
// Arguments:
//   parent : The object's parent.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:14:20 PST 2008
//
// Modifications:
//   
// ****************************************************************************

QvisKeyframeDelegate::QvisKeyframeDelegate(QObject *parent) : QAbstractItemDelegate(parent)
{
    drawer = new QvisKeyframeDrawer(this);
}

// ****************************************************************************
// Method: QvisKeyframeDelegate::~QvisKeyframeDelegate
//
// Purpose: 
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:14:39 PST 2008
//
// Modifications:
//   
// ****************************************************************************

QvisKeyframeDelegate::~QvisKeyframeDelegate()
{
}

// ****************************************************************************
// Method: QvisKeyframeDelegate::paint
//
// Purpose: 
//   This method paints the delegate. We get a little data from the model 
//   using the model index and use that to call drawing methods on our
//   QvisKeyframeDrawer object that lets us share drawing code between the
//   delegate and the widgets.
//
// Arguments:
//   p      : The painter to use.
//   option : The style options to use.
//   index  : The model index of the data that we're drawing.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:15:56 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisKeyframeDelegate::paint(QPainter *p, const QStyleOptionViewItem &option, 
    const QModelIndex &index) const
{
    if (qVariantCanConvert<PlotRangeData>(index.data()))
    {
        PlotRangeData s = index.data().value<PlotRangeData>();
        drawer->setCurrentIndex(s.currentIndex);
        drawer->setNumFrames(s.numFrames);
        drawer->drawPlotRange(p, option.rect, s.start, s.end);
    }
    else if(qVariantCanConvert<KeyframePoints>(index.data()))
    {
        KeyframePoints s = index.data().value<KeyframePoints>();

        // Make a list of the valid points.
        QList<int> pts;
        for(QMap<int,int>::const_iterator it = s.idToIndex.begin();
            it != s.idToIndex.end(); ++it)
            if(it.value() != -1)
                pts.append(it.value());

        bool diamond = delegateType(index) == KeyframeDataModel::PlotAttsDelegate;
        drawer->setCurrentIndex(s.currentIndex);
        drawer->setNumFrames(s.numFrames);
        if(delegateType(index) == KeyframeDataModel::CurrentFrameDelegate)
            drawer->drawTimeHandle(p, option.rect, s.currentIndex);
        else 
        {
            bool diamond = delegateType(index) == KeyframeDataModel::PlotAttsDelegate;
            drawer->drawPoints(p, option.rect, pts, diamond);
        }
    }
    else
    {
        int currentIndex = index.data(KeyframeDataModel::CurrentIndexRole).toInt();
        int numFrames = index.data(KeyframeDataModel::NumFramesRole).toInt();
        drawer->setCurrentIndex(currentIndex);
        drawer->setNumFrames(numFrames);
        drawer->drawBackground(p, option.rect);
    }
}

// ****************************************************************************
// Method: QvisKeyframeDelegate::sizeHint
//
// Purpose: 
//   Returns a size hint.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:17:56 PST 2008
//
// Modifications:
//   
// ****************************************************************************

QSize
QvisKeyframeDelegate::sizeHint(const QStyleOptionViewItem &option, 
    const QModelIndex &index) const
{
    return QSize(200,20);
}

// ****************************************************************************
// Method: QvisKeyframeDelegate::createEditor
//
// Purpose: 
//   Creates an editor widget for the specified model index.
//
// Arguments:
//   parent : The widget parent.
//   option : The options to use.
//   index  : The model index containing the data that needs an editor.
//
// Returns:    A new widget or 0.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:18:16 PST 2008
//
// Modifications:
//   
// ****************************************************************************

QWidget *
QvisKeyframeDelegate::createEditor(QWidget *parent, 
    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QWidget *w = 0;
    if (qVariantCanConvert<PlotRangeData>(index.data()))
    {
        QvisKeyframePlotRangeWidget *edit = new QvisKeyframePlotRangeWidget(parent);
        edit->setDrawer(drawer);
        connect(edit, SIGNAL(commit(QWidget *)),
                this, SIGNAL(commitData(QWidget *)));
        w = edit;
    }
    else if(qVariantCanConvert<KeyframePoints>(index.data()))
    {
        QvisKeyframeWidget *edit = new QvisKeyframeWidget(parent);
        edit->setDrawer(drawer);
        if(delegateType(index) == KeyframeDataModel::PlotAttsDelegate)
            edit->setPointStyle(QvisKeyframeWidget::Point_Diamond);
        else if(delegateType(index) == KeyframeDataModel::CurrentFrameDelegate)
            edit->setPointStyle(QvisKeyframeWidget::Point_Time);
        connect(edit, SIGNAL(commit(QWidget *)),
                this, SIGNAL(commitData(QWidget *)));
        w = edit;
    }
    return w;
}

// ****************************************************************************
// Method: QvisKeyframeDelegate::updateEditorGeometry
//
// Purpose: 
//   This method resizes the editor widget to the size of the delegate.
//
// Arguments:
//   editor : The editor widget.
//   option : The options to use (includes rect)
//   index  : The model index.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:19:19 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisKeyframeDelegate::updateEditorGeometry(QWidget *editor, 
    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

// ****************************************************************************
// Method: QvisKeyframeDelegate::setEditorData
//
// Purpose: 
//   This method data from the data model into the editor widget.
//
// Arguments:
//   editor : The editor widget.
//   index  : The model index that will provide the data.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:20:11 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisKeyframeDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (qVariantCanConvert<PlotRangeData>(index.data()))
    {
        PlotRangeData s = index.data().value<PlotRangeData>();
        QvisKeyframePlotRangeWidget *edit = (QvisKeyframePlotRangeWidget *)editor;
        drawer->setCurrentIndex(s.currentIndex);
        drawer->setNumFrames(s.numFrames);
        edit->setData(s.start, s.end);
    }
    else if(qVariantCanConvert<KeyframePoints>(index.data()))
    {
        KeyframePoints s = index.data().value<KeyframePoints>();
        QvisKeyframeWidget *edit = (QvisKeyframeWidget *)editor;
        drawer->setCurrentIndex(s.currentIndex);
        drawer->setNumFrames(s.numFrames);
        edit->setData(s.idToIndex);
    }
}

// ****************************************************************************
// Method: QvisKeyframeDelegate::setModelData
//
// Purpose: 
//   This method puts widget data back into the data model.
//
// Arguments:
//   editor : The editor widget.
//   model  : The model that will accept the data.
//   index  : The model index that will accept the new data from the widget.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:21:33 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisKeyframeDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, 
    const QModelIndex &index) const
{
    if (qVariantCanConvert<PlotRangeData>(index.data()))
    {
        PlotRangeData s(index.data().value<PlotRangeData>());
        QvisKeyframePlotRangeWidget *edit = (QvisKeyframePlotRangeWidget *)editor;
        edit->getData(s.start, s.end);
        model->setData(index, qVariantFromValue(s));
    }
    else if (qVariantCanConvert<KeyframePoints>(index.data()))
    {
        KeyframePoints s(index.data().value<KeyframePoints>());
        QvisKeyframeWidget *edit = (QvisKeyframeWidget *)editor;
        edit->getData(s.idToIndex);
        model->setData(index, qVariantFromValue(s));
    }
}

// ****************************************************************************
// Method: QvisKeyframeDelegate::delegateType
//
// Purpose: 
//   Returns the delegate type for the specified model index.
//
// Arguments:
//   index : The model index.
//
// Returns:    The delegate type for the specified model index.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:22:41 PST 2008
//
// Modifications:
//   
// ****************************************************************************

int
QvisKeyframeDelegate::delegateType(const QModelIndex &index) const
{
    return index.data(KeyframeDataModel::DelegateTypeRole).toInt(); 
}
