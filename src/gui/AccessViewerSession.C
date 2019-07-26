// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//   Brad Whitlock, Tue Apr 22 12:19:30 PDT 2008
//   Skip the XML tag.
//
// ****************************************************************************
DataNode *
AccessViewerSession::ReadConfigFile(std::istream& in)
{
    // Skip the XML tag
    FinishTag(in);

    root = new DataNode("root");
    ReadObject(in, root);

    return GetRootNode();
}

DataNode *
AccessViewerSession::ReadConfigFile(const char *filename)
{
    const char *mName = "AccessViewerSession::ReadConfigFile: ";
    DataNode *node = 0;
    std::ifstream inf;
    // Try and open the file for reading.
    inf.open(filename, ios::in); // | ios::trunc); //"rt"
    if(inf.is_open() == false)
    {
        debug1 << mName << "Could not read " << filename << endl;
        return node;
    }

    node = ReadConfigFile(inf);

    inf.close();

    return node;
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
AccessViewerSession::WriteConfigFile(std::ostream& out)
{
    // Write the output file to stdout for now.
    out << "<?xml version=\"1.0\"?>\n";
    WriteObject(out, GetRootNode());

    return true;
}

bool
AccessViewerSession::WriteConfigFile(const char *filename)
{
    // If we have no root then return.
    if(root == 0)
        return false;
    std::ofstream outf;
    // Try to open the output file.
    outf.open(filename, ios::out | ios::trunc);
    if(outf.is_open() == false)
        return false;

    bool res = WriteConfigFile(outf);

    // close the file
    outf.close();

    return res;
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
//   Brad Whitlock, Thu Jul 24 17:01:29 PDT 2008
//   Fixed a bug where we could not locate the VIEWER node.
//
// ****************************************************************************

DataNode *
AccessViewerSession::GetRootNode() const
{
    DataNode *ret = 0;
    if(root != 0)
        ret = root->SearchForNode("VIEWER");
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
        ret = root->SearchForNode("ViewerSubject");
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
        else
        {
            debug1 << mName << "Could not find SourceMap node." << endl;
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

 
