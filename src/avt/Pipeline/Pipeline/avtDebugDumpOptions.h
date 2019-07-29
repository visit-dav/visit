// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_DEBUG_DUMP_OPTIONS_H
#define AVT_DEBUG_DUMP_OPTIONS_H

#include <pipeline_exports.h>
#include <visitstream.h>
#include <string>

// ****************************************************************************
// Class: avtDebugDumpOptions
//
// Purpose:
//     Provides static members that define global debug dump options.
//     These options were migrated from the avtFilter, avtTerminatingSink
//     and avtDataRepresentation classes b/c of growing redundancy. 
//
//
// Programmer: Cyrus Harrison
// Creation:   Feburary 13, 2009
//
// Modifications: 
//
// ****************************************************************************

class PIPELINE_API avtDebugDumpOptions
{
  public:
    virtual                  ~avtDebugDumpOptions();
    
    static void               EnableDump();
    static void               DisableDump();
    static bool               DumpEnabled() {return doDump;}
    
    static void               EnableDatasetDump();
    static void               DisableDatasetDump();
    static bool               DatasetDumpEnabled() {return doDatasetDump;}
    
    static const std::string &GetDumpDirectory() {return outputDir;}
    static void               SetDumpDirectory(const std::string &);
    
  private:
    // we dont want anyone creating an instance of this class, so make
    // its constructor private.
    avtDebugDumpOptions();
    
    // static members
    static std::string   outputDir;
    static bool          doDump;
    static bool          doDatasetDump;
    
};


#endif


