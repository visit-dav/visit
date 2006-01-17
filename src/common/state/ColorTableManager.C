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
//   Brad Whitlock, Thu Nov 13 11:55:17 PDT 2003
//   I made it throw a VisItException if the file can't be opened.
//
//   Brad Whitlock, Thu Dec 18 11:18:06 PDT 2003
//   I made it call CreateNode with the new completeSave flag set to false.
//
//   Brad Whitlock, Thu Feb 17 15:55:29 PST 2005
//   I removed the exception and made the function return a bool.
//
// ****************************************************************************

bool
ColorTableManager::WriteConfigFile(const char *filename)
{
    DataNode topLevel("topLevel");

    // Create the color table node.
    DataNode *ctNode = new DataNode("ColorTable");
    topLevel.AddNode(ctNode);
    ctNode->AddNode(new DataNode("Version", std::string(VERSION)));

    // Let the color table create add its information to tbe node.
    ccpl.CreateNode(ctNode, false, true);

    // Try to open the output file.
    if((fp = fopen(filename, "wb")) == 0)
    {
        return false;
    }

    // Write the output file.
    fprintf(fp, "<?xml version=\"1.0\"?>\n");
    WriteObject(ctNode);

    // Close the file
    fclose(fp);
    fp = 0;

    return true;
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
//   Brad Whitlock, Tue Jan 17 11:17:09 PDT 2006
//   I added code to prevent the file path from being added to the color
//   table name.
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
            std::string::size_type pos = ctFileName.rfind(SLASH_STRING);
            if(pos == std::string::npos)
                ctName = ctFileName;
            else
                ctName = ctFileName.substr(pos + 1, ctFileName.size() - pos - 1 - 3);
            
            debug4 << "Imported color table " << ctFileName.c_str() << " as "
                   << ctName.c_str() << endl;
            ColorControlPointList ccpl2;
            ccpl2.SetFromNode(node);
            ccpl2.SetExternalFlag(true);
            ctAtts->AddColorTable(ctName, ccpl2);
            delete node;
        }
        else
            debug4 << "Could not read " << ctFileName.c_str() << "!" << endl;
    }
}

