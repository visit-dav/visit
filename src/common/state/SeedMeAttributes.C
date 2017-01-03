/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
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

#include <SeedMeAttributes.h>
#include <DataNode.h>

//
// Enum conversion methods for SeedMeAttributes::CollectionMode
//

static const char *CollectionMode_strings[] = {
"COLLECTION_NEW", "COLLECTION_UPDATE"};

std::string
SeedMeAttributes::CollectionMode_ToString(SeedMeAttributes::CollectionMode t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return CollectionMode_strings[index];
}

std::string
SeedMeAttributes::CollectionMode_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return CollectionMode_strings[index];
}

bool
SeedMeAttributes::CollectionMode_FromString(const std::string &s, SeedMeAttributes::CollectionMode &val)
{
    val = SeedMeAttributes::COLLECTION_NEW;
    for(int i = 0; i < 2; ++i)
    {
        if(s == CollectionMode_strings[i])
        {
            val = (CollectionMode)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for SeedMeAttributes::Sharing
//

static const char *Sharing_strings[] = {
"SHARING_PRIVATE", "SHARING_GROUP", "SHARING_PUBLIC"
};

std::string
SeedMeAttributes::Sharing_ToString(SeedMeAttributes::Sharing t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return Sharing_strings[index];
}

std::string
SeedMeAttributes::Sharing_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return Sharing_strings[index];
}

bool
SeedMeAttributes::Sharing_FromString(const std::string &s, SeedMeAttributes::Sharing &val)
{
    val = SeedMeAttributes::SHARING_PRIVATE;
    for(int i = 0; i < 3; ++i)
    {
        if(s == Sharing_strings[i])
        {
            val = (Sharing)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for SeedMeAttributes::QueryAction
//

static const char *QueryAction_strings[] = {
"LIST_COL", "FIND_COL", "QUERY_COL"
};

std::string
SeedMeAttributes::QueryAction_ToString(SeedMeAttributes::QueryAction t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return QueryAction_strings[index];
}

std::string
SeedMeAttributes::QueryAction_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return QueryAction_strings[index];
}

bool
SeedMeAttributes::QueryAction_FromString(const std::string &s, SeedMeAttributes::QueryAction &val)
{
    val = SeedMeAttributes::LIST_COL;
    for(int i = 0; i < 3; ++i)
    {
        if(s == QueryAction_strings[i])
        {
            val = (QueryAction)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for SeedMeAttributes::QueryCollectionValues
//

static const char *QueryCollectionValues_strings[] = {
"QUERY_ALL", "QUERY_KEY_VALUE", "QUERY_TICKER", 
"QUERY_URL"};

std::string
SeedMeAttributes::QueryCollectionValues_ToString(SeedMeAttributes::QueryCollectionValues t)
{
    int index = int(t);
    if(index < 0 || index >= 4) index = 0;
    return QueryCollectionValues_strings[index];
}

std::string
SeedMeAttributes::QueryCollectionValues_ToString(int t)
{
    int index = (t < 0 || t >= 4) ? 0 : t;
    return QueryCollectionValues_strings[index];
}

bool
SeedMeAttributes::QueryCollectionValues_FromString(const std::string &s, SeedMeAttributes::QueryCollectionValues &val)
{
    val = SeedMeAttributes::QUERY_ALL;
    for(int i = 0; i < 4; ++i)
    {
        if(s == QueryCollectionValues_strings[i])
        {
            val = (QueryCollectionValues)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for SeedMeAttributes::Download
//

static const char *Download_strings[] = {
"DOWNLOAD_ALL", "DOWNLOAD_VIDEO", "DOWNLOAD_NAME"
};

std::string
SeedMeAttributes::Download_ToString(SeedMeAttributes::Download t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return Download_strings[index];
}

std::string
SeedMeAttributes::Download_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return Download_strings[index];
}

bool
SeedMeAttributes::Download_FromString(const std::string &s, SeedMeAttributes::Download &val)
{
    val = SeedMeAttributes::DOWNLOAD_ALL;
    for(int i = 0; i < 3; ++i)
    {
        if(s == Download_strings[i])
        {
            val = (Download)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for SeedMeAttributes::DownloadQuality
//

static const char *DownloadQuality_strings[] = {
"DOWNLOAD_Q_ALL", "DOWNLOAD_Q_BEST", "DOWNLOAD_Q_HIGHEST", 
"DOWNLOAD_Q_HIGH", "DOWNLOAD_Q_MEDIUM", "DOWNLOAD_Q_LOW", 
"DOWNLOAD_Q_LOWEST"};

std::string
SeedMeAttributes::DownloadQuality_ToString(SeedMeAttributes::DownloadQuality t)
{
    int index = int(t);
    if(index < 0 || index >= 7) index = 0;
    return DownloadQuality_strings[index];
}

std::string
SeedMeAttributes::DownloadQuality_ToString(int t)
{
    int index = (t < 0 || t >= 7) ? 0 : t;
    return DownloadQuality_strings[index];
}

bool
SeedMeAttributes::DownloadQuality_FromString(const std::string &s, SeedMeAttributes::DownloadQuality &val)
{
    val = SeedMeAttributes::DOWNLOAD_Q_ALL;
    for(int i = 0; i < 7; ++i)
    {
        if(s == DownloadQuality_strings[i])
        {
            val = (DownloadQuality)i;
            return true;
        }
    }
    return false;
}

// ****************************************************************************
// Method: SeedMeAttributes::SeedMeAttributes
//
// Purpose: 
//   Init utility for the SeedMeAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void SeedMeAttributes::Init()
{
    collectionMode = COLLECTION_NEW;
    sharing = SHARING_PRIVATE;
    overwriteFiles = false;
    uploadCurrentFile = true;
    uploadSequenceFile = false;
    createVideo = false;
    frameRate = 30;
    queryAction = LIST_COL;
    queryCollectionValues = QUERY_ALL;
    downloadType = DOWNLOAD_ALL;
    downloadName = "*";
    quickSharing = SHARING_PRIVATE;
    quickFrameRate = 10;
    quickDownloadType = DOWNLOAD_Q_ALL;

    SeedMeAttributes::SelectAll();
}

// ****************************************************************************
// Method: SeedMeAttributes::SeedMeAttributes
//
// Purpose: 
//   Copy utility for the SeedMeAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void SeedMeAttributes::Copy(const SeedMeAttributes &obj)
{
    collectionMode = obj.collectionMode;
    collectionID = obj.collectionID;
    sharing = obj.sharing;
    collectionTitle = obj.collectionTitle;
    collectionDescription = obj.collectionDescription;
    overwriteFiles = obj.overwriteFiles;
    keyValue = obj.keyValue;
    collectionEmails = obj.collectionEmails;
    uploadCurrentFile = obj.uploadCurrentFile;
    currentTitle = obj.currentTitle;
    currentDescription = obj.currentDescription;
    uploadSequenceFile = obj.uploadSequenceFile;
    sequenceTitle = obj.sequenceTitle;
    sequenceDescription = obj.sequenceDescription;
    createVideo = obj.createVideo;
    frameRate = obj.frameRate;
    operationResult = obj.operationResult;
    queryAction = obj.queryAction;
    queryColID = obj.queryColID;
    queryKeyValue = obj.queryKeyValue;
    queryCollectionValues = obj.queryCollectionValues;
    downloadCollectionID = obj.downloadCollectionID;
    downloadType = obj.downloadType;
    downloadName = obj.downloadName;
    quickSharing = obj.quickSharing;
    quickCollectionTitle = obj.quickCollectionTitle;
    quickCollectionEmails = obj.quickCollectionEmails;
    quickFrameRate = obj.quickFrameRate;
    quickDownloadType = obj.quickDownloadType;

    SeedMeAttributes::SelectAll();
}

// Type map format string
const char *SeedMeAttributes::TypeMapFormatString = SEEDMEATTRIBUTES_TMFS;
const AttributeGroup::private_tmfs_t SeedMeAttributes::TmfsStruct = {SEEDMEATTRIBUTES_TMFS};


// ****************************************************************************
// Method: SeedMeAttributes::SeedMeAttributes
//
// Purpose: 
//   Default constructor for the SeedMeAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

SeedMeAttributes::SeedMeAttributes() : 
    AttributeSubject(SeedMeAttributes::TypeMapFormatString)
{
    SeedMeAttributes::Init();
}

// ****************************************************************************
// Method: SeedMeAttributes::SeedMeAttributes
//
// Purpose: 
//   Constructor for the derived classes of SeedMeAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

SeedMeAttributes::SeedMeAttributes(private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs)
{
    SeedMeAttributes::Init();
}

// ****************************************************************************
// Method: SeedMeAttributes::SeedMeAttributes
//
// Purpose: 
//   Copy constructor for the SeedMeAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

SeedMeAttributes::SeedMeAttributes(const SeedMeAttributes &obj) : 
    AttributeSubject(SeedMeAttributes::TypeMapFormatString)
{
    SeedMeAttributes::Copy(obj);
}

// ****************************************************************************
// Method: SeedMeAttributes::SeedMeAttributes
//
// Purpose: 
//   Copy constructor for derived classes of the SeedMeAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

SeedMeAttributes::SeedMeAttributes(const SeedMeAttributes &obj, private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs)
{
    SeedMeAttributes::Copy(obj);
}

// ****************************************************************************
// Method: SeedMeAttributes::~SeedMeAttributes
//
// Purpose: 
//   Destructor for the SeedMeAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

SeedMeAttributes::~SeedMeAttributes()
{
    // nothing here
}

// ****************************************************************************
// Method: SeedMeAttributes::operator = 
//
// Purpose: 
//   Assignment operator for the SeedMeAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

SeedMeAttributes& 
SeedMeAttributes::operator = (const SeedMeAttributes &obj)
{
    if (this == &obj) return *this;

    SeedMeAttributes::Copy(obj);

    return *this;
}

// ****************************************************************************
// Method: SeedMeAttributes::operator == 
//
// Purpose: 
//   Comparison operator == for the SeedMeAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

bool
SeedMeAttributes::operator == (const SeedMeAttributes &obj) const
{
    // Create the return value
    return ((collectionMode == obj.collectionMode) &&
            (collectionID == obj.collectionID) &&
            (sharing == obj.sharing) &&
            (collectionTitle == obj.collectionTitle) &&
            (collectionDescription == obj.collectionDescription) &&
            (overwriteFiles == obj.overwriteFiles) &&
            (keyValue == obj.keyValue) &&
            (collectionEmails == obj.collectionEmails) &&
            (uploadCurrentFile == obj.uploadCurrentFile) &&
            (currentTitle == obj.currentTitle) &&
            (currentDescription == obj.currentDescription) &&
            (uploadSequenceFile == obj.uploadSequenceFile) &&
            (sequenceTitle == obj.sequenceTitle) &&
            (sequenceDescription == obj.sequenceDescription) &&
            (createVideo == obj.createVideo) &&
            (frameRate == obj.frameRate) &&
            (operationResult == obj.operationResult) &&
            (queryAction == obj.queryAction) &&
            (queryColID == obj.queryColID) &&
            (queryKeyValue == obj.queryKeyValue) &&
            (queryCollectionValues == obj.queryCollectionValues) &&
            (downloadCollectionID == obj.downloadCollectionID) &&
            (downloadType == obj.downloadType) &&
            (downloadName == obj.downloadName) &&
            (quickSharing == obj.quickSharing) &&
            (quickCollectionTitle == obj.quickCollectionTitle) &&
            (quickCollectionEmails == obj.quickCollectionEmails) &&
            (quickFrameRate == obj.quickFrameRate) &&
            (quickDownloadType == obj.quickDownloadType));
}

// ****************************************************************************
// Method: SeedMeAttributes::operator != 
//
// Purpose: 
//   Comparison operator != for the SeedMeAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

bool
SeedMeAttributes::operator != (const SeedMeAttributes &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: SeedMeAttributes::TypeName
//
// Purpose: 
//   Type name method for the SeedMeAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

const std::string
SeedMeAttributes::TypeName() const
{
    return "SeedMeAttributes";
}

// ****************************************************************************
// Method: SeedMeAttributes::CopyAttributes
//
// Purpose: 
//   CopyAttributes method for the SeedMeAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

bool
SeedMeAttributes::CopyAttributes(const AttributeGroup *atts)
{
    if(TypeName() != atts->TypeName())
        return false;

    // Call assignment operator.
    const SeedMeAttributes *tmp = (const SeedMeAttributes *)atts;
    *this = *tmp;

    return true;
}

// ****************************************************************************
// Method: SeedMeAttributes::CreateCompatible
//
// Purpose: 
//   CreateCompatible method for the SeedMeAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

AttributeSubject *
SeedMeAttributes::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;
    if(TypeName() == tname)
        retval = new SeedMeAttributes(*this);
    // Other cases could go here too. 

    return retval;
}

// ****************************************************************************
// Method: SeedMeAttributes::NewInstance
//
// Purpose: 
//   NewInstance method for the SeedMeAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

AttributeSubject *
SeedMeAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new SeedMeAttributes(*this);
    else
        retval = new SeedMeAttributes;

    return retval;
}

// ****************************************************************************
// Method: SeedMeAttributes::SelectAll
//
// Purpose: 
//   Selects all attributes.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void
SeedMeAttributes::SelectAll()
{
    Select(ID_collectionMode,        (void *)&collectionMode);
    Select(ID_collectionID,          (void *)&collectionID);
    Select(ID_sharing,               (void *)&sharing);
    Select(ID_collectionTitle,       (void *)&collectionTitle);
    Select(ID_collectionDescription, (void *)&collectionDescription);
    Select(ID_overwriteFiles,        (void *)&overwriteFiles);
    Select(ID_keyValue,              (void *)&keyValue);
    Select(ID_collectionEmails,      (void *)&collectionEmails);
    Select(ID_uploadCurrentFile,     (void *)&uploadCurrentFile);
    Select(ID_currentTitle,          (void *)&currentTitle);
    Select(ID_currentDescription,    (void *)&currentDescription);
    Select(ID_uploadSequenceFile,    (void *)&uploadSequenceFile);
    Select(ID_sequenceTitle,         (void *)&sequenceTitle);
    Select(ID_sequenceDescription,   (void *)&sequenceDescription);
    Select(ID_createVideo,           (void *)&createVideo);
    Select(ID_frameRate,             (void *)&frameRate);
    Select(ID_operationResult,       (void *)&operationResult);
    Select(ID_queryAction,           (void *)&queryAction);
    Select(ID_queryColID,            (void *)&queryColID);
    Select(ID_queryKeyValue,         (void *)&queryKeyValue);
    Select(ID_queryCollectionValues, (void *)&queryCollectionValues);
    Select(ID_downloadCollectionID,  (void *)&downloadCollectionID);
    Select(ID_downloadType,          (void *)&downloadType);
    Select(ID_downloadName,          (void *)&downloadName);
    Select(ID_quickSharing,          (void *)&quickSharing);
    Select(ID_quickCollectionTitle,  (void *)&quickCollectionTitle);
    Select(ID_quickCollectionEmails, (void *)&quickCollectionEmails);
    Select(ID_quickFrameRate,        (void *)&quickFrameRate);
    Select(ID_quickDownloadType,     (void *)&quickDownloadType);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SeedMeAttributes::CreateNode
//
// Purpose: 
//   This method creates a DataNode representation of the object so it can be saved to a config file.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

bool
SeedMeAttributes::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if(parentNode == 0)
        return false;

    SeedMeAttributes defaultObject;
    bool addToParent = false;
    // Create a node for SeedMeAttributes.
    DataNode *node = new DataNode("SeedMeAttributes");

    if(completeSave || !FieldsEqual(ID_collectionMode, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("collectionMode", CollectionMode_ToString(collectionMode)));
    }

    if(completeSave || !FieldsEqual(ID_collectionID, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("collectionID", collectionID));
    }

    if(completeSave || !FieldsEqual(ID_sharing, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("sharing", Sharing_ToString(sharing)));
    }

    if(completeSave || !FieldsEqual(ID_collectionTitle, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("collectionTitle", collectionTitle));
    }

    if(completeSave || !FieldsEqual(ID_collectionDescription, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("collectionDescription", collectionDescription));
    }

    if(completeSave || !FieldsEqual(ID_overwriteFiles, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("overwriteFiles", overwriteFiles));
    }

    if(completeSave || !FieldsEqual(ID_keyValue, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("keyValue", keyValue));
    }

    if(completeSave || !FieldsEqual(ID_collectionEmails, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("collectionEmails", collectionEmails));
    }

    if(completeSave || !FieldsEqual(ID_uploadCurrentFile, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("uploadCurrentFile", uploadCurrentFile));
    }

    if(completeSave || !FieldsEqual(ID_currentTitle, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("currentTitle", currentTitle));
    }

    if(completeSave || !FieldsEqual(ID_currentDescription, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("currentDescription", currentDescription));
    }

    if(completeSave || !FieldsEqual(ID_uploadSequenceFile, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("uploadSequenceFile", uploadSequenceFile));
    }

    if(completeSave || !FieldsEqual(ID_sequenceTitle, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("sequenceTitle", sequenceTitle));
    }

    if(completeSave || !FieldsEqual(ID_sequenceDescription, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("sequenceDescription", sequenceDescription));
    }

    if(completeSave || !FieldsEqual(ID_createVideo, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("createVideo", createVideo));
    }

    if(completeSave || !FieldsEqual(ID_frameRate, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("frameRate", frameRate));
    }

    if(completeSave || !FieldsEqual(ID_operationResult, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("operationResult", operationResult));
    }

    if(completeSave || !FieldsEqual(ID_queryAction, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("queryAction", QueryAction_ToString(queryAction)));
    }

    if(completeSave || !FieldsEqual(ID_queryColID, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("queryColID", queryColID));
    }

    if(completeSave || !FieldsEqual(ID_queryKeyValue, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("queryKeyValue", queryKeyValue));
    }

    if(completeSave || !FieldsEqual(ID_queryCollectionValues, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("queryCollectionValues", QueryCollectionValues_ToString(queryCollectionValues)));
    }

    if(completeSave || !FieldsEqual(ID_downloadCollectionID, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("downloadCollectionID", downloadCollectionID));
    }

    if(completeSave || !FieldsEqual(ID_downloadType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("downloadType", Download_ToString(downloadType)));
    }

    if(completeSave || !FieldsEqual(ID_downloadName, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("downloadName", downloadName));
    }

    if(completeSave || !FieldsEqual(ID_quickSharing, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("quickSharing", Sharing_ToString(quickSharing)));
    }

    if(completeSave || !FieldsEqual(ID_quickCollectionTitle, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("quickCollectionTitle", quickCollectionTitle));
    }

    if(completeSave || !FieldsEqual(ID_quickCollectionEmails, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("quickCollectionEmails", quickCollectionEmails));
    }

    if(completeSave || !FieldsEqual(ID_quickFrameRate, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("quickFrameRate", quickFrameRate));
    }

    if(completeSave || !FieldsEqual(ID_quickDownloadType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("quickDownloadType", DownloadQuality_ToString(quickDownloadType)));
    }


    // Add the node to the parent node.
    if(addToParent || forceAdd)
        parentNode->AddNode(node);
    else
        delete node;

    return (addToParent || forceAdd);
}

// ****************************************************************************
// Method: SeedMeAttributes::SetFromNode
//
// Purpose: 
//   This method sets attributes in this object from values in a DataNode representation of the object.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void
SeedMeAttributes::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("SeedMeAttributes");
    if(searchNode == 0)
        return;

    DataNode *node;
    if((node = searchNode->GetNode("collectionMode")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetCollectionMode(CollectionMode(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            CollectionMode value;
            if(CollectionMode_FromString(node->AsString(), value))
                SetCollectionMode(value);
        }
    }
    if((node = searchNode->GetNode("collectionID")) != 0)
        SetCollectionID(node->AsInt());
    if((node = searchNode->GetNode("sharing")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetSharing(Sharing(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            Sharing value;
            if(Sharing_FromString(node->AsString(), value))
                SetSharing(value);
        }
    }
    if((node = searchNode->GetNode("collectionTitle")) != 0)
        SetCollectionTitle(node->AsString());
    if((node = searchNode->GetNode("collectionDescription")) != 0)
        SetCollectionDescription(node->AsString());
    if((node = searchNode->GetNode("overwriteFiles")) != 0)
        SetOverwriteFiles(node->AsBool());
    if((node = searchNode->GetNode("keyValue")) != 0)
        SetKeyValue(node->AsString());
    if((node = searchNode->GetNode("collectionEmails")) != 0)
        SetCollectionEmails(node->AsString());
    if((node = searchNode->GetNode("uploadCurrentFile")) != 0)
        SetUploadCurrentFile(node->AsBool());
    if((node = searchNode->GetNode("currentTitle")) != 0)
        SetCurrentTitle(node->AsString());
    if((node = searchNode->GetNode("currentDescription")) != 0)
        SetCurrentDescription(node->AsString());
    if((node = searchNode->GetNode("uploadSequenceFile")) != 0)
        SetUploadSequenceFile(node->AsBool());
    if((node = searchNode->GetNode("sequenceTitle")) != 0)
        SetSequenceTitle(node->AsString());
    if((node = searchNode->GetNode("sequenceDescription")) != 0)
        SetSequenceDescription(node->AsString());
    if((node = searchNode->GetNode("createVideo")) != 0)
        SetCreateVideo(node->AsBool());
    if((node = searchNode->GetNode("frameRate")) != 0)
        SetFrameRate(node->AsInt());
    if((node = searchNode->GetNode("operationResult")) != 0)
        SetOperationResult(node->AsString());
    if((node = searchNode->GetNode("queryAction")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetQueryAction(QueryAction(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            QueryAction value;
            if(QueryAction_FromString(node->AsString(), value))
                SetQueryAction(value);
        }
    }
    if((node = searchNode->GetNode("queryColID")) != 0)
        SetQueryColID(node->AsString());
    if((node = searchNode->GetNode("queryKeyValue")) != 0)
        SetQueryKeyValue(node->AsString());
    if((node = searchNode->GetNode("queryCollectionValues")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 4)
                SetQueryCollectionValues(QueryCollectionValues(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            QueryCollectionValues value;
            if(QueryCollectionValues_FromString(node->AsString(), value))
                SetQueryCollectionValues(value);
        }
    }
    if((node = searchNode->GetNode("downloadCollectionID")) != 0)
        SetDownloadCollectionID(node->AsInt());
    if((node = searchNode->GetNode("downloadType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetDownloadType(Download(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            Download value;
            if(Download_FromString(node->AsString(), value))
                SetDownloadType(value);
        }
    }
    if((node = searchNode->GetNode("downloadName")) != 0)
        SetDownloadName(node->AsString());
    if((node = searchNode->GetNode("quickSharing")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetQuickSharing(Sharing(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            Sharing value;
            if(Sharing_FromString(node->AsString(), value))
                SetQuickSharing(value);
        }
    }
    if((node = searchNode->GetNode("quickCollectionTitle")) != 0)
        SetQuickCollectionTitle(node->AsString());
    if((node = searchNode->GetNode("quickCollectionEmails")) != 0)
        SetQuickCollectionEmails(node->AsString());
    if((node = searchNode->GetNode("quickFrameRate")) != 0)
        SetQuickFrameRate(node->AsInt());
    if((node = searchNode->GetNode("quickDownloadType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 7)
                SetQuickDownloadType(DownloadQuality(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            DownloadQuality value;
            if(DownloadQuality_FromString(node->AsString(), value))
                SetQuickDownloadType(value);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
SeedMeAttributes::SetCollectionMode(SeedMeAttributes::CollectionMode collectionMode_)
{
    collectionMode = collectionMode_;
    Select(ID_collectionMode, (void *)&collectionMode);
}

void
SeedMeAttributes::SetCollectionID(int collectionID_)
{
    collectionID = collectionID_;
    Select(ID_collectionID, (void *)&collectionID);
}

void
SeedMeAttributes::SetSharing(SeedMeAttributes::Sharing sharing_)
{
    sharing = sharing_;
    Select(ID_sharing, (void *)&sharing);
}

void
SeedMeAttributes::SetCollectionTitle(const std::string &collectionTitle_)
{
    collectionTitle = collectionTitle_;
    Select(ID_collectionTitle, (void *)&collectionTitle);
}

void
SeedMeAttributes::SetCollectionDescription(const std::string &collectionDescription_)
{
    collectionDescription = collectionDescription_;
    Select(ID_collectionDescription, (void *)&collectionDescription);
}

void
SeedMeAttributes::SetOverwriteFiles(bool overwriteFiles_)
{
    overwriteFiles = overwriteFiles_;
    Select(ID_overwriteFiles, (void *)&overwriteFiles);
}

void
SeedMeAttributes::SetKeyValue(const std::string &keyValue_)
{
    keyValue = keyValue_;
    Select(ID_keyValue, (void *)&keyValue);
}

void
SeedMeAttributes::SetCollectionEmails(const std::string &collectionEmails_)
{
    collectionEmails = collectionEmails_;
    Select(ID_collectionEmails, (void *)&collectionEmails);
}

void
SeedMeAttributes::SetUploadCurrentFile(bool uploadCurrentFile_)
{
    uploadCurrentFile = uploadCurrentFile_;
    Select(ID_uploadCurrentFile, (void *)&uploadCurrentFile);
}

void
SeedMeAttributes::SetCurrentTitle(const std::string &currentTitle_)
{
    currentTitle = currentTitle_;
    Select(ID_currentTitle, (void *)&currentTitle);
}

void
SeedMeAttributes::SetCurrentDescription(const std::string &currentDescription_)
{
    currentDescription = currentDescription_;
    Select(ID_currentDescription, (void *)&currentDescription);
}

void
SeedMeAttributes::SetUploadSequenceFile(bool uploadSequenceFile_)
{
    uploadSequenceFile = uploadSequenceFile_;
    Select(ID_uploadSequenceFile, (void *)&uploadSequenceFile);
}

void
SeedMeAttributes::SetSequenceTitle(const std::string &sequenceTitle_)
{
    sequenceTitle = sequenceTitle_;
    Select(ID_sequenceTitle, (void *)&sequenceTitle);
}

void
SeedMeAttributes::SetSequenceDescription(const std::string &sequenceDescription_)
{
    sequenceDescription = sequenceDescription_;
    Select(ID_sequenceDescription, (void *)&sequenceDescription);
}

void
SeedMeAttributes::SetCreateVideo(bool createVideo_)
{
    createVideo = createVideo_;
    Select(ID_createVideo, (void *)&createVideo);
}

void
SeedMeAttributes::SetFrameRate(int frameRate_)
{
    frameRate = frameRate_;
    Select(ID_frameRate, (void *)&frameRate);
}

void
SeedMeAttributes::SetOperationResult(const std::string &operationResult_)
{
    operationResult = operationResult_;
    Select(ID_operationResult, (void *)&operationResult);
}

void
SeedMeAttributes::SetQueryAction(SeedMeAttributes::QueryAction queryAction_)
{
    queryAction = queryAction_;
    Select(ID_queryAction, (void *)&queryAction);
}

void
SeedMeAttributes::SetQueryColID(const std::string &queryColID_)
{
    queryColID = queryColID_;
    Select(ID_queryColID, (void *)&queryColID);
}

void
SeedMeAttributes::SetQueryKeyValue(const std::string &queryKeyValue_)
{
    queryKeyValue = queryKeyValue_;
    Select(ID_queryKeyValue, (void *)&queryKeyValue);
}

void
SeedMeAttributes::SetQueryCollectionValues(SeedMeAttributes::QueryCollectionValues queryCollectionValues_)
{
    queryCollectionValues = queryCollectionValues_;
    Select(ID_queryCollectionValues, (void *)&queryCollectionValues);
}

void
SeedMeAttributes::SetDownloadCollectionID(int downloadCollectionID_)
{
    downloadCollectionID = downloadCollectionID_;
    Select(ID_downloadCollectionID, (void *)&downloadCollectionID);
}

void
SeedMeAttributes::SetDownloadType(SeedMeAttributes::Download downloadType_)
{
    downloadType = downloadType_;
    Select(ID_downloadType, (void *)&downloadType);
}

void
SeedMeAttributes::SetDownloadName(const std::string &downloadName_)
{
    downloadName = downloadName_;
    Select(ID_downloadName, (void *)&downloadName);
}

void
SeedMeAttributes::SetQuickSharing(SeedMeAttributes::Sharing quickSharing_)
{
    quickSharing = quickSharing_;
    Select(ID_quickSharing, (void *)&quickSharing);
}

void
SeedMeAttributes::SetQuickCollectionTitle(const std::string &quickCollectionTitle_)
{
    quickCollectionTitle = quickCollectionTitle_;
    Select(ID_quickCollectionTitle, (void *)&quickCollectionTitle);
}

void
SeedMeAttributes::SetQuickCollectionEmails(const std::string &quickCollectionEmails_)
{
    quickCollectionEmails = quickCollectionEmails_;
    Select(ID_quickCollectionEmails, (void *)&quickCollectionEmails);
}

void
SeedMeAttributes::SetQuickFrameRate(int quickFrameRate_)
{
    quickFrameRate = quickFrameRate_;
    Select(ID_quickFrameRate, (void *)&quickFrameRate);
}

void
SeedMeAttributes::SetQuickDownloadType(SeedMeAttributes::DownloadQuality quickDownloadType_)
{
    quickDownloadType = quickDownloadType_;
    Select(ID_quickDownloadType, (void *)&quickDownloadType);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

SeedMeAttributes::CollectionMode
SeedMeAttributes::GetCollectionMode() const
{
    return CollectionMode(collectionMode);
}

int
SeedMeAttributes::GetCollectionID() const
{
    return collectionID;
}

SeedMeAttributes::Sharing
SeedMeAttributes::GetSharing() const
{
    return Sharing(sharing);
}

const std::string &
SeedMeAttributes::GetCollectionTitle() const
{
    return collectionTitle;
}

std::string &
SeedMeAttributes::GetCollectionTitle()
{
    return collectionTitle;
}

const std::string &
SeedMeAttributes::GetCollectionDescription() const
{
    return collectionDescription;
}

std::string &
SeedMeAttributes::GetCollectionDescription()
{
    return collectionDescription;
}

bool
SeedMeAttributes::GetOverwriteFiles() const
{
    return overwriteFiles;
}

const std::string &
SeedMeAttributes::GetKeyValue() const
{
    return keyValue;
}

std::string &
SeedMeAttributes::GetKeyValue()
{
    return keyValue;
}

const std::string &
SeedMeAttributes::GetCollectionEmails() const
{
    return collectionEmails;
}

std::string &
SeedMeAttributes::GetCollectionEmails()
{
    return collectionEmails;
}

bool
SeedMeAttributes::GetUploadCurrentFile() const
{
    return uploadCurrentFile;
}

const std::string &
SeedMeAttributes::GetCurrentTitle() const
{
    return currentTitle;
}

std::string &
SeedMeAttributes::GetCurrentTitle()
{
    return currentTitle;
}

const std::string &
SeedMeAttributes::GetCurrentDescription() const
{
    return currentDescription;
}

std::string &
SeedMeAttributes::GetCurrentDescription()
{
    return currentDescription;
}

bool
SeedMeAttributes::GetUploadSequenceFile() const
{
    return uploadSequenceFile;
}

const std::string &
SeedMeAttributes::GetSequenceTitle() const
{
    return sequenceTitle;
}

std::string &
SeedMeAttributes::GetSequenceTitle()
{
    return sequenceTitle;
}

const std::string &
SeedMeAttributes::GetSequenceDescription() const
{
    return sequenceDescription;
}

std::string &
SeedMeAttributes::GetSequenceDescription()
{
    return sequenceDescription;
}

bool
SeedMeAttributes::GetCreateVideo() const
{
    return createVideo;
}

int
SeedMeAttributes::GetFrameRate() const
{
    return frameRate;
}

const std::string &
SeedMeAttributes::GetOperationResult() const
{
    return operationResult;
}

std::string &
SeedMeAttributes::GetOperationResult()
{
    return operationResult;
}

SeedMeAttributes::QueryAction
SeedMeAttributes::GetQueryAction() const
{
    return QueryAction(queryAction);
}

const std::string &
SeedMeAttributes::GetQueryColID() const
{
    return queryColID;
}

std::string &
SeedMeAttributes::GetQueryColID()
{
    return queryColID;
}

const std::string &
SeedMeAttributes::GetQueryKeyValue() const
{
    return queryKeyValue;
}

std::string &
SeedMeAttributes::GetQueryKeyValue()
{
    return queryKeyValue;
}

SeedMeAttributes::QueryCollectionValues
SeedMeAttributes::GetQueryCollectionValues() const
{
    return QueryCollectionValues(queryCollectionValues);
}

int
SeedMeAttributes::GetDownloadCollectionID() const
{
    return downloadCollectionID;
}

SeedMeAttributes::Download
SeedMeAttributes::GetDownloadType() const
{
    return Download(downloadType);
}

const std::string &
SeedMeAttributes::GetDownloadName() const
{
    return downloadName;
}

std::string &
SeedMeAttributes::GetDownloadName()
{
    return downloadName;
}

SeedMeAttributes::Sharing
SeedMeAttributes::GetQuickSharing() const
{
    return Sharing(quickSharing);
}

const std::string &
SeedMeAttributes::GetQuickCollectionTitle() const
{
    return quickCollectionTitle;
}

std::string &
SeedMeAttributes::GetQuickCollectionTitle()
{
    return quickCollectionTitle;
}

const std::string &
SeedMeAttributes::GetQuickCollectionEmails() const
{
    return quickCollectionEmails;
}

std::string &
SeedMeAttributes::GetQuickCollectionEmails()
{
    return quickCollectionEmails;
}

int
SeedMeAttributes::GetQuickFrameRate() const
{
    return quickFrameRate;
}

SeedMeAttributes::DownloadQuality
SeedMeAttributes::GetQuickDownloadType() const
{
    return DownloadQuality(quickDownloadType);
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
SeedMeAttributes::SelectCollectionTitle()
{
    Select(ID_collectionTitle, (void *)&collectionTitle);
}

void
SeedMeAttributes::SelectCollectionDescription()
{
    Select(ID_collectionDescription, (void *)&collectionDescription);
}

void
SeedMeAttributes::SelectKeyValue()
{
    Select(ID_keyValue, (void *)&keyValue);
}

void
SeedMeAttributes::SelectCollectionEmails()
{
    Select(ID_collectionEmails, (void *)&collectionEmails);
}

void
SeedMeAttributes::SelectCurrentTitle()
{
    Select(ID_currentTitle, (void *)&currentTitle);
}

void
SeedMeAttributes::SelectCurrentDescription()
{
    Select(ID_currentDescription, (void *)&currentDescription);
}

void
SeedMeAttributes::SelectSequenceTitle()
{
    Select(ID_sequenceTitle, (void *)&sequenceTitle);
}

void
SeedMeAttributes::SelectSequenceDescription()
{
    Select(ID_sequenceDescription, (void *)&sequenceDescription);
}

void
SeedMeAttributes::SelectOperationResult()
{
    Select(ID_operationResult, (void *)&operationResult);
}

void
SeedMeAttributes::SelectQueryColID()
{
    Select(ID_queryColID, (void *)&queryColID);
}

void
SeedMeAttributes::SelectQueryKeyValue()
{
    Select(ID_queryKeyValue, (void *)&queryKeyValue);
}

void
SeedMeAttributes::SelectDownloadName()
{
    Select(ID_downloadName, (void *)&downloadName);
}

void
SeedMeAttributes::SelectQuickCollectionTitle()
{
    Select(ID_quickCollectionTitle, (void *)&quickCollectionTitle);
}

void
SeedMeAttributes::SelectQuickCollectionEmails()
{
    Select(ID_quickCollectionEmails, (void *)&quickCollectionEmails);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SeedMeAttributes::GetFieldName
//
// Purpose: 
//   This method returns the name of a field given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

std::string
SeedMeAttributes::GetFieldName(int index) const
{
    switch (index)
    {
    case ID_collectionMode:        return "collectionMode";
    case ID_collectionID:          return "collectionID";
    case ID_sharing:               return "sharing";
    case ID_collectionTitle:       return "collectionTitle";
    case ID_collectionDescription: return "collectionDescription";
    case ID_overwriteFiles:        return "overwriteFiles";
    case ID_keyValue:              return "keyValue";
    case ID_collectionEmails:      return "collectionEmails";
    case ID_uploadCurrentFile:     return "uploadCurrentFile";
    case ID_currentTitle:          return "currentTitle";
    case ID_currentDescription:    return "currentDescription";
    case ID_uploadSequenceFile:    return "uploadSequenceFile";
    case ID_sequenceTitle:         return "sequenceTitle";
    case ID_sequenceDescription:   return "sequenceDescription";
    case ID_createVideo:           return "createVideo";
    case ID_frameRate:             return "frameRate";
    case ID_operationResult:       return "operationResult";
    case ID_queryAction:           return "queryAction";
    case ID_queryColID:            return "queryColID";
    case ID_queryKeyValue:         return "queryKeyValue";
    case ID_queryCollectionValues: return "queryCollectionValues";
    case ID_downloadCollectionID:  return "downloadCollectionID";
    case ID_downloadType:          return "downloadType";
    case ID_downloadName:          return "downloadName";
    case ID_quickSharing:          return "quickSharing";
    case ID_quickCollectionTitle:  return "quickCollectionTitle";
    case ID_quickCollectionEmails: return "quickCollectionEmails";
    case ID_quickFrameRate:        return "quickFrameRate";
    case ID_quickDownloadType:     return "quickDownloadType";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: SeedMeAttributes::GetFieldType
//
// Purpose: 
//   This method returns the type of a field given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

AttributeGroup::FieldType
SeedMeAttributes::GetFieldType(int index) const
{
    switch (index)
    {
    case ID_collectionMode:        return FieldType_enum;
    case ID_collectionID:          return FieldType_int;
    case ID_sharing:               return FieldType_enum;
    case ID_collectionTitle:       return FieldType_string;
    case ID_collectionDescription: return FieldType_string;
    case ID_overwriteFiles:        return FieldType_bool;
    case ID_keyValue:              return FieldType_string;
    case ID_collectionEmails:      return FieldType_string;
    case ID_uploadCurrentFile:     return FieldType_bool;
    case ID_currentTitle:          return FieldType_string;
    case ID_currentDescription:    return FieldType_string;
    case ID_uploadSequenceFile:    return FieldType_bool;
    case ID_sequenceTitle:         return FieldType_string;
    case ID_sequenceDescription:   return FieldType_string;
    case ID_createVideo:           return FieldType_bool;
    case ID_frameRate:             return FieldType_int;
    case ID_operationResult:       return FieldType_string;
    case ID_queryAction:           return FieldType_enum;
    case ID_queryColID:            return FieldType_string;
    case ID_queryKeyValue:         return FieldType_string;
    case ID_queryCollectionValues: return FieldType_enum;
    case ID_downloadCollectionID:  return FieldType_int;
    case ID_downloadType:          return FieldType_enum;
    case ID_downloadName:          return FieldType_string;
    case ID_quickSharing:          return FieldType_enum;
    case ID_quickCollectionTitle:  return FieldType_string;
    case ID_quickCollectionEmails: return FieldType_string;
    case ID_quickFrameRate:        return FieldType_int;
    case ID_quickDownloadType:     return FieldType_enum;
    default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: SeedMeAttributes::GetFieldTypeName
//
// Purpose: 
//   This method returns the name of a field type given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

std::string
SeedMeAttributes::GetFieldTypeName(int index) const
{
    switch (index)
    {
    case ID_collectionMode:        return "enum";
    case ID_collectionID:          return "int";
    case ID_sharing:               return "enum";
    case ID_collectionTitle:       return "string";
    case ID_collectionDescription: return "string";
    case ID_overwriteFiles:        return "bool";
    case ID_keyValue:              return "string";
    case ID_collectionEmails:      return "string";
    case ID_uploadCurrentFile:     return "bool";
    case ID_currentTitle:          return "string";
    case ID_currentDescription:    return "string";
    case ID_uploadSequenceFile:    return "bool";
    case ID_sequenceTitle:         return "string";
    case ID_sequenceDescription:   return "string";
    case ID_createVideo:           return "bool";
    case ID_frameRate:             return "int";
    case ID_operationResult:       return "string";
    case ID_queryAction:           return "enum";
    case ID_queryColID:            return "string";
    case ID_queryKeyValue:         return "string";
    case ID_queryCollectionValues: return "enum";
    case ID_downloadCollectionID:  return "int";
    case ID_downloadType:          return "enum";
    case ID_downloadName:          return "string";
    case ID_quickSharing:          return "enum";
    case ID_quickCollectionTitle:  return "string";
    case ID_quickCollectionEmails: return "string";
    case ID_quickFrameRate:        return "int";
    case ID_quickDownloadType:     return "enum";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: SeedMeAttributes::FieldsEqual
//
// Purpose: 
//   This method compares two fields and return true if they are equal.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

bool
SeedMeAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const SeedMeAttributes &obj = *((const SeedMeAttributes*)rhs);
    bool retval = false;
    switch (index_)
    {
    case ID_collectionMode:
        {  // new scope
        retval = (collectionMode == obj.collectionMode);
        }
        break;
    case ID_collectionID:
        {  // new scope
        retval = (collectionID == obj.collectionID);
        }
        break;
    case ID_sharing:
        {  // new scope
        retval = (sharing == obj.sharing);
        }
        break;
    case ID_collectionTitle:
        {  // new scope
        retval = (collectionTitle == obj.collectionTitle);
        }
        break;
    case ID_collectionDescription:
        {  // new scope
        retval = (collectionDescription == obj.collectionDescription);
        }
        break;
    case ID_overwriteFiles:
        {  // new scope
        retval = (overwriteFiles == obj.overwriteFiles);
        }
        break;
    case ID_keyValue:
        {  // new scope
        retval = (keyValue == obj.keyValue);
        }
        break;
    case ID_collectionEmails:
        {  // new scope
        retval = (collectionEmails == obj.collectionEmails);
        }
        break;
    case ID_uploadCurrentFile:
        {  // new scope
        retval = (uploadCurrentFile == obj.uploadCurrentFile);
        }
        break;
    case ID_currentTitle:
        {  // new scope
        retval = (currentTitle == obj.currentTitle);
        }
        break;
    case ID_currentDescription:
        {  // new scope
        retval = (currentDescription == obj.currentDescription);
        }
        break;
    case ID_uploadSequenceFile:
        {  // new scope
        retval = (uploadSequenceFile == obj.uploadSequenceFile);
        }
        break;
    case ID_sequenceTitle:
        {  // new scope
        retval = (sequenceTitle == obj.sequenceTitle);
        }
        break;
    case ID_sequenceDescription:
        {  // new scope
        retval = (sequenceDescription == obj.sequenceDescription);
        }
        break;
    case ID_createVideo:
        {  // new scope
        retval = (createVideo == obj.createVideo);
        }
        break;
    case ID_frameRate:
        {  // new scope
        retval = (frameRate == obj.frameRate);
        }
        break;
    case ID_operationResult:
        {  // new scope
        retval = (operationResult == obj.operationResult);
        }
        break;
    case ID_queryAction:
        {  // new scope
        retval = (queryAction == obj.queryAction);
        }
        break;
    case ID_queryColID:
        {  // new scope
        retval = (queryColID == obj.queryColID);
        }
        break;
    case ID_queryKeyValue:
        {  // new scope
        retval = (queryKeyValue == obj.queryKeyValue);
        }
        break;
    case ID_queryCollectionValues:
        {  // new scope
        retval = (queryCollectionValues == obj.queryCollectionValues);
        }
        break;
    case ID_downloadCollectionID:
        {  // new scope
        retval = (downloadCollectionID == obj.downloadCollectionID);
        }
        break;
    case ID_downloadType:
        {  // new scope
        retval = (downloadType == obj.downloadType);
        }
        break;
    case ID_downloadName:
        {  // new scope
        retval = (downloadName == obj.downloadName);
        }
        break;
    case ID_quickSharing:
        {  // new scope
        retval = (quickSharing == obj.quickSharing);
        }
        break;
    case ID_quickCollectionTitle:
        {  // new scope
        retval = (quickCollectionTitle == obj.quickCollectionTitle);
        }
        break;
    case ID_quickCollectionEmails:
        {  // new scope
        retval = (quickCollectionEmails == obj.quickCollectionEmails);
        }
        break;
    case ID_quickFrameRate:
        {  // new scope
        retval = (quickFrameRate == obj.quickFrameRate);
        }
        break;
    case ID_quickDownloadType:
        {  // new scope
        retval = (quickDownloadType == obj.quickDownloadType);
        }
        break;
    default: retval = false;
    }

    return retval;
}

///////////////////////////////////////////////////////////////////////////////
// User-defined methods.
///////////////////////////////////////////////////////////////////////////////

