// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SINGLE_ATTRIBUTE_MANAGER_H
#define SINGLE_ATTRIBUTE_MANAGER_H
#include <ConfigManager.h>
#include <AttributeGroup.h>
#include <string>

class DataNode;

// ****************************************************************************
// Class: SingleAttributeConfigManager
//
// Purpose:
//   Reads and writes single attributes to/from files.
//
// Notes:
//
// Programmer: Jeremy Meredith
// Creation:   January  2, 2009
//
// Modifications:
//   Jeremy Meredith, Thu Apr 29 12:14:13 EDT 2010
//   Added ability to do a selective save.
//
// ****************************************************************************

class STATE_API SingleAttributeConfigManager : public ConfigManager
{
public:
    SingleAttributeConfigManager(AttributeGroup*);
    virtual ~SingleAttributeConfigManager();

    bool              Export(const std::string &filename,
                             bool completeSave = true);
    bool              Import(const std::string &filename);

    bool              Export(std::ostream& out,
                             bool completeSave = true);
    bool              Import(std::istream& in);

    virtual bool      WriteConfigFile(const char *filename);
    virtual DataNode *ReadConfigFile(const char *filename);

    virtual bool      WriteConfigFile(std::ostream& out);
    virtual DataNode *ReadConfigFile(std::istream& in);
protected:
private:
    AttributeGroup *attribute;
    bool completeSave;
};

#endif
