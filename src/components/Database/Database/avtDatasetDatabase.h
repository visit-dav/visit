// ************************************************************************* //
//                            avtDatasetDatabase.h                           //
// ************************************************************************* //

#ifndef AVT_DATASET_DATABASE_H
#define AVT_DATASET_DATABASE_H
#include <database_exports.h>


#include <avtDatabase.h>


class     avtSourceFromDatabase;


// ****************************************************************************
//  Class: avtDatasetDatabase
//
//  Purpose:
//      A database that serves up avtDatasets.
//
//  Programmer: Hank Childs
//  Creation:   August 17, 2001
//
// ****************************************************************************

class DATABASE_API avtDatasetDatabase : public avtDatabase
{
  public:
                              avtDatasetDatabase() {;}
    virtual                  ~avtDatasetDatabase() {;}

    avtDataObjectSource      *CreateSource(const char *, int);

    virtual avtDataTree_p     GetOutput(avtDataSpecification_p,
                                        avtSourceFromDatabase *) = 0;
};


#endif


