/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
#include <AccessViewerSession.h>
#include <DataNode.h>
#include <DebugStream.h>
#include <snprintf.h>

// ****************************************************************************
// Method: AccessViewerSession::AccessViewerSession
//
// Purpose: 
//   Constructor
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 13 15:43:58 PST 2006
//
// Modifications:
//   
// ****************************************************************************

AccessViewerSession::AccessViewerSession() : ConfigManager()
{
    root = 0;
}

// ****************************************************************************
// Method: AccessViewerSession::~AccessViewerSession
//
// Purpose: 
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 13 15:44:13 PST 2006
//
// Modifications:
//   
// ****************************************************************************

AccessViewerSession::~AccessViewerSession()
{
    delete root;
}

// ****************************************************************************
// Method: AccessViewerSession::ReadConfigFile
//
// Purpose: 
//   Reads the session file and stores it internally.
//
// Arguments:
//   filename : The name of the session file to open.
//
// Returns:    A pointer to the root data node or 0.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 13 15:44:26 PST 2006
//
// Modifications:
//   
// ****************************************************************************

DataNode *
AccessViewerSession::ReadConfigFile(const char *filename)
{
    const char *mName = "AccessViewerSession::ReadConfigFile: ";
    DataNode *node = 0;

    // Try and open the file for reading.
    if((fp = fopen(filename, "rt")) == 0)
    {
        debug1 << mName << "Could not read " << filename << endl;
        return node;
    }

    root = new DataNode("root");
    ReadObject(root);
    fclose(fp); fp = 0;

    return GetRootNode();
}

// ****************************************************************************
// Method: AccessViewerSession::WriteConfigFile
//
// Purpose: 
//   Writes the modified data tree to a session file.
//
// Arguments:
//   filename : The name of the file to write.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 13 15:45:25 PST 2006
//
// Modifications:
//   
// ****************************************************************************

bool
AccessViewerSession::WriteConfigFile(const char *filename)
{
    // If we have no root then return.
    if(root == 0)
        return false;

    // Try to open the output file.
    if((fp = fopen(filename, "wt")) == 0)
        return false;

    // Write the output file to stdout for now.
    fprintf(fp, "<?xml version=\"1.0\"?>\n");
    WriteObject(GetRootNode());

    // close the file
    fclose(fp);
    fp = 0;

    return true;
}

// ****************************************************************************
// Method: AccessViewerSession::GetRootNode
//
// Purpose: 
//   Returns the pointer to the root node.
//
// Returns:    The pointer to the root node.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 13 15:45:59 PST 2006
//
// Modifications:
//   
// ****************************************************************************

DataNode *
AccessViewerSession::GetRootNode() const
{
    DataNode *ret = 0;
    if(root != 0)
        ret = root->GetNode("VisIt");
    return ret;
}

// ****************************************************************************
// Method: AccessViewerSession::GetVSNode
//
// Purpose: 
//   Returns the pointer to the ViewerSubject node.
//
// Returns:    The pointer to the ViewerSubject node or 0.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 13 15:46:25 PST 2006
//
// Modifications:
//   
// ****************************************************************************

DataNode *
AccessViewerSession::GetVSNode() const
{
    DataNode *ret = 0;
    if(root != 0)
        ret = root->GetNode("ViewerSubject");
    return ret;
}

// ****************************************************************************
// Method: AccessViewerSession::GetSourceMap
//
// Purpose: 
//   Reads the "SourceMap" entry from the session file if it exists.
//
// Arguments:
//   keys   : The source identifiers.
//   values : The source names.
//   uses   : How the sources are used.
//
// Returns:    True on success; False otherwise.
//
// Note:       This method reads the source map from a viewer session file so
//             we can provide UI controls to the user that allow him to change
//             the list of sources being visualized. We also scan the plots
//             in the session file so we can indicate how each source is used.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 13 15:46:53 PST 2006
//
// Modifications:
//   
// ****************************************************************************

bool
AccessViewerSession::GetSourceMap(stringVector &keys, stringVector &values,
    std::map<std::string, stringVector> &uses)
{
    const char *mName = "AccessViewerSession::GetSourceMap: ";

    DataNode *vsNode = GetVSNode();
    bool ret = false;
    if(vsNode != 0)
    {
        DataNode *smNode = vsNode->GetNode("SourceMap");
        if(smNode != 0)
        {
            keys.clear();
            values.clear();

            DataNode **children = smNode->GetChildren();
            for(int i = 0; i < smNode->GetNumChildren(); ++i)
            {
                if(children[i]->GetNodeType() == STRING_NODE)
                {
                    keys.push_back(children[i]->GetKey());
                    values.push_back(children[i]->AsString());
                }
            }

            ret = keys.size() > 0;            
        }

        // NOTE: This section knows a lot about viewer session files, which
        //       means that if the viewer session format changes then this
        //       code must also change.
        //
        // Look through the plots in the session file and determine
        // where each source is used so we can give a little more
        // information to the user.
        DataNode *wmNode = vsNode->GetNode("ViewerWindowManager");
        if(wmNode != 0)
        {
            DataNode *winNode = wmNode->GetNode("Windows");
            if(winNode != 0)
            {
                DataNode **wins = winNode->GetChildren();
                for(int i = 0; i < winNode->GetNumChildren(); ++i)
                {
                    if(wins[i]->GetNodeType() == INTERNAL_NODE &&
                       wins[i]->GetKey() == "ViewerWindow")
                    {
                        DataNode *vpl = wins[i]->GetNode("ViewerPlotList");
                        if(vpl != 0)
                        {
                            char tmp[1000];
                            int ploti = 0;
                            DataNode *plotNode = 0;
                            do
                            {
                                SNPRINTF(tmp, 1000, "plot%02d", ploti++);
                                plotNode = vpl->GetNode(tmp);
                                if(plotNode != 0)
                                {
                                    DataNode *pluginIDNode = 0,
                                             *varNameNode = 0,
                                             *sourceIDNode = 0;
                                    pluginIDNode = plotNode->GetNode("pluginID");
                                    sourceIDNode = plotNode->GetNode("sourceID");
                                    varNameNode = plotNode->GetNode("variableName");

                                    if(sourceIDNode != 0 && 
                                       sourceIDNode->GetNodeType() == STRING_NODE &&
                                       pluginIDNode != 0 && 
                                       pluginIDNode->GetNodeType() == STRING_NODE &&
                                       varNameNode != 0 && 
                                       varNameNode->GetNodeType() == STRING_NODE)
                                    {
                                        std::string source(sourceIDNode->AsString());
                                        std::string varName(varNameNode->AsString());
                                        std::string plotName(pluginIDNode->AsString());

                                        std::string::size_type pos = plotName.rfind("_");
                                        if(pos != std::string::npos)
                                            plotName = plotName.substr(0, pos);

                                        SNPRINTF(tmp, 1000, "Window %d, %s plot of %s",
                                            i+1, plotName.c_str(), varName.c_str());

                                        if(uses.find(source) == uses.end())
                                            uses[source] = stringVector();
                                        uses[source].push_back(std::string(tmp));
                                    }
                                    else
                                    {
                                        debug1 << mName << "pluginID, sourceID, or "
                                        "variableName nodes not located or they "
                                        "were the wrong types." << endl;
                                    }
                                }
                            } while(plotNode != 0);
                        }
                    }
                }
            }
        }

    }

    return ret;
}

 
