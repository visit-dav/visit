// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtDatabaseFactory.h                             //
// ************************************************************************* //

#ifndef AVT_DATABASE_FACTORY_H
#define AVT_DATABASE_FACTORY_H

#include <database_exports.h>

#include <stdlib.h> // For NULL
#include <string>
#include <vector>

#include <FileOpenOptions.h>
#include <avtTypes.h>

class avtDatabase;
class CommonDatabasePluginInfo;
class DatabasePluginManager;

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
//
//    Hank Childs, Thu Jan 11 15:56:53 PST 2007
//    Added argument for list of plugins attempted.
//
//    Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//    Added bool to support to treat all databases as time varying to various
//    methods
//
//    Kathleen Bonnell, Tue Oct  9 14:40:10 PDT 2007 
//    Added support for turning on/off automatic creation of MeshQuality and
//    TimeDerivative expressions. 
//
//    Cyrus Harrison, Wed Nov 28 10:38:17 PST 2007
//    Added preference for automatic creation of Vector Magnitude expressions. 
//
//    Jeremy Meredith, Wed Jan 23 16:14:13 EST 2008
//    Store the default file opening options for database plugins.
//
//    Jeremy Meredith, Thu Jan 24 14:45:14 EST 2008
//    Added ability to specify a format to try *before* the ones guessed
//    from file extensions.
//
//    Jeremy Meredith, Wed Mar 19 14:06:16 EDT 2008
//    Renamed default format to fallback format for clarity.
//
//    Brad Whitlock, Tue Jun 24 15:49:33 PDT 2008
//    Pass the database plugin manager in because it's no longer a singleton.
//
//    Mark C. Miller, Tue May 19 21:26:53 PDT 2009
//    Added methods to retrieve fallback and first format as well as default
//    file open options.
//
//    Jeremy Meredith, Wed Dec 30 16:32:35 EST 2009
//    Removed fallback and assumed format entirely.  They have been subsumed
//    by a more advanced file format detection method.
//
//    Jeremy Meredith, Fri Jan  8 16:15:02 EST 2010
//    Added ability to turn on stricter file format error checking.
//
//    Hank Childs, Sun Sep 19 09:07:09 PDT 2010
//    Add argument to SetupDatabase for setting times explicitly.
//
//    Dave Pugmire, Fri Feb  8 17:22:01 EST 2013
//    Added support for ensemble databases. (multiple time values)
//
//    Kathleen Biagas, Wed Aug  7 12:44:37 PDT 2013
//    Add methods for setting/getting precision type specified by user.
//
//    Cameron Christensen, Wednesday, June 11, 2014
//    Add methods for setting/getting backend type specified by user.
//
//    Brad Whitlock, Thu Sep 18 23:02:56 PDT 2014
//    Added methods for bypassing file permission checks.
//
//    Kathleen Biagas, Mon Dec 22 10:59:56 PST 2014
//    Added methods for setting the user preference for duplicate node removal.
//
// ****************************************************************************

class DATABASE_API avtDatabaseFactory
{
  public:
    static avtDatabase          *FileList(DatabasePluginManager *,
                                          const char * const *, int, int,
                                          std::vector<std::string> &,
                                          const char * = NULL, bool = false,
                                          bool = false,
                                          int = -1);
    static avtDatabase          *VisitFile(DatabasePluginManager *,
                                           const char *, int,
                                           std::vector<std::string> &,
                                           const char * = NULL, bool = false,
                                           bool = false);
    static void                  SetDefaultFileOpenOptions(const
                                                           FileOpenOptions &f);

    static const FileOpenOptions &GetDefaultFileOpenOptions()
                                     { return defaultFileOpenOptions;};

    static void                  SetCreateMeshQualityExpressions(bool f)
                                     {createMeshQualityExpressions = f;}
    static void                  SetCreateTimeDerivativeExpressions(bool f)
                                     {createTimeDerivativeExpressions = f;}
    static void                  SetCreateVectorMagnitudeExpressions(bool f)
                                     {createVectorMagnitudeExpressions= f;}
                                     
    static bool                  GetCreateMeshQualityExpressions(void)
                                  {return createMeshQualityExpressions;}
    static bool                  GetCreateTimeDerivativeExpressions(void)
                                  {return createTimeDerivativeExpressions;}
    static bool                  GetCreateVectorMagnitudeExpressions(void)
                                  {return createVectorMagnitudeExpressions;}

    static void                  SetRemoveDuplicateNodes(bool f)
                                     {removeDuplicateNodes = f;}
    static bool                  GetRemoveDuplicateNodes()
                                     { return removeDuplicateNodes;}

    static void                  SetPrecisionType(const int pType);
    static avtPrecisionType      GetPrecisionType()
                                     { return precisionType;}
    static void                  SetBackendType(const int bType);
    static avtBackendType        GetBackendType()
                                     { return backendType;}

    static void                  SetCheckFilePermissions(bool val)
                                     { checkFilePermissions = val; }
    static bool                  GetCheckFilePermissions()
                                     { return checkFilePermissions; }
  protected:
    static avtDatabase          *SetupDatabase(CommonDatabasePluginInfo *,
                                               const char * const *, int,
                                               int, int, int, bool, bool,bool,
                                               const std::vector<double> &,
                                               bool isEnsemble);

    static bool                  createMeshQualityExpressions;
    static bool                  createTimeDerivativeExpressions;
    static bool                  createVectorMagnitudeExpressions;
    static bool                  checkFilePermissions;
    static bool                  removeDuplicateNodes;
    static FileOpenOptions       defaultFileOpenOptions;
    static avtPrecisionType      precisionType;
    static avtBackendType        backendType;
};
#endif
