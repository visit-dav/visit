// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_SOURCE_MANAGER_WIDGET_H
#define QVIS_SOURCE_MANAGER_WIDGET_H
#include <gui_exports.h>

#include <QAction>
#include <QMap>
#include <QStringList>
#include <QWidget>
#include <QGroupBox>

#include <GUIBase.h>
#include <SimpleObserver.h>


// Forward declarations.
class  PlotList;
class  FileServerList;
class  GlobalAttributes;
class  WindowInformation;

class  QComboBox;
class  QGridLayout;
class  QLabel;
class  QToolBar;
class  QToolButton;

// ****************************************************************************
// Class: QvisSourceManagerWidget
//
// Purpose:
//      Holds widgets that provide source selection and manipulation.
//
// Note:
//   These widgets were moved out of QvisPlotManager into this class.
//
// Programmer: Cyrus Harrison
// Creation:   Fri Mar 12 10:55:05 PST 2010
//
// Modifications:
//
// ****************************************************************************

class GUI_API QvisSourceManagerWidget : public QGroupBox, public GUIBase,
    public SimpleObserver
{
    Q_OBJECT
public:
    QvisSourceManagerWidget(QWidget *parent = 0);
    ~QvisSourceManagerWidget();

    virtual void Update(Subject *);
    virtual void SubjectRemoved(Subject *);

    void ConnectPlotList(PlotList *);
    void ConnectGlobalAttributes(GlobalAttributes *);
    void ConnectFileServer(FileServerList *);
    void ConnectWindowInformation(WindowInformation *);

signals:
    void activateFileOpenWindow();

private:
    void UpdateSourceList(bool updateActiveSourceOnly);
    void UpdateDatabaseIconEnabledStates();

private slots:
    void closeCurrentSource();
    void reOpenCurrentSource();
    void replaceWithCurrentSource();
    void overlayWithCurrentSource();
    void sourceChanged(int);

private:
    // State objects that this widget observes.
    PlotList                *plotList;
    GlobalAttributes        *globalAtts;
    WindowInformation       *windowInfo;

    QGridLayout  *topLayout;

    QToolBar     *dbActionsToolbar;
    QAction      *dbOpenAction;
    QAction      *dbReopenAction;
    QAction      *dbCloseAction;
    QAction      *dbReplaceAction;
    QAction      *dbOverlayAction;

    QLabel       *sourceLabel;
    QComboBox    *sourceComboBox;
};

#endif
