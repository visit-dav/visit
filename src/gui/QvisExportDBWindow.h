// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_EXPORT_DB_WINDOW_H
#define QVIS_EXPORT_DB_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowSimpleObserver.h>

// Forward declarations.
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSlider;
class QSpinBox;
class QvisVariableButton;
class ExportDBAttributes;
class DBPluginInfoAttributes;
class PlotList;

// ****************************************************************************
// Class: QvisExportDBWindow
//
// Purpose:
//   This class contains the code necessary to create a window for exporting
//   databases.
//
// Notes:      
//
// Programmer: Hank Childs
// Creation:   May 25, 2005
//
// Modifications:
//   Brad Whitlock, Mon Jun 27 13:51:01 PST 2005
//   Added a directory selection button.
//
//   Brad Whitlock, Thu Nov 2 18:01:00 PST 2006
//   I made a widget be a class member so I can disable it.
//
//   Jeremy Meredith, Tue Jul 17 11:29:24 EDT 2007
//   Converted to a simple observer so we can watch not only the 
//   export atts but also the DB plugin info atts.
//
//   Brad Whitlock, Wed Apr  9 11:57:54 PDT 2008
//   QString for caption, shortName.
//
//   Brad Whitlock, Thu Mar 20 13:52:11 PDT 2014
//   Improvements for delimiters and for showing licenses.
//   Work partially supported by DOE Grant SC0007548.
//
//   Kathleen Biagas, Wed Apr 23 14:46:51 MST 2014
//   Add plotList.
//
//   Brad Whitlock, Thu Jul 24 13:55:34 EDT 2014
//   Added check box for doing all time steps.
//
//   Brad Whitlock, Mon Aug 10 17:16:32 PDT 2015
//   Add support for grouping.
//
// ****************************************************************************

class GUI_API QvisExportDBWindow : public QvisPostableWindowSimpleObserver
{
    Q_OBJECT
public:
    QvisExportDBWindow(const QString &caption = QString(),
                       const QString &shortName = QString(),
                       QvisNotepadArea *notepad = 0);
    virtual ~QvisExportDBWindow();

    virtual void Update(Subject *TheChangedSubject);
    virtual void SubjectRemoved(Subject *TheRemovedSubject);
    virtual void ConnectSubjects(ExportDBAttributes *edb,
                                 DBPluginInfoAttributes *dbp,
                                 PlotList *pl);
public slots:
    virtual void apply();
protected:
    virtual void CreateWindowContents();
    void UpdateWindow(bool doAll);
    void GetCurrentValues(int which_widget);
    void Apply(bool ignore = false);
    QString Delimiter() const;
    void UpdateVariablesList();
protected slots:
    void variableProcessText();
    void processFilenameText();
    void processDirectoryNameText();
    void fileFormatChanged(int);
    void exportDB();
    void exportButtonClicked();
    void addVariable(const QString &);
    void selectOutputDirectory();
    void delimiterChanged(int);
    void allTimesToggled(bool);
    void processTimeStateFormatText();
    void coordinateGroupsToggled(bool);
    void groupSizeChanged(int);
private:
    DBPluginInfoAttributes *dbPluginInfoAtts;
    ExportDBAttributes     *exportDBAtts;
    PlotList               *plotList;
    bool                    localPlot;

    int                     delimiter;
    QStringList             licenseShown;

    QLineEdit           *filenameLineEdit;
    QLineEdit           *directoryNameLineEdit;
    QPushButton         *directorySelectButton;
    QLabel              *directoryNameLabel;
    QComboBox           *fileFormatComboBox;
    QvisVariableButton  *varsButton;
    QLineEdit           *varsLineEdit;
    QCheckBox           *allTimes;
    QLabel              *timeStateFormatLabel;
    QLineEdit           *timeStateFormatEdit;
    QCheckBox           *coordinateGroups;
    QLabel              *groupSizeLabel;
    QSpinBox            *groupSize;
};

#endif
