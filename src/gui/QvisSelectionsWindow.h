// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//   Brad Whitlock, Wed Oct 12 12:18:09 PDT 2011
//   Make histogram variable into a variable button instead of combo box.
//
// ****************************************************************************

class GUI_API QvisSelectionsWindow : public QvisPostableWindowSimpleObserver
{
    Q_OBJECT
public:
    QvisSelectionsWindow(const QString &caption = QString(),
                         const QString &shortName = QString(),
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
                            int minBin, int maxBin, bool useBins,
                            double minAxisValue, double maxAxisValue);
    void    UpdateHistogram();
    void    UpdateHistogramTitle();
    void    UpdateMinMaxBins(bool, bool, bool);
    void    Apply(bool forceUpdate, bool updatePlots, bool allowCache);
    void    GetCurrentValues(int);

    QString GetLoadHost() const;

    void    NewEnabled(QString &, QString &);

private slots:
    void    idVariableTypeChanged(int);
    void    idVariableChanged(const QString &);

    void    automaticallyApplyToggled(bool val);
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
    void    histogramVariableChanged(const QString &);
    void    histogramNumBinsChanged(int);
    void    histogramAutoScaleNumBinsToggled(bool val);
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
    QButtonGroup       *idVariableType;
    QvisVariableButton *idVariableButton;

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
    QLabel                  *cqHistogramMinAxisLabel;
    QLabel                  *cqHistogramMaxAxisLabel;
    QLabel                  *cqHistogramAxisLabel;
    QButtonGroup            *cqHistogramType;
    QRadioButton            *cqHistogramVariableButton;
    QvisVariableButton      *cqHistogramVariable;
    QLabel                  *cqHistogramNumBinsLabel;
    QSpinBox                *cqHistogramNumBins;
    QCheckBox               *cqHistogramAutoScaleNumBins;
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
    bool                allowCaching;

    int                 selectionCounter;
};

#endif
