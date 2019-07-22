// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SPREADSHEET_TABLE_H
#define SPREADSHEET_TABLE_H
#include <string>
#include <QTableView>
#include <list>

#include <vtkDataArray.h>

class avtLookupTable;
class vtkRectilinearGrid;

// ****************************************************************************
// Class: SpreadsheetTable
//
// Purpose:
//   Subclass of QTable that can directly display VTK data without needing
//   its own internal representation.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 11:15:35 PDT 2007
//
// Modifications:
//   Gunther H. Weber,  Fri Sep 14 11:39:24 PDT 2007
//   Added list of pick letters to be displayed for selected cells
//
//   Gunther H. Weber, Thu Sep 27 13:33:36 PDT 2007
//   Add support for updating column width after changing the font
//
//   Brad Whitlock, Tue Aug 26 15:09:37 PDT 2008
//   Qt 4.
//
//   Brad Whitlock, Fri May  8 12:07:49 PDT 2009
//   I added functions to return the vtkIdTypes for a row and col.
//
//   Brad Whitlock, Wed Dec  1 16:40:44 PST 2010
//   I added setCurveData.
//
//   Brad Whitlock, Thu Jan  5 13:47:15 PST 2012
//   I added missing data support.
//
// ****************************************************************************

class SpreadsheetTable : public QTableView
{
    Q_OBJECT
public:
    typedef enum {SliceX, SliceY, SliceZ, UCDCell, UCDNode, CurveData} DisplayMode;

    SpreadsheetTable(QWidget *parent = 0);
    virtual ~SpreadsheetTable();

    void setLUT(avtLookupTable *);
    void setRenderInColor(bool);
    void setFormatString(const QString &);

    void setCurveData(vtkRectilinearGrid *rgrid);

    void setDataArray(vtkDataArray *arr, 
                      vtkDataArray *ghosts, vtkDataArray *missingData,
                      int d[3], DisplayMode dm, int sliceindex,
                      int base_index[3]);
    void clearDataArray();

    QString selectedCellsAsText() const;
    double  selectedCellsSum() const;
    double  selectedCellsAverage() const;
    vtkIdType *selectedColumnIndices(int &nvals) const;
    vtkIdType *selectedRowIndices(int &nvals) const;

    void addSelectedCellLabel(int r, int c, const QString &label);
    void clearSelectedCellLabels();

    void setFont(QFont&);

signals:
    void tableSelectionChanged();
public slots:
    void selectAll();
    void selectNone();
protected:
    void updateColumnWidths();
};

#endif
