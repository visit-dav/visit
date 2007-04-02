#ifndef QVIS_COMMAND_WINDOW_H
#define QVIS_COMMAND_WINDOW_H
#include <QvisPostableWindow.h>

class QButtonGroup;
class QPushButton;
class QTabWidget;
class QTextEdit;

// ****************************************************************************
// Class: QvisCommandWindow
//
// Purpose:
//   This class implements a window that lets you type commands to be
//   interpreted.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon May 9 10:20:44 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

class QvisCommandWindow : public QvisPostableWindow
{
    Q_OBJECT
public:
    QvisCommandWindow(const char *captionString = 0,
                      const char *shortName = 0,
                      QvisNotepadArea *n = 0);
    virtual ~QvisCommandWindow();
    virtual void CreateWindowContents();
    virtual void CreateNode(DataNode *);
signals:
    void runCommand(const QString &);
private slots:
    void executeClicked(int);
    void clearClicked(int);

    void textChanged0();
    void textChanged1();
    void textChanged2();
    void textChanged3();
    void textChanged4();
    void textChanged5();
    void textChanged6();
    void textChanged7();
private:
    QString fileName(int index) const;
    void LoadScripts();
    void SaveScripts();

    QTabWidget      *tabWidget;
    QButtonGroup    *executeButtonsGroup;
    QPushButton    **executeButtons;
    QButtonGroup    *clearButtonsGroup;
    QPushButton    **clearButtons;
    QTextEdit      **lineEdits;
};

#endif
