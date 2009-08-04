/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#include <SingleAttributeConfigManager.h>
#include <DataNode.h>
#include <DebugStream.h>
#include <Utility.h>
#include <visit-config.h>

// ****************************************************************************
// Method: SingleAttributeConfigManager::SingleAttributeConfigManager
//
// Purpose: 
//   Constructor for the SingleAttributeConfigManager class.
//
// Programmer: Jeremy Meredith
// Creation:   January  2, 2009
//
// Modifications:
//   
// ****************************************************************************

SingleAttributeConfigManager::SingleAttributeConfigManager(AttributeGroup *att)
{
    attribute = att;
}

// ****************************************************************************
// Method: SingleAttributeConfigManager::~SingleAttributeConfigManager
//
// Purpose: 
//   Destructor for the SingleAttributeConfigManager class.
//
// Programmer: Jeremy Meredith
// Creation:   January  2, 2009
//
// Modifications:
//   
// ****************************************************************************

SingleAttributeConfigManager::~SingleAttributeConfigManager()
{
}

// ****************************************************************************
// Method: SingleAttributeConfigManager::Export
//
// Purpose: 
//   Exports the attribute to the named file.
//
// Returns:    true on success; false otherwise
//
// Programmer: Jeremy Meredith
// Creation:   January  2, 2009
//
// Modifications:
//
// ****************************************************************************

bool
SingleAttributeConfigManager::Export(const std::string &filename)
{
    return WriteConfigFile(filename.c_str());
}

// ****************************************************************************
// Method: SingleAttributeConfigManager::Import
//
// Purpose: 
//   Imports the atribute from the named file.
//
// Returns:    true on success; false otherwise
//
// Programmer: Jeremy Meredith
// Creation:   January  2, 2009
//
// Modifications:
//
//   Tom Fogal, Fri Mar  6 10:26:12 MST 2009
//   Fix: function wasn't returning anything in the `success' case.
//   
// ****************************************************************************

bool
SingleAttributeConfigManager::Import(const std::string &filename)
{
    DataNode *node = ReadConfigFile(filename.c_str());

    if (!node)
        return false;

    attribute->SetFromNode(node);
    return true;
}

// ****************************************************************************
// Method: SingleAttributeConfigManager::WriteConfigFile
//
// Purpose: 
//   Writes the attribute to a file.
//
// Arguments:
//   filename : The name of the file to write.
//
// Programmer: Jeremy Meredith
// Creation:   January  2, 2009
//
// Modifications:
//
// ****************************************************************************

bool
SingleAttributeConfigManager::WriteConfigFile(const char *filename)
{
    // We need to start with a top-level container data node, but that's
    // not what we wind up saving; get the actual one from inside it.
    DataNode topLevel("topLevel");
    attribute->CreateNode(&topLevel, true, true);
    DataNode *actualNode = topLevel.GetNode(attribute->TypeName());
    if (!actualNode)
        return false;

    // Try to open the output file.
    if((fp = fopen(filename, "wb")) == 0)
    {
        return false;
    }

    // Write the output file.
    fprintf(fp, "<?xml version=\"1.0\"?>\n");
    WriteObject(actualNode);

    // Close the file
    fclose(fp);
    fp = 0;

    return true;
}

// ****************************************************************************
// Method: SingleAttributeConfigManager::ReadConfigFile
//
// Purpose: 
//   Reads the attribute from a file.
//
// Arguments:
//   filename : The name of the file to open.
//
// Programmer: Jeremy Meredith
// Creation:   January  2, 2009
//
// Modifications:
//   
// ****************************************************************************

DataNode *
SingleAttributeConfigManager::ReadConfigFile(const char *filename)
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

