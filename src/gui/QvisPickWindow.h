#ifndef QVIS_PICK_WINDOW_H
#define QVIS_PICK_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>

#define MAX_PICK_TABS 8

class QCheckBox;
class QLineEdit;
class QMultiLineEdit;
class QTabWidget;
class QVBox;
class PickAttributes;

// ****************************************************************************
// Class: QvisPickWindow
//
// Purpose:
//   This class is a postable window that watches pick 
//   attributes and always presents the current state.
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   December 3, 2001 
//
// Modifications:
//   Brad Whitlock, Fri Feb 15 17:15:30 PST 2002
//   Changed the pages and infoLists arrays to be length MAX_PICK_TABS to
//   get around a weird memory problem where I could not seem to free those
//   arrays.
//
//   Kathleen Bonnell, Tue Mar 26 15:23:11 PST 2002  
//   Added ClearPages method. 
//
//   Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002  
//   Removed makeDefault and reset. 
//
//   Kathleen Bonnell, Fri Dec 27 14:09:40 PST 2002   
//   Added useNodeCoords checkbox.
//
//   Brad Whitlock, Wed Aug 27 08:35:44 PDT 2003
//   Added the autoShow flag and CreateNode and SetFromNode methods.
//
//   Brad Whitlock, Tue Sep 9 09:02:03 PDT 2003
//   I made it use QMultiLineEdit instead of QListBox.
//
// ****************************************************************************

class GUI_API QvisPickWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisPickWindow( PickAttributes *subj,
                    const char *caption = 0,
                    const char *shortName = 0,
                    QvisNotepadArea *notepad = 0);
    virtual ~QvisPickWindow();

    virtual void CreateNode(DataNode *);
    virtual void SetFromNode(DataNode *, const int *borders);

public slots:
    virtual void apply();
protected:
    virtual void CreateWindowContents();
    void UpdateWindow(bool doAll);
    void GetCurrentValues(int which_widget);
    void Apply(bool ignore = false);
private slots:
    void variableProcessText();
    void useNodeCoordsToggled(bool val);
    void logicalCoordsToggled(bool val);
    void autoShowToggled(bool);
private:
    void UpdatePage(void);
    void ClearPages(void);
    void AddInformation(QString &, const QString &, int &) const;

    bool            autoShow;
    int             nextPage;
    QString         lastLetter;

    QTabWidget     *tabWidget;
    QVBox          *pages[MAX_PICK_TABS];
    QMultiLineEdit *infoLists[MAX_PICK_TABS];
    QCheckBox      *useNodeCoords;
    QCheckBox      *logicalCoords;
    QCheckBox      *autoShowCheckBox;

    QLineEdit      *varsLineEdit;
    PickAttributes *pickAtts;
};
#endif
