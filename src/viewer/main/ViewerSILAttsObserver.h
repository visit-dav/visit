// ************************************************************************* //
//                           ViewerSILAttsObserver.h                         //
// ************************************************************************* //

#ifndef VIEWER_SILATTS_OBSERVER_H
#define VIEWER_SILATTS_OBSERVER_H
#include <viewer_exports.h>
#include <qobject.h>
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
