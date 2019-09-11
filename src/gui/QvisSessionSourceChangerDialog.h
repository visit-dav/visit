// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_SESSION_SOURCE_CHANGER_DIALOG_H
#define QVIS_SESSION_SOURCE_CHANGER_DIALOG_H

#include <QDialog>
#include <QvisSessionSourceChanger.h>

// ****************************************************************************
// Class: QvisSessionSourceChangerDialog
//
// Purpose:
//   This class implements a dialog that we can use to change the sources
//   for session files.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 15:27:03 PST 2006
//
// Modifications:
//   Cyrus Harrison, Tue Jul  1 09:14:16 PDT 2008
//   Initial Qt4 Port.
//
//   Kathleen Bonnell, Fri May 13 14:04:25 PDT 2011
//   Added setFallbackPath.
//
// ****************************************************************************

class QvisSessionSourceChangerDialog : public QDialog
{
    Q_OBJECT
public:
    QvisSessionSourceChangerDialog(QWidget *parent);
    virtual ~QvisSessionSourceChangerDialog();

    void setSources(const stringVector &keys, const stringVector &values,
                    const std::map<std::string, stringVector> &uses);
    const stringVector &getSources() const;

    void setFallbackPath(const QString &);

protected:
    virtual void keyPressEvent(QKeyEvent *e);
private:
    QString SplitPrompt(const QString &s) const;

    QvisSessionSourceChanger *body;
};

#endif
