#ifndef VIEWER_CONFIGURATION_MANAGER_H
#define VIEWER_CONFIGURATION_MANAGER_H
#include <viewer_exports.h>
#include <ConfigManager.h>
#include <string>

// Forward declarations
class AttributeSubject;
class DataNode;
class ViewerSubject;

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
// ****************************************************************************

class VIEWER_API ViewerConfigManager : public ConfigManager
{
public:
    ViewerConfigManager(ViewerSubject *);
    virtual ~ViewerConfigManager();

    virtual void WriteConfigFile(const char *filename);
    virtual DataNode *ReadConfigFile(const char *filename);
    void ProcessConfigSettings(DataNode *);
    void Notify();
    void ClearSubjects();

    void Add(AttributeSubject *subject);

    void ExportEntireState(const std::string &filename);
    void ImportEntireState(const std::string &filename, bool);

private:
    bool                             writeDetail;
    std::vector<AttributeSubject *>  subjectList;
    ViewerSubject                   *parent;
};

#endif
