// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_FILE_OPEN_DIALOG_H
#define QVIS_FILE_OPEN_DIALOG_H

#include <QvisFileOpenWindow.h>

// ****************************************************************************
// Class: QvisFileOpenDialog
//
// Purpose:
//   This is a dialog version of the file open window.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 15 13:28:12 PST 2006
//
// Modifications:
//   Brad Whitlock, Wed Apr  9 10:40:07 PDT 2008
//   Made captions use QString.
//
//   Kathleen Bonnell, Fri May 13 14:20:19 PDT 2011
//   Added getOpenFileNameWithFallbackPath, and added fallbackPath arg to
//   delayedChangePath and getOpenFileNamEx.
//
//   David Camp, Thu Aug 27 09:40:00 PDT 2015
//   Needed access to private functions for new Session dialog box.
//
// ****************************************************************************

class QvisFileOpenDialog : public QvisFileOpenWindow
{
    Q_OBJECT
public:
    static const int Accepted;
    static const int Rejected;

    QvisFileOpenDialog(const QString &caption);
    virtual ~QvisFileOpenDialog();

    // Blocking function to use the dialog to get a filename.
    static QString getOpenFileName(const QString &initialFile, 
                                   const QString &caption);

    static QString getOpenFileName(const QString &initialFile, 
                                   const QString &filter,
                                   const QString &caption);
    static QString getOpenFileNameWithFallbackPath(const QString &initialFile, 
                                   const QString &caption,
                                   const QString &fallbackPath);

    int exec();
    void setFilename(const QString &);
    QString getFilename() const;
signals:
    void quitloop();
private slots:
    void accept();
    void reject();
    void changeThePath();
    void userSelectedFile(const QString &s);
protected:
    void setResult(int);
    int result() const;
    void delayedChangePath(const QString &initialFile, const QString &fltr, const QString &fallbackPath);
    void restoreHostPathFilter(const QString &host,
                               const QString &path, const QString &filter);
    QString getOpenFileNameEx(const QString &initialFile, const QString &filter,
                              const QString &fallbacKPath = "");
    void done(int r);

private:
    QString filename;
    QString filter;
    QString fallbackPath;
    bool    in_loop;
    int     _result;
};

#endif
