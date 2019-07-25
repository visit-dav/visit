// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          ViewerCommandFromSimObserver.h                   //
// ************************************************************************* //

#ifndef VIEWER_COMMAND_FROM_SIM_OBSERVER_H
#define VIEWER_COMMAND_FROM_SIM_OBSERVER_H
#include <viewer_exports.h>
#include <QObject>
#include <Observer.h>
#include <string>
#include <EngineKey.h>

class avtDatabaseMetaData;

// ****************************************************************************
//  Class: ViewerCommandFromSimObserver
//
//  Purpose:
//    ViewerCommandFromSimObserver observes commands that a simulation may 
//    send to the viewer and emits a Qt signal when the subjects are updated.
//
//  Note:
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Jan 25 14:08:09 PST 2007
//
//  Modifications:
//
// ****************************************************************************

class VIEWER_API ViewerCommandFromSimObserver : public QObject, public Observer
{
    Q_OBJECT
  public:
    ViewerCommandFromSimObserver(Subject *,
        const EngineKey &k, const std::string &f);
    ~ViewerCommandFromSimObserver();
    virtual void Update(Subject *);
  signals:
    void execute(const EngineKey &ek, const std::string &f,
                 const std::string &command);
  private:
    EngineKey   key;
    std::string file;
};

#endif
