// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            DatabasePluginInfo.h                           //
// ************************************************************************* //

#ifndef DATABASE_PLUGIN_INFO_H
#define DATABASE_PLUGIN_INFO_H
#include <plugin_exports.h>
#include <plugin_entry_point.h>
#include <stdio.h>

#include <string>
#include <vector>

enum DatabaseType
{
    DB_TYPE_STSD,
    DB_TYPE_STMD,
    DB_TYPE_MTSD,
    DB_TYPE_MTMD,
    DB_TYPE_CUSTOM
};

// Forward declarations.
class avtDatabase;
class avtDatabaseWriter;
class DBOptionsAttributes;
class DatabasePluginManager;

// ****************************************************************************
//  Class: *DatabasePluginInfo
//
//  Purpose:
//    Classes that provide all the information about the database plugin.
//    Portions are separated into pieces relevant to the appropriate
//    components of VisIt.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 21, 2002
//
//  Modifications:
//
//    Hank Childs, Wed Sep 10 07:05:54 PDT 2003
//    Added DatabaseWriter.
//
//    Jeremy Meredith, Wed Nov  5 10:28:29 PST 2003
//    Added ability to disable plugins by default.
//
//    Hank Childs, Thu Feb 19 10:01:47 PST 2004
//    Added GetFilenames.  Made GetDefaultExtensions not be pure virtual.
//
//    Jeremy Meredith, Tue Feb 22 18:36:54 PST 2005
//    Moved GetWriter to the engine so the mdserver doesn't need it.
//    Added the general plugin info method HasWriter so the mdserver
//    can still check if it is supported by the given plugin.
//
//    Hank Childs, Tue Mar 22 16:06:15 PST 2005
//    Make destructor virtual.
//
//    Hank Childs, Mon May 23 16:31:36 PDT 2005
//    Add DBOptions.
//
//    Mark C. Miller, Mon Aug  6 13:36:16 PDT 2007
//    Added GetDfltExtsFromGen and GetFilenamesFromGen as newer ways to
//    obtain this information instead of GetDefaultExtensions and GetFilenames
//    on CommonDatabasePluginInfo. Note, however, we will leave the older
//    methods around for backward compatibility for plugins VisIt does not own.
//
//    Brad Whitlock, Tue Jun 24 16:22:13 PDT 2008
//    Added a pointer back to the plugin manager in the common info because
//    certain database readers need to access the plugin manager and it's
//    no longer a singleton. The plugin manager will add a pointer to itself
//    into the common info when it reads the info.
//
//    Jeremy Meredith, Thu Aug  7 16:22:24 EDT 2008
//    Use const char* for the functions likely to return a string literal.
//
//    Jeremy Meredith, Mon Dec 28 15:08:45 EST 2009
//    Changed file matching behavior to use patterns and a strictness
//    flag.  Made pure virtual so plugin developers will know to re-run info
//    xml code generation tool.  Removed deprecated versions of this code.
//
//    Brad Whitlock, Thu Mar 20 14:11:30 PDT 2014
//    Add a method to return license.
//    Work partially supported by DOE Grant SC0007548.
//
// ****************************************************************************

class PLUGIN_API GeneralDatabasePluginInfo
{
  public:
    virtual ~GeneralDatabasePluginInfo() {;};
    virtual const char *GetName() const = 0;
    virtual const char *GetVersion() const = 0;
    virtual const char *GetID() const = 0;
    virtual bool  EnabledByDefault() const { return true; }
    virtual bool  HasWriter() const { return false; }
    virtual std::vector<std::string>  GetDefaultFilePatterns() const = 0;
    virtual bool  AreDefaultFilePatternsStrict() const { return false; }
    virtual bool  OpensWholeDirectory() const { return false; }
};

class PLUGIN_API CommonDatabasePluginInfo : public virtual GeneralDatabasePluginInfo
{
  public:
                                      CommonDatabasePluginInfo();
    virtual                          ~CommonDatabasePluginInfo();

    virtual DatabaseType              GetDatabaseType() = 0;
    virtual avtDatabase              *SetupDatabase(const char * const *list,
                                                    int nList, int nBlock) = 0;

    virtual DBOptionsAttributes      *GetReadOptions(void) const;
    virtual DBOptionsAttributes      *GetWriteOptions(void) const;
    virtual void                      SetReadOptions(DBOptionsAttributes *);
    virtual void                      SetWriteOptions(DBOptionsAttributes *);
    virtual std::string               GetLicense() const { return std::string(); }

    void SetPluginManager(DatabasePluginManager *ptr);
    DatabasePluginManager *GetPluginManager() const;
  protected:
    DBOptionsAttributes              *readOptions;
    DBOptionsAttributes              *writeOptions;
    DatabasePluginManager            *pluginManager;
};

class PLUGIN_API MDServerDatabasePluginInfo : public virtual CommonDatabasePluginInfo
{
  public:
    // this makes compilers happy... remove if we ever have functions here
    virtual void dummy() = 0;
};

class PLUGIN_API EngineDatabasePluginInfo : public virtual CommonDatabasePluginInfo
{
  public:
    virtual avtDatabaseWriter        *GetWriter(void) { return NULL; };
};

#endif
