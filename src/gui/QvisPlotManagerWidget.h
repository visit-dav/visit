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

#ifndef QVIS_PLOT_MANAGER_WIDGET_H
#define QVIS_PLOT_MANAGER_WIDGET_H
#include <gui_exports.h>
#include <qaction.h>
#include <qpopupmenu.h>
#include <vector>
#include <string>
#include <qwidget.h>
#include <qstringlist.h>
#include <GUIBase.h>
#include <SimpleObserver.h>
#include <VariableMenuPopulator.h>

// Forward declarations.
class  avtDatabaseMetaData;
class  PlotList;
class  ExpressionList;
class  FileServerList;
class  GlobalAttributes;
class  PluginManagerAttributes;
class  WindowInformation;

class  QComboBox;
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
    QString                pluginName;
    QString                menuName;
    QIconSet               icon;
    QvisVariablePopupMenu *varMenu;
    int                    varTypes;
    int                    varMask;
    int                    id;
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
//   hideThisPlot, deleteThisPlot, drawThisPlot
//
//   Brad Whitlock, Thu Dec 20 11:05:48 PST 2007
//   Added methods to make recreating the plot menu easier.
//
// ****************************************************************************

class GUI_API QvisPlotManagerWidget : public QWidget, public GUIBase,
    public SimpleObserver
{
    Q_OBJECT
public:
    QvisPlotManagerWidget(QMenuBar *menuBar, QWidget *parent = 0,
        const char *name = 0);
    ~QvisPlotManagerWidget();
    virtual void Update(Subject *);
    virtual void SubjectRemoved(Subject *);
    void ConnectPlotList(PlotList *);
    void ConnectFileServer(FileServerList *);
    void ConnectGlobalAttributes(GlobalAttributes *);
    void ConnectExpressionList(ExpressionList *);
    void ConnectPluginManagerAttributes(PluginManagerAttributes *);
    void ConnectWindowInformation(WindowInformation *);
    void ConnectDatabaseMetaData(avtDatabaseMetaData *);

    void AddPlotType(const char *plotName, const int varTypes,
                     const char **iconData = 0);
    void AddOperatorType(const char *operatorName, const int varTypes,
                         const int varMask, bool userSelectable,
                         const char **iconData = 0);
    void EnablePluginMenus();

    void SetSourceVisible(bool);
public slots:
    void hideThisPlot();
    void deleteThisPlot();
    void drawThisPlot();
    void clearThisPlot();
    void copyThisPlot();
    void copyToWinThisPlot();
    void redrawThisPlot();
    void setActivePlot();
    
signals:
    void activateSubsetWindow();
    void activatePlotWindow(int index);
    void activateOperatorWindow(int index);
    void addPlot(int, const QString &);
    void addOperator(int);
protected:
    virtual void keyReleaseEvent(QKeyEvent *key);
private:
    void CreateMenus(QMenuBar *);
    void DestroyPlotMenuItem(int index);
    void CreatePlotMenuItem(int index);
    void DestroyVariableMenu();
    void CreateVariableMenu();
    void UpdatePlotList();
    bool PopulateVariableLists(VariableMenuPopulator &,
                               const QualifiedFilename &filename);
    void UpdatePlotVariableMenu();
    void UpdateVariableMenu();
    void UpdateSourceList(bool updateActiveSourceOnly);
    void UpdatePlotAndOperatorMenuEnabledState();
    void UpdateHideDeleteDrawButtonsEnabledState() const;
private slots:
    void setActivePlots();
    void hidePlots();
    void deletePlots();
    void drawPlots();
    void copyPlotToWin(int winIndex);
    void changeVariable(int, const QString &varName);
    void promoteOperator(int operatorIndex);
    void demoteOperator(int operatorIndex);
    void removeOperator(int operatorIndex);

    void addPlotHelper(int plotType, const QString &varName);
    void operatorAction(int);
    void applyOperatorToggled(bool val);
    void sourceChanged(int);

private:
    bool                     sourceVisible;

    // Plot manager widgets
    QGridLayout             *topLayout;
    QLabel                  *sourceLabel;
    QComboBox               *sourceComboBox;
    QLabel                  *activePlots;
    QvisPlotListBox         *plotListBox;
    QPushButton             *hideButton;
    QPushButton             *deleteButton;
    QPushButton             *drawButton;
    QCheckBox               *applyOperatorToggle;
    //et QPopupMenu              *WindowChoiceMenu;  //et comming soon for context menu...
    //et QAction                 *win1Act;
    //et QAction                 *win2Act;

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
    avtDatabaseMetaData     *metaData;
    PlotList                *plotList;
    GlobalAttributes        *globalAtts;
    ExpressionList          *exprList;
    PluginManagerAttributes *pluginAtts;
    WindowInformation       *windowInfo;
    
};

#endif
