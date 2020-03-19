// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_PLOT_MANAGER_WIDGET_H
#define QVIS_PLOT_MANAGER_WIDGET_H
#include <gui_exports.h>
#include <vector>
#include <string>

#include <QAction>
#include <QMap>
#include <QMenu>
#include <QStringList>
#include <QWidget>
#include <QGroupBox>

#include <GUIBase.h>
#include <SimpleObserver.h>
#include <VariableMenuPopulator.h>

// Forward declarations.
class  Plot;
class  PlotList;
class  ExpressionList;
class  FileServerList;
class  GlobalAttributes;
class  PluginManagerAttributes;
class  SelectionList;
class  WindowInformation;

class  QComboBox;
class  QGridLayout;
class  QVBoxLayout;
class  QLabel;
class  QListWidgetItem;
class  QCheckBox;
class  QButtonGroup;
class  QRadioButton;
class  QGroupBox;
class  QMenuBar;
class  QMenu;
class  QPushButton;
struct QualifiedFilename;
class  QToolBar;
class  QToolButton;
class  QvisPlotListBox;
class  QvisVariablePopupMenu;

// Some typedefs used for plugin management.
typedef struct
{
    QString                id;
    QString                pluginName;
    QString                menuName;
    QIcon                  icon;
    QvisVariablePopupMenu *varMenu;
    int                    varTypes;
    int                    varMask;
    QAction               *action;
    QAction               *menuAction;
} PluginEntry;

typedef std::vector<PluginEntry> PluginEntryVector;

// ****************************************************************************
// Class: QvisPlotManagerWidget
//
// Purpose:
//   This widget observes the viewer's plot list and displays it.
//
// Notes:
//   The widget updates whenever the viewer's plot list changes.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 6 20:27:20 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Sep 11 16:17:30 PST 2000
//   I changed the class so it inherits from SimpleObserver too. It
//   needs to also watch the FileServer.
//
//   Hank Childs, Tue Jan 16 15:35:51 PST 2001
//   Add signals for volume plots.
//
//   Brad Whitlock, Fri Mar 23 17:02:15 PST 2001
//   Modified to handle treatment of plots and operators as generic plugins.
//
//   Jeremy Meredith, Wed Sep  5 14:05:07 PDT 2001
//   Added plugin manager attributes.
//
//   Kathleen Bonnell, Wed Oct 17 15:03:25 PDT 2001 
//   Added method ParseVariable, member subsetVars.
//
//   Eric Brugger, Thu Nov 29 12:51:40 PST 2001
//   I removed the private data member matVars.
//
//   Jeremy Meredith, Tue Dec 18 10:32:13 PST 2001
//   Added some species var support.
//
//   Brad Whitlock, Wed Feb 6 10:50:40 PDT 2002
//   Changed the type of the var menu.
//
//   Brad Whitlock, Wed Feb 6 16:25:07 PST 2002
//   Removed operatorCheckbox and added vars used to delay when the plot
//   and operator windows become enabled.
//
//   Brad Whitlock, Mon Mar 4 14:12:21 PST 2002
//   Added AutoUpdate support.
//
//   Brad Whitlock, Thu May 2 15:03:19 PST 2002
//   Made it inherit from GUIBase so it automatically gets better support
//   for application-wide settings.
//
//   Brad Whitlock, Thu May 9 13:32:46 PST 2002
//   Made a couple of the widgets class members.
//
//   Brad Whitlock, Thu May 9 16:47:36 PST 2002
//   Removed fileServer since it is now a static member of the base class.
//
//   Brad Whitlock, Tue May 14 14:00:06 PST 2002
//   Added an override of the virtual keyReleaseEvent method.
//
//   Brad Whitlock, Tue Aug 20 17:19:55 PST 2002
//   I added an internal method to split variable names.
//
//   Kathleen Bonnell, Wed Sep  4 16:14:12 PDT 2002 
//   Removed method ParseVariable. 
//
//   Brad Whitlock, Thu Mar 13 09:28:18 PDT 2003
//   I added support for icons in the plot and operator menus. I also removed
//   all of the code that was responsible for creating cascading menus and
//   put it in the new VariableMenuPopulator class which this widget now uses.
//
//   Brad Whitlock, Tue Apr 8 10:43:10 PDT 2003
//   I enhanced the widget so it supports moving operators around in a pipeline.
//
//   Brad Whitlock, Mon Jul 28 17:21:17 PST 2003
//   Added UpdatePlotAndOperatorMenuEnabledState.
//
//   Brad Whitlock, Wed Sep 10 09:07:02 PDT 2003
//   Added UpdateHideDeleteDrawButtonsEnabledState.
//
//   Brad Whitlock, Fri Aug 15 15:06:56 PST 2003
//   Added the menu bar pointer in the constructor.
//
//   Brad Whitlock, Thu Jan 29 19:52:34 PST 2004
//   Added the active source combo box.
//
//   Brad Whitlock, Wed Feb 25 11:16:23 PDT 2004
//   Added members to help speed up menu creation and update.
//
//   Jeremy Meredith, Wed Aug 25 11:12:53 PDT 2004
//   Have it observe updating metadata directly.
//
//   Brad Whitlock, Tue Dec 14 09:11:08 PDT 2004
//   Added addPlot, addOperator signals so the code to actually perform
//   those actions can be moved out of this class.
//
//   Brad Whitlock, Tue Apr 25 16:29:44 PST 2006
//   Added support for operators that influence the plot variable menu.
//
//   Ellen Tarwate, Fri, May 25
//   adding Context Menu to the Active Plots list
//   hideThisPlot, deleteThisPlot, drawThisPlot,
//   cloneThisPlot, clearThisPlot, redrawThisPlot,
//   copyToWinThisPlot, disconnectThisPlot...
//
//   Brad Whitlock, Thu Dec 20 11:05:48 PST 2007
//   Added methods to make recreating the plot menu easier.
//
//   Gunther H. Weber, Mon Jan 28 15:35:16 PST 2008
//   Split "Apply operators and selections ..." checkbox into an apply
//   operators and an apply selection checkbox.
//
//   Brad Whitlock, Fri Apr 25 10:22:11 PDT 2008
//   Use QString for plot and operator names to support internationalization.
//
//   Cyrus Harrison, Thu Jul  3 09:16:15 PDT 2008
//   Initial Qt4 Port.
//
//   Brad Whitlock, Tue Sep  9 10:40:33 PDT 2008
//   Removed metaData and pluginAtts since they were not used.
//
//   Jeremy Meredith, Thu Feb  4 17:05:30 EST 2010
//   Made UpdatePlotList public, since it needs to be triggered externally;
//   this is used e.g. when you disable selected files to show full filenames.
//
//   Brad Whitlock, Fri Feb  5 16:37:10 PST 2010
//   I added support for grouping operators.
//
//   Jeremy Meredith, Fri Feb 19 20:35:02 EST 2010
//   Big redesign, adding icons and functionality and shuffling arrangement.
//
//   Cyrus Harrison, Thu Feb 25 13:47:45 PST 2010
//   Change from QPushButtons QToolbar/QToolButtons
//
//   Cyrus Harrison, Mon Mar 15 11:57:22 PDT 2010
//   Moved source related controls into QvisSourceManagerWidget.
//
//   Brad Whitlock, Fri May  7 14:05:19 PDT 2010
//   I transplanted some methods.
//
//   Brad Whitlock, Fri Jul 23 15:23:49 PDT 2010
//   I made the widget observe the selection list.
//
//   Brad Whitlock, Fri Sep 13 12:29:07 PDT 2013
//   Add animateThisPlot method.
//
//   Mark C. Miller, Tue May  8 18:35:59 PDT 2018
//   Adjust default values for destroyMenuItems and forceUpdate in calls
//   where they are used to match old logic.
// ****************************************************************************

class GUI_API QvisPlotManagerWidget : public QGroupBox, public GUIBase,
    public SimpleObserver
{
    Q_OBJECT
public:
    QvisPlotManagerWidget(QMenuBar *menuBar, QWidget *parent = 0);
    ~QvisPlotManagerWidget();
    virtual void Update(Subject *);
    virtual void SubjectRemoved(Subject *);
    void ConnectPlotList(PlotList *);
    void ConnectFileServer(FileServerList *);
    void ConnectGlobalAttributes(GlobalAttributes *);
    void ConnectExpressionList(ExpressionList *);
    void ConnectWindowInformation(WindowInformation *);
    void ConnectPluginManagerAttributes(PluginManagerAttributes *);
    void ConnectSelectionList(SelectionList *);

    void AddPlotType(const QString &id, const QString &plotName, const int varTypes,
                     const char **iconData = 0);
    void AddOperatorType(const QString &is, const QString &operatorName,
                         const int varTypes, const int varMask, bool userSelectable,
                         const char **iconData = 0);
    void FinishAddingOperators();
    void EnablePluginMenus();
    void UpdateOperatorCategories();
    void SetSourceVisible(bool);

    QMenu *getOperatorMenu() { return operatorMenu; };
    QMenu *getVariableMenu() { return (QMenu*) varMenu; };

public slots:
    void UpdatePlotList();
signals:
    void activateSubsetWindow();
    void activateSelectionsWindow(const QString &);
    void activatePlotWindow(int);
    void activateOperatorWindow(int);
    void addPlot(int, const QString &);
    void addOperator(int);
protected:
    virtual void keyPressEvent(QKeyEvent *key);
private:
    void CreateMenus(QMenuBar *);
    void DestroyPlotMenuItem(int index);
    void CreatePlotMenuItem(int index);
    void DestroyVariableMenu();
    void CreateVariableMenu();
    bool PopulateVariableLists(VariableMenuPopulator &,
                               const QualifiedFilename &filename);
    void UpdatePlotVariableMenu(bool destroyMenuItems=true, bool forceUpdate=false);
    void UpdateVariableMenu(bool destroyMenuItems=true, bool forceUpdate=false);
    void UpdatePlotAndOperatorMenuEnabledState();
    void UpdateHideDeleteDrawButtonsEnabledState() const;


private slots:
    void hideThisPlot();
    void deleteThisPlot();
    void drawThisPlot();
    void clearThisPlot();
    void cloneThisPlot();
    void redrawThisPlot();
    void followTimeSliderThisPlot(bool);
    void animateThisPlot(bool val);
    void setActivePlot();

    void setActivePlots();
    void hidePlots();
    void deletePlots();
    void drawPlots();
    void changeVariable(int, const QString &varName);
    void promoteOperator(int operatorIndex);
    void demoteOperator(int operatorIndex);
    void removeOperator(int operatorIndex);

    void activatePlotWindow(QAction *);
    void activateOperatorWindow(QAction *);

    void setPlotDescription(int, const QString &);
    void moveThisPlotTowardFirst();
    void moveThisPlotTowardLast();
    void makeThisPlotFirst();
    void makeThisPlotLast();

    void addPlotHelper(int plotType, const QString &varName);
    void operatorAction(QAction *);

    void applyWindowChanged(int val);
    void applyOperatorToggled(bool val);
    void applySelectionToggled(bool val);
private:
    bool                    sourceVisible;
    // Plot manager widgets
    QVBoxLayout             *topLayout;
    QvisPlotListBox         *plotListBox;

    // Menu widgets
    QMenu                   *plotMenu;
    QvisVariablePopupMenu   *varMenu;
    QMenu                   *operatorMenu;

    QMenu                   *plotAttsMenu;
    QAction                 *plotAttsMenuAct;
    QMenu                   *operatorAttsMenu;
    QAction                 *operatorAttsMenuAct;
    QAction                 *operatorRemoveLastAct;
    QAction                 *operatorRemoveAllAct;

    QWidget                 *applyWindow;
    QLabel                  *applyWindowLabel;
    QButtonGroup            *applyWindowButtonGroup;
    QRadioButton            *applyWindowActiveRadioButton;
    QRadioButton            *applyWindowAllRadioButton;
    QCheckBox               *applyOperatorCheckBox;
    QCheckBox               *applySelectionCheckBox;

    bool                     updatePlotVariableMenuEnabledState;
    bool                     updateOperatorMenuEnabledState;
    bool                     updateVariableMenuEnabledState;
    int                      maxVarCount;
    int                      varMenuFlags;

    VariableMenuPopulator    menuPopulator, varMenuPopulator;

    // Structures to keep track of the registered plugin types.
    PluginEntryVector        plotPlugins;
    PluginEntryVector        operatorPlugins;
    bool                     pluginsLoaded;

    // State objects that this window observes.
    PlotList                *plotList;
    GlobalAttributes        *globalAtts;
    ExpressionList          *exprList;
    WindowInformation       *windowInfo;
    SelectionList           *selectionList;

    // Various Toolbar actions for the plot manager
    QToolBar    *plotActionsToolbar;
    QAction     *plotAddMenuAction;
    QAction     *operMenuAction;
    QAction     *plotDeleteAction;
    QAction     *plotHideShowAction;
    QAction     *plotDrawAction;
    QAction     *varMenuAction;

    PluginManagerAttributes *pluginAtts;
};

#endif
