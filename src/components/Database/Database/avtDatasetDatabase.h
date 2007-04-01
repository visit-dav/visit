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
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class DATABASE_API avtDatasetDatabase : public avtDatabase
{
  public:
                              avtDatasetDatabase();
    virtual                  ~avtDatasetDatabase();

    avtDataObjectSource      *CreateSource(const char *, int);

    virtual avtDataTree_p     GetOutput(avtDataSpecification_p,
                                        avtSourceFromDatabase *) = 0;
};


#endif


