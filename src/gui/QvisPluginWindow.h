#ifndef QVIS_PLUGIN_WINDOW_H
#define QVIS_PLUGIN_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>
#include <vector>
#include <string>

// Forward declarations.
class PluginManagerAttributes;
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QTabWidget;
class QVBox;
class QListView;
class QCheckListItem;

// ****************************************************************************
// Class: QvisPluginWindow
//
// Purpose:
//   This window displays and manipulates the available plugins.
//
// Programmer: Jeremy Meredith
// Creation:   August 31, 2001
//
// Modifications:
//    Jeremy Meredith, Fri Sep 28 13:52:35 PDT 2001
//    Removed default settings capabilities.
//    Added a pluginSettingsChanged signal.
//    Added ability to keep track of which items are checked/unchecked.
//
// ****************************************************************************

class GUI_API QvisPluginWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisPluginWindow(PluginManagerAttributes *subj,
                     const char *caption = 0,
                     const char *shortName = 0,
                     QvisNotepadArea *notepad = 0);
    virtual ~QvisPluginWindow();
    virtual void CreateWindowContents();

    virtual void CreateNode(DataNode *);
    virtual void SetFromNode(DataNode *, const int *borders);
    virtual void Update(Subject *TheChangedSubject);
signals:
    void pluginSettingsChanged();
protected:
    virtual void UpdateWindow(bool doAll);
    void Apply(bool dontIgnore = false);
private slots:
    virtual void apply();
    void tabSelected(const QString &tabLabel);
private:
    PluginManagerAttributes *pluginAtts;

    QTabWidget      *tabs;
    QVBox           *pagePlots;
    QListView       *listPlots;
    QVBox           *pageOperators;
    QListView       *listOperators;
    QVBox           *pageDatabases;
    QListView       *listDatabases;

    std::vector<QCheckListItem*> plotItems;
    std::vector<std::string>     plotIDs;
    std::vector<QCheckListItem*> operatorItems;
    std::vector<std::string>     operatorIDs;

    int             activeTab;
    bool            pluginsInitialized;
};

#endif
