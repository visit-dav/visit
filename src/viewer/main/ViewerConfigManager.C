#include <ViewerConfigManager.h>
#include <visit-config.h> // To get the version number

// For the state objects we're going to save out/read in.
#include <AttributeSubject.h>
#include <ViewerSubject.h>
#include <ViewerMessaging.h>

// ****************************************************************************
//  Method: ViewerConfigManager::ViewerConfigManager
//
//  Purpose: 
//    Constructor for the ViewerConfigManager class.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Sep 29 17:46:00 PST 2000
//
//  Modifications:
//    Kathleen Bonnell, Wed Oct 11 09:03:03 PDT 2000
//    Added initialization for onionpeelAtts. 
//
//    Brad Whitlock, Fri Nov 10 15:26:29 PST 2000
//    Added initialization for materialAtts.
//
//    Eric Brugger, Thu Dec 21 09:18:32 PST 2000
//    I modified the class to work with a list of AttributeSubjects
//    instead of specific ones.
//
//    Brad Whitlock, Tue Feb 19 13:06:06 PST 2002
//    Removed the configSettings member.
//
//    Brad Whitlock, Mon Jun 30 12:25:34 PDT 2003
//    I made it require a pointer to its parent ViewerSubject.
//
// ****************************************************************************

ViewerConfigManager::ViewerConfigManager(ViewerSubject *vs) : ConfigManager()
{
    parent = vs;
    writeDetail = false;
}

// ****************************************************************************
//  Method: ViewerConfigManager::~ViewerConfigManager
//
//  Purpose: 
//    Destructor for the ViewerConfigManager class.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Sep 29 17:46:33 PST 2000
//
//  Modifications:
//    Kathleen Bonnell, Wed Oct 11 09:03:03 PDT 2000
//    Added onionpeelAtts. 
//
//    Brad Whitlock, Fri Nov 10 15:27:02 PST 2000
//    Added materialAtts.
//
//    Eric Brugger, Thu Dec 21 09:18:32 PST 2000
//    I modified the class to work with a list of AttributeSubjects
//    instead of specific ones.
//
// ****************************************************************************

ViewerConfigManager::~ViewerConfigManager()
{
}

// ****************************************************************************
//  Method: ViewerConfigManager::WriteConfigFile
//
//  Purpose: 
//    Writes some of the viewer's state objects to an XML config file.
//
//  Arguments:
//    filename : The name of the config file to write.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Sep 28 14:31:52 PST 2000
//
//  Modifications:
//    Kathleen Bonnell, Wed Oct 11 09:03:03 PDT 2000
//    Added onionpeelAtts under viewer node. 
//
//    Brad Whitlock, Fri Nov 10 15:27:15 PST 2000
//    Added materialAtts under the viewer node.
//
//    Eric Brugger, Thu Dec 21 09:18:32 PST 2000
//    I modified the class to work with a list of AttributeSubjects
//    instead of specific ones.
//
//    Brad Whitlock, Thu Mar 29 17:31:00 PST 2001
//    I changed the code so it no longer reads the config file before writing
//    the config file. The reason for doing this is that the GUI and the
//    viewer no longer share the same config file.
//
//    Brad Whitlock, Tue May 20 14:05:32 PST 2003
//    I added a flag to CreateNode.
//
//    Brad Whitlock, Mon Jun 30 12:26:41 PDT 2003
//    I let the parent ViewerSubject add its data to the data node.
//
// ****************************************************************************

void
ViewerConfigManager::WriteConfigFile(const char *filename)
{
    DataNode topLevel("topLevel");

    // Create the VisIt node.
    DataNode *visitNode = new DataNode("VisIt");
    topLevel.AddNode(visitNode);
    visitNode->AddNode(new DataNode("Version", std::string(VERSION)));

    // Create a "VIEWER" node and add it under "VisIt".
    DataNode *viewerNode = new DataNode("VIEWER");
    visitNode->AddNode(viewerNode);

    // Create a "DEFAULT_VALUES" node and add it under "VIEWER".
    DataNode *defaultsNode = new DataNode("DEFAULT_VALUES");
    viewerNode->AddNode(defaultsNode);

    // Add the attributes under the "DEFAULT_VALUES" node.
    std::vector<AttributeSubject *>::iterator pos;
    for (pos = subjectList.begin(); pos != subjectList.end(); ++pos)
    {
        (*pos)->CreateNode(defaultsNode, false);
    }

    // Let the parent write its data to the "VIEWER" node.
    parent->CreateNode(viewerNode, writeDetail);

    // Try to open the output file.
    if((fp = fopen(filename, "wb")) == 0)
        return;

    // Write the output file to stdout for now.
    fprintf(fp, "<?xml version=\"1.0\"?>\n");
    WriteObject(visitNode);

    // close the file
    fclose(fp);
    fp = 0;
}

// ****************************************************************************
//  Method: ViewerConfigManager::ReadConfigFile
//
//  Purpose: 
//    Reads the VisIt config file and looks at the VIEWER section for
//    the state objects that the viewer wants. It then tells those
//    state objects to update themselves using the DataNode tree.
//
//  Arguments:
//    filename : The name of the VisIt config file.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Sep 29 17:47:27 PST 2000
//
//  Modifications:
//    Kathleen Bonnell, Wed Oct 11 09:03:03 PDT 2000
//    Added support for onionpeelAtts. 
//
//    Brad Whitlock, Fri Nov 10 15:27:57 PST 2000
//    Added support for materialAtts.
//
//    Eric Brugger, Thu Dec 21 09:18:32 PST 2000
//    I modified the class to work with a list of AttributeSubjects
//    instead of specific ones.
//
//    Brad Whitlock, Tue Feb 19 13:04:24 PST 2002
//    Made it return the DataNode that was read from the file.
//
//    Brad Whitlock, Wed Jan 8 17:15:50 PST 2003
//    I set the file pointer to 0 so the destructor does not attempt to
//    close the file again.
//
// ****************************************************************************

DataNode *
ViewerConfigManager::ReadConfigFile(const char *filename)
{
    DataNode *node = 0;

    // Try and open the file for reading.
    if((fp = fopen(filename, "r")) == 0)
        return node;

    // Read the XML tag and ignore it.
    FinishTag();

    // Create a root node and use it to read the visit tree.
    node = new DataNode("FileRoot");
    ReadObject(node);
    fclose(fp);
    fp = 0;

    return node;
}

// ****************************************************************************
// Method: ViewerConfigManager::ProcessConfigSettings
//
// Purpose: 
//   Processes the config settings that were read by the call to
//   ReadConfigFile.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 5 10:02:32 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Feb 19 13:04:59 PST 2002
//   Passed in the DataNode that is to be processed and removed the Notify
//   calls on each subject.
//
//   Brad Whitlock, Fri Mar 21 10:06:56 PDT 2003
//   I added code to process old versions and modify the viewer DataNode.
//
// ****************************************************************************

void
ViewerConfigManager::ProcessConfigSettings(DataNode *node)
{
    // Return if the config settings have not been read.
    if(node == 0)
        return;

    // Get the VisIt node.
    DataNode *visitRoot = node->GetNode("VisIt");
    if(visitRoot == 0)
        return;

    // Get the viewer node.
    DataNode *viewerNode = visitRoot->GetNode("VIEWER");
    if(viewerNode == 0)
        return;

    // Get the defaults node. If it cannot be found, use the VIEWER node so
    // we still support older config files.
    DataNode *defaultsNode = visitRoot->GetNode("DEFAULT_VALUES");
    if(defaultsNode == 0)
        defaultsNode = viewerNode;

    // Get the version
    DataNode *version = visitRoot->GetNode("Version");
    if(version != 0)
    {
        // Do any modifications on the tree that need to be done.
        std::string configVersion(version->AsString());
        std::vector<AttributeSubject *>::iterator pos;
        for (pos = subjectList.begin(); pos != subjectList.end(); ++pos)
        {
            (*pos)->ProcessOldVersions(defaultsNode, configVersion.c_str());
        }
    }

    // Read the attributes into the state objects and notify the observers.
    std::vector<AttributeSubject *>::iterator pos;
    for (pos = subjectList.begin(); pos != subjectList.end(); ++pos)
    {
        (*pos)->SetFromNode(defaultsNode);
    }
}

// ****************************************************************************
// Method: ViewerConfigManager::Notify
//
// Purpose: 
//   Calls Notify on all subjects registered with the config manager.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 19 13:31:52 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerConfigManager::Notify()
{
    // Call Notify() on all subjects.
    std::vector<AttributeSubject *>::iterator pos;
    for (pos = subjectList.begin(); pos != subjectList.end(); ++pos)
    {
        (*pos)->Notify();
    }
}

// ****************************************************************************
// Method: ViewerConfigManager::ClearSubjects
//
// Purpose: 
//   Removes all of the subjects in the list.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 5 10:49:45 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerConfigManager::ClearSubjects()
{
    subjectList.clear();
}

// ****************************************************************************
//  Method: ViewerConfigManager::Add
//
//  Purpose: 
//    Add an AttributeSubject to the list of subjects to read/write
//    from the config file.
//
//  Arguments:
//    subject : The AttributeSubject to add to the list.
//
//  Programmer: Eric Brugger
//  Creation:   Thu Dec 21 09:18:32 PST 2000
//
//  Modifications:
//   
// ****************************************************************************

void
ViewerConfigManager::Add(AttributeSubject *subject)
{
    subjectList.push_back(subject);
}

// ****************************************************************************
// Method: ViewerConfigManager::ExportEntireState
//
// Purpose: 
//   Exports the viewer's entire state to an XML file.
//
// Arguments:
//   filename : The name of the file to which the state is exported.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 9 13:06:12 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerConfigManager::ExportEntireState(const std::string &filename)
{
    writeDetail = true;
    WriteConfigFile(filename.c_str());
    writeDetail = false;

    std::string str("VisIt exported the current session to: ");
    str += filename;
    str += ".";
    Message(str.c_str());
}

// ****************************************************************************
// Method: ViewerConfigManager::ImportEntireState
//
// Purpose: 
//   Imports the entire state from the named file.
//
// Arguments:
//   filename : The state file to use.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 9 13:06:37 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerConfigManager::ImportEntireState(const std::string &filename)
{
    DataNode *node = ReadConfigFile(filename.c_str());
    if(node)
    {
        // Make the hooked up objects get their settings.
        ProcessConfigSettings(node);

        // Get the VisIt node.
        DataNode *visitRoot = node->GetNode("VisIt");
        if(visitRoot != 0)
        {
            // Get the viewer node.
            DataNode *viewerNode = visitRoot->GetNode("VIEWER");
            if(viewerNode != 0)
            {
                // Let the parent read its settings.
                parent->SetFromNode(viewerNode);

                std::string str("VisIt imported an old session from: ");
                str += filename;
                str += ".";
                Message(str.c_str());
                return;
            }
        }
    }

    std::string str("VisIt could not import a session from the file: " +
                    filename);
    Warning(str.c_str());
}

