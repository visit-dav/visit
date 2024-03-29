// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  File: MFEMPluginInfo.C
// ****************************************************************************

#include <MFEMPluginInfo.h>

#include <visit-config.h>
VISIT_PLUGIN_VERSION(MFEM,DBP_EXPORT)

VISIT_DATABASE_PLUGIN_ENTRY(MFEM,General)

// ****************************************************************************
//  Method: MFEMGeneralPluginInfo::GetName
//
//  Purpose:
//    Return the name of the database plugin.
//
//  Returns:    A pointer to the name of the database plugin.
//
//  Programmer: generated by xml2info
//  Creation:   omitted
//
// ****************************************************************************

const char *
MFEMGeneralPluginInfo::GetName() const
{
    return "MFEM";
}

// ****************************************************************************
//  Method: MFEMGeneralPluginInfo::GetVersion
//
//  Purpose:
//    Return the version of the database plugin.
//
//  Returns:    A pointer to the version of the database plugin.
//
//  Programmer: generated by xml2info
//  Creation:   omitted
//
// ****************************************************************************

const char *
MFEMGeneralPluginInfo::GetVersion() const
{
    return "1.0";
}

// ****************************************************************************
//  Method: MFEMGeneralPluginInfo::GetID
//
//  Purpose:
//    Return the id of the database plugin.
//
//  Returns:    A pointer to the id of the database plugin.
//
//  Programmer: generated by xml2info
//  Creation:   omitted
//
// ****************************************************************************

const char *
MFEMGeneralPluginInfo::GetID() const
{
    return "MFEM_1.0";
}
// ****************************************************************************
//  Method: MFEMGeneralPluginInfo::EnabledByDefault
//
//  Purpose:
//    Return true if this plugin should be enabled by default; false otherwise.
//
//  Returns:    true/false
//
//  Programmer: generated by xml2info
//  Creation:   omitted
//
// ****************************************************************************

bool
MFEMGeneralPluginInfo::EnabledByDefault() const
{
    return true;
}
// ****************************************************************************
//  Method: MFEMGeneralPluginInfo::HasWriter
//
//  Purpose:
//    Return true if this plugin has a database writer.
//
//  Returns:    true/false
//
//  Programmer: generated by xml2info
//  Creation:   omitted
//
// ****************************************************************************

bool
MFEMGeneralPluginInfo::HasWriter() const
{
    return false;
}
// ****************************************************************************
//  Method:  MFEMGeneralPluginInfo::GetDefaultFilePatterns
//
//  Purpose:
//    Returns the default patterns for a MFEM database.
//
//  Programmer:  generated by xml2info
//  Creation:    omitted
//
// ****************************************************************************
std::vector<std::string>
MFEMGeneralPluginInfo::GetDefaultFilePatterns() const
{
    std::vector<std::string> defaultPatterns;
    defaultPatterns.push_back("*.mfem_root");
    defaultPatterns.push_back("*.mesh");

    return defaultPatterns;
}

// ****************************************************************************
//  Method:  MFEMGeneralPluginInfo::AreDefaultFilePatternsStrict
//
//  Purpose:
//    Returns if the file patterns for a MFEM database are
//    intended to be interpreted strictly by default.
//
//  Programmer:  generated by xml2info
//  Creation:    omitted
//
// ****************************************************************************
bool
MFEMGeneralPluginInfo::AreDefaultFilePatternsStrict() const
{
    return false;
}

// ****************************************************************************
//  Method:  MFEMGeneralPluginInfo::OpensWholeDirectory
//
//  Purpose:
//    Returns if the MFEM plugin opens a whole directory name
//    instead of a single file.
//
//  Programmer:  generated by xml2info
//  Creation:    omitted
//
// ****************************************************************************
bool
MFEMGeneralPluginInfo::OpensWholeDirectory() const
{
    return false;
}
