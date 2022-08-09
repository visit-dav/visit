// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <ColorTableManager.h>
#include <DataNode.h>
#include <DebugStream.h>
#include <InstallationFunctions.h>
#include <FileFunctions.h>
#include <visit-config.h>
#include <ColorControlPoint.h>

// ****************************************************************************
// Method: ColorTableManager::ColorTableManager
//
// Purpose:
//   Constructor for the ColorTableManager class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 3 18:23:22 PST 2003
//
// Modifications:
//    Kathleen Biagas, Fri Aug 8 08:32:12 PDT 2014
//    Initialize 'importingPersonal' flag.
//
// ****************************************************************************

ColorTableManager::ColorTableManager() : ccpl()
{
    ctAtts = 0;
    importingPersonal = false;
}

// ****************************************************************************
// Method: ColorTableManager::~ColorTableManager
//
// Purpose:
//   Destructor for the ColorTableManager class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 3 18:23:39 PST 2003
//
// Modifications:
//
// ****************************************************************************

ColorTableManager::~ColorTableManager()
{
}

// ****************************************************************************
// Method: ColorTableManager::Export
//
// Purpose:
//   Exports a color table to the named file.
//
// Arguments:
//   ctName : The name of the color table we're exporting.
//   ccpl_  : The color table definition.
//
// Returns:    A string indicating what happened.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 3 18:23:57 PST 2003
//
// Modifications:
//   Brad Whitlock, Thu Nov 13 11:35:44 PDT 2003
//   I fixed a bug where you could not save out color tables that were already
//   external. I also changed how the message is returned.
//
//   Brad Whitlock, Thu Feb 17 15:57:41 PST 2005
//   I changed how WriteConfigFile is used.
//
// ****************************************************************************

bool
ColorTableManager::Export(const std::string &ctName,
    const ColorControlPointList &ccpl_, std::string &message)
{
    ccpl = ccpl_;

    //
    // Make sure that if the color table already contains the colortable
    // directory that we don't decorate the name.
    //
    std::string ctDir(GetUserVisItDirectory());
    std::string ctFileName(ctDir + ctName + ".ct");
    if(ctName.size() > ctDir.size() &&
       ctName.substr(0, ctDir.size()) == ctDir)
    {
        ctFileName = ctName;
    }

    bool retval;
    if((retval = WriteConfigFile(ctFileName.c_str())) == true)
        message = ctFileName;
    else
    {
        message = std::string("VisIt could not export ") + ctName +
                  std::string(" to ") + ctFileName + ".";
    }

    return retval;
}

// ****************************************************************************
// Method: ColorTableManager::ImportColorTables
//
// Purpose:
//   Imports VisIt's external color tables.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 3 18:24:52 PST 2003
//
// Modifications:
//   Brad Whitlock, Fri Apr 27 17:36:00 PDT 2012
//   Look in a system place too.
//
//   Kathleeen Biagas, Fri Aug 8 08:33:27 PDT 2014
//   Set 'importingPersonal' when importing user defined tables.
//
// ****************************************************************************

bool
ColorTableManager::ImportColorTables(ColorTableAttributes *cta)
{
    //
    // Read the user's home VisIt directory and import all of the color tables.
    //
    ctAtts = cta;
    importingPersonal = false;
    std::string ctdir(GetVisItResourcesDirectory(VISIT_RESOURCES_COLORTABLES));
    bool r1 = FileFunctions::ReadAndProcessDirectory(ctdir,
                                                     ImportHelper,
                                                     (void*)this, false);
    importingPersonal = true;
    bool r2 = FileFunctions::ReadAndProcessDirectory(GetUserVisItDirectory(),
                                                     ImportHelper,
                                                     (void*)this, false);
    return r1 || r2;
}

// ****************************************************************************
// Method: ColorTableManager::WriteConfigFile
//
// Purpose:
//   Writes a the color table to a file.
//
// Arguments:
//   filename : The name of the file to write.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 3 18:27:28 PST 2003
//
// Modifications:
//   Brad Whitlock, Thu Nov 13 11:55:17 PDT 2003
//   I made it throw a VisItException if the file can't be opened.
//
//   Brad Whitlock, Thu Dec 18 11:18:06 PDT 2003
//   I made it call CreateNode with the new completeSave flag set to false.
//
//   Brad Whitlock, Thu Feb 17 15:55:29 PST 2005
//   I removed the exception and made the function return a bool.
//
//   Kathleen Biagas, Fri Aug 8 08:34:29 PDT 2014
//   Set default category name to 'UserDefined'.
// 
//   Justin Privitera, Thu Jun 16 18:01:49 PDT 2022
//   Added tags removed categories.
//
// ****************************************************************************
bool
ColorTableManager::WriteConfigFile(std::ostream& out)
{
    DataNode topLevel("topLevel");
    // Create the color table node.
    DataNode *ctNode = new DataNode("ColorTable");
    topLevel.AddNode(ctNode);
    ctNode->AddNode(new DataNode("Version", std::string(VISIT_VERSION)));

    // Let the color table create and add its information to the node.
    ccpl.CreateNode(ctNode, false, true);
    // add the user-defined tag
    if (ctNode->GetNode("ColorControlPointList")->GetNode("tags"))
    {
        stringVector tags = ctNode->GetNode("ColorControlPointList")->GetNode("tags")->AsStringVector();
        // make sure it's not already here!
        bool found = false;
        for (int i = 0; i < tags.size(); i ++)
            if (tags[i] == "User Defined")
                found = true;
        if (!found)
            tags.push_back("User Defined");
        ctNode->GetNode("ColorControlPointList")->GetNode("tags")->SetStringVector(tags);
    }
    else
    {        
        ctNode->GetNode("ColorControlPointList")->AddNode(new DataNode("tags"));
        stringVector tags;
        tags.push_back("User Defined");
        ctNode->GetNode("ColorControlPointList")->GetNode("tags")->SetStringVector(tags);
    }

    // Write the output file.
    out << "<?xml version=\"1.0\"?>\n";
    WriteObject(out, ctNode);

    return true;
}

bool
ColorTableManager::WriteConfigFile(const char *filename)
{
    std::ofstream outf;

    // Try to open the output file.
    outf.open(filename, ios::out | ios::binary);
    if(outf.is_open() == false)
        return false;

    bool res = WriteConfigFile(outf);

    // Close the file
    outf.close();

    return res;
}

// ****************************************************************************
// Method: ColorTableManager::ReadConfigFile
//
// Purpose:
//   Reads a color table from a file.
//
// Arguments:
//   filename : The name of the file to open.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 3 18:27:57 PST 2003
//
// Modifications:
//
// ****************************************************************************
DataNode *
ColorTableManager::ReadConfigFile(std::istream& in)
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
ColorTableManager::ReadConfigFile(const char *filename)
{
    std::ifstream inf;
    DataNode* node = NULL;

    // Try and open the file for reading.
    inf.open(filename, ios::in);
    if(inf.is_open() == false)
        return node;

    node = ReadConfigFile(inf);

    inf.close();

    return node;
}

// ****************************************************************************
// Method: ColorTableManager::ImportHelper
//
// Purpose:
//   Static callback function for ReadAndProcessDirectory.
//
// Arguments:
//   data       : Callback data.
//   ctFileName : The name of the color table file to open.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 7 15:42:51 PST 2003
//
// Modifications:
//
// ****************************************************************************

void
ColorTableManager::ImportHelper(void *data, const std::string &ctFileName,
    bool, bool, long)
{
    ColorTableManager *cMgr = (ColorTableManager *)data;
    cMgr->ImportColorTable(ctFileName);
}

// ****************************************************************************
// Method: ColorTableManager::ImportColorTable
//
// Purpose:
//   Tries to import the named file as a color table.
//
// Arguments:
//   ctFileName : The name of the file to open.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 7 15:43:49 PST 2003
//
// Modifications:
//   Brad Whitlock, Fri Jul 11 15:44:40 PST 2003
//   Fixed so it works on Windows.
//
//   Brad Whitlock, Tue Jan 17 11:17:09 PDT 2006
//   I added code to prevent the file path from being added to the color
//   table name.
//
//   Brad Whitlock, Wed May 28 15:22:57 PDT 2008
//   Fixed reading of color tables.
//
//   Kathleen Biagas, Fri Aug 8 08:35:43 PDT 2014
//   Set default category name if needed.
// 
//   Justin Privitera, Thu Jun 16 18:01:49 PDT 2022
//   Added tags removed categories.
// 
//   Justin Privitera, Wed Jul 27 12:16:06 PDT 2022
//   Set builtin flag to false for user defined CTs.
//
// ****************************************************************************

void
ColorTableManager::ImportColorTable(const std::string &ctFileName)
{
    if(ctFileName.size() > 3 &&
       ctFileName.substr(ctFileName.size() - 3) == ".ct")
    {
        //
        // Read the color table from the XML file.
        //
        DataNode *node = ReadConfigFile(ctFileName.c_str());

        if(node != 0)
        {
            std::string ctName;
            std::string::size_type pos = ctFileName.rfind(VISIT_SLASH_STRING);
            if(pos == std::string::npos)
                ctName = ctFileName;
            else
                ctName = ctFileName.substr(pos + 1, ctFileName.size() - pos - 1 - 3);

            // Look for the ColorTable node.
            DataNode *node2 = node->SearchForNode("ColorTable");
            if(node2 == 0)
                return;

            ColorControlPointList ccpl2;
            ccpl2.SetFromNode(node2);
            ccpl2.SetExternalFlag(true);

            // add tags to imported color tables
            if (importingPersonal)
            {
                // add the user defined tag if it is user defined
                if (! ccpl2.HasTag("User Defined"))
                    ccpl2.AddTag("User Defined");
                ccpl2.SetBuiltIn(false);
            }
            else
            {
                // add the "No Tags" tag if there are no tags
                if (ccpl2.GetNumTags() == 0)
                    ccpl2.AddTag("No Tags");
            }

            // Check for errors that would break code down the line
            int ii;
            bool broken = false;
            for (ii = 0 ; ii < ccpl2.GetNumControlPoints() ; ii++)
            {
                float pos = ccpl2[ii].GetPosition();
                if (pos < 0.0f || pos > 1.0)
                {
                    broken = true;
                    break;
                }
                if (ii >= 1)
                {
                    float prevPos = ccpl2[ii-1].GetPosition();
                    if (prevPos > pos)
                    {
                        broken = true;
                        break;
                    }
                }
            }
            if (broken)
            {
                debug4 << "Could not read " << ctFileName.c_str() << "!" << endl;
            }
            else
            {
                ctAtts->AddColorTable(ctName, ccpl2);

                debug4 << "Imported color table " << ctFileName.c_str()
                       << " as " << ctName.c_str() << endl;
            }
            delete node;
        }
        else
            debug4 << "Could not read " << ctFileName.c_str() << "!" << endl;
    }
}

