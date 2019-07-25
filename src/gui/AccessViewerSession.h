// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef ACCESS_VIEWER_SESSION_H
#define ACCESS_VIEWER_SESSION_H

#include <ConfigManager.h>
#include <vectortypes.h>
#include <map>

// ****************************************************************************
// Class: AccessViewerSession
//
// Purpose:
//   Provides access to a viewer session file.
//
// Notes:      This class exists primarily so we can read the SourceMap
//             section of the session file.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 13 10:01:41 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

class AccessViewerSession : public ConfigManager
{
public:
    AccessViewerSession();
   ~AccessViewerSession();

    virtual bool WriteConfigFile(const char *filename);
    virtual DataNode *ReadConfigFile(const char *filename);

    virtual bool WriteConfigFile(std::ostream& out);
    virtual DataNode *ReadConfigFile(std::istream& in);

    DataNode *GetRootNode() const;
    DataNode *GetVSNode() const;

    bool GetSourceMap(stringVector &keys, stringVector &values,
                      std::map<std::string, stringVector> &uses);
private:
    DataNode *root;
};

#endif
