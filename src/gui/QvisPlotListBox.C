/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

#include <QMessageBox>
#include <QTimer>

#include <QvisPlotListBox.h>
#include <QvisPlotListBoxItem.h>
#include <PlotList.h>
#include <Plot.h>
#include <QvisPlotListBoxItem.h>
#include <GUIBase.h>
#include <QvisPlotManagerWidget.h>

#include <QItemDelegate>
#include <QLineEdit>
#include <QPainter>
#include <QvisPlotListBoxItem.h>

// ****************************************************************************
// Class: QPlotDelegate
//
// Purpose:
//   This delegate serves as a bridge to QvisPlotListBoxItem and we have it
//   so key height and paint methods on the item get called.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 30 14:49:08 PDT 2008
//
// Modifications:
//   Brad Whitlock, Thu Oct 22 11:06:53 PDT 2009
//   I added methods that let the user edit the plot name with a line edit.
//
//   Brad Whitlock, Tue Apr 20 11:46:16 PDT 2010
//   I added an event filter to prevent crashes when pressing Esc to work
//   around a bug in Qt.
//
// ****************************************************************************

class QPlotDelegate : public QItemDelegate
{
public:
    QPlotDelegate(QObject *parent) : QItemDelegate(parent)
    {
    }

    virtual ~QPlotDelegate()
    {
    }

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, 
                       const QModelIndex &index) const
    {
        qulonglong addr = index.data(Qt::UserRole).toULongLong();
        QvisPlotListBoxItem *currentItem = (QvisPlotListBoxItem*)addr;

        if(currentItem != 0)
        {
            QRect r(currentItem->listWidget()->visualItemRect(currentItem));
            painter->save();
            painter->translate(QPoint(r.x(), r.y()));
            currentItem->paint(painter);
            painter->restore();
        }
    }

    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        qulonglong addr = index.data(Qt::UserRole).toULongLong();
        QvisPlotListBoxItem *currentItem = (QvisPlotListBoxItem*)addr;

        QSize size(QItemDelegate::sizeHint(option, index));
        if(currentItem != 0)
        {
            int h = currentItem->height(currentItem->listWidget());
            size.setHeight(h);
        }

        return size;
    }

    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QLineEdit *lineEdit = new QLineEdit(parent);
        return lineEdit;
    }

    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        if(!index.isValid())
            return;

        QLineEdit *lineEdit = (QLineEdit *)editor;
        lineEdit->setText(index.data().toString());
        lineEdit->selectAll();
    }

    virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        if(!index.isValid())
            return;

        qulonglong addr = index.data(Qt::UserRole).toULongLong();
        QvisPlotListBoxItem *currentItem = (QvisPlotListBoxItem*)addr;
        if(currentItem != 0)
        {
            QRect r(currentItem->listWidget()->visualItemRect(currentItem));
            int x = currentItem->textX();
            editor->move(QPoint(x, r.y()));
            editor->resize(QSize(r.width() - x, editor->height()));
        }
        editor->show();
        editor->setFocus();
    }

    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
    {
        if(!index.isValid())
            return;

        qulonglong addr = index.data(Qt::UserRole).toULongLong();
        QvisPlotListBoxItem *currentItem = (QvisPlotListBoxItem*)addr;
        if(currentItem != 0)
        {
            QLineEdit *lineEdit = (QLineEdit *)editor;
            QString s(lineEdit->text());
            currentItem->listWidget()->closePersistentEditor(currentItem);

            QvisPlotListBox *lb = (QvisPlotListBox *)currentItem->listWidget();
            lb->triggerPlotRename(lb->row(currentItem), s);
        }
    }

    virtual bool eventFilter(QObject *object, QEvent *event)
    {
        QWidget *editor = qobject_cast<QWidget*>(object);
        if (!editor)
            return false;
        if (event->type() == QEvent::KeyPress)
        {
            bool retval = true;
            switch (static_cast<QKeyEvent *>(event)->key())
            {
            case Qt::Key_Escape:
                emit closeEditor(editor, QAbstractItemDelegate::RevertModelCache);
                break;
            default:
                retval = QItemDelegate::eventFilter(object, event);
            }

            return retval;
        }

        return QItemDelegate::eventFilter(object, event);
    }
};

// ****************************************************************************
// Method: QvisPlotListBox::QvisPlotListBox
//
// Purpose: 
//   This is the constructor for the QvisPlotListBox class.
//
// Arguments:
//   parent : The widget's parent.
//   name   : The widget's name.
//   f      : window flags.
//
// Notes:
//   This is a QListWidget widget with a new version of the doubleClicked
//   signal that can also send the point that was doubled-clicked.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 11 09:57:03 PDT 2000
//
// Modifications:
//   Brad Whitlock, Tue Apr 8 16:07:17 PST 2003
//   I connected this widget's clicked signal to its new itemClicked slot.
//
//   Ellen Tarwater, Mon, June25, 2007
//   Added in the context menu set-up.
//
//   Brad Whitlock, Fri May 30 15:44:43 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QvisPlotListBox::QvisPlotListBox(QWidget *parent) : QListWidget(parent),
                                                    plotContextMenu(0)
{
    contextMenuCreateActions();
    contextMenuCreate();
    setItemDelegate(new QPlotDelegate(this));
}

// ****************************************************************************
// Method: QvisPlotListBox::~QvisPlotListBox
//
// Purpose: 
//   This is the destructor for the QvisPlotListBox class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 11 09:58:53 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

QvisPlotListBox::~QvisPlotListBox()
{
    // nothing
}

// ****************************************************************************
// Method: QvisPlotListBox::mousePressEvent
//
// Purpose: 
//   This event handler is called when the user releases the mouse in the
//   viewport area of the list box. We use the position of the mouse click
//   to determine whether any special areas of the listbox item were clicked.
//
// Arguments:
//   e : The mouse event.
//
// Programmer: Brad Whitlock
// Creation:   Wed Apr 9 12:52:59 PDT 2003
//
// Modifications:
//   Brad Whitlock, Fri May 30 16:05:33 PDT 2008
//   Qt 4.
//
//   Brad Whitlock, Fri Apr 23 14:31:43 PDT 2010
//   I changed the code to work better with extended selection.
//
// ****************************************************************************

void
QvisPlotListBox::mousePressEvent(QMouseEvent *e)
{
    clickHandler(e->pos(), e->button() == Qt::RightButton, false, e->modifiers());
    QListWidget::mousePressEvent(e);
}

// ****************************************************************************
// Method: QvisPlotListBox::mouseDoubleClickEvent
//
// Purpose: 
//   This is an internal method of QListWidget that was re-implemented
//   for this class.
//
// Arguments:
//   e : The mouse event.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 11 09:59:58 PDT 2000
//
// Modifications:
//   Brad Whitlock, Fri Apr 23 14:31:43 PDT 2010
//   I changed the code to work better with extended selection.
//   
// ****************************************************************************

void
QvisPlotListBox::mouseDoubleClickEvent(QMouseEvent *e)
{
    QPoint p = e->pos();
    clickHandler(p, e->button() == Qt::RightButton, true, Qt::NoModifier);
    QListWidget::mouseDoubleClickEvent (e);
}

// ****************************************************************************
// Method: QvisPlotListBox::clickHandler
//
// Purpose: 
//   Handles clicks for us.
//
// Arguments:
//   clickLocation : The location of the click.
//   doubleClicked : Whether or not we're handling a double click.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 16:16:32 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Nov 20 14:34:44 PST 2007
//   Fixed plot expansion.
//
//   Brad Whitlock, Fri May 30 15:57:35 PDT 2008
//   Qt 4. Emit itemExpansionChanged so we can record the item expansion in the
//   plot list. Also call reset() to invalidate the model so the items get
//   drawn properly (e.g. the background gets cleared).
//
//   Sean Ahern, Thu Aug 21 14:32:54 EDT 2008
//   Fixed deleting operators on non-selected plots.
//
//   Cyrus Harrison, Thu Apr 15 08:45:34 PDT 2010
//   Proper offset calc for non drawn items.
//
//   Brad Whitlock, Fri Apr 23 14:31:43 PDT 2010
//   I changed the code to work better with extended selection.
//
// ****************************************************************************

void
QvisPlotListBox::clickHandler(const QPoint &clickLocation, bool rightClick,
    bool doubleClicked, Qt::KeyboardModifiers modifiers)
{
    QPoint itemClickLocation(clickLocation);
    int action = -1, opId = -1;
    bool bs = signalsBlocked();
    bool emitted = true;

    // Walk through all of the items, checking if we've clicked in each one.
    for (size_t i = 0; i < count(); ++i)
    {
        QListWidgetItem *current = item(i);
        QvisPlotListBoxItem *item2 = (QvisPlotListBoxItem *)current;
        int y = visualItemRect(current).y();
        int h = visualItemRect(current).height();
        if (clickLocation.y() >= y && clickLocation.y() < (y + h))
        {
            if(!rightClick &&
               (modifiers & (Qt::ControlModifier | Qt::MetaModifier)) > 0)
            {
                // If we're using Ctrl or Meta + mouse click then switch
                // the item to its reverse selection.
                blockSignals(false);
                current->setSelected(!current->isSelected());
                blockSignals(bs);
            }
            else
            {
                // If the item is not selected, select it.
                blockSignals(false);
                current->setSelected(true);
                blockSignals(bs);

                // Reduce the y location of the click location to be local             // item.
                // to the item
                itemClickLocation.setY(clickLocation.y() - y);

                // Handle the click.
                if (action == -1)
                {
                    action = item2->clicked(itemClickLocation, doubleClicked, 
                                            opId);
                }
            }
        }
        else if(rightClick || doubleClicked)
        {
            blockSignals(false);
            current->setSelected(false);
            blockSignals(bs);
        }
    }

    switch(action)
    {
    case 0: // expand clicked
        emit itemExpansionChanged();
        reset();
        break;
    case 1: // subset clicked
         emit activateSubsetWindow();
        break;
    case 2: // plot clicked
         emit activatePlotWindow(opId);
        break;
    case 3: // operator clicked
         emit activateOperatorWindow(opId);
        break;
    case 4: // promote clicked
         emit promoteOperator(opId);
        break;
    case 5: // demote clicked
        emit demoteOperator(opId);
        break;
    case 6: // delete clicked
         emit removeOperator(opId);
        break;
    default:
        if(rightClick)
        {
            emit itemSelectionChanged();
        }
        else
            emitted = false;
        break;
    }
}

// ****************************************************************************
// Method: QvisPlotListBox::isExpanded
//
// Purpose: 
//   Returns whether or not the id'th item is expanded.
//
// Arguments:
//   id : The index of the item that we want to check.
//
// Returns:    True if the item is expanded; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 13:35:13 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool
QvisPlotListBox::isExpanded(int id) const
{
   bool retval = false;

   if(id >= 0 && id < count())
   {
       QvisPlotListBoxItem *obj = (QvisPlotListBoxItem *)item(id);
       retval = obj ? obj->isExpanded() : false;
   }

   return retval;
}

// ****************************************************************************
// Method: QvisPlotListBox::activeOperatorIndex
//
// Purpose: 
//   Returns the id'th item's active operator.
//
// Arguments:
//   id : The index of the item that we want to check.
//
// Returns:    The active operator for the id'th item.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 13:35:13 PST 2003
//
// Modifications:
//   
// ****************************************************************************

int
QvisPlotListBox::activeOperatorIndex(int id) const
{
   int retval = -1;

   if(id >= 0 && id < count())
   {
       QvisPlotListBoxItem *obj = (QvisPlotListBoxItem *)item(id);
       retval = obj ? obj->activeOperatorIndex() : -1;
   }

   return retval;
}

// ****************************************************************************
// Method: QvisPlotListBox::NeedsToBeRegenerated
//
// Purpose: 
//   Determines whether or not the plot listbox needs to be updated.
//
// Arguments:
//   pl : The plot list object that we observe.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Apr 16 13:18:27 PST 2003
//
// Modifications:
//   Brad Whitlock, Fri Dec 5 16:20:07 PST 2003
//   I changed the method so having a different Active flag is not cause
//   to regenerate the items. I also renamed the method.
//
//   Brad Whitlock, Wed Jul 28 17:39:36 PST 2004
//   I made it check the file prefixes.
//
//   Brad Whitlock, Tue Oct 20 15:13:07 PDT 2009
//   Check plot descriptions.
//
// ****************************************************************************

bool
QvisPlotListBox::NeedsToBeRegenerated(const PlotList *pl,
    const stringVector &prefixes) const
{
    bool retval = true;

    if(pl->GetNumPlots() == count() && prefixes.size() == count())
    {
        for(int i = 0; i < pl->GetNumPlots(); ++i)
        {
            QvisPlotListBoxItem *lbi = (QvisPlotListBoxItem *)item(i);
            const Plot &newPlot = pl->operator[](i);
            const Plot &currentPlot = lbi->GetPlot();

            // See if the prefixes are different.
            if(prefixes[i] != std::string(lbi->GetPrefix().toStdString()))
                 return true;

            // See if the plots are different
            bool nu = newPlot.GetStateType() != currentPlot.GetStateType() ||
                   newPlot.GetPlotType() != currentPlot.GetPlotType() ||
                   newPlot.GetHiddenFlag() != currentPlot.GetHiddenFlag() ||
                   newPlot.GetExpandedFlag() != currentPlot.GetExpandedFlag() ||
                   newPlot.GetActiveOperator() != currentPlot.GetActiveOperator() ||
                   newPlot.GetPlotVar() != currentPlot.GetPlotVar() ||
                   newPlot.GetDatabaseName() != currentPlot.GetDatabaseName() ||
                   newPlot.GetOperators() != currentPlot.GetOperators() ||
                   newPlot.GetDescription() != currentPlot.GetDescription();

            if(nu) return true;
        }
        return false;
    }

    return retval;
}

// ****************************************************************************
// Method: QvisPlotListBox::NeedToUpdateSelection
//
// Purpose: 
//   Determines if the selection needs to be updated.
//
// Arguments:
//   pl : The new plot list to compare to the one that we have.
//
// Returns:    True if the selection needs to be updated; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 5 16:20:58 PST 2003
//
// Modifications:
//   Brad Whitlock, Fri Feb 23 14:17:32 PST 2007
//   Changed code so it checks all plots and updates any active flags that
//   are different in the plot objects. This ensures that the list box will
//   stay consistent between updates that cause its items to be regenerated.
//
// ****************************************************************************

bool
QvisPlotListBox::NeedToUpdateSelection(const PlotList *pl) const
{
    bool retval = true;

    if(pl->GetNumPlots() == count())
    {
        bool anyDiffer = false;
        for(int i = 0; i < pl->GetNumPlots(); ++i)
        {
            QvisPlotListBoxItem *lbi = (QvisPlotListBoxItem *)item(i);
            const Plot &newPlot = pl->operator[](i);
            Plot &currentPlot = lbi->GetPlot();

            if(newPlot.GetActiveFlag() != currentPlot.GetActiveFlag())
            {
                currentPlot.SetActiveFlag(newPlot.GetActiveFlag());
                anyDiffer = true;
            }
        }
        return anyDiffer;
    }

    return retval;
}

// ****************************************************************************
// Method: QvisPlotListBox::IsSelecting
//
// Purpose: 
//   Returns whether we're in selecting mode (the user is selecting items with
//   the mouse)
//
// Returns:    True if we're in selecting mode.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 23 14:32:30 PDT 2010
//
// Modifications:
//   
// ****************************************************************************

bool
QvisPlotListBox::IsSelecting() const
{
    return state() == DragSelectingState;
}

// ****************************************************************************
// Method: QvisPlotListBox::contextMenuCreateActions
//
// Purpose: 
//   Creates the actions required for the context menu & then builds the menu
//
// Arguments:
//
// Programmer: Ellen Tarwater
// Creation:   Mon June 11 2007
//
// Modifications:
//   Kathleen Bonnell, Wed Nov 28 09:36:22 PST 2007
//   Fix compile errror on Windows -- make maxNwins const.
//
//   Ellen Tarwater, Thurs, Dec 27, 2007
//   Commenting out CopyToWin option... incomplete.
//
//   Brad Whitlock, Fri May 30 15:49:57 PDT 2008
//   Qt 4.
//
//   Cyrus Harrison, Tue Jul  8 13:38:08 PDT 2008
//   Initial Qt4 Port. 
//
//   Brad Whitlock, Tue Oct 20 15:28:07 PDT 2009
//   I added menu items that let the user change the plot list.
//
// ****************************************************************************

void
QvisPlotListBox::contextMenuCreateActions()
{
// this should be updated so that nothing is enabled unless a plot is selected!

     //  - need to get the max # of wins for copyToWin...
     // const int maxNwins = 10;
     // int selectWindow[maxNwins];
     
     hideShowAct = new QAction(tr("&Hide/Show"), this);
     hideShowAct->setStatusTip(tr("Hide or Show this plot"));
     hideShowAct->setCheckable(true);
     connect( hideShowAct, SIGNAL(toggled(bool)), this, SIGNAL(hideThisPlot()));
     
     drawAct = new QAction(tr("Draw"), this);
     drawAct->setStatusTip(tr("Draw this plot"));
     connect( drawAct, SIGNAL(triggered()), this, SIGNAL(drawThisPlot()));    

     clearAct = new QAction(tr("Clear"), this);
     clearAct->setStatusTip(tr("Clear this plot"));
     connect( clearAct, SIGNAL(triggered()), this, SIGNAL(clearThisPlot()));    

     redrawAct = new QAction(tr("Redraw"), this);
     redrawAct->setStatusTip(tr("Redraw this plot"));
     connect( redrawAct, SIGNAL(triggered()), this, SIGNAL(redrawThisPlot()));  
     
     deleteAct = new QAction(tr("Delete"), this);
     deleteAct->setStatusTip(tr("Delete this plot"));
     connect( deleteAct, SIGNAL(triggered()), this, SIGNAL(deleteThisPlot()));
          
     cloneAct = new QAction(tr("Clone"), this);
     cloneAct->setStatusTip(tr("Clone this plot"));
     connect( cloneAct, SIGNAL(triggered()), this, SIGNAL(cloneThisPlot()));  

    setPlotDescriptionAct = new QAction(tr("Edit plot description"), this);
    setPlotDescriptionAct->setStatusTip(tr("Add a meaningful name for the plot"));
    connect(setPlotDescriptionAct, SIGNAL(triggered()), this, SLOT(setPlotDescription()));

    moveThisPlotTowardFirstAct = new QAction(tr("Move toward first"), this);
    moveThisPlotTowardFirstAct->setStatusTip(tr("Move this plot one slot toward the first plot in the plot list"));
    connect(moveThisPlotTowardFirstAct, SIGNAL(triggered()), this, SIGNAL(moveThisPlotTowardFirst()));

    moveThisPlotTowardLastAct = new QAction(tr("Move toward last"), this);
    moveThisPlotTowardLastAct->setStatusTip(tr("Move this plot one slot toward the last plot in the plot list"));
    connect(moveThisPlotTowardLastAct, SIGNAL(triggered()), this, SIGNAL(moveThisPlotTowardLast()));

    makeThisPlotFirstAct = new QAction(tr("Make first"), this);
    makeThisPlotFirstAct->setStatusTip(tr("Make this plot be the first in the plot list"));
    connect(makeThisPlotFirstAct, SIGNAL(triggered()), this, SIGNAL(makeThisPlotFirst()));

    makeThisPlotLastAct = new QAction(tr("Make last"), this);
    makeThisPlotLastAct->setStatusTip(tr("Make this plot be the last in the plot list"));
    connect(makeThisPlotLastAct, SIGNAL(triggered()), this, SIGNAL(makeThisPlotLast()));

   
// copy to window incomplete!!!!! Commented out below...
//      copyToWinAct = new QAction(tr("Copy To Window"), 0);
//      copyToWinAct->setStatusTip(tr("Copy this plot to different window"));
//      connect( copyToWinAct, SIGNAL(triggered()), this, SIGNAL(copyToWinThisPlot()));    
//      win1Act = new QAction(tr("Window 1"), 0);
//      win2Act = new QAction(tr("Window 2"), 0);
//      win1Act->setStatusTip(tr("Copy this plot to different window"));
//      win2Act->setStatusTip(tr("Copy this plot to different window"));
//      connect( win1Act, SIGNAL(triggered()), this, SIGNAL(copyToWinThisPlot()));    
//      connect( win2Act, SIGNAL(triggered()), this, SIGNAL(copyToWinThisPlot()));    

    // adding a popup menu to the copyToWin option:
    // how many windows are open? hard-coding to 3 while debugging...
//    int nWindows = 3;    
//    for (int i = 0; i < nWindows; ++i)
//    {
//        selectWindow[i] = -1;
//    }
    
//    copyWinSubMenu = new QMenu(this);
//    win1Act->addTo(copyWinSubMenu);
//    win2Act->addTo(copyWinSubMenu);

     // Now, for the "Disconnect From TimeSlider" option:
     disconnectAct = new QAction(tr("Disconnect From time slider"), this);
     disconnectAct->setStatusTip(tr("Disconnect this plot from time slider"));
     disconnectAct->setCheckable(true);
     connect( disconnectAct, SIGNAL(toggled(bool)), this, SIGNAL(disconnectThisPlot()));
}

// ****************************************************************************
// Method: QvisPlotListBox::contextMenuCreate
//
// Purpose: 
//   Builds the context menu
//
// Arguments:
//
// Creationist: Allen Sanderson
// Creation:    Fri April 230 2010
//
// Modifications:
//
// ****************************************************************************
void
QvisPlotListBox::contextMenuCreate()
{
    QMenu *operatorMenu =
      ((QvisPlotManagerWidget*) parentWidget())->getOperatorMenu();
    QMenu *variableMenu =
      ((QvisPlotManagerWidget*) parentWidget())->getVariableMenu();

    // Incase it is being rebuilt.
    if( plotContextMenu )
      delete plotContextMenu;

    // build the menu
    plotContextMenu = new QMenu(this);
    if( operatorMenu )
      plotContextMenu->addMenu(operatorMenu);
    if( variableMenu )
      plotContextMenu->addMenu(variableMenu);
    if( operatorMenu || variableMenu )
      plotContextMenu->addSeparator();

    plotContextMenu->addAction(hideShowAct);
    plotContextMenu->addAction(clearAct);
    plotContextMenu->addAction(drawAct);
    plotContextMenu->addAction(redrawAct);
    plotContextMenu->addSeparator();
//  plotContextMenu->insertItem(tr("Copy To Window"), copyWinSubMenu);
    plotContextMenu->addAction(cloneAct);
    plotContextMenu->addAction(deleteAct);
    plotContextMenu->addSeparator();
    plotContextMenu->addAction(makeThisPlotFirstAct);
    plotContextMenu->addAction(moveThisPlotTowardFirstAct);
    plotContextMenu->addAction(moveThisPlotTowardLastAct);
    plotContextMenu->addAction(makeThisPlotLastAct);
    plotContextMenu->addSeparator();
    plotContextMenu->addAction(setPlotDescriptionAct);
    plotContextMenu->addSeparator();
 
    plotContextMenu->addAction(disconnectAct);   
}

// ****************************************************************************
// Method: QvisPlotListBox::contextMenuEvent
//
// Purpose: 
//   This is an internal method of QWidget that was re-implemented
//   for this class - to pop up context menu...
//
// Arguments:
//   e : The mouse event.
//
// Programmer: Ellen Tarwater
// Creation:   Mon June 11 2007
//
// Modifications:
//    Ellen Tarwater December 28, 2007
//    disabling menu items if no plots active
//
//    Brad Whitlock, Fri May 30 15:59:13 PDT 2008
//    Qt 4.
//
//    Brad Whitlock, Tue Oct 20 15:36:48 PDT 2009
//    I added actions that modify the plot list.
//
// ****************************************************************************

void
QvisPlotListBox::contextMenuEvent(QContextMenuEvent *e)
{
    // setEnabled(false) if no active plots (plots are 'active' without being
    // highlighted/selected, non-intuitive...??)
    
    bool anyActive = false;
    for(size_t i = 0; i < count(); ++i)
    {
        QvisPlotListBoxItem *lbi = (QvisPlotListBoxItem *)item(i);
        Plot &currentPlot = lbi->GetPlot();

        if(currentPlot.GetActiveFlag())
        {
            anyActive = true;
        }
    }

    hideShowAct->setEnabled(anyActive);
    deleteAct->setEnabled(anyActive);
    drawAct->setEnabled(anyActive);
    clearAct->setEnabled(anyActive);
    cloneAct->setEnabled(anyActive);
    redrawAct->setEnabled(anyActive);
    disconnectAct->setEnabled(anyActive);

    makeThisPlotFirstAct->setEnabled(anyActive && count() > 1);
    moveThisPlotTowardFirstAct->setEnabled(anyActive && count() > 1);
    moveThisPlotTowardLastAct->setEnabled(anyActive && count() > 1);
    makeThisPlotLastAct->setEnabled(anyActive && count() > 1);
    setPlotDescriptionAct->setEnabled(anyActive);

    plotContextMenu->exec( e->globalPos() );
    
    QListWidget::contextMenuEvent(e);
}

// ****************************************************************************
// Method: QvisPlotListBox::triggerPlotRename
//
// Purpose: 
//   This is a helper method that lets the delegate cause the plot list box to
//   emit a renamePlot signal.
//
// Arguments:
//   row : The row of the plot
//   newName : The new name of the plot.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 22 10:52:44 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
QvisPlotListBox::triggerPlotRename(int row, const QString &newName)
{
    emit renamePlot(row, newName);
}

// ****************************************************************************
// Method: QvisPlotListBox::setPlotDescription
//
// Purpose: 
//   This causes the delegate to open a line edit that lets us edit the
//   name of the plot.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 22 10:52:09 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
QvisPlotListBox::setPlotDescription()
{
    QListWidgetItem *item = currentItem();
    if(item != 0)
        openPersistentEditor(item);
}
