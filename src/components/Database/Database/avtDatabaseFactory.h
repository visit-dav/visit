// ************************************************************************* //
//                          avtDatabaseFactory.h                             //
// ************************************************************************* //

#ifndef AVT_DATABASE_FACTORY_H
#define AVT_DATABASE_FACTORY_H

#include <database_exports.h>

#include <stdlib.h> // For NULL


class avtDatabase;
class CommonDatabasePluginInfo;


// ****************************************************************************
//  Class: avtDatabaseFactory
//
//  Purpose:
//      Takes a file list, determines the type of file and instantiates the
//      correct avtDatabase.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2000
//
//  Modifications:
//
//    Jeremy Meredith, Tue Aug 27 15:08:29 PDT 2002
//    Removed all formats and placed them in database plugins.
//
//    Hank Childs, Mon Mar  1 08:48:26 PST 2004
//    Added the timestep as an argument.
//
//    Hank Childs, Mon Mar 22 09:41:34 PST 2004
//    Added string specifying format type.  Also removed outdated comments
//    from the era when the DB factory knew about specific formats.
//
//    Hank Childs, Sun May  9 11:41:45 PDT 2004
//    Allow for default file format type to be set.
//
//    Mark C. Miller, Tue May 31 20:12:42 PDT 2005
//    Added bool args for forcing reading of all cycles/times
// ****************************************************************************

class DATABASE_API avtDatabaseFactory
{
  public:
    static avtDatabase           *FileList(const char * const *, int, int,
                                           const char * = NULL, bool = false);
    static avtDatabase           *VisitFile(const char *, int,
                                            const char * = NULL, bool = false);
    static void                   SetDefaultFormat(const char *);

  protected:
    static avtDatabase           *SetupDatabase(CommonDatabasePluginInfo *,
                                                const char * const *, int,
                                                int, int, int, bool);

    static char                  *defaultFormat;
};


#endif


