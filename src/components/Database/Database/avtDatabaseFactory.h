// ************************************************************************* //
//                          avtDatabaseFactory.h                             //
// ************************************************************************* //

#ifndef AVT_DATABASE_FACTORY_H
#define AVT_DATABASE_FACTORY_H
#include <database_exports.h>


class avtDatabase;


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
//    Hank Childs, Fri Mar  2 11:24:48 PST 2001
//    Reflected changes in database and removed SiloObj.  Added VTK.
// 
//    Hank Childs, Wed Feb 13 16:36:58 PST 2002
//    Added better support for multiple blocks with STSD file formats.
//
//    Hank Childs, Fri Feb 15 17:28:01 PST 2002
//    Add support for time-varying Exodus files.
//
//    Hank Childs, Fri May  3 16:04:43 PDT 2002
//    Add support for PLOT3D.
//
//    Hank Childs, Fri May 24 16:32:28 PDT 2002
//    Add support for STL.
//
//    Hank Childs, Tue May 28 14:53:04 PDT 2002
//    Add support for curve files.
//
//    Jeremy Meredith, Tue Aug 27 15:08:29 PDT 2002
//    Removed all formats and placed them in database plugins.
//
//    Hank Childs, Mon Mar  1 08:48:26 PST 2004
//    Added the timestep as an argument.
//
// ****************************************************************************

class DATABASE_API avtDatabaseFactory
{
  public:
    static avtDatabase           *FileList(const char * const *, int, int);
    static avtDatabase           *VisitFile(const char *, int);
};


#endif


