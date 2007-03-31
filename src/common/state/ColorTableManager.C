#include <ColorTableManager.h>
#include <DataNode.h>
#include <DebugStream.h>
#include <Utility.h>
#include <visit-config.h>

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
//   
// ****************************************************************************

ColorTableManager::ColorTableManager() : ccpl()
{
    ctAtts = 0;
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
//   
// ****************************************************************************

std::string
ColorTableManager::Export(const std::string &ctName,
    const ColorControlPointList &ccpl_)
{
    ccpl = ccpl_;

    std::string ctFileName(GetUserVisItDirectory() + ctName + ".ct");
    WriteConfigFile(ctFileName.c_str());

    return std::string("VisIt exported color table \"") + ctName +
           std::string( "\" to the file: ") + ctFileName + ". You can share "
           "that file with colleagues who want to use your color table. Simply "
           "put the file in their .visit directory, run VisIt and the color table "
           "will appear in their list of color tables when VisIt starts up.";
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
//   
// ****************************************************************************

bool
ColorTableManager::ImportColorTables(ColorTableAttributes *cta)
{
    //
    // Read the user's home VisIt directory and import all of the color tables.
    //
    ctAtts = cta;
    return ReadAndProcessDirectory(GetUserVisItDirectory(), ImportHelper,
                                   (void*)this, false);
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
//   
// ****************************************************************************

void
ColorTableManager::WriteConfigFile(const char *filename)
{
    DataNode topLevel("topLevel");

    // Create the color table node.
    DataNode *ctNode = new DataNode("ColorTable");
    topLevel.AddNode(ctNode);
    ctNode->AddNode(new DataNode("Version", std::string(VERSION)));

    // Let the color table create add its information to tbe node.
    ccpl.CreateNode(ctNode, true);

    // Try to open the output file.
    if((fp = fopen(filename, "wb")) == 0)
        return;

    // Write the output file.
    fprintf(fp, "<?xml version=\"1.0\"?>\n");
    WriteObject(ctNode);

    // Close the file
    fclose(fp);
    fp = 0;
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
ColorTableManager::ReadConfigFile(const char *filename)
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
            debug4 << "Imported color table " << ctFileName.c_str() << endl;
            ColorControlPointList ccpl2;
            ccpl2.SetFromNode(node);
            ccpl2.SetExternalFlag(true);
            ctAtts->AddColorTable(ctFileName, ccpl2);
            delete node;
        }
        else
            debug4 << "Could not read " << ctFileName.c_str() << "!" << endl;
    }
}

