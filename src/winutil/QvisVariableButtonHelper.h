// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_VARIABLE_BUTTON_HELPER_H
#define QVIS_VARIABLE_BUTTON_HELPER_H
#include <QObject>
#include <winutil_exports.h>

class QvisBaseVariableButton;

// ****************************************************************************
// Class: QvisVariableButtonHelper
//
// Purpose:
//   This class is a helper class for the QvisVariableButton class.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 9 15:38:39 PST 2004
//
// Modifications:
//   Brad Whitlock, Fri May  9 12:26:41 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class WINUTIL_API QvisVariableButtonHelper : public QObject
{
    Q_OBJECT
public:
    QvisVariableButtonHelper(QObject *parent);
    virtual ~QvisVariableButtonHelper();

    void setButton(QvisBaseVariableButton *);

public slots:
    void activated(int, const QString &);
private:
    QvisBaseVariableButton *btn;
};

#endif
