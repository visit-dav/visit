/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <ViewerConfigManager.h>
#include <visit-config.h> // To get the version number

// For the state objects we're going to save out/read in.
#include <AttributeSubject.h>
#include <DebugStream.h>
#include <InstallationFunctions.h>
#include <FileFunctions.h>

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
//    Brad Whitlock, Mon Feb 12 18:02:12 PST 2007
//    Made it use ViewerBase.
//
//    Brad Whitlock, Fri May 23 11:09:47 PDT 2008
//    Removed name on ViewerBase.
//
// ****************************************************************************

ViewerConfigManager::ViewerConfigManager() : ViewerBase(),
    ConfigManager(), writeDetail(false), subjectList(), writeCallback(NULL),
    writeCallbackData(NULL)
{
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
// Method: ViewerConfigManager::SetWriteDetail
//
// Purpose:
//   Controls whether the config file will be written with high detail.
//
// Arguments:
//   val : True for high detail and false for lower detail.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 28 22:28:07 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerConfigManager::SetWriteDetail(bool val)
{
    writeDetail = val;
}

// ****************************************************************************
// Method: ViewerConfigManager::SetWriteConfigFileCallback
//
// Purpose:
//   Set a callback that can permit this class to write additional data into
//   the config file.
//
// Arguments:
//   cb : The callback to be called in WriteConfig.
//   cbdata : Data to be passed to the write callback.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 28 22:28:49 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void 
ViewerConfigManager::SetWriteConfigFileCallback(WriteConfigFileCallback cb, void *cbdata)
{
    writeCallback = cb;
    writeCallbackData = cbdata;
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
//    Brad Whitlock, Thu Dec 18 11:36:42 PDT 2003
//    I made the default values state objects be completely printed out if
//    we're writing a session file.
//
//    Brad Whitlock, Tue Feb 24 10:26:48 PDT 2004
//    I made it write the file in text mode.
//
//    Brad Whitlock, Thu Feb 17 15:07:41 PST 2005
//    Added code to expand tilde and made it return a bool.
//
//    Brad Whitlock, Thu Aug 28 22:25:40 PDT 2014
//    Use callback function.
//    
// ****************************************************************************

bool
ViewerConfigManager::WriteConfigFile(std::ostream& out)
{
    DataNode topLevel("topLevel");

    // Create the VisIt node.
    DataNode *visitNode = new DataNode("VisIt");
    topLevel.AddNode(visitNode);
    visitNode->AddNode(new DataNode("Version", std::string(VISIT_VERSION)));

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
        (*pos)->CreateNode(defaultsNode, writeDetail, false);
    }

    // Let the parent write its data to the "VIEWER" node.
    if(writeCallback != NULL)
        (*writeCallback)(viewerNode, writeDetail, writeCallbackData);

    // Write the output file to stdout for now.
    out << "<?xml version=\"1.0\"?>\n";
    WriteObject(out, visitNode);

    return true;
}

bool
ViewerConfigManager::WriteConfigFile(const char *filename)
{
    // Try to open the output file.
    std::string expandedFile(FileFunctions::ExpandPath(filename));
    std::ofstream outf(expandedFile.c_str(), ios::out | ios::trunc);
    if(outf.is_open() == false)
        return false;

    bool res = WriteConfigFile(outf);

    // close the file
    outf.close();

    return res;
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
//    Brad Whitlock, Tue Feb 24 10:27:10 PDT 2004
//    I made it read the file in text mode.
//
//    Brad Whitlock, Thu Feb 17 15:08:48 PST 2005
//    Added tilde expansion code.
//
// ****************************************************************************

DataNode *
ViewerConfigManager::ReadConfigFile(std::istream& in)
{
    DataNode *node = 0;

    // Read the XML tag and ignore it.
    FinishTag(in);

    // Create a root node and use it to read the visit tree.
    node = new DataNode("FileRoot");
    ReadObject(in, node);

    return node;
}

DataNode *
ViewerConfigManager::ReadConfigFile(const char *filename)
{
    DataNode *node = 0;

    // Try and open the file for reading.
    std::string expandedFile(FileFunctions::ExpandPath(filename));
    std::ifstream inf(expandedFile.c_str(), ios::in); // | ios::trunc);
    if(inf.is_open() == false)
        return node;

    node = ReadConfigFile(inf);

    inf.close();

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
//   Brad Whitlock, Mon Aug 25 14:36:52 PST 2003
//   Added code to unselect all fields in the AttributeSubjects so we have
//   the option of later only sending the ones that changed.
//
//   Hank Childs, Thu Oct 28 11:11:33 PDT 2010
//   Add an argument to specify whether we should process the plugin attributes.
//
// ****************************************************************************

void
ViewerConfigManager::ProcessConfigSettings(DataNode *node, 
                                           bool processPluginAtts)
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

    // Get the defaults node.
    DataNode *defaultsNode = viewerNode->GetNode("DEFAULT_VALUES");
    if(defaultsNode == 0)
        return;

    //
    // Unselect all fields in the connected AttributeSubjects so we can
    // later send only the ones that changed to the client.
    //
    std::vector<AttributeSubject *>::iterator pos;
    for (pos = subjectList.begin(); pos != subjectList.end(); ++pos)
        (*pos)->UnSelectAll();

    // Get the version
    DataNode *version = visitRoot->GetNode("Version");
    if(version != 0)
    {
        // Do any modifications on the tree that need to be done.
        std::string configVersion(version->AsString());
        for (pos = subjectList.begin(); pos != subjectList.end(); ++pos)
            (*pos)->ProcessOldVersions(defaultsNode, configVersion.c_str());
    }

    // Read the attributes into the state objects and notify the observers.
    debug4 << "ViewerConfigManager::ProcessConfigSettings" << endl;
    for (pos = subjectList.begin(); pos != subjectList.end(); ++pos)
    {
        if (!processPluginAtts && (*pos)->TypeName()  == "PluginManagerAttributes")
            continue;
        debug4 << "\t" << (*pos)->TypeName().c_str() << " setting from node" << endl;
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
    debug4 << "ViewerConfigManager::Notify()" << endl;
    for (pos = subjectList.begin(); pos != subjectList.end(); ++pos)
    {
        debug4 << "\t" << (*pos)->TypeName().c_str() << " sending to client" << endl;
        (*pos)->Notify();
    }
}

// ****************************************************************************
// Method: ViewerConfigManager::NotifyIfSelected
//
// Purpose: 
//   Calls Notify on all subjects registered with the config manager that have
//   some fields that have been selected.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 25 14:32:02 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerConfigManager::NotifyIfSelected()
{
    // Call Notify() on all subjects.
    std::vector<AttributeSubject *>::iterator pos;
    for (pos = subjectList.begin(); pos != subjectList.end(); ++pos)
    {
        if((*pos)->NumAttributesSelected() > 0)
        {
            debug4 << "Sending " << (*pos)->TypeName().c_str()
                   << " to client." << endl;
            (*pos)->Notify();
        }
        else
        {
            debug4 << "Not sending " << (*pos)->TypeName().c_str()
                   << " to client." << endl;
        }
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


