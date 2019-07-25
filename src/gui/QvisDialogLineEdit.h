// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_DIALOG_LINE_EDIT_H
#define QVIS_DIALOG_LINE_EDIT_H

#include <QWidget>
#include <gui_exports.h>

class QLineEdit;
class QPushButton;

// ****************************************************************************
// Class: QvisDialogLineEdit
//
// Purpose:
//   Incorporates a line edit with a file dialog.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 10 15:13:00 PST 2006
//
// Modifications:
//   Brad Whitlock, Fri Mar 16 15:01:10 PST 2007
//   Renamed the class and added the ChooseFont option.
//
//   Brad Whitlock, Thu Jun 19 11:23:38 PDT 2008
//   QT 4.
//
//   Kathleen Bonnell, Fri May 13 14:18:37 PDT 2011
//   Added fallbackPath.
//
//   Kathleen Biagas, Thu Dec 27 16:24:51 PST 2018
//   Added ChooseSaveFile dialog mode.
//
// ****************************************************************************

class GUI_API QvisDialogLineEdit : public QWidget
{
    Q_OBJECT
public:
    typedef enum {
        ChooseFile,
        ChooseLocalFile,
        ChooseSaveFile,
        ChooseDirectory,
        ChooseFont
    } DialogMode;

    QvisDialogLineEdit(QWidget *parent);
    virtual ~QvisDialogLineEdit();

    void setText(const QString &);
    QString text();
    QString displayText();

    void setDialogFilter(const QString &);
    void setDialogMode(DialogMode m);
    void setDialogCaption(const QString &);
    void setFallbackPath(const QString &);

signals:
    void returnPressed();
    void textChanged(const QString &);
protected:
    virtual void changeEvent(QEvent *);
private slots:
    void pushButtonClicked();
private:
    DialogMode   dialogMode;
    QString      dialogFilter;
    QString      dialogCaption;
    QLineEdit   *lineEdit;
    QPushButton *pushButton;
    QString      fallbackPath;
};

#endif
