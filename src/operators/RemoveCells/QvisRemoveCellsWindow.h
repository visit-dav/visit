// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISREMOVECELLSWINDOW_H
#define QVISREMOVECELLSWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class RemoveCellsAttributes;
class QLineEdit;
class QListWidget;

// ****************************************************************************
// Class: QvisRemoveCellsWindow
//
// Purpose: 
//   Defines QvisRemoveCellsWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Thu Jul 17 15:33:35 PST 2003
//
// Modifications:
//   Cyrus Harrison, Thu Aug 21 16:11:16 PDT 2008
//   Qt4 Port.
//
// ****************************************************************************

class QvisRemoveCellsWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisRemoveCellsWindow(const int type,
                         RemoveCellsAttributes *subj,
                         const QString &caption = QString(),
                         const QString &shortName = QString(),
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisRemoveCellsWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void cellProcessText();
    void domainProcessText();
    void addButtonClicked();
    void removeButtonClicked();
    void changeButtonClicked();
    void selectionChanged(int);
  private:
    void    ParseEntryString(const QString &str,int &cell, int &domain);
    QString CreateEntryString(int cell, int domain);
    void    GetCurrentCell(int &cell,int &domain);
        
    QLineEdit *cell;
    QLineEdit *domain;

    QListWidget  *cellList;

    QPushButton *addButton;
    QPushButton *removeButton;
    QPushButton *changeButton;

    RemoveCellsAttributes *atts;
};



#endif
