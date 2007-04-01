// ************************************************************************* //
//                           ViewerMetaDataObserver.C                        //
// ************************************************************************* //

#include <ViewerMetaDataObserver.h>
#include <avtDatabaseMetaData.h>

using std::string;

// ****************************************************************************
//  Method: ViewerMetaDataObserver constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   April 22, 2004
//
//  Modifications:
//
// ****************************************************************************

ViewerMetaDataObserver::ViewerMetaDataObserver(Subject *subj,
                                               const string &h,
                                               const string &f)
    : Observer(subj), host(h), file(f)
{
}

// ****************************************************************************
//  Method: ViewerMetaDataObserver destructor
//
//  Programmer: Jeremy Meredith
//  Creation:   April 22, 2004
//
//  Modifications:
//
// ****************************************************************************

ViewerMetaDataObserver::~ViewerMetaDataObserver()
{
}

// ****************************************************************************
//  Method: ViewerMetaDataObserver::Update
//
//  Purpose:
//      Update MetaData in the subject.
//
//  Arguments:
//      s       The subject which contains the metadata information.
//
//  Programmer: Jeremy Meredith
//  Creation:   April 22, 2004
//
//  Modifications:
//
// ****************************************************************************

void
ViewerMetaDataObserver::Update(Subject *s)
{
    // Tell the ViewerSubject the meta data was updated
    avtDatabaseMetaData *md = (avtDatabaseMetaData*)s;
    emit metaDataUpdated(host, file, md);
}

