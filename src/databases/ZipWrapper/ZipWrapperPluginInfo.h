// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//                               ZipWrapperPluginInfo.h
// ****************************************************************************

#ifndef ZIPWRAPPER_PLUGIN_INFO_H
#define ZIPWRAPPER_PLUGIN_INFO_H
#include <DatabasePluginInfo.h>
#include <database_plugin_exports.h>

class avtDatabase;
class avtDatabaseWriter;

// ****************************************************************************
//  Class: ZipWrapperDatabasePluginInfo
//
//  Purpose:
//    Classes that provide all the information about the ZipWrapper plugin.
//    Portions are separated into pieces relevant to the appropriate
//    components of VisIt.
//
//  Programmer: generated by xml2info
//  Creation:   omitted
//
//  Modifications:
//
// ****************************************************************************

class ZipWrapperGeneralPluginInfo : public virtual GeneralDatabasePluginInfo
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

class ZipWrapperCommonPluginInfo : public virtual CommonDatabasePluginInfo, public virtual ZipWrapperGeneralPluginInfo
{
  public:
    virtual DatabaseType              GetDatabaseType();
    virtual avtDatabase              *SetupDatabase(const char * const *list,
                                                    int nList, int nBlock);
    virtual DBOptionsAttributes *GetReadOptions() const;
    virtual DBOptionsAttributes *GetWriteOptions() const;
};

class ZipWrapperMDServerPluginInfo : public virtual MDServerDatabasePluginInfo, public virtual ZipWrapperCommonPluginInfo
{
  public:
    // this makes compilers happy... remove if we ever have functions here
    virtual void dummy();
};

class ZipWrapperEnginePluginInfo : public virtual EngineDatabasePluginInfo, public virtual ZipWrapperCommonPluginInfo
{
  public:
    virtual avtDatabaseWriter        *GetWriter(void);
};

#endif
