#ifndef QVISINTERPRETER_H
#define QVISINTERPRETER_H
#include <gui_exports.h>
#include <qobject.h>
#include <GUIBase.h>
#include <deque>

// ****************************************************************************
// Class: QvisInterpreter
//
// Purpose:
//   This class provides an interpreter interface.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri May 6 09:41:51 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

class GUI_API QvisInterpreter : public QObject, public GUIBase
{
    Q_OBJECT
public:
    QvisInterpreter(QObject *parent = 0, const char *name = 0);
    virtual ~QvisInterpreter();
signals:
    void Synchronize();
public slots:
    void Interpret(const QString &s);
    void ProcessCommands();
private:
    bool Initialize();
    bool NoInterpretingClient() const;

    std::deque<std::string> pendingCommands;
};

#endif
