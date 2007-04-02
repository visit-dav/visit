#ifndef QVIS_MOVIE_PROGRESS_DIALOG_H
#define QVIS_MOVIE_PROGRESS_DIALOG_H
#include <qdialog.h>
#include <qlabel.h>
#include <qprogressbar.h>

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
//   
// ****************************************************************************

class QvisMovieProgressDialog : public QDialog
{
    Q_OBJECT
public:
    QvisMovieProgressDialog(QWidget *parent = 0, const char *name = 0);
    virtual ~QvisMovieProgressDialog();

    void setLabelText(const QString &t) { labelTextLabel->setText(t); }
    QString labelText() const           { return labelTextLabel->text(); }

    void setProgress(int val);
    int  progress() const               { return progressBar->progress(); }

    void setTotalSteps(int val)         { progressBar->setTotalSteps(val); }
    int  totalSteps() const             { return progressBar->totalSteps(); }
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
