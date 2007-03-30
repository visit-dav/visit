#ifndef QVIS_QUERY_WINDOW_H
#define QVIS_QUERY_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowSimpleObserver.h>
#include <vectortypes.h>

// Forward declarations
class QueryAttributes;
class PickAttributes;
class PlotList;
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
    void UpdateQueryList(bool doAll);
    void UpdateResults(bool doAll);
    void UpdateArgumentPanel(int index);

    void Apply(bool ignore = false);
    bool GetPoint(int index, const QString &pname, int rep, double pt[3]);
    bool GetNumber(int index, int *num);
    bool GetVars(int index, stringVector &vars);
private slots:
    void apply();
    void handleText();
    void selectQuery();
    void clearResultText();
private:
    QueryList       *queries;
    QueryAttributes *queryAtts;
    PickAttributes  *pickAtts;
    PlotList        *plotList;

    QLabel          *coordLabel;
    QListBox        *queryList;
    QPushButton     *queryButton;
    QMultiLineEdit  *resultText;
    QGroupBox       *argPanel;
    QLabel          *labels[4];
    QLineEdit       *textFields[4];
};

#endif

