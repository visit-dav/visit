#ifndef QVIS_DATABASECORRELATIONLIST_WINDOW_H
#define QVIS_DATABASECORRELATIONLIST_WINDOW_H
#include <QvisPostableWindowObserver.h>
#include <map>

class DatabaseCorrelationList;
class QCheckBox;
class QComboBox;
class QListBox;
class QPushButton;

// ****************************************************************************
// Class: QvisDatabaseCorrelationListWindow
//
// Purpose:
//   This window allows users to create new database correlations and edit
//   existing correlations.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 30 15:14:01 PST 2004
//
// Modifications:
//   
// ****************************************************************************

class GUI_API QvisDatabaseCorrelationListWindow :
    public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisDatabaseCorrelationListWindow(DatabaseCorrelationList *cL,
                                      const char *caption = 0,
                                      const char *shortName = 0,
                                      QvisNotepadArea *notepad = 0);

    virtual ~QvisDatabaseCorrelationListWindow();

public slots:
    virtual void showMinimized();
    virtual void showNormal();
protected slots:
    void apply();
    void newCorrelation();
    void editCorrelation();
    void deleteCorrelation();
    void highlightCorrelation(int);
    void promptUserChecked(bool);
    void whenToCorrelateChanged(int);
    void defaultCorrelationMethodChanged(int);

    void deleteWindow(QvisWindowBase *);
    void delayedDeleteWindows();
protected:
    virtual void UpdateWindow(bool doAll);
    virtual void CreateWindowContents();
    void UpdateButtons();
    void Apply(bool = false);

    int                                highlightedCorrelation;
    int                                newCorrelationCounter;
    std::map<std::string, std::string> nameMap;

    WindowBaseVector                   activeCorrelationWindows;
    WindowBaseVector                   windowsToDelete;

    DatabaseCorrelationList *correlationList;

    // Widgets and layouts.
    QPushButton             *newButton;
    QPushButton             *editButton;
    QPushButton             *deleteButton;
    QListBox                *correlationListBox;
    QCheckBox               *promptUser;
    QComboBox               *whenToCorrelate;
    QComboBox               *defaultCorrelationMethod;
};

#endif
