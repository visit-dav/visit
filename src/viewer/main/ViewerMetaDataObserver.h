// ************************************************************************* //
//                          ViewerMetaDataObserver.h                         //
// ************************************************************************* //

#ifndef VIEWER_METADATA_OBSERVER_H
#define VIEWER_METADATA_OBSERVER_H
#include <viewer_exports.h>
#include <qobject.h>
#include <Observer.h>
#include <string>

class avtDatabaseMetaData;

// ****************************************************************************
//  Class: ViewerMetaDataObserver
//
//  Purpose:
//    ViewerMetaDataObserver observes database meta data and emits a Qt
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

class VIEWER_API ViewerMetaDataObserver : public QObject, public Observer
{
    Q_OBJECT
  public:
    ViewerMetaDataObserver(Subject *,
                           const std::string &h, const std::string &f);
    ~ViewerMetaDataObserver();
    virtual void Update(Subject *);
  signals:
    void metaDataUpdated(const std::string &h, const std::string &f,
                         const avtDatabaseMetaData *md);
  private:
    std::string           host;
    std::string           file;
};

#endif
