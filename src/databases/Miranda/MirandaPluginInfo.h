// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  MirandaPluginInfo.h
// ****************************************************************************

#ifndef MIRANDA_PLUGIN_INFO_H
#define MIRANDA_PLUGIN_INFO_H
#include <DatabasePluginInfo.h>
#include <database_plugin_exports.h>

class avtDatabase;
class avtDatabaseWriter;

// ****************************************************************************
//  Class: MirandaDatabasePluginInfo
//
//  Purpose:
//    Classes that provide all the information about the Miranda plugin.
//    Portions are separated into pieces relevant to the appropriate
//    components of VisIt.
//
//  Programmer: generated by xml2info
//  Creation:   omitted
//
//  Modifications:
//
// ****************************************************************************

class MirandaGeneralPluginInfo : public virtual GeneralDatabasePluginInfo
{
  public:
    virtual const char *GetName() const;
    virtual const char *GetVersion() const;
    virtual const char *GetID() const;
    virtual bool  EnabledByDefault() const;
    virtual bool  HasWriter() const;
    virtual std::vector<std::string> GetDefaultFilePatterns() const;
    virtual bool  AreDefaultFilePatternsStrict() const;
    virtual bool  OpensWholeDirectory() const;
};

class MirandaCommonPluginInfo : public virtual CommonDatabasePluginInfo, public virtual MirandaGeneralPluginInfo
{
  public:
    virtual DatabaseType              GetDatabaseType();
    virtual avtDatabase              *SetupDatabase(const char * const *list,
                                                    int nList, int nBlock);
    virtual DBOptionsAttributes      *GetReadOptions() const;
    virtual DBOptionsAttributes      *GetWriteOptions() const;
};

class MirandaMDServerPluginInfo : public virtual MDServerDatabasePluginInfo, public virtual MirandaCommonPluginInfo
{
  public:
    // this makes compilers happy... remove if we ever have functions here
    virtual void dummy();
};

class MirandaEnginePluginInfo : public virtual EngineDatabasePluginInfo, public virtual MirandaCommonPluginInfo
{
  public:
    virtual avtDatabaseWriter        *GetWriter(void);
};

#endif
