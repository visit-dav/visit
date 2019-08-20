// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_CONFIGURATION_MANAGER_H
#define VIEWER_CONFIGURATION_MANAGER_H
#include <viewercore_exports.h>
#include <ConfigManager.h>
#include <ViewerBase.h>
#include <string>
#include <vectortypes.h>

// Forward declarations
class AttributeSubject;
class DataNode;

// ****************************************************************************
//  Class: ViewerConfigManager
//
//  Purpose:
//    This class writes the viewer's portion of the VisIt configuration
//    file.
//
//  Notes:
//    Objects of this class do not own the state objects that they save
//    to the config file. They just maintain pointers to them so the
//    config file can be written easily.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Sep 28 10:29:28 PDT 2000
//
//  Modifications:
//    Kathleen Bonnell, Wed Oct 11 09:03:03 PDT 2000
//    I added OnionPeelAttributes state object.
//
//    Brad Whitlock, Fri Nov 10 15:25:38 PST 2000
//    I added the MaterialAttributes state object.
//
//    Eric Brugger, Thu Dec 21 08:00:03 PST 2000
//    I modified the class to accept a list of AttributeSubjects
//    instead of specific ones.
//
//    Brad Whitlock, Wed Sep 5 09:58:05 PDT 2001
//    Modified the class so it splits the reading of the config file from
//    the processing of the config file.
//
//    Brad Whitlock, Tue Feb 19 13:02:55 PST 2002
//    Made it conform to the base class's new interface.
//
//    Brad Whitlock, Fri Mar 21 10:04:30 PDT 2003
//    I removed the ProcessOldVersions method.
//
//    Brad Whitlock, Mon Jun 30 12:24:43 PDT 2003
//    I made it require a pointer to its parent ViewerSubject.
//
//    Brad Whitlock, Wed Jul 9 12:44:08 PDT 2003
//    I added methods to export the entire state and import the entire state.
//
//    Brad Whitlock, Mon Aug 25 14:28:23 PST 2003
//    Added NotifyIfSelected.
//
//    Brad Whitlock, Thu Feb 17 16:06:02 PST 2005
//    I made WriteConfig return a bool.
//
//    Brad Whitlock, Fri Nov 10 10:58:11 PDT 2006
//    I added arguments to ImportEntireState.
//
//    Brad Whitlock, Mon Feb 12 18:01:20 PST 2007
//    Use ViewerBase base class.
//
//    Hank Childs, Thu Oct 28 11:11:33 PDT 2010
//    Add a Boolean that allows for plugin attributes to be skipped.
//
//    Brad Whitlock, Thu Aug 28 15:23:53 PDT 2014
//    Change to a callback. Move session file stuff out.
//
// ****************************************************************************

class VIEWERCORE_API ViewerConfigManager : public ViewerBase, public ConfigManager
{
public:
    ViewerConfigManager();
    virtual ~ViewerConfigManager();

    virtual bool WriteConfigFile(const char *filename);
    virtual DataNode *ReadConfigFile(const char *filename);

    virtual bool WriteConfigFile(std::ostream& out);
    virtual DataNode *ReadConfigFile(std::istream& in);

    void ProcessConfigSettings(DataNode *, bool processPluginAtts = true);
    void Notify();
    void NotifyIfSelected();
    void ClearSubjects();

    void Add(AttributeSubject *subject);

    void SetWriteDetail(bool val);

    // Permit callbacks to be installed 
    typedef void (*WriteConfigFileCallback)(DataNode *, bool, void *);
    void SetWriteConfigFileCallback(WriteConfigFileCallback cb, void *cbdata);

private:
    bool                             writeDetail;
    std::vector<AttributeSubject *>  subjectList;
    WriteConfigFileCallback          writeCallback;
    void                            *writeCallbackData;

};

#endif
