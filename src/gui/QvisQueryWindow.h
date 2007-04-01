#ifndef QVIS_QUERY_WINDOW_H
#define QVIS_QUERY_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowSimpleObserver.h>
#include <vectortypes.h>

// Forward declarations
class QueryAttributes;
class PickAttributes;
class PlotList;
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QListBox;
class QMultiLineEdit;
class QPushButton;
class QueryList;

// ****************************************************************************
// Class: QvisQueryWindow
//
// Purpose:
//   This class displays the list of available queries and
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 6 15:41:05 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Mon Sep 30 14:38:33 PDT 2002 
//   Add queryAtts. 
//
//   Brad Whitlock, Thu Dec 26 17:46:18 PST 2002
//   I changed the interface to GetVars.
//
//   Brad Whitlock, Fri May 9 17:14:54 PST 2003
//   I added a button to clear the window.
//
//   Kathleen Bonnell, Tue Dec  2 08:39:46 PST 2003 
//   Added a button to specify whether the query should apply to the current
//   plot or the database. 
//
//   Kathleen Bonnell, Thu Apr  1 18:42:52 PST 2004
//   Added button in support of creating queries over time.
//
//   Kathleen Bonnell, Sat Sep  4 11:49:58 PDT 2004 
//   Added displayMode combo box, Removed unneeded argument from 
//   UpdateQueryList, changed arg in UpdateArgumentPanel from int to QString.
//   All to allow query lists to be sorted and grouped functionally.
//
//   Kathleen Bonnell, Wed Sep  8 10:06:16 PDT 2004 
//   Removed coordLabel, removed 'rep' arg from GetPoint.
//
//   Kathleen Bonnell, Wed Dec 15 17:16:17 PST 2004 
//   Added 'useGlobal' checkbox.
//
// ****************************************************************************

class GUI_API QvisQueryWindow : public QvisPostableWindowSimpleObserver
{
    Q_OBJECT
public:
    QvisQueryWindow(const char *caption = 0,
                    const char *shortName = 0,
                    QvisNotepadArea *n = 0);
    virtual ~QvisQueryWindow();
    virtual void CreateWindowContents();
    virtual void SubjectRemoved(Subject *TheRemovedSubject);

    void ConnectQueryList(QueryList *q);
    void ConnectQueryAttributes(QueryAttributes *p);
    void ConnectPickAttributes(PickAttributes *p);
    void ConnectPlotList(PlotList *pl);
protected:
    virtual void UpdateWindow(bool doAll);
    virtual void CreateEntireWindow();
private:
    void UpdateQueryButton();
    void UpdateTimeQueryButton();
    void UpdateQueryList();
    void UpdateResults(bool doAll);
    void UpdateArgumentPanel(const QString &);

    void Apply(bool ignore = false, bool doTime = false);
    bool GetPoint(int index, const QString &pname, double pt[3]);
    bool GetNumber(int index, int *num);
    bool GetVars(int index, stringVector &vars);
private slots:
    void apply();
    void timeApply();
    void handleText();
    void selectQuery();
    void clearResultText();
    void displayModeChanged(int);

private:
    QueryList       *queries;
    QueryAttributes *queryAtts;
    PickAttributes  *pickAtts;
    PlotList        *plotList;

    QComboBox       *displayMode;
    QListBox        *queryList;
    QPushButton     *queryButton;
    QPushButton     *timeQueryButton;
    QMultiLineEdit  *resultText;
    QGroupBox       *argPanel;
    QLabel          *labels[4];
    QLineEdit       *textFields[4];
    QCheckBox       *useGlobal;
  
    QButtonGroup    *dataOpts;
};

#endif

