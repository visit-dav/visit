// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef COLORTABLE_MANAGER_H
#define COLORTABLE_MANAGER_H
#include <ConfigManager.h>
#include <ColorControlPointList.h>
#include <ColorTableAttributes.h>
#include <string>

class DataNode;

// ****************************************************************************
// Class: ColorTableManager
//
// Purpose:
//   Reads and writes ColorControlPointLists to/from files.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 3 17:49:53 PST 2003
//
// Modifications:
//   Brad Whitlock, Thu Nov 13 11:51:47 PDT 2003
//   I changed how messages are passed out of Export.
//
//   Brad Whitlock, Thu Feb 17 15:54:26 PST 2005
//   Made WriteConfigFile return bool.
//
//   Katahleen Biagas, Fri Aug 8 08:36:27 PDT 2014
//   Add 'importingPersonal' flag to distinguish between a users's color tables
//   and the 'extra' standard tables that VisIt imports from resources.
//
// ****************************************************************************

class STATE_API ColorTableManager : public ConfigManager
{
public:
    ColorTableManager();
    virtual ~ColorTableManager();

    bool              Export(const std::string &ctName,
                             const ColorControlPointList &ccpl,
                             std::string &message);
    bool              ImportColorTables(ColorTableAttributes *cta);

    virtual bool      WriteConfigFile(const char *filename);
    virtual DataNode *ReadConfigFile(const char *filename);

    virtual bool      WriteConfigFile(std::ostream& out);
    virtual DataNode *ReadConfigFile(std::istream& in);
protected:
    void              ImportColorTable(const std::string &ctFileName);
    static void       ImportHelper(void *, const std::string &,
                                   bool, bool, long);
private:
    ColorTableAttributes *ctAtts;
    ColorControlPointList ccpl;
    bool importingPersonal;
};

#endif
