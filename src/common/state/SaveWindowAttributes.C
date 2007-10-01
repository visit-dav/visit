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

#include <SaveWindowAttributes.h>
#include <DataNode.h>

//
// Enum conversion methods for SaveWindowAttributes::FileFormat
//

static const char *FileFormat_strings[] = {
"BMP", "CURVE", "JPEG", 
"OBJ", "PNG", "POSTSCRIPT", 
"POVRAY", "PPM", "RGB", 
"STL", "TIFF", "ULTRA", 
"VTK"};

std::string
SaveWindowAttributes::FileFormat_ToString(SaveWindowAttributes::FileFormat t)
{
    int index = int(t);
    if(index < 0 || index >= 13) index = 0;
    return FileFormat_strings[index];
}

std::string
SaveWindowAttributes::FileFormat_ToString(int t)
{
    int index = (t < 0 || t >= 13) ? 0 : t;
    return FileFormat_strings[index];
}

bool
SaveWindowAttributes::FileFormat_FromString(const std::string &s, SaveWindowAttributes::FileFormat &val)
{
    val = SaveWindowAttributes::BMP;
    for(int i = 0; i < 13; ++i)
    {
        if(s == FileFormat_strings[i])
        {
            val = (FileFormat)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for SaveWindowAttributes::CompressionType
//

static const char *CompressionType_strings[] = {
"None", "PackBits", "Jpeg", 
"Deflate"};

std::string
SaveWindowAttributes::CompressionType_ToString(SaveWindowAttributes::CompressionType t)
{
    int index = int(t);
    if(index < 0 || index >= 4) index = 0;
    return CompressionType_strings[index];
}

std::string
SaveWindowAttributes::CompressionType_ToString(int t)
{
    int index = (t < 0 || t >= 4) ? 0 : t;
    return CompressionType_strings[index];
}

bool
SaveWindowAttributes::CompressionType_FromString(const std::string &s, SaveWindowAttributes::CompressionType &val)
{
    val = SaveWindowAttributes::None;
    for(int i = 0; i < 4; ++i)
    {
        if(s == CompressionType_strings[i])
        {
            val = (CompressionType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for SaveWindowAttributes::ResConstraint
//

static const char *ResConstraint_strings[] = {
"NoConstaint", "EqualWidthHeight", "ScreenProportions"
};

std::string
SaveWindowAttributes::ResConstraint_ToString(SaveWindowAttributes::ResConstraint t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return ResConstraint_strings[index];
}

std::string
SaveWindowAttributes::ResConstraint_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return ResConstraint_strings[index];
}

bool
SaveWindowAttributes::ResConstraint_FromString(const std::string &s, SaveWindowAttributes::ResConstraint &val)
{
    val = SaveWindowAttributes::NoConstaint;
    for(int i = 0; i < 3; ++i)
    {
        if(s == ResConstraint_strings[i])
        {
            val = (ResConstraint)i;
            return true;
        }
    }
    return false;
}

// Type map format string
const char *SaveWindowAttributes::TypeMapFormatString = "bssbiiibbibbsbibi";

// ****************************************************************************
// Method: SaveWindowAttributes::SaveWindowAttributes
//
// Purpose: 
//   Constructor for the SaveWindowAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Sep 27 17:37:29 PST 2007
//
// Modifications:
//   
// ****************************************************************************

SaveWindowAttributes::SaveWindowAttributes() : 
    AttributeSubject(SaveWindowAttributes::TypeMapFormatString)
{
#if defined(_WIN32)
    outputToCurrentDirectory = false;
#else
    outputToCurrentDirectory = true;
#endif
#if defined(_WIN32)
    const char *visitHome = getenv("VISITUSERHOME");
    if(visitHome != 0)
    {
        outputDirectory = std::string(visitHome) + "\\My images";
    }
    else
    {
        visitHome = getenv("VISITHOME");
        if(visitHome != 0)
        {
            outputDirectory = std::string(visitHome) + "\\My images";
        }
        else
        {
            outputDirectory = ".";
        }
    }
#else
    outputDirectory = ".";
#endif
    fileName = "visit";
    family = true;
    format = TIFF;
    width = 1024;
    height = 1024;
    screenCapture = false;
    saveTiled = false;
    quality = 80;
    progressive = false;
    binary = false;
    stereo = false;
    compression = PackBits;
    forceMerge = false;
    resConstraint = ScreenProportions;
}

// ****************************************************************************
// Method: SaveWindowAttributes::SaveWindowAttributes
//
// Purpose: 
//   Copy constructor for the SaveWindowAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Sep 27 17:37:29 PST 2007
//
// Modifications:
//   
// ****************************************************************************

SaveWindowAttributes::SaveWindowAttributes(const SaveWindowAttributes &obj) : 
    AttributeSubject(SaveWindowAttributes::TypeMapFormatString)
{
    outputToCurrentDirectory = obj.outputToCurrentDirectory;
    outputDirectory = obj.outputDirectory;
    fileName = obj.fileName;
    family = obj.family;
    format = obj.format;
    width = obj.width;
    height = obj.height;
    screenCapture = obj.screenCapture;
    saveTiled = obj.saveTiled;
    quality = obj.quality;
    progressive = obj.progressive;
    binary = obj.binary;
    lastRealFilename = obj.lastRealFilename;
    stereo = obj.stereo;
    compression = obj.compression;
    forceMerge = obj.forceMerge;
    resConstraint = obj.resConstraint;

    SelectAll();
}

// ****************************************************************************
// Method: SaveWindowAttributes::~SaveWindowAttributes
//
// Purpose: 
//   Destructor for the SaveWindowAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Sep 27 17:37:29 PST 2007
//
// Modifications:
//   
// ****************************************************************************

SaveWindowAttributes::~SaveWindowAttributes()
{
    // nothing here
}

// ****************************************************************************
// Method: SaveWindowAttributes::operator = 
//
// Purpose: 
//   Assignment operator for the SaveWindowAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Sep 27 17:37:29 PST 2007
//
// Modifications:
//   
// ****************************************************************************

SaveWindowAttributes& 
SaveWindowAttributes::operator = (const SaveWindowAttributes &obj)
{
    if (this == &obj) return *this;
    outputToCurrentDirectory = obj.outputToCurrentDirectory;
    outputDirectory = obj.outputDirectory;
    fileName = obj.fileName;
    family = obj.family;
    format = obj.format;
    width = obj.width;
    height = obj.height;
    screenCapture = obj.screenCapture;
    saveTiled = obj.saveTiled;
    quality = obj.quality;
    progressive = obj.progressive;
    binary = obj.binary;
    lastRealFilename = obj.lastRealFilename;
    stereo = obj.stereo;
    compression = obj.compression;
    forceMerge = obj.forceMerge;
    resConstraint = obj.resConstraint;

    SelectAll();
    return *this;
}

// ****************************************************************************
// Method: SaveWindowAttributes::operator == 
//
// Purpose: 
//   Comparison operator == for the SaveWindowAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Sep 27 17:37:29 PST 2007
//
// Modifications:
//   
// ****************************************************************************

bool
SaveWindowAttributes::operator == (const SaveWindowAttributes &obj) const
{
    // Create the return value
    return ((outputToCurrentDirectory == obj.outputToCurrentDirectory) &&
            (outputDirectory == obj.outputDirectory) &&
            (fileName == obj.fileName) &&
            (family == obj.family) &&
            (format == obj.format) &&
            (width == obj.width) &&
            (height == obj.height) &&
            (screenCapture == obj.screenCapture) &&
            (saveTiled == obj.saveTiled) &&
            (quality == obj.quality) &&
            (progressive == obj.progressive) &&
            (binary == obj.binary) &&
            (lastRealFilename == obj.lastRealFilename) &&
            (stereo == obj.stereo) &&
            (compression == obj.compression) &&
            (forceMerge == obj.forceMerge) &&
            (resConstraint == obj.resConstraint));
}

// ****************************************************************************
// Method: SaveWindowAttributes::operator != 
//
// Purpose: 
//   Comparison operator != for the SaveWindowAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Sep 27 17:37:29 PST 2007
//
// Modifications:
//   
// ****************************************************************************

bool
SaveWindowAttributes::operator != (const SaveWindowAttributes &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: SaveWindowAttributes::TypeName
//
// Purpose: 
//   Type name method for the SaveWindowAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Sep 27 17:37:29 PST 2007
//
// Modifications:
//   
// ****************************************************************************

const std::string
SaveWindowAttributes::TypeName() const
{
    return "SaveWindowAttributes";
}

// ****************************************************************************
// Method: SaveWindowAttributes::CopyAttributes
//
// Purpose: 
//   CopyAttributes method for the SaveWindowAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Sep 27 17:37:29 PST 2007
//
// Modifications:
//   
// ****************************************************************************

bool
SaveWindowAttributes::CopyAttributes(const AttributeGroup *atts)
{
    if(TypeName() != atts->TypeName())
        return false;

    // Call assignment operator.
    const SaveWindowAttributes *tmp = (const SaveWindowAttributes *)atts;
    *this = *tmp;

    return true;
}

// ****************************************************************************
// Method: SaveWindowAttributes::CreateCompatible
//
// Purpose: 
//   CreateCompatible method for the SaveWindowAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Sep 27 17:37:29 PST 2007
//
// Modifications:
//   
// ****************************************************************************

AttributeSubject *
SaveWindowAttributes::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;
    if(TypeName() == tname)
        retval = new SaveWindowAttributes(*this);
    // Other cases could go here too. 

    return retval;
}

// ****************************************************************************
// Method: SaveWindowAttributes::NewInstance
//
// Purpose: 
//   NewInstance method for the SaveWindowAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Sep 27 17:37:29 PST 2007
//
// Modifications:
//   
// ****************************************************************************

AttributeSubject *
SaveWindowAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new SaveWindowAttributes(*this);
    else
        retval = new SaveWindowAttributes;

    return retval;
}

// ****************************************************************************
// Method: SaveWindowAttributes::SelectAll
//
// Purpose: 
//   Selects all attributes.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Sep 27 17:37:29 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
SaveWindowAttributes::SelectAll()
{
    Select(0, (void *)&outputToCurrentDirectory);
    Select(1, (void *)&outputDirectory);
    Select(2, (void *)&fileName);
    Select(3, (void *)&family);
    Select(4, (void *)&format);
    Select(5, (void *)&width);
    Select(6, (void *)&height);
    Select(7, (void *)&screenCapture);
    Select(8, (void *)&saveTiled);
    Select(9, (void *)&quality);
    Select(10, (void *)&progressive);
    Select(11, (void *)&binary);
    Select(12, (void *)&lastRealFilename);
    Select(13, (void *)&stereo);
    Select(14, (void *)&compression);
    Select(15, (void *)&forceMerge);
    Select(16, (void *)&resConstraint);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SaveWindowAttributes::CreateNode
//
// Purpose: 
//   This method creates a DataNode representation of the object so it can be saved to a config file.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Sep 27 17:37:29 PST 2007
//
// Modifications:
//   
// ****************************************************************************

bool
SaveWindowAttributes::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if(parentNode == 0)
        return false;

    SaveWindowAttributes defaultObject;
    bool addToParent = false;
    // Create a node for SaveWindowAttributes.
    DataNode *node = new DataNode("SaveWindowAttributes");

    if(completeSave || !FieldsEqual(0, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("outputToCurrentDirectory", outputToCurrentDirectory));
    }

    if(completeSave || !FieldsEqual(1, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("outputDirectory", outputDirectory));
    }

    if(completeSave || !FieldsEqual(2, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("fileName", fileName));
    }

    if(completeSave || !FieldsEqual(3, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("family", family));
    }

    if(completeSave || !FieldsEqual(4, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("format", FileFormat_ToString(format)));
    }

    if(completeSave || !FieldsEqual(5, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("width", width));
    }

    if(completeSave || !FieldsEqual(6, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("height", height));
    }

    if(completeSave || !FieldsEqual(7, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("screenCapture", screenCapture));
    }

    if(completeSave || !FieldsEqual(8, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("saveTiled", saveTiled));
    }

    if(completeSave || !FieldsEqual(9, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("quality", quality));
    }

    if(completeSave || !FieldsEqual(10, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("progressive", progressive));
    }

    if(completeSave || !FieldsEqual(11, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("binary", binary));
    }

    if(completeSave || !FieldsEqual(12, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("lastRealFilename", lastRealFilename));
    }

    if(completeSave || !FieldsEqual(13, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("stereo", stereo));
    }

    if(completeSave || !FieldsEqual(14, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("compression", CompressionType_ToString(compression)));
    }

    if(completeSave || !FieldsEqual(15, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("forceMerge", forceMerge));
    }

    if(completeSave || !FieldsEqual(16, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("resConstraint", ResConstraint_ToString(resConstraint)));
    }


    // Add the node to the parent node.
    if(addToParent || forceAdd)
        parentNode->AddNode(node);
    else
        delete node;

    return (addToParent || forceAdd);
}

// ****************************************************************************
// Method: SaveWindowAttributes::SetFromNode
//
// Purpose: 
//   This method sets attributes in this object from values in a DataNode representation of the object.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Sep 27 17:37:29 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
SaveWindowAttributes::SetFromNode(DataNode *parentNode)
{
    int i;
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("SaveWindowAttributes");
    if(searchNode == 0)
        return;

    DataNode *node;
    if((node = searchNode->GetNode("outputToCurrentDirectory")) != 0)
        SetOutputToCurrentDirectory(node->AsBool());
    if((node = searchNode->GetNode("outputDirectory")) != 0)
        SetOutputDirectory(node->AsString());
    if((node = searchNode->GetNode("fileName")) != 0)
        SetFileName(node->AsString());
    if((node = searchNode->GetNode("family")) != 0)
        SetFamily(node->AsBool());
    if((node = searchNode->GetNode("format")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 13)
                SetFormat(FileFormat(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            FileFormat value;
            if(FileFormat_FromString(node->AsString(), value))
                SetFormat(value);
        }
    }
    if((node = searchNode->GetNode("width")) != 0)
        SetWidth(node->AsInt());
    if((node = searchNode->GetNode("height")) != 0)
        SetHeight(node->AsInt());
    if((node = searchNode->GetNode("screenCapture")) != 0)
        SetScreenCapture(node->AsBool());
    if((node = searchNode->GetNode("saveTiled")) != 0)
        SetSaveTiled(node->AsBool());
    if((node = searchNode->GetNode("quality")) != 0)
        SetQuality(node->AsInt());
    if((node = searchNode->GetNode("progressive")) != 0)
        SetProgressive(node->AsBool());
    if((node = searchNode->GetNode("binary")) != 0)
        SetBinary(node->AsBool());
    if((node = searchNode->GetNode("lastRealFilename")) != 0)
        SetLastRealFilename(node->AsString());
    if((node = searchNode->GetNode("stereo")) != 0)
        SetStereo(node->AsBool());
    if((node = searchNode->GetNode("compression")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 4)
                SetCompression(CompressionType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            CompressionType value;
            if(CompressionType_FromString(node->AsString(), value))
                SetCompression(value);
        }
    }
    if((node = searchNode->GetNode("forceMerge")) != 0)
        SetForceMerge(node->AsBool());
    if((node = searchNode->GetNode("resConstraint")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetResConstraint(ResConstraint(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            ResConstraint value;
            if(ResConstraint_FromString(node->AsString(), value))
                SetResConstraint(value);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
SaveWindowAttributes::SetOutputToCurrentDirectory(bool outputToCurrentDirectory_)
{
    outputToCurrentDirectory = outputToCurrentDirectory_;
    Select(0, (void *)&outputToCurrentDirectory);
}

void
SaveWindowAttributes::SetOutputDirectory(const std::string &outputDirectory_)
{
    outputDirectory = outputDirectory_;
    Select(1, (void *)&outputDirectory);
}

void
SaveWindowAttributes::SetFileName(const std::string &fileName_)
{
    fileName = fileName_;
    Select(2, (void *)&fileName);
}

void
SaveWindowAttributes::SetFamily(bool family_)
{
    family = family_;
    Select(3, (void *)&family);
}

void
SaveWindowAttributes::SetFormat(SaveWindowAttributes::FileFormat format_)
{
    format = format_;
    Select(4, (void *)&format);
}

void
SaveWindowAttributes::SetWidth(int width_)
{
    width = width_;
    Select(5, (void *)&width);
}

void
SaveWindowAttributes::SetHeight(int height_)
{
    height = height_;
    Select(6, (void *)&height);
}

void
SaveWindowAttributes::SetScreenCapture(bool screenCapture_)
{
    screenCapture = screenCapture_;
    Select(7, (void *)&screenCapture);
}

void
SaveWindowAttributes::SetSaveTiled(bool saveTiled_)
{
    saveTiled = saveTiled_;
    Select(8, (void *)&saveTiled);
}

void
SaveWindowAttributes::SetQuality(int quality_)
{
    quality = quality_;
    Select(9, (void *)&quality);
}

void
SaveWindowAttributes::SetProgressive(bool progressive_)
{
    progressive = progressive_;
    Select(10, (void *)&progressive);
}

void
SaveWindowAttributes::SetBinary(bool binary_)
{
    binary = binary_;
    Select(11, (void *)&binary);
}

void
SaveWindowAttributes::SetLastRealFilename(const std::string &lastRealFilename_)
{
    lastRealFilename = lastRealFilename_;
    Select(12, (void *)&lastRealFilename);
}

void
SaveWindowAttributes::SetStereo(bool stereo_)
{
    stereo = stereo_;
    Select(13, (void *)&stereo);
}

void
SaveWindowAttributes::SetCompression(SaveWindowAttributes::CompressionType compression_)
{
    compression = compression_;
    Select(14, (void *)&compression);
}

void
SaveWindowAttributes::SetForceMerge(bool forceMerge_)
{
    forceMerge = forceMerge_;
    Select(15, (void *)&forceMerge);
}

void
SaveWindowAttributes::SetResConstraint(SaveWindowAttributes::ResConstraint resConstraint_)
{
    resConstraint = resConstraint_;
    Select(16, (void *)&resConstraint);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

bool
SaveWindowAttributes::GetOutputToCurrentDirectory() const
{
    return outputToCurrentDirectory;
}

const std::string &
SaveWindowAttributes::GetOutputDirectory() const
{
    return outputDirectory;
}

std::string &
SaveWindowAttributes::GetOutputDirectory()
{
    return outputDirectory;
}

const std::string &
SaveWindowAttributes::GetFileName() const
{
    return fileName;
}

std::string &
SaveWindowAttributes::GetFileName()
{
    return fileName;
}

bool
SaveWindowAttributes::GetFamily() const
{
    return family;
}

SaveWindowAttributes::FileFormat
SaveWindowAttributes::GetFormat() const
{
    return FileFormat(format);
}

int
SaveWindowAttributes::GetWidth() const
{
    return width;
}

int
SaveWindowAttributes::GetHeight() const
{
    return height;
}

bool
SaveWindowAttributes::GetScreenCapture() const
{
    return screenCapture;
}

bool
SaveWindowAttributes::GetSaveTiled() const
{
    return saveTiled;
}

int
SaveWindowAttributes::GetQuality() const
{
    return quality;
}

bool
SaveWindowAttributes::GetProgressive() const
{
    return progressive;
}

bool
SaveWindowAttributes::GetBinary() const
{
    return binary;
}

const std::string &
SaveWindowAttributes::GetLastRealFilename() const
{
    return lastRealFilename;
}

std::string &
SaveWindowAttributes::GetLastRealFilename()
{
    return lastRealFilename;
}

bool
SaveWindowAttributes::GetStereo() const
{
    return stereo;
}

SaveWindowAttributes::CompressionType
SaveWindowAttributes::GetCompression() const
{
    return CompressionType(compression);
}

bool
SaveWindowAttributes::GetForceMerge() const
{
    return forceMerge;
}

SaveWindowAttributes::ResConstraint
SaveWindowAttributes::GetResConstraint() const
{
    return ResConstraint(resConstraint);
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
SaveWindowAttributes::SelectOutputDirectory()
{
    Select(1, (void *)&outputDirectory);
}

void
SaveWindowAttributes::SelectFileName()
{
    Select(2, (void *)&fileName);
}

void
SaveWindowAttributes::SelectLastRealFilename()
{
    Select(12, (void *)&lastRealFilename);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SaveWindowAttributes::GetFieldName
//
// Purpose: 
//   This method returns the name of a field given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Sep 27 17:37:29 PST 2007
//
// Modifications:
//   
// ****************************************************************************

std::string
SaveWindowAttributes::GetFieldName(int index) const
{
    switch (index)
    {
        case 0:  return "Output to current directory";
        case 1:  return "outputDirectory";
        case 2:  return "fileName";
        case 3:  return "family";
        case 4:  return "format";
        case 5:  return "width";
        case 6:  return "height";
        case 7:  return "screenCapture";
        case 8:  return "saveTiled";
        case 9:  return "quality";
        case 10:  return "progressive";
        case 11:  return "binary";
        case 12:  return "lastRealFilename";
        case 13:  return "stereo";
        case 14:  return "compression";
        case 15:  return "forceMerge";
        case 16:  return "resConstraint";
        default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: SaveWindowAttributes::GetFieldType
//
// Purpose: 
//   This method returns the type of a field given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Sep 27 17:37:29 PST 2007
//
// Modifications:
//   
// ****************************************************************************

AttributeGroup::FieldType
SaveWindowAttributes::GetFieldType(int index) const
{
    switch (index)
    {
        case 0:  return FieldType_bool;
        case 1:  return FieldType_string;
        case 2:  return FieldType_string;
        case 3:  return FieldType_bool;
        case 4:  return FieldType_enum;
        case 5:  return FieldType_int;
        case 6:  return FieldType_int;
        case 7:  return FieldType_bool;
        case 8:  return FieldType_bool;
        case 9:  return FieldType_int;
        case 10:  return FieldType_bool;
        case 11:  return FieldType_bool;
        case 12:  return FieldType_string;
        case 13:  return FieldType_bool;
        case 14:  return FieldType_enum;
        case 15:  return FieldType_bool;
        case 16:  return FieldType_enum;
        default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: SaveWindowAttributes::GetFieldTypeName
//
// Purpose: 
//   This method returns the name of a field type given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Sep 27 17:37:29 PST 2007
//
// Modifications:
//   
// ****************************************************************************

std::string
SaveWindowAttributes::GetFieldTypeName(int index) const
{
    switch (index)
    {
        case 0:  return "bool";
        case 1:  return "string";
        case 2:  return "string";
        case 3:  return "bool";
        case 4:  return "enum";
        case 5:  return "int";
        case 6:  return "int";
        case 7:  return "bool";
        case 8:  return "bool";
        case 9:  return "int";
        case 10:  return "bool";
        case 11:  return "bool";
        case 12:  return "string";
        case 13:  return "bool";
        case 14:  return "enum";
        case 15:  return "bool";
        case 16:  return "enum";
        default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: SaveWindowAttributes::FieldsEqual
//
// Purpose: 
//   This method compares two fields and return true if they are equal.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Sep 27 17:37:29 PST 2007
//
// Modifications:
//   
// ****************************************************************************

bool
SaveWindowAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const SaveWindowAttributes &obj = *((const SaveWindowAttributes*)rhs);
    bool retval = false;
    switch (index_)
    {
    case 0:
        {  // new scope
        retval = (outputToCurrentDirectory == obj.outputToCurrentDirectory);
        }
        break;
    case 1:
        {  // new scope
        retval = (outputDirectory == obj.outputDirectory);
        }
        break;
    case 2:
        {  // new scope
        retval = (fileName == obj.fileName);
        }
        break;
    case 3:
        {  // new scope
        retval = (family == obj.family);
        }
        break;
    case 4:
        {  // new scope
        retval = (format == obj.format);
        }
        break;
    case 5:
        {  // new scope
        retval = (width == obj.width);
        }
        break;
    case 6:
        {  // new scope
        retval = (height == obj.height);
        }
        break;
    case 7:
        {  // new scope
        retval = (screenCapture == obj.screenCapture);
        }
        break;
    case 8:
        {  // new scope
        retval = (saveTiled == obj.saveTiled);
        }
        break;
    case 9:
        {  // new scope
        retval = (quality == obj.quality);
        }
        break;
    case 10:
        {  // new scope
        retval = (progressive == obj.progressive);
        }
        break;
    case 11:
        {  // new scope
        retval = (binary == obj.binary);
        }
        break;
    case 12:
        {  // new scope
        retval = (lastRealFilename == obj.lastRealFilename);
        }
        break;
    case 13:
        {  // new scope
        retval = (stereo == obj.stereo);
        }
        break;
    case 14:
        {  // new scope
        retval = (compression == obj.compression);
        }
        break;
    case 15:
        {  // new scope
        retval = (forceMerge == obj.forceMerge);
        }
        break;
    case 16:
        {  // new scope
        retval = (resConstraint == obj.resConstraint);
        }
        break;
    default: retval = false;
    }

    return retval;
}

///////////////////////////////////////////////////////////////////////////////
// User-defined methods.
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
//  Method: CurrentFormatIsImageFormat
//
//  Purpose:
//      Determines if the current file format is image-based.
//
//  Programmer: Hank Childs
//  Creation:   October 15, 2003
//
// ****************************************************************************

bool
SaveWindowAttributes::CurrentFormatIsImageFormat(void)
{
    FileFormat ff = GetFormat();
    if (ff == BMP)
        return true;
    if (ff == JPEG)
        return true;
    if (ff == PNG)
        return true;
    if (ff == POSTSCRIPT)
        return true;
    if (ff == PPM)
        return true;
    if (ff == RGB)
        return true;
    if (ff == TIFF)
        return true;

    return false;
}

