/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <VLIFileManager.h>
#include <DataNode.h>
#include <DebugStream.h>
#include <Utility.h>
#include <visit-config.h>

// ****************************************************************************
// Method: VLIFileManager::VLIFileManager
//
// Purpose: 
//   Constructor for the VLIFileManager class.
//
// Programmer: Markus Glatter 
// Creation:   Fri Aug 10 11:13:00 EDT 2007
//
// Modifications:
//   
// ****************************************************************************

VLIFileManager::VLIFileManager()
{
    this->dataset = NULL;
    this->axis[0] = this->axis[1] = this->axis[2] = this->axis[3] = -1;
    this->name = std::string();
    this->syscall = std::string();
    this->server = std::string();
    this->datafile = std::string();
    this->nservers = -1;
}

// ****************************************************************************
// Method: VLIFileManager::~VLIFileManager
//
// Purpose: 
//   Destructor for the VLIFileManager class.
//
// Programmer: Markus Glatter 
// Creation:   Fri Aug 10 11:13:00 EDT 2007
//
// Modifications:
//   
// ****************************************************************************

VLIFileManager::~VLIFileManager()
{
}


// ****************************************************************************
// Method: VLIFileManager::WriteConfigFile
//
// Purpose: 
//   Writes a vli configuration to a file.
//
// Arguments:
//   filename : The name of the file to write.
//
// Programmer: Markus Glatter 
// Creation:   Fri Aug 10 11:13:00 EDT 2007
//
// Modifications:
//
// ****************************************************************************

bool
VLIFileManager::WriteConfigFile(const char *filename)
{
    DataNode topLevel("topLevel");

    // Create the color table node.
    DataNode *ctNode = new DataNode("VLIFile");
    topLevel.AddNode(ctNode);
    ctNode->AddNode(new DataNode("Version", std::string(VERSION)));

    // Let the vli file manager create and add its information to the node.
    ctNode->AddNode(Export());

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
// Method: VLIFileManager::ReadConfigFile
//
// Purpose: 
//   Reads a vli configuration from a file.
//
// Arguments:
//   filename : The name of the file to open.
//
// Programmer: Markus Glatter 
// Creation:   Fri Aug 10 11:13:00 EDT 2007
//
// Modifications:
//   
// ****************************************************************************

DataNode *
VLIFileManager::ReadConfigFile(const char *filename)
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
    
    return Import(node);
}

// ****************************************************************************
// Method: VLIFileManager::Export
//
// Purpose: 
//   Tries to export the vli configuration to the named file.
//
// Programmer: Markus Glatter 
// Creation:   Fri Aug 10 11:13:00 EDT 2007
//
// Modifications:
//
// ****************************************************************************

DataNode *
VLIFileManager::Export()
{
    DataNode *node = new DataNode("vli");
    node->AddNode(new DataNode("name", name));
    node->AddNode(new DataNode("axis", axis, 4));
    node->AddNode(new DataNode("nservers", nservers));
    node->AddNode(new DataNode("syscall", syscall));
    node->AddNode(new DataNode("server", server));
    node->AddNode(new DataNode("datafile", datafile));
    
    if (this->dataset != NULL) node->AddNode(dataset->Export(true));

    return node;
}

// ****************************************************************************
// Method: VLIFileManager::Import
//
// Purpose: 
//   Tries to import the named file as a vli configuration.
//
// Programmer: Markus Glatter 
// Creation:   Fri Aug 10 11:13:00 EDT 2007
//
// Modifications:
//
// ****************************************************************************

DataNode *
VLIFileManager::Import(DataNode *node)
{
    DataNode *vliNode = node->GetNode("vli");
    
    if (vliNode == NULL) return NULL;
    if ((vliNode->GetNode("axis") == NULL) || (vliNode->GetNode("nservers") == NULL) || (vliNode->GetNode("dataset") == NULL)) return NULL;
    if ((vliNode->GetNode("syscall") == NULL) || (vliNode->GetNode("server") == NULL) || (vliNode->GetNode("datafile") == NULL)) return NULL;
    
    if (vliNode->GetNode("name") != NULL) this->name = std::string(vliNode->GetNode("name")->AsString());
    if (vliNode->GetNode("axis") != NULL) this->setAxis(vliNode->GetNode("axis")->AsIntArray());
    if (vliNode->GetNode("nservers") != NULL) this->setNoDataServers(vliNode->GetNode("nservers")->AsInt());
    if (vliNode->GetNode("syscall") != NULL) this->syscall = std::string(vliNode->GetNode("syscall")->AsString());
    if (vliNode->GetNode("server") != NULL) this->server = std::string(vliNode->GetNode("server")->AsString());
    if (vliNode->GetNode("datafile") != NULL) this->datafile = std::string(vliNode->GetNode("datafile")->AsString());
    if (vliNode->GetNode("dataset") != NULL) {
        this->dataset = new VLIDataset();
        if (this->dataset->Import(vliNode->GetNode("dataset")) == false) return NULL;
    }
    
    return node;
}

// ****************************************************************************
// Methods: VLIFileManager::getDataset
//          VLIFileManager::setDataset
//          VLIFileManager::getName
//          VLIFileManager::setName
//          VLIFileManager::getAxis
//          VLIFileManager::setAxis
//          VLIFileManager::getNoDataServers
//          VLIFileManager::setNoDataServers
//          VLIFileManager::getSyscall
//          VLIFileManager::setSyscall
//          VLIFileManager::getServer
//          VLIFileManager::getDatafile
//          VLIFileManager::setDatafile
//
// Purpose: 
//   Various functions to get and set class properties.
//
// Programmer: Markus Glatter 
// Creation:   Fri Aug 10 11:13:00 EDT 2007
//
// Modifications:
//
// ****************************************************************************

VLIDataset *
VLIFileManager::getDataset()
{
    return this->dataset;
}

void
VLIFileManager::setDataset(VLIDataset *dataset)
{
    this->dataset = dataset;
}

std::string
VLIFileManager::getName()
{
    return std::string(this->name);
}

void
VLIFileManager::setName(std::string name)
{
    this->name = std::string(name);
}

const int*
VLIFileManager::getAxis()
{
    int *a = new int[4];
    for (int i = 0; i < 4; ++i) a[i] = this->axis[i];
    
    return a;
}

void
VLIFileManager::setAxis(const int* axis)
{
    for (int i = 0; i < 4; ++i) this->axis[i] = axis[i];
}
    
int               
VLIFileManager::getNoDataServers()
{
    return this->nservers;
}

void              
VLIFileManager::setNoDataServers(int n) 
{
    this->nservers = n;
}
    
std::string       
VLIFileManager::getSyscall()
{
    return std::string(this->syscall);
}

void              
VLIFileManager::setSyscall(std::string syscall)
{
    this->syscall = std::string(syscall);
}

std::string       
VLIFileManager::getServer()
{
    return std::string(this->server);
}

void              
VLIFileManager::setServer(std::string server)
{
    this->server = std::string(server);
}

std::string       
VLIFileManager::getDatafile()
{
    return std::string(this->datafile);
}

void              
VLIFileManager::setDatafile(std::string datafile)
{
    this->datafile = std::string(datafile);
}

// ****************************************************************************
// Method: VLIAttribute::VLIAttribute
//
// Purpose: 
//   Constructors for the VLIAttribute class.
//
// Programmer: Markus Glatter 
// Creation:   Fri Aug 10 11:13:00 EDT 2007
//
// Modifications:
//   
// ****************************************************************************

VLIAttribute::VLIAttribute()
{
    this->Set("", 0.0, 0.0, true);
}

VLIAttribute::VLIAttribute(std::string name, double min, double max, bool isScaled)
{
    this->Set(name, min, max, isScaled);
}

VLIAttribute::VLIAttribute(DataNode *node)
{
    this->Import(node);
}

// ****************************************************************************
// Method: VLIAttribute::~VLIAttribute
//
// Purpose: 
//   Destructor for the VLIAttribute class.
//
// Programmer: Markus Glatter 
// Creation:   Fri Aug 10 11:13:00 EDT 2007
//
// Modifications:
//   
// ****************************************************************************

VLIAttribute::~VLIAttribute()
{
}

// ****************************************************************************
// Method: VLIAttribute::Set
//
// Purpose: 
//   Sets properties of VLIAttribute object.
//
// Programmer: Markus Glatter 
// Creation:   Fri Aug 10 11:13:00 EDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
VLIAttribute::Set(std::string name, double min, double max, bool isScaled) 
{
    this->name = std::string(name);
    this->min = min;
    this->max = max;
    this->isScaled = isScaled;
}

// ****************************************************************************
// Method: VLIAttribute::Import
//
// Purpose: 
//   Tries to import a VLIAttribute object from a given data node
//
// Programmer: Markus Glatter 
// Creation:   Fri Aug 10 11:13:00 EDT 2007
//
// Modifications:
//   
// ****************************************************************************

bool
VLIAttribute::Import(DataNode *node)
{
    this->Set("", 0.0, 0.0, true);
    
    if ((node != NULL) && (node->GetKey().find("attribute") != std::string::npos)) {
        if (node->GetNode("name")   != NULL) this->name     = std::string(node->GetNode("name")->AsString()); else return false;
        if (node->GetNode("min")    != NULL) this->min      = node->GetNode("min")->AsDouble(); else return false;
        if (node->GetNode("max")    != NULL) this->max      = node->GetNode("max")->AsDouble(); else return false;
        if (node->GetNode("scaled") != NULL) this->isScaled = node->GetNode("scaled")->AsBool();
        
        return true;
    }
    
    return false;
}

// ****************************************************************************
// Method: VLIAttribute::Export
//
// Purpose: 
//   Tries to export the VLIAttribute object to a data node
//
// Programmer: Markus Glatter 
// Creation:   Fri Aug 10 11:13:00 EDT 2007
//
// Modifications:
//   
// ****************************************************************************

DataNode *
VLIAttribute::Export()
{
    DataNode *node = new DataNode("attribute");
    node->AddNode(new DataNode("name", name));
    node->AddNode(new DataNode("min", min));
    node->AddNode(new DataNode("max", max));
    node->AddNode(new DataNode("scaled", isScaled));
    
    return node;
}

// ****************************************************************************
// Method: VLIAttribute::GetName
//         VLIAttribute::GetMin 
//         VLIAttribute::GetMax 
//         VLIAttribute::GetIsScaled 
//
// Purpose: 
//   Various get methods for VLIAttribute properties.
//
// Programmer: Markus Glatter 
// Creation:   Fri Aug 10 11:13:00 EDT 2007
//
// Modifications:
//   
// ****************************************************************************

std::string
VLIAttribute::GetName()
{
    return std::string(this->name);
}

double      
VLIAttribute::GetMin()
{
    return this->min;
}

double      
VLIAttribute::GetMax()
{
    return this->max;
}

bool      
VLIAttribute::GetIsScaled()
{
    return this->isScaled;
}

// ****************************************************************************
// Method: VLIDataset::VLIDataset
//
// Purpose: 
//   Constructors for the VLIDataset class.
//
// Programmer: Markus Glatter 
// Creation:   Fri Aug 10 11:13:00 EDT 2007
//
// Modifications:
//   
// ****************************************************************************

VLIDataset::VLIDataset()
{
    this->attributes = NULL;
    this->Set(0, "", 0);
}
    
VLIDataset::VLIDataset(const int nattr, const std::string format, const long nitems)
{
    this->attributes = NULL;
    this->Set(nattr, format, nitems);
}

VLIDataset::VLIDataset(DataNode *node)
{
    this->Import(node);
}

// ****************************************************************************
// Method: VLIDataset::~VLIDataset
//
// Purpose: 
//   Destructor for the VLIDataset class.
//
// Programmer: Markus Glatter 
// Creation:   Fri Aug 10 11:13:00 EDT 2007
//
// Modifications:
//   
// ****************************************************************************

VLIDataset::~VLIDataset()
{
    if (this->attributes != NULL) delete[] this->attributes;
}

// ****************************************************************************
// Method: VLIDataset::Set
//
// Purpose: 
//   Sets properties of VLIDataset object.
//
// Programmer: Markus Glatter 
// Creation:   Fri Aug 10 11:13:00 EDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
VLIDataset::Set(const int nattr, const std::string format, const long nitems)
{
    this->nattr = nattr;
    this->format = std::string(format);
    this->nitems = nitems;
    if (this->attributes != NULL) delete[] this->attributes;
    if (this->nattr > 0) this->attributes = new VLIAttribute[nattr];
}

// ****************************************************************************
// Method: VLIDataset::Export
//
// Purpose: 
//   Tries to export the VLIDataset object to a data node
//
// Programmer: Markus Glatter 
// Creation:   Fri Aug 10 11:13:00 EDT 2007
//
// Modifications:
//   
// ****************************************************************************

DataNode *
VLIDataset::Export(bool withAttributes)
{
    DataNode *node = new DataNode("dataset");
    node->AddNode(new DataNode("attributes", nattr));
    node->AddNode(new DataNode("format", std::string(format)));
    node->AddNode(new DataNode("nitems", nitems));
    
    if (withAttributes == true) {
        for (int i = 0; i < nattr; ++i) {
            char buf[15];
            sprintf(buf, "attribute %d", i);
            DataNode *aNode = this->attributes[i].Export();
            aNode->SetKey(std::string(buf));
	    node->AddNode(aNode);
        }
    }
    
    return node;
}

// ****************************************************************************
// Method: VLIDataset::Import
//
// Purpose: 
//   Tries to import a VLIDataset object from a given data node
//
// Programmer: Markus Glatter 
// Creation:   Fri Aug 10 11:13:00 EDT 2007
//
// Modifications:
//   
// ****************************************************************************

bool
VLIDataset::Import(DataNode *node)
{
    this->Set(0, "", 0);
    
    if ((node != NULL) && (node->GetKey().find("dataset") != std::string::npos)) {
        if (node->GetNode("attributes") != NULL) this->nattr = node->GetNode("attributes")->AsInt(); else return false;
        if (node->GetNode("format")  != NULL) this->format = std::string(node->GetNode("format")->AsString());
        if (node->GetNode("nitems")  != NULL) this->nitems = node->GetNode("nitems")->AsLong();
        
        this->attributes = new VLIAttribute[nattr];

        for (int i = 0; i < nattr; ++i) {
            char buf[15];
            sprintf(buf, "attribute %d", i);
            DataNode *aNode = node->GetNode(std::string(buf));
            aNode->SetKey("attribute");
            if (attributes[i].Import(aNode) != true) return false;
        }
        
        return true;
    }
    
    return false;
}

// ****************************************************************************
// Method: VLIDataset::getFormat
//         VLIDataset::getNItems 
//         VLIDataset::getNAttributes 
//         VLIDataset::getAttribute 
//
// Purpose: 
//   Various get methods for VLIDataset properties.
//
// Programmer: Markus Glatter 
// Creation:   Fri Aug 10 11:13:00 EDT 2007
//
// Modifications:
//   
// ****************************************************************************

std::string
VLIDataset::getFormat()
{
    return std::string(this->format);
}

long
VLIDataset::getNItems()
{
    return this->nitems;
}

int
VLIDataset::getNAttributes()
{
    return this->nattr;
}

VLIAttribute *
VLIDataset::getAttribute(const int i)
{
    if ((i >= this->nattr) || (this->attributes == NULL)) return NULL;
    
    return &(this->attributes[i]);
}

