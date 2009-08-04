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
#ifndef SPREADSHEET_TABLE_H
#define SPREADSHEET_TABLE_H
#include <string>
#include <QTableView>
#include <list>

#include <vtkDataArray.h>

class avtLookupTable;

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
// ****************************************************************************

class SpreadsheetTable : public QTableView
{
    Q_OBJECT
public:
    typedef enum {SliceX, SliceY, SliceZ, UCDCell, UCDNode} DisplayMode;

    SpreadsheetTable(QWidget *parent = 0);
    virtual ~SpreadsheetTable();

    void setLUT(avtLookupTable *);
    void setRenderInColor(bool);
    void setFormatString(const QString &);

    void setDataArray(vtkDataArray *arr, vtkDataArray *ghosts,
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
    void selectionChanged();
public slots:
    void selectAll();
    void selectNone();
protected:
    void updateColumnWidths();
};

#endif
