// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           ViewerSILAttsObserver.h                         //
// ************************************************************************* //

#ifndef VIEWER_SILATTS_OBSERVER_H
#define VIEWER_SILATTS_OBSERVER_H
#include <viewer_exports.h>
#include <QObject>
#include <Observer.h>
#include <string>

class SILAttributes;

// ****************************************************************************
//  Class: ViewerSILAttsObserver
//
//  Purpose:
//    ViewerSILAttsObserver observes database sill and emits a Qt
//    signal when the subjects are updated.
//
//  Note:
//
//  Programmer: Jeremy Meredith
//  Creation:   April 22, 2004
//
//  Modifications:
//
// ****************************************************************************

class VIEWER_API ViewerSILAttsObserver : public QObject, public Observer
{
    Q_OBJECT
  public:
    ViewerSILAttsObserver(Subject *,
                           const std::string &h, const std::string &f);
    ~ViewerSILAttsObserver();
    virtual void Update(Subject *);
  signals:
    void silAttsUpdated(const std::string &h, const std::string &f,
                        const SILAttributes *sa);
  private:
    std::string           host;
    std::string           file;
};

#endif
