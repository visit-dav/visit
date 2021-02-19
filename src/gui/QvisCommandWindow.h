// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_COMMAND_WINDOW_H
#define QVIS_COMMAND_WINDOW_H
#include <QvisPostableWindow.h>

class QButtonGroup;
class QCheckBox;
class QComboBox;
class QPushButton;
class QTabWidget;
class QTextEdit;
class QSyntaxHighlighter;
class QVBox;

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
//   Brad Whitlock, Fri Jan 6 13:35:40 PST 2006
//   Added new buttons for recording macros.
//
//   Brad Whitlock, Fri Jun 15 13:31:37 PST 2007
//   Added a new tab for visitrc macros.
//
//   Brad Whitlock, Wed Apr  9 11:35:25 PDT 2008
//   QString for captionString, shortName.
//
//   Cyrus Harrison, Tue Jun 10 15:00:05 PDT 2008
//   Initial Qt4 Port.
//
//   Cyrus Harrison, Mon Feb  8 15:01:39 PST 2010
//   Added syntax highlighters.
//
// ****************************************************************************

class QvisCommandWindow : public QvisPostableWindow
{
    Q_OBJECT
public:
    QvisCommandWindow(const QString &captionString = QString(),
                      const QString &shortName = QString(),
                      QvisNotepadArea *n = 0);
    virtual ~QvisCommandWindow();
    virtual void CreateWindowContents();
    virtual void CreateNode(DataNode *);
    virtual void SetFromNode(DataNode *, const int *borders);
signals:
    void runCommand(const QString &);
public slots:
    void acceptRecordedMacro(const QString &);
private slots:
    void executeClicked(int);
    void clearClicked(int);

    void macroRecordClicked();
    void macroPauseClicked();
    void macroEndClicked();
    void macroAppendClicked(bool);
    void macroStorageActivated(int);
    void macroClearClicked();
    void macroUpdateClicked();
    void macroCreate(int);

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
    QString RCFileName() const;
    void LoadScripts();
    void SaveScripts();
    void UpdateMacroCheckBoxes();
    void CreateMacroFromText(const QString &);

    QTabWidget      *tabWidget;
    QButtonGroup    *executeButtonsGroup;
    QPushButton    **executeButtons;
    QButtonGroup    *clearButtonsGroup;
    QPushButton    **clearButtons;
    QButtonGroup    *addMacroButtonsGroup;
    QPushButton    **addMacroButtons;

    QTextEdit          **editors;
    QSyntaxHighlighter **highlighters;

    QPushButton     *macroRecord;
    QPushButton     *macroPause;
    QPushButton     *macroEnd;
    QCheckBox       *macroAppendCheckBox;
    QComboBox       *macroStorageComboBox;

    QWidget            *macroTab;
    QTextEdit          *macroEdit;
    QPushButton        *macroUpdateButton;
    QPushButton        *macroClearButton;
    QSyntaxHighlighter *macroHighlighter;

    int              macroStorageMode;
    bool             macroAppend;
    int              maxUserMacro;
};

#endif
