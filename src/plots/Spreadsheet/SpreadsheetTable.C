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
#include <SpreadsheetTable.h>

#include <QAbstractItemModel>
#include <QItemDelegate>
#include <QMap>
#include <QPainter>
#include <QPalette>
#include <QStyle>
#include <QFontMetrics>

#include <avtLookupTable.h>
#include <vtkLookupTable.h>

#define GetDataRole  1000

#define MODEL ((DataArrayModel *)model())
#define DELEGATE ((DataArrayDelegate *)itemDelegate())

// Create a less than operator for QSize so we can use QSize as a map key.
bool operator < (const QSize &a, const QSize &b)
{
    if(a.width() < b.width())
        return true;
    else if(a.height() < b.height())
        return true;
    return false;
}

// ****************************************************************************
// Class: DataArrayModel
//
// Purpose:
//   This is a subclass of QAbstractItemModel that lets us show vtkDataArray
//   data in Qt's item view classes. We use QTableView for the SpreadSheet plot.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 28 14:43:57 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

class DataArrayModel : public QAbstractItemModel
{
public:
    DataArrayModel(QObject *parent=0);
    virtual ~DataArrayModel() { }

    void setDataArray(vtkDataArray *arr, vtkDataArray *ghosts,
                      int d[3], SpreadsheetTable::DisplayMode dm, int sliceindex,
                      int base_index[3]);
    void clearDataArray();

    // This would be better in the delegate but it's easier to put here.
    void    setFormatString(const QString &fmt) { formatString = fmt;  }
    QString getFormatString() const             { return formatString; }

    void addSelectedCellLabel(int r, int c, const QString &label);
    void clearSelectedCellLabels();
    int  numSelectedCellLabels() const;

    // Methods that implement QAbstractItemModel
    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;
    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;

    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
    virtual QModelIndex parent ( const QModelIndex & index ) const;

private:
    int                           nRows;
    int                           nColumns;
    vtkDataArray                 *dataArray;
    vtkDataArray                 *ghostArray;
    int                           dims[3];
    int                           base_index[3];
    SpreadsheetTable::DisplayMode displayMode;
    int                           sliceIndex;
    QString                       formatString;
    QMap<QSize, QString>          selectedCellLabels;
};

// ****************************************************************************
// Method: DataArrayModel::DataArrayModel
//
// Purpose: 
//   Constructor for the DataArrayModel class.
//
// Arguments:
//   parent : The object's parent.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 28 14:45:01 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

DataArrayModel::DataArrayModel(QObject *parent) : QAbstractItemModel(parent),
    formatString("%1.6f"), selectedCellLabels()
{
    nRows = 0;
    nColumns = 0;
    dataArray = 0;
    ghostArray = 0;
    dims[0] = dims[1] = dims[2] = 0;
    base_index[0] = base_index[1] = base_index[2] = 0;
    displayMode = SpreadsheetTable::SliceZ;
    sliceIndex = 0;
}

// ****************************************************************************
// Method: DataArrayModel::setDataArray
//
// Purpose: 
//   Provides data to the model.
//
// Arguments:
//   arr : The data array to use for values.
//   ghosts : The data array to use for ghosting values.
//   d      : The i,j,k dimensions of the dataset.
//   dm     : The display mode (which way the data is sliced in the spreadsheet).
//   sliceindex : The index of the slice in the prescribed dimension
//   base_index : The base_index values to add to i,j,k when displaying the
//                indices that identify a cell or node.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 28 16:21:31 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
DataArrayModel::setDataArray(vtkDataArray *arr, vtkDataArray *ghosts,
    int d[3], SpreadsheetTable::DisplayMode dm, int sliceindex, int base_index[3])
{
    // The data arrays.
    dataArray = arr;
    ghostArray = ghosts;

    // The size of the data arrays.
    dims[0] = d[0];
    dims[1] = d[1];
    dims[2] = d[2];

    // The slice that this table will display.
    displayMode = dm;
    sliceIndex = sliceindex;

    if(displayMode == SpreadsheetTable::SliceX)
    {
        nColumns = dims[2];
        nRows = dims[1];
    }
    else if(displayMode == SpreadsheetTable::SliceY)
    {
        nColumns = dims[2];
        nRows = dims[0];
    }
    else if(displayMode == SpreadsheetTable::SliceZ)
    {
        nColumns = dims[0];
        nRows = dims[1];
    }
    else if(displayMode == SpreadsheetTable::UCDCell)
    {
        nColumns = 1;;
        nRows = dims[1];
    }
    else if(displayMode == SpreadsheetTable::UCDNode)
    {
        nColumns = 1;
        nRows = dims[1];
    }

    reset();
}

// ****************************************************************************
// Method: DataArrayModel::clearDataArray
//
// Purpose: 
//   Clears out the model's data.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 28 16:22:16 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
DataArrayModel::clearDataArray()
{
    dims[0] = dims[1] = dims[2] = 0;
    nColumns = 0;
    nRows = 0;
    dataArray = 0;
    ghostArray = 0;
    reset();
}

// ****************************************************************************
// Method: DataArrayModel::addSelectedCellLabel
//
// Purpose: 
//   Adds a selected cell label to the model.
//
// Arguments:
//   row   : The row for the cell that will contain the label.
//   col   : The column for the cell that will contain the label.
//   label : The pick label to use.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 28 16:22:31 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
DataArrayModel::addSelectedCellLabel(int row, int col, const QString &label)
{
    QSize key(row, col);
    selectedCellLabels.insert(key, label);
    reset();
}

// ****************************************************************************
// Method: DataArrayModel::clearSelectedCellLabels
//
// Purpose: 
//   Clears the selected cell labels.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 28 16:23:26 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
DataArrayModel::clearSelectedCellLabels()
{
    selectedCellLabels.clear();
    reset();
}

// ****************************************************************************
// Method: DataArrayModel::numSelectedCellLabels
//
// Purpose: 
//   Returns the number of selected cell labels.
//
// Returns:    The number of selected cell labels.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 28 16:23:51 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

int
DataArrayModel::numSelectedCellLabels() const
{
    return selectedCellLabels.size();
}

//
// Methods that implement QAbstractItemModel
//

// ****************************************************************************
// Method: DataArrayModel::columnCount
//
// Purpose: 
//   Returns the number of columns in the model.
//
// Returns:    The number of columns.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 28 16:24:18 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

int
DataArrayModel::columnCount(const QModelIndex &) const
{
    return nColumns;
}

// ****************************************************************************
// Method: DataArrayModel::rowCount
//
// Purpose: 
//   Returns the number of rows in the model.
//
// Returns:    The number of rows.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 28 16:24:40 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

int
DataArrayModel::rowCount(const QModelIndex &) const
{
    return nRows;
}

// ****************************************************************************
// Method: DataArrayModel::data
//
// Purpose: 
//   Returns data for the specified model index.
//
// Arguments:
//   index : The model index for which we want data.
//   role  : The type of data that we want to return.
//
// Returns:    Data for the specified model index.
//
// Note:       We implement the custom GetDataRole to return the actual double
//             data for the cell. We then use that data later for coloring the
//             cell when drawing and to do various summing operations, etc. on
//             the selected cells.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 28 16:25:05 PDT 2008
//
// Modifications:
//   Cyrus Harrison, Mon Sep  8 15:05:23 PDT 2008
//   Changed the way the vtk id is obtained from the model to avoid AIX/xlC
//   problems.
//
// ****************************************************************************

QVariant
DataArrayModel::data(const QModelIndex &index, int role) const
{
    if(role == Qt::DisplayRole)
    {
        // This role returns the string that is displayed in the cells.
        if(dataArray == 0)
            return QVariant();
        else
        {
            void *ptr = index.internalPointer();
            vtkIdType id = *((vtkIdType*)&ptr);
            double value = 0.;
            if(id < dataArray->GetNumberOfTuples())
                value = dataArray->GetTuple1(id);
            QString s; s.sprintf(formatString.toStdString().c_str(), value);

            // See if we need to prepend a pick letter.
            QSize key(index.row(), index.column());
            QMap<QSize,QString>::const_iterator it = selectedCellLabels.find(key);
            if(it != selectedCellLabels.end())
                s = it.value() + QString("=") + s;
            return QVariant(s);
        }
    }
    else if(role == GetDataRole)
    {
        // This role returns the actual data so we can use it in various operations.
        if(dataArray == 0)
            return QVariant();
        else
        {
            void *ptr = index.internalPointer();
            vtkIdType id = *((vtkIdType*)&ptr);
            double value = 0.;
            if(id < dataArray->GetNumberOfTuples())
                value = dataArray->GetTuple1(id);
            return QVariant(value);
        }
    }
    else if(role == Qt::BackgroundRole)
    {
        // This role returns the background brush: gray for ghost, default otherwise.
        bool ghost = false;
        if(ghostArray != 0)
        {
            void *ptr = index.internalPointer();
            vtkIdType index = *((vtkIdType*)&ptr);

            // By convention, the ghost zones array contains unsigned char.
            const unsigned char *ghosts = (const unsigned char *)ghostArray->
                GetVoidPointer(0);
            if(index < ghostArray->GetNumberOfTuples())
                ghost = ghosts[index] > 0;
        }

        return ghost ? QVariant(QBrush(QColor(200,200,200))) : QVariant();
    }

    return QVariant();
}

// ****************************************************************************
// Method: DataArrayModel::headerData
//
// Purpose: 
//   Returns a label for a particular header cell.
//
// Arguments:
//   section     : The index of the header cell.
//   orientation : vertical or horizontal
//   role        : The type of header data that we want.
//
// Returns:    The header data for a particular header cell.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 28 16:25:59 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QVariant
DataArrayModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole)
    {
        QString label;

        if(displayMode == SpreadsheetTable::SliceX)
        {
            if(orientation == Qt::Horizontal)
                label.sprintf("k=%d", section + base_index[2]);
            else
                label.sprintf("j=%d", dims[1]-1-section + base_index[1]);
        }
        else if(displayMode == SpreadsheetTable::SliceY)
        {
            if(orientation == Qt::Horizontal)
                label.sprintf("k=%d", section + base_index[2]);
            else
                label.sprintf("i=%d", dims[0]-1-section + base_index[0]);
        }
        else if(displayMode == SpreadsheetTable::SliceZ)
        {
            if(orientation == Qt::Horizontal)
                label.sprintf("i=%d", section + base_index[0]);
            else
                label.sprintf("j=%d", dims[1]-1-section + base_index[1]);
        }
        else if(displayMode == SpreadsheetTable::UCDCell)
        {
            if(orientation == Qt::Horizontal)
                label = tr("cell value");
            else
                label.sprintf("%d", section + base_index[0]);
        }
        else if(displayMode == SpreadsheetTable::UCDNode)
        {
            if(orientation == Qt::Horizontal)
                label = tr("node value");
            else
                label.sprintf("%d", section + base_index[0]);
        }

        return QVariant(label);
    }

    return QAbstractItemModel::headerData(section, orientation, role);
}

// ****************************************************************************
// Method: DataArrayModel::index
//
// Purpose: 
//   Constructs a model index for the specified row and column.
//
// Arguments:
//   row    : The table row that we want.
//   column : The table column that we want.
//
// Returns:    The model index for the specified cell.
//
// Note:       This data model has a 3D array and a slice so we have different
//             ways of slicing through the data. We calculate the index of the
//             vtkDataArray tuple that is addressable given row,column and
//             the current slicing mode. We then put that index into the
//             model index's void* so we can use it in the data() method.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 28 16:28:01 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QModelIndex
DataArrayModel::index(int row, int column, const QModelIndex &) const
{
    vtkIdType index = 0;
    vtkIdType nxny = dims[0] * dims[1];
#define STRUCTURED_INDEX3(z,y,x) (((z)*nxny) + ((y)*dims[0]) + (x))
    if(displayMode == SpreadsheetTable::SliceX)
    {
        vtkIdType i = sliceIndex;
        vtkIdType j = dims[1]-1-row;
        vtkIdType k = column;
        index = STRUCTURED_INDEX3(k, j, i);
    }
    else if(displayMode == SpreadsheetTable::SliceY)
    {
        vtkIdType i = dims[0]-1-row;
        vtkIdType j = sliceIndex;
        vtkIdType k = column;
        index = STRUCTURED_INDEX3(k, j, i);
    }
    else if(displayMode == SpreadsheetTable::SliceZ)
    {
        vtkIdType i = column;
        vtkIdType j = dims[1]-1-row;
        vtkIdType k = sliceIndex;
        index = STRUCTURED_INDEX3(k, j, i);
    }
    else
        index = row;

    // Store the index in the space for the void*
    void *ptr = (void *)index;
    return createIndex(row, column, ptr);
}

// ****************************************************************************
// Method: DataArrayModel::parent
//
// Purpose: 
//   Returns that no model indices in this model have parents.
//
// Returns:    An empty model index.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 28 16:30:41 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QModelIndex
DataArrayModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

// ****************************************************************************
// Class: DataArrayDelegate
//
// Purpose:
//   This special purpose delegate lets us render table cells in color.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 28 16:31:20 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

class DataArrayDelegate : public QItemDelegate
{
public:
    DataArrayDelegate(QAbstractItemModel *m, QObject *parent=0);
    virtual ~DataArrayDelegate() { }

    void setLUT(avtLookupTable *L)  { lut = L; }
    void setRenderInColor(bool val) { renderInColor = val; }
    bool getRenderInColor() const   { return renderInColor; }

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    bool                renderInColor;
    avtLookupTable     *lut;
    QAbstractItemModel *model;
};

// ****************************************************************************
// Method: DataArrayDelegate::DataArrayDelegate
//
// Purpose: 
//   Constructor for the DataArrayDelegate class.
//
// Arguments:
//   m      : The data model that we'll use.
//   parent : The object's parent.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 28 16:32:09 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

DataArrayDelegate::DataArrayDelegate(QAbstractItemModel *m, QObject *parent) : 
    QItemDelegate(parent)
{
    model = m;
    renderInColor = false;
    lut = 0;
}

// ****************************************************************************
// Method: DataArrayDelegate::paint
//
// Purpose: 
//   Override of the paint method that lets us change the text color based
//   on the model's data value.
//
// Arguments:
//   painter : The painter that's being used to draw the cell.
//   option  : The options for drawing the cell.
//   index   : The model index for the cell.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 28 16:33:43 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
DataArrayDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, 
    const QModelIndex &index) const
{
    if(renderInColor)
    {
        // Get the actual numeric value from the data model so we can
        // map it to a color.
        double value = model->data(index, GetDataRole).toDouble();
        const unsigned char *rgb = lut->GetLookupTable()->MapValue(value);
        QColor c(rgb[0], rgb[1], rgb[2]);

        QStyleOptionViewItem option2(option);
        option2.palette.setColor(QPalette::Normal, QPalette::Text, c);
        option2.palette.setColor(QPalette::Normal, QPalette::HighlightedText, c);
        option2.palette.setColor(QPalette::Disabled, QPalette::Text, c);
        option2.palette.setColor(QPalette::Disabled, QPalette::HighlightedText, c);
        option2.palette.setColor(QPalette::Inactive, QPalette::Text, c);
        option2.palette.setColor(QPalette::Inactive, QPalette::HighlightedText, c);
        QItemDelegate::paint(painter, option2, index);
    }
    else
        QItemDelegate::paint(painter, option, index);
}

// ****************************************************************************
// Method: SpreadsheetTable::SpreadsheetTable
//
// Purpose: 
//   Constructor for the SpreadsheetTable class.
//
// Arguments:
//   parent : The parent widget.
//   name   : This widget's name.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 15:32:53 PST 2007
//
// Modifications:
//   Gunther H. Weber, Fri Sep 14 11:40:18 PDT 2007
//   Changed focus style due to display problems of current cell when set
//   from pick attributes.
//
//   Brad Whitlock, Wed Aug 27 14:06:08 PDT 2008
//   Qt 4.
//
// ****************************************************************************

SpreadsheetTable::SpreadsheetTable(QWidget *parent) : QTableView(parent)
{
    // Create a data array model that we'll use to access the VTK data.
    DataArrayModel *m = new DataArrayModel(this);
    setModel(m);

    // Create a delegate to help us draw the cells.
    DataArrayDelegate *d = new DataArrayDelegate(m, this);
    setItemDelegate(d);

    connect(selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            this, SIGNAL(selectionChanged()));
}

// ****************************************************************************
// Method: SpreadsheetTable::~SpreadsheetTable
//
// Purpose: 
//   Destructor for the SpreadsheetTable class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 15:33:26 PST 2007
//
// Modifications:
//   
// ****************************************************************************

SpreadsheetTable::~SpreadsheetTable()
{
}

// ****************************************************************************
// Method: SpreadsheetTable::setLUT
//
// Purpose: 
//   Sets the LUT that should be used when we need to map data to colors.
//
// Arguments:
//   L  : The LUT to use.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 15:33:44 PST 2007
//
// Modifications:
//   Brad Whitlock, Wed Aug 27 10:51:39 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
SpreadsheetTable::setLUT(avtLookupTable *L)
{
    DELEGATE->setLUT(L);
}

// ****************************************************************************
// Method: SpreadsheetTable::setRenderInColor
//
// Purpose: 
//   Sets whether the table should render its text in color.
//
// Arguments:
//   ric : True for rendering in color; Otherwise false.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 15:34:15 PST 2007
//
// Modifications:
//   Brad Whitlock, Wed Aug 27 10:52:00 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void 
SpreadsheetTable::setRenderInColor(bool ric)
{
    if(DELEGATE->getRenderInColor() != ric)
    {
        DELEGATE->setRenderInColor(ric);
        if(viewport() != 0)
            viewport()->update();
    }
}

// ****************************************************************************
// Method: SpreadsheetTable::setFormatString
//
// Purpose: 
//   Sets the format string to use for drawing text.
//
// Arguments:
//   fmt : The format string.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 15:34:46 PST 2007
//
// Modifications:
//   Gunther H. Weber, Thu Sep 27 11:37:18 PDT 2007
//   Adapt column width to fit displayed values
//
//   Brad Whitlock, Wed Aug 27 10:54:37 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
SpreadsheetTable::setFormatString(const QString &fmt)
{
    if(fmt != MODEL->getFormatString())
    {
        MODEL->setFormatString(fmt);
        updateColumnWidths();
        update();
    }
}

// ****************************************************************************
// Method: SpreadsheetTable::setDataArray
//
// Purpose: 
//   Sets the data array and other parameters used to render the data in
//   the spreadsheet.
//
// Arguments:
//   arr : The data array to use for values.
//   ghosts : The data array to use for ghosting values.
//   d      : The i,j,k dimensions of the dataset.
//   dm     : The display mode (which way the data is sliced in the spreadsheet).
//   sliceindex : The index of the slice in the prescribed dimension
//   base_index : The base_index values to add to i,j,k when displaying the
//                indices that identify a cell or node.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 15:35:22 PST 2007
//
// Modifications:
//   Gunther H. Weber, Thu Sep 27 11:37:18 PDT 2007
//   Adapt column width to fit displayed values
//
//   Brad Whitlock, Wed Aug 27 09:36:11 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
SpreadsheetTable::setDataArray(vtkDataArray *arr, vtkDataArray *ghosts,
    int d[3], DisplayMode dm, int sliceindex, int base_index[3])
{
    MODEL->setDataArray(arr, ghosts, d, dm, sliceindex, base_index);
    updateColumnWidths();
}

// ****************************************************************************
// Method: SpreadsheetTable
//
// Purpose: 
//   Disassociates the current data and ghost arrays from the spreadsheeet.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 15:37:38 PST 2007
//
// Modifications:
//   Brad Whitlock, Wed Aug 27 09:36:36 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
SpreadsheetTable::clearDataArray()
{
    MODEL->clearDataArray();
    selectNone();
}

// ****************************************************************************
// Method: SpreadsheetTable::selectedCellsAsText
//
// Purpose: 
//   Gets the selected cells as text.
//
// Returns:    The string containing the text for all selected cells of 
//             the table.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 10:47:33 PDT 2007
//
// Modifications:
//   Brad Whitlock, Wed Aug 27 16:22:38 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QString
SpreadsheetTable::selectedCellsAsText() const
{
    QString txt;

    for(int row = 0; row < model()->rowCount(); ++row)
    {
        bool lineHasText = false;
        for(int col = 0; col < model()->columnCount(); ++col)
        {
            QModelIndex index(model()->index(row, col));
            if(selectionModel()->isSelected(index))
            {
                txt += model()->data(index, Qt::DisplayRole).toString();
                if(col < col < model()->columnCount())
                    txt += " ";
                lineHasText = true;
            }
        }
        if(lineHasText)
            txt += "\n";
    }

    return txt;
}

// ****************************************************************************
// Method: SpreadsheetTable::selectedCellsSum
//
// Purpose: 
//   Sums the data values over the selected cells.
//
// Returns:    The sum of the values in the selected cells.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 12:28:57 PDT 2007
//
// Modifications:
//   Brad Whitlock, Wed Aug 27 15:59:08 PDT 2008
//   Rewrote for Qt 4.
//
// ****************************************************************************

double
SpreadsheetTable::selectedCellsSum() const
{
    double sum = 0.;

    QModelIndexList sel(selectedIndexes());
    for(QModelIndexList::iterator it = sel.begin(); it != sel.end(); ++it)
        sum += model()->data(*it, GetDataRole).toDouble();

    return sum;
}

// ****************************************************************************
// Method: SpreadsheetTable::selectedCellsAverage
//
// Purpose: 
//   Average the values of the selected cells.
//
// Returns:    The average of the selected cells.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 12:29:26 PDT 2007
//
// Modifications:
//   Brad Whitlock, Wed Aug 27 15:59:08 PDT 2008
//   Rewrote for Qt 4.
//   
// ****************************************************************************

double
SpreadsheetTable::selectedCellsAverage() const
{
    double avg = 0., sum = 0.;
    int ncells = 0;
    QModelIndexList sel(selectedIndexes());
    for(QModelIndexList::iterator it = sel.begin(); it != sel.end(); ++it, ++ncells)
        sum += model()->data(*it, GetDataRole).toDouble();
    avg = (ncells > 0) ? (sum / double(ncells)) : sum;
    return avg;
}

// ****************************************************************************
// Method: SpreadsheetTable::addSelectedCellLabel()
//
// Purpose: 
//   Add a label (pick letter) for a cell, which will be displayed if the
//   cell is selected.
//
// Arguments:
//   row : The row of the cell.
//   col : The column of the cell.
//   label: The label, i.e., pick letter
//
// Programmer: Gunther H. Weber
// Creation:   Fri Sep 14 11:44:31 PDT 2007
//
// Modifications:
//   Brad Whitlock, Thu Aug 28 09:40:22 PDT 2008
//   Rewrote.
//
// ****************************************************************************

void
SpreadsheetTable::addSelectedCellLabel(int row, int col, const QString &label)
{
    MODEL->addSelectedCellLabel(row, col, label);
}

// ****************************************************************************
// Method: SpreadsheetTable::clearSelectedCellLabels()
//
// Purpose: 
//   Clear the list of cell labels (pick letters for cells).
//
// Programmer: Gunther H. Weber
// Creation:   Fri Sep 14 11:44:31 PDT 2007
//
// Modifications:
//   Brad Whitlock, Thu Aug 28 09:40:46 PDT 2008
//   Rewrote.
//
// ****************************************************************************

void
SpreadsheetTable::clearSelectedCellLabels()
{
    MODEL->clearSelectedCellLabels();
}

// ****************************************************************************
// Method: SpreadsheetTable::setFont()
//
// Purpose: 
//   Update column widths after a font change
//
// Programmer: Gunther H. Weber
// Creation:   Thu Sep 27 13:37:40 PDT 2007
//
// Modifications:
//   Brad Whitlock, Wed Aug 27 14:07:06 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
SpreadsheetTable::setFont(QFont&f)
{
    QTableView::setFont(f);
    updateColumnWidths();
}

// ****************************************************************************
// Method: SpreadsheetTable::selectAll
//
// Purpose: 
//   Selects all of the cells in the table.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 14:31:06 PST 2007
//
// Modifications:
//   Brad Whitlock, Wed Aug 27 16:44:22 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
SpreadsheetTable::selectAll()
{
    QModelIndex topLeft = model()->index(0,0);
    QModelIndex bottomRight = model()->index(model()->rowCount()-1, model()->columnCount()-1);
    QItemSelection selection;
    selection.select(topLeft, bottomRight);
    selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
}

// ****************************************************************************
// Method: SpreadsheetTable::selectNone
//
// Purpose: 
//   Removes all of the selections from the table.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 14:31:23 PST 2007
//
// Modifications:
//   Brad Whitlock, Wed Aug 27 16:44:16 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
SpreadsheetTable::selectNone()
{
    selectionModel()->clear();
}

// ****************************************************************************
// Method: SpreadsheetTable::updateColumnWidths
//
// Purpose: 
//   Change table column width so that numeric values (and labels) displayed
//   in cells are not truncated
//
// Programmer: Gunther H. Weber
// Creation:   Thu Sep 27 13:43:05 PDT 2007
//
// Modifications:
//   Brad Whitlock, Wed Aug 27 10:59:56 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
SpreadsheetTable::updateColumnWidths()
{
    QFontMetrics fm(font());
    QString lengthProbeString;
    lengthProbeString.sprintf(MODEL->getFormatString().toStdString().c_str(),
        -3.33333333333333333333);
    int columnWidth = fm.width(lengthProbeString);
    if(MODEL->numSelectedCellLabels() > 0)
        columnWidth += fm.width(" AA=");
    else
        columnWidth += fm.width(" ");
    for (int i=0; i < model()->columnCount(); ++i)
        setColumnWidth(i, columnWidth);
}
