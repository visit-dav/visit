// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_MOVIE_PROGRESS_DIALOG_H
#define QVIS_MOVIE_PROGRESS_DIALOG_H
#include <QDialog>
#include <QLabel>
#include <QProgressBar>

class QPixmap;
class QPushButton;

// ****************************************************************************
// Class: QvisMovieProgressDialog
//
// Purpose:
//   A movie progress dialog.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 20 15:07:09 PST 2005
//
// Modifications:
//   Cyrus Harrison, Tue Jul  1 09:14:16 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

class QvisMovieProgressDialog : public QDialog
{
    Q_OBJECT
public:
    QvisMovieProgressDialog(QPixmap &pix, QWidget *parent = 0);
    virtual ~QvisMovieProgressDialog();

    void setLabelText(const QString &t) { labelTextLabel->setText(t); }
    QString labelText() const           { return labelTextLabel->text(); }

    void setProgress(int val);
    int  progress() const               { return progressBar->value(); }

    void setTotalSteps(int val)         { progressBar->setMaximum(val); }
    int  totalSteps() const             { return progressBar->maximum(); }
signals:
    void cancelled();
private slots:
    void cancelClicked();
private:
    QLabel       *picture;
    QLabel       *labelTextLabel;
    QProgressBar *progressBar;
    QPushButton  *cancelButton;
};

#endif
