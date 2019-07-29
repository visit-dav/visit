// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisSequenceView.h>
#include <QvisSequenceMappingModel.h>
#include <QAbstractItemDelegate>
#include <QHeaderView>
#include <QPainter>

#define MODEL ((QvisSequenceMappingModel *)model())

// ****************************************************************************
// Class: SequenceDelegate
//
// Purpose:
//  This is a specialized delegate that renders the sequence items in the 
//  sequence view such that the icons are drawn above the names. We also
//  provide special size hints so the table cells are sized to accept the
//  sequence items.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 22 14:53:04 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

class SequenceDelegate : public QAbstractItemDelegate
{
public:
    SequenceDelegate(QObject *parent) : QAbstractItemDelegate(parent) { }
    virtual ~SequenceDelegate() { }

    virtual void paint(QPainter *painter, 
                       const QStyleOptionViewItem &option, 
                       const QModelIndex &index) const
    {
        QString name = index.data(Qt::DisplayRole).toString();
        QPixmap pix = index.data(Qt::DecorationRole).value<QPixmap>();
        int tw = option.fontMetrics.boundingRect(name).width();

        painter->save();

        // Fill the background
        QColor textColor;
        if (option.showDecorationSelected && (option.state & QStyle::State_Selected))
        {
            QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
                                      ? QPalette::Normal : QPalette::Disabled;
            if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
                cg = QPalette::Inactive;
            textColor = option.palette.color(cg, QPalette::HighlightedText);
            painter->fillRect(option.rect, option.palette.brush(cg, QPalette::Highlight));
        }
        else
        {
            textColor = option.palette.color(QPalette::Text);
            QVariant value = index.data(Qt::BackgroundRole);
            if (value.canConvert<QBrush>()) {
                QPointF oldBO = painter->brushOrigin();
                painter->setBrushOrigin(option.rect.topLeft());
                painter->fillRect(option.rect, qvariant_cast<QBrush>(value));
                painter->setBrushOrigin(oldBO);
            }
        }
        painter->setPen(textColor);

        // Draw the icon and text.
        if(pix.isNull())
        {
            int tx = option.rect.x() + (option.rect.width() - tw) / 2;
            int ty = option.rect.y() + (option.rect.height()/2);
            int th = option.rect.height()/2 - padding;
            painter->drawText(QRect(tx, ty, tw, th), name);
        }
        else
        {
            int tx = option.rect.x() + (option.rect.width() - tw) / 2;
            int ty = option.rect.y() + pix.height() + 10;
            int th = option.rect.height()/2 - padding;

            int px = option.rect.x() + (option.rect.width() - pix.width()) / 2;
            int py = option.rect.y() + padding;
            painter->drawPixmap(QPoint(px, py), pix);
            painter->drawText(QRect(tx, ty, tw, th), name);
        }
        painter->restore();
    }

    virtual QSize sizeHint(const QStyleOptionViewItem &option, 
                           const QModelIndex &index) const
    {
        QString name = index.data(Qt::DisplayRole).toString();
        QPixmap pix = index.data(Qt::DecorationRole).value<QPixmap>();
        int tw = option.fontMetrics.boundingRect(name).width();
        int pw = pix.width();
        int w = qMax(tw, pw) + 2*padding;

        int ph = pix.height();
        int th = option.fontMetrics.height();
        int h = qMax(ph + th + 3*padding, 30);
        return QSize(w,h);
    }

    static const int padding;
};
const int SequenceDelegate::padding = 5;


// ****************************************************************************
// Method: QvisSequenceView::QvisSequenceView
//
// Purpose: 
//   Constructor for the QvisSequenceView class.
//
// Arguments:
//   parent : The widget's parent.
//   name   : The name of the widget.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:55:38 PDT 2006
//
// Modifications:
//   Brad Whitlock, Fri Oct 17 14:59:02 PDT 2008
//   Total rewrite.
//
// ****************************************************************************

QvisSequenceView::QvisSequenceView(QWidget *parent) : QTableView(parent)
{
    // Create a sequence mapping model that we'll use to store sequence data.
    QvisSequenceMappingModel *m = new QvisSequenceMappingModel(this);
    setModel(m);
    connect(m,    SIGNAL(updatedMapping(const QString &,const QStringList &)),
            this, SIGNAL(updatedMapping(const QString &,const QStringList &)));
    connect(m,    SIGNAL(updatedMapping(const QString &,const QStringList &,const QString &,const QStringList &)),
            this, SIGNAL(updatedMapping(const QString &,const QStringList &,const QString &,const QStringList &)));

    setItemDelegate(new SequenceDelegate(this));

    setShowGrid(false);
    setAlternatingRowColors(true);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::SingleSelection);

    horizontalHeader()->hide();
    verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    setTextElideMode(Qt::ElideMiddle);

    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
}

// ****************************************************************************
// Method: QvisSequenceView::~QvisSequenceView
//
// Purpose: 
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:56:14 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

QvisSequenceView::~QvisSequenceView()
{
}

// ****************************************************************************
// Method: QvisSequenceView::clear
//
// Purpose: 
//   Clears out all of the sequences.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:56:33 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSequenceView::clear()
{
    MODEL->clear();
}

// ****************************************************************************
// Method: QvisSequenceView::addViewport
//
// Purpose: 
//   Adds a new viewport.
//
// Arguments:
//   viewportName : The name of the new viewport.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:59:19 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSequenceView::addViewport(const QString &viewportName)
{
    MODEL->addViewport(viewportName);
}

// ****************************************************************************
// Method: QvisSequenceView::addSequenceToViewport
//
// Purpose: 
//   Adds a new sequence to the viewport.
//
// Arguments:
//   vpt     : The name of the viewport.
//   seqName : The name of the new sequence.
//   seqType : The type of sequence.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:59:50 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSequenceView::addSequenceToViewport(const QString &vpt, 
    const QString &seqName, int seqType)
{
    addSequenceToViewport(vpt, seqName, QPixmap(), seqType);
}

// ****************************************************************************
// Method: QvisSequenceView::addSequenceToViewport
//
// Purpose: 
//   Adds a new sequence to the viewport.
//
// Arguments:
//   vpt     : The name of the viewport.
//   seqName : The name of the new sequence.
//   pix     : The sequence's pixmap.
//   seqType : The type of sequence.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:59:50 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSequenceView::addSequenceToViewport(const QString &vpt, 
    const QString &seqName, const QPixmap &pix, int seqType)
{
    MODEL->addSequenceToViewport(vpt, seqName, pix, seqType);
}

// ****************************************************************************
// Method: QvisSequenceView::removeSequence
//
// Purpose: 
//   Remove the named sequence.
//
// Arguments:
//   name : The name of the sequence to remove.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 12:04:57 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSequenceView::removeSequence(const QString &name)
{
    MODEL->removeSequence(name);
}

// ****************************************************************************
// Method: QvisSequenceView::selectSequence
//
// Purpose: 
//   Selects the sequence.
//
// Arguments:
//   name : The name of the sequence to select.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 22 11:22:20 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisSequenceView::selectSequence(const QString &name)
{
    QModelIndex index = MODEL->indexForSequence(name);
    if(index.isValid())
    {
        selectionModel()->clear();
        selectionModel()->select(index, QItemSelectionModel::Select);
    }
}
