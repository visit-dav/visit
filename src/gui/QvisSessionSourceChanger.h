// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_SESSION_SOURCE_CHANGER_H
#define QVIS_SESSION_SOURCE_CHANGER_H

#include <QWidget>
#include <vectortypes.h>
#include <map>

class QListWidget;
class QvisDialogLineEdit;

// ****************************************************************************
// Class: QvisSessionSourceChanger
//
// Purpose:
//   This is a widget that allows the user to alter the sources associated
//   with source ids.
//
// Notes:      We use this class in the movie wizard and in session file
//             restoration so we can allow the user to use different
//             sources with restored sessions.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 13 10:57:19 PDT 2006
//
// Modifications:
//   Cyrus Harrison, Tue Jul  1 09:14:16 PDT 2008
//   Initial Qt4 Port.
//
//   Kathleen Bonnell, Fri May 13 14:26:19 PDT 2011
//   Added setFallbackpath.
//
// ****************************************************************************

class QvisSessionSourceChanger : public QWidget
{
    Q_OBJECT
public:
    QvisSessionSourceChanger(QWidget *parent);
    virtual ~QvisSessionSourceChanger();

    void setSources(const stringVector &keys, const stringVector &values,
                    const std::map<std::string, stringVector> &uses);
    const stringVector &getSources() const;

    void setFallbackPath(const QString &path);

private slots:
    void selectedSourceChanged();
    void sourceChanged();
    void sourceChanged(const QString &);
private:
    void updateControls(int ci);

    QListWidget        *sourceList;
    QvisDialogLineEdit *fileLineEdit;
    QListWidget        *useList;

    stringVector sourceIds;
    stringVector sources;
    std::map<std::string, stringVector> sourceUses;
};

#endif
