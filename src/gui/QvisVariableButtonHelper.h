#ifndef QVIS_VARIABLE_BUTTON_HELPER_H
#define QVIS_VARIABLE_BUTTON_HELPER_H
#include <qobject.h>

class QvisVariableButton;

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
//   
// ****************************************************************************

class QvisVariableButtonHelper : public QObject
{
    Q_OBJECT
public:
    QvisVariableButtonHelper(QObject *parent, const char *name = 0);
    virtual ~QvisVariableButtonHelper();

    void setButton(QvisVariableButton *);

public slots:
    void activated(int, const QString &);
private:
    QvisVariableButton *btn;
};

#endif
