#include <QFile>
#include <QTextStream>
#include "CommandParser.h"

CommandParser::CommandParser(VisItViewer *v) : QObject(), viewer(v)
{
    // This is called after VisItViewer::Setup. This is CRITICAL!
    sync = new Synchronizer(
        v->State()->GetSyncAttributes(),    // The object that we observe
        v->DelayedState()->GetSyncAttributes() // The object that we use to post syncs
        );
}

CommandParser::~CommandParser()
{
}

void CommandParser::ProcessCommands(const QString &filename)
{
    commands.clear();

    QFile f(filename);
    if(f.open(QIODevice::ReadOnly))
    {
        QTextStream s(&f);
        while(!s.atEnd())
            commands.push_back(s.readLine());
    }

    if(commands.size() > 0)
    {
        connect(sync, SIGNAL(synchronized()),
                this, SLOT(ProcessOneCommand()));
        sync->PostSynchronize();
    }
}

void CommandParser::ProcessOneCommand()
{
    if(commands.empty())
    {
        disconnect(sync, SIGNAL(synchronized()),
                   this, SLOT(ProcessOneCommand()));
        return;
    }

    QString cmd = commands.front();
    commands.pop_front();

    // translate the command into viewer calls
    if(cmd.left(8) == "openFile")
    {
        qDebug("Executing command: %s", cmd.toStdString().c_str());
        std::string var =  cmd.right(cmd.size()-8-1).toStdString();
        emit openFile(QString(var.c_str()));
    }
    else if(cmd.left(14) == "changeVariable")
    {
        qDebug("Executing command: %s", cmd.toStdString().c_str());
        std::string var =  cmd.right(cmd.size()-14-1).toStdString();
        emit changeVariable(QString(var.c_str()));
    }
    else if(cmd.left(14) == "changePlotType")
    {
        qDebug("Executing command: %s", cmd.toStdString().c_str());
        QString pt = cmd.right(cmd.size()-14-1);
        emit changePlotType(pt.toInt());
    }
    else if(cmd.left(12) == "setNContours")
    {
        qDebug("Executing command: %s", cmd.toStdString().c_str());
        QString nc = cmd.right(cmd.size()-12-1);
        emit setNContours(nc.toInt());
    }
    else if(cmd == "saveWindow")
    {
        qDebug("Executing command: %s", cmd.toStdString().c_str());
        emit saveWindow();
    }

    // issue a synchronize. We'll get back to this method when
    // the synchronizer issues its signal.
    sync->PostSynchronize();
}
