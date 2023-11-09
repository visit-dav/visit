// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  WavefrontOBJPluginInfo.h
// ****************************************************************************

#ifndef WAVEFRONTOBJ_PLUGIN_INFO_H
#define WAVEFRONTOBJ_PLUGIN_INFO_H
#include <DatabasePluginInfo.h>
#include <database_plugin_exports.h>

class avtDatabase;
class avtDatabaseWriter;

// ****************************************************************************
//  Class: WavefrontOBJDatabasePluginInfo
//
//  Purpose:
//    Classes that provide all the information about the WavefrontOBJ plugin.
//    Portions are separated into pieces relevant to the appropriate
//    components of VisIt.
//
//  Programmer: generated by xml2info
//  Creation:   omitted
//
//  Modifications:
//    Justin Privitera, Fri Nov  3 15:25:32 PDT 2023
//    Added GetWriteOptions().
//
// ****************************************************************************

class WavefrontOBJGeneralPluginInfo : public virtual GeneralDatabasePluginInfo
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

class WavefrontOBJCommonPluginInfo : public virtual CommonDatabasePluginInfo, public virtual WavefrontOBJGeneralPluginInfo
{
  public:
    virtual DatabaseType              GetDatabaseType();
    virtual avtDatabase              *SetupDatabase(const char * const *list,
                                                    int nList, int nBlock);
    virtual DBOptionsAttributes      *GetWriteOptions() const;
};

class WavefrontOBJMDServerPluginInfo : public virtual MDServerDatabasePluginInfo, public virtual WavefrontOBJCommonPluginInfo
{
  public:
    // this makes compilers happy... remove if we ever have functions here
    virtual void dummy();
};

class WavefrontOBJEnginePluginInfo : public virtual EngineDatabasePluginInfo, public virtual WavefrontOBJCommonPluginInfo
{
  public:
    virtual avtDatabaseWriter        *GetWriter(void);
};

#endif
