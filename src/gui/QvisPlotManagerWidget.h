#ifndef QVIS_PLOT_MANAGER_WIDGET_H
#define QVIS_PLOT_MANAGER_WIDGET_H
#include <gui_exports.h>
#include <vector>
#include <string>
#include <qwidget.h>
#include <qstringlist.h>
#include <GUIBase.h>
#include <SimpleObserver.h>
#include <VariableMenuPopulator.h>

// Forward declarations.
class  ViewerProxy;
class  PlotList;
class  ExpressionList;
class  FileServerList;
class  GlobalAttributes;
class  PluginManagerAttributes;
class  QGridLayout;
class  QLabel;
class  QListBoxItem;
class  QCheckBox;
class  QMenuBar;
class  QPopupMenu;
class  QPushButton;
struct QualifiedFilename;
class  QvisPlotListBox;
class  QvisVariablePopupMenu;

// Some typedefs used for plugin management.
typedef struct
{
    QvisVariablePopupMenu *varMenu;
    int                   varTypes;
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
// ****************************************************************************

class GUI_API QvisPlotManagerWidget : public QWidget, public GUIBase,
    public SimpleObserver
{
    Q_OBJECT
public:
    QvisPlotManagerWidget(QWidget *parent = 0, const char *name = 0);
    ~QvisPlotManagerWidget();
    virtual void Update(Subject *);
    virtual void SubjectRemoved(Subject *);
    void ConnectPlotList(PlotList *);
    void ConnectFileServer(FileServerList *);
    void ConnectGlobalAttributes(GlobalAttributes *);
    void ConnectExpressionList(ExpressionList *);
    void ConnectPluginManagerAttributes(PluginManagerAttributes *);

    void AddPlotType(const char *plotName, const int varTypes,
                     const char **iconData = 0);
    void AddOperatorType(const char *operatorName, const char **iconData = 0);
    void EnablePluginMenus();
signals:
    void activateSubsetWindow();
    void activatePlotWindow(int index);
    void activateOperatorWindow(int index);
protected:
    virtual void keyReleaseEvent(QKeyEvent *key);
private:
    void CreateMenus();
    void UpdatePlotList();
    void PopulateVariableLists(const QualifiedFilename &filename);
    void UpdatePlotVariableMenu();
    void UpdateVariableMenu();
    void UpdatePlotAndOperatorMenuEnabledState() const;

private slots:
    void setActivePlots();
    void hidePlots();
    void deletePlots();
    void drawPlots();
    void changeVariable(int, const QString &varName);
    void promoteOperator(int operatorIndex);
    void demoteOperator(int operatorIndex);
    void removeOperator(int operatorIndex);

    void addPlot(int plotType, const QString &varName);
    void operatorAction(int);
    void applyOperatorToggled(bool val);
private:
    // Plot manager widgets
    QGridLayout             *topLayout;
    QLabel                  *activePlots;
    QvisPlotListBox         *plotListBox;
    QPushButton             *hideButton;
    QPushButton             *deleteButton;
    QPushButton             *drawButton;
    QCheckBox               *applyOperatorToggle;

    // Menu widgets
    QMenuBar                *plotMenuBar;
    QPopupMenu              *plotMenu;
    int                      plotMenuId;
    QPopupMenu              *plotAttsMenu;
    int                      plotAttsMenuId;
    QPopupMenu              *operatorAttsMenu;
    int                      operatorAttsMenuId;
    QvisVariablePopupMenu   *varMenu;
    int                      varMenuId;
    QPopupMenu              *operatorMenu;
    int                      operatorMenuId;

    VariableMenuPopulator    menuPopulator;

    // Structures to keep track of the registered plugin types.
    PluginEntryVector        plotPlugins;
    bool                     pluginsLoaded;

    // State objects that this window observes.
    PlotList                *plotList;
    GlobalAttributes        *globalAtts;
    ExpressionList          *exprList;
    PluginManagerAttributes *pluginAtts;
};

#endif
