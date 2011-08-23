/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
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

#ifndef QVIS_SELECTIONS_WINDOW_H
#define QVIS_SELECTIONS_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowSimpleObserver.h>
#include <AttributeSubject.h>
#include <SelectionProperties.h>

// Forward declarations
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QListWidget;
class QPushButton;
class QRadioButton;
class QSpinBox;
class QTableWidget;
class QTabWidget;

class QvisHistogram;
class QvisVariableButton;
class QvisVariableListLimiter;

class PlotList;
class SelectionList;
class EngineList;
class WindowInformation;

// ****************************************************************************
// Class: QvisSelectionsWindow
//
// Purpose:
//   This class contains the widgets that manipulate selections
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug  6 15:35:12 PDT 2010
//
// Modifications:
//
// ****************************************************************************

class GUI_API QvisSelectionsWindow : public QvisPostableWindowSimpleObserver
{
    Q_OBJECT
public:
    QvisSelectionsWindow(const QString &caption = QString::null,
                         const QString &shortName = QString::null,
                         QvisNotepadArea * notepad = 0);
    virtual ~QvisSelectionsWindow();
    virtual void CreateWindowContents();

    void ConnectSelectionList(SelectionList *);
    void ConnectPlotList(PlotList *);
    void ConnectEngineList(EngineList *);
    void ConnectWindowInformation(WindowInformation *);

    virtual void SubjectRemoved(Subject *);

public  slots:
    void highlightSelection(const QString &);
protected:
    void    UpdateWindow(bool doAll);
    void    UpdateSelectionProperties();
    void    UpdateSelectionSummary();
    void    UpdateHistogram(const double *values, int nvalues,
                            int minBin, int maxBin, bool useBins);
    void    UpdateHistogram();
    void    UpdateHistogramTitle();
    void    UpdateMinMaxBins(bool, bool, bool);
    void    Apply(bool forceUpdate, bool updatePlots);
    void    GetCurrentValues(int);

    QString GetLoadHost() const;

    void    NewEnabled(QString &, QString &);

private slots:
    void    automaticallyApplyChanged(bool val);
    void    UpdateWindowSingleItem();
    void    addSelection();
    void    deleteSelection();
    void    saveSelection();
    void    loadSelection();
    void    updateSelection();
    void    updateQuery();

    void    cumulativeQueryClicked(bool);
    void    addVariable(const QString &);
    void    setVariableRange(const QString &,float,float);
    void    deleteVariable(const QString &);
    void    initializeVariableList();
    void    processTimeMin();
    void    processTimeMax();
    void    processTimeStride();

    void    histogramTypeChanged(int value);
    void    summationChanged(int);
    void    histogramVariableChanged(int);
    void    histogramNumBinsChanged(int);
    void    histogramStartChanged(int);
    void    histogramEndChanged(int);

private:
    QWidget *CreatePropertiesTab(QWidget *parent);
    QWidget *CreateStatisticsTab(QWidget *parent);

    QWidget *CreateCQRangeControls(QWidget *parent);
    QWidget *CreateCQHistogramControls(QWidget *parent);
    QGroupBox *CreateTimeControls(QWidget *parent);

    // Widgets and layouts.
    QCheckBox          *automaticallyApply;
    QListWidget        *selectionListBox;
    QPushButton        *newButton;
    QPushButton        *deleteButton;
    QPushButton        *loadButton;
    QPushButton        *saveButton;
    QPushButton        *updateSelectionButton;
    QPushButton        *updateQueryButton1;
    QPushButton        *updateQueryButton2;

    QTabWidget         *editorTabs;

    QLabel             *plotNameLabel;

    QGroupBox               *cqControls;
    QTabWidget              *cqTabs;
    QvisVariableButton      *cqVarButton;
    QPushButton             *cqInitializeVarButton;
    QvisVariableListLimiter *cqLimits;
    QGroupBox               *cqTimeGroupBox;
    QLineEdit               *cqTimeMin;
    QLineEdit               *cqTimeMax;
    QLineEdit               *cqTimeStride;

    QvisHistogram           *cqHistogram;
    QLabel                  *cqHistogramTitle;
    QButtonGroup            *cqHistogramType;
    QRadioButton            *cqHistogramVariableButton;
    QComboBox               *cqHistogramVariable;
    QLabel                  *cqHistogramNumBinsLabel;
    QSpinBox                *cqHistogramNumBins;
    QLabel                  *cqHistogramMinLabel;
    QSpinBox                *cqHistogramMin;
    QLabel                  *cqHistogramMaxLabel;
    QSpinBox                *cqHistogramMax;
    QComboBox               *cqSummation;

    QTableWidget            *statVars;
    QLabel                  *statSelectedCells;
    QLabel                  *statTotalCells;

    // State information
    SelectionList      *selectionList;
    PlotList           *plotList;
    EngineList         *engineList;
    WindowInformation  *windowInformation;

    bool                selectionPropsValid;
    SelectionProperties selectionProps;

    int                 selectionCounter;
};

#endif
