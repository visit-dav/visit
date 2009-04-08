#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H
#include <QObject>
#include <QStringList>

#include <VisItViewer.h>
#include "Synchronizer.h"

class CommandParser : public QObject
{
   Q_OBJECT
public:
   CommandParser(VisItViewer *v);
   virtual ~CommandParser();

   void ProcessCommands(const QString &filename);
signals:
    void openFile(const QString &);
    void changeVariable(const QString &);
    void changePlotType(int);
    void setNContours(int);
    void saveWindow();

private slots:
   void ProcessOneCommand();
private:
    VisItViewer  *viewer;
    QStringList   commands;
    Synchronizer *sync;
};

#endif
