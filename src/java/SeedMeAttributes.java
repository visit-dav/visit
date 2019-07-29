// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

package llnl.visit;


// ****************************************************************************
// Class: SeedMeAttributes
//
// Purpose:
//    This class contains the attributes needed to interact with SeedMe.
//
// Notes:      Autogenerated by xml2java.
//
// Programmer: xml2java
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

public class SeedMeAttributes extends AttributeSubject
{
    private static int SeedMeAttributes_numAdditionalAtts = 30;

    // Enum values
    public final static int COLLECTIONMODE_COLLECTION_NEW = 0;
    public final static int COLLECTIONMODE_COLLECTION_UPDATE = 1;

    public final static int SHARING_SHARING_PRIVATE = 0;
    public final static int SHARING_SHARING_GROUP = 1;
    public final static int SHARING_SHARING_PUBLIC = 2;

    public final static int QUERYACTION_LIST_COL = 0;
    public final static int QUERYACTION_FIND_COL = 1;
    public final static int QUERYACTION_QUERY_COL = 2;

    public final static int QUERYCOLLECTIONVALUES_QUERY_ALL = 0;
    public final static int QUERYCOLLECTIONVALUES_QUERY_KEY_VALUE = 1;
    public final static int QUERYCOLLECTIONVALUES_QUERY_TICKER = 2;
    public final static int QUERYCOLLECTIONVALUES_QUERY_URL = 3;

    public final static int DOWNLOAD_DOWNLOAD_ALL = 0;
    public final static int DOWNLOAD_DOWNLOAD_VIDEO = 1;
    public final static int DOWNLOAD_DOWNLOAD_NAME = 2;

    public final static int DOWNLOADQUALITY_DOWNLOAD_Q_ALL = 0;
    public final static int DOWNLOADQUALITY_DOWNLOAD_Q_BEST = 1;
    public final static int DOWNLOADQUALITY_DOWNLOAD_Q_HIGHEST = 2;
    public final static int DOWNLOADQUALITY_DOWNLOAD_Q_HIGH = 3;
    public final static int DOWNLOADQUALITY_DOWNLOAD_Q_MEDIUM = 4;
    public final static int DOWNLOADQUALITY_DOWNLOAD_Q_LOW = 5;
    public final static int DOWNLOADQUALITY_DOWNLOAD_Q_LOWEST = 6;


    public SeedMeAttributes()
    {
        super(SeedMeAttributes_numAdditionalAtts);

        collectionMode = COLLECTIONMODE_COLLECTION_NEW;
        collectionID = 0;
        sharing = SHARING_SHARING_PRIVATE;
        collectionTitle = new String("");
        collectionDescription = new String("");
        overwriteFiles = false;
        keyValue = new String("");
        collectionEmails = new String("");
        uploadCurrentFile = true;
        currentTitle = new String("");
        currentDescription = new String("");
        uploadSequenceFile = false;
        sequenceTitle = new String("");
        sequenceDescription = new String("");
        createVideo = false;
        frameRate = 30;
        operationResult = new String("");
        queryAction = QUERYACTION_LIST_COL;
        queryColID = new String("");
        queryKeyValue = new String("");
        queryCollectionValues = QUERYCOLLECTIONVALUES_QUERY_ALL;
        downloadCollectionID = 0;
        downloadType = DOWNLOAD_DOWNLOAD_ALL;
        downloadName = new String("*");
        quickSharing = SHARING_SHARING_PRIVATE;
        quickCollectionTitle = new String("");
        quickCollectionEmails = new String("");
        quickFrameRate = 10;
        quickDownloadType = DOWNLOADQUALITY_DOWNLOAD_Q_ALL;
        clearAllTabsOnClose = true;
    }

    public SeedMeAttributes(int nMoreFields)
    {
        super(SeedMeAttributes_numAdditionalAtts + nMoreFields);

        collectionMode = COLLECTIONMODE_COLLECTION_NEW;
        collectionID = 0;
        sharing = SHARING_SHARING_PRIVATE;
        collectionTitle = new String("");
        collectionDescription = new String("");
        overwriteFiles = false;
        keyValue = new String("");
        collectionEmails = new String("");
        uploadCurrentFile = true;
        currentTitle = new String("");
        currentDescription = new String("");
        uploadSequenceFile = false;
        sequenceTitle = new String("");
        sequenceDescription = new String("");
        createVideo = false;
        frameRate = 30;
        operationResult = new String("");
        queryAction = QUERYACTION_LIST_COL;
        queryColID = new String("");
        queryKeyValue = new String("");
        queryCollectionValues = QUERYCOLLECTIONVALUES_QUERY_ALL;
        downloadCollectionID = 0;
        downloadType = DOWNLOAD_DOWNLOAD_ALL;
        downloadName = new String("*");
        quickSharing = SHARING_SHARING_PRIVATE;
        quickCollectionTitle = new String("");
        quickCollectionEmails = new String("");
        quickFrameRate = 10;
        quickDownloadType = DOWNLOADQUALITY_DOWNLOAD_Q_ALL;
        clearAllTabsOnClose = true;
    }

    public SeedMeAttributes(SeedMeAttributes obj)
    {
        super(obj);

        collectionMode = obj.collectionMode;
        collectionID = obj.collectionID;
        sharing = obj.sharing;
        collectionTitle = new String(obj.collectionTitle);
        collectionDescription = new String(obj.collectionDescription);
        overwriteFiles = obj.overwriteFiles;
        keyValue = new String(obj.keyValue);
        collectionEmails = new String(obj.collectionEmails);
        uploadCurrentFile = obj.uploadCurrentFile;
        currentTitle = new String(obj.currentTitle);
        currentDescription = new String(obj.currentDescription);
        uploadSequenceFile = obj.uploadSequenceFile;
        sequenceTitle = new String(obj.sequenceTitle);
        sequenceDescription = new String(obj.sequenceDescription);
        createVideo = obj.createVideo;
        frameRate = obj.frameRate;
        operationResult = new String(obj.operationResult);
        queryAction = obj.queryAction;
        queryColID = new String(obj.queryColID);
        queryKeyValue = new String(obj.queryKeyValue);
        queryCollectionValues = obj.queryCollectionValues;
        downloadCollectionID = obj.downloadCollectionID;
        downloadType = obj.downloadType;
        downloadName = new String(obj.downloadName);
        quickSharing = obj.quickSharing;
        quickCollectionTitle = new String(obj.quickCollectionTitle);
        quickCollectionEmails = new String(obj.quickCollectionEmails);
        quickFrameRate = obj.quickFrameRate;
        quickDownloadType = obj.quickDownloadType;
        clearAllTabsOnClose = obj.clearAllTabsOnClose;

        SelectAll();
    }

    public int Offset()
    {
        return super.Offset() + super.GetNumAdditionalAttributes();
    }

    public int GetNumAdditionalAttributes()
    {
        return SeedMeAttributes_numAdditionalAtts;
    }

    public boolean equals(SeedMeAttributes obj)
    {
        // Create the return value
        return ((collectionMode == obj.collectionMode) &&
                (collectionID == obj.collectionID) &&
                (sharing == obj.sharing) &&
                (collectionTitle.equals(obj.collectionTitle)) &&
                (collectionDescription.equals(obj.collectionDescription)) &&
                (overwriteFiles == obj.overwriteFiles) &&
                (keyValue.equals(obj.keyValue)) &&
                (collectionEmails.equals(obj.collectionEmails)) &&
                (uploadCurrentFile == obj.uploadCurrentFile) &&
                (currentTitle.equals(obj.currentTitle)) &&
                (currentDescription.equals(obj.currentDescription)) &&
                (uploadSequenceFile == obj.uploadSequenceFile) &&
                (sequenceTitle.equals(obj.sequenceTitle)) &&
                (sequenceDescription.equals(obj.sequenceDescription)) &&
                (createVideo == obj.createVideo) &&
                (frameRate == obj.frameRate) &&
                (operationResult.equals(obj.operationResult)) &&
                (queryAction == obj.queryAction) &&
                (queryColID.equals(obj.queryColID)) &&
                (queryKeyValue.equals(obj.queryKeyValue)) &&
                (queryCollectionValues == obj.queryCollectionValues) &&
                (downloadCollectionID == obj.downloadCollectionID) &&
                (downloadType == obj.downloadType) &&
                (downloadName.equals(obj.downloadName)) &&
                (quickSharing == obj.quickSharing) &&
                (quickCollectionTitle.equals(obj.quickCollectionTitle)) &&
                (quickCollectionEmails.equals(obj.quickCollectionEmails)) &&
                (quickFrameRate == obj.quickFrameRate) &&
                (quickDownloadType == obj.quickDownloadType) &&
                (clearAllTabsOnClose == obj.clearAllTabsOnClose));
    }

    // Property setting methods
    public void SetCollectionMode(int collectionMode_)
    {
        collectionMode = collectionMode_;
        Select(0);
    }

    public void SetCollectionID(int collectionID_)
    {
        collectionID = collectionID_;
        Select(1);
    }

    public void SetSharing(int sharing_)
    {
        sharing = sharing_;
        Select(2);
    }

    public void SetCollectionTitle(String collectionTitle_)
    {
        collectionTitle = collectionTitle_;
        Select(3);
    }

    public void SetCollectionDescription(String collectionDescription_)
    {
        collectionDescription = collectionDescription_;
        Select(4);
    }

    public void SetOverwriteFiles(boolean overwriteFiles_)
    {
        overwriteFiles = overwriteFiles_;
        Select(5);
    }

    public void SetKeyValue(String keyValue_)
    {
        keyValue = keyValue_;
        Select(6);
    }

    public void SetCollectionEmails(String collectionEmails_)
    {
        collectionEmails = collectionEmails_;
        Select(7);
    }

    public void SetUploadCurrentFile(boolean uploadCurrentFile_)
    {
        uploadCurrentFile = uploadCurrentFile_;
        Select(8);
    }

    public void SetCurrentTitle(String currentTitle_)
    {
        currentTitle = currentTitle_;
        Select(9);
    }

    public void SetCurrentDescription(String currentDescription_)
    {
        currentDescription = currentDescription_;
        Select(10);
    }

    public void SetUploadSequenceFile(boolean uploadSequenceFile_)
    {
        uploadSequenceFile = uploadSequenceFile_;
        Select(11);
    }

    public void SetSequenceTitle(String sequenceTitle_)
    {
        sequenceTitle = sequenceTitle_;
        Select(12);
    }

    public void SetSequenceDescription(String sequenceDescription_)
    {
        sequenceDescription = sequenceDescription_;
        Select(13);
    }

    public void SetCreateVideo(boolean createVideo_)
    {
        createVideo = createVideo_;
        Select(14);
    }

    public void SetFrameRate(int frameRate_)
    {
        frameRate = frameRate_;
        Select(15);
    }

    public void SetOperationResult(String operationResult_)
    {
        operationResult = operationResult_;
        Select(16);
    }

    public void SetQueryAction(int queryAction_)
    {
        queryAction = queryAction_;
        Select(17);
    }

    public void SetQueryColID(String queryColID_)
    {
        queryColID = queryColID_;
        Select(18);
    }

    public void SetQueryKeyValue(String queryKeyValue_)
    {
        queryKeyValue = queryKeyValue_;
        Select(19);
    }

    public void SetQueryCollectionValues(int queryCollectionValues_)
    {
        queryCollectionValues = queryCollectionValues_;
        Select(20);
    }

    public void SetDownloadCollectionID(int downloadCollectionID_)
    {
        downloadCollectionID = downloadCollectionID_;
        Select(21);
    }

    public void SetDownloadType(int downloadType_)
    {
        downloadType = downloadType_;
        Select(22);
    }

    public void SetDownloadName(String downloadName_)
    {
        downloadName = downloadName_;
        Select(23);
    }

    public void SetQuickSharing(int quickSharing_)
    {
        quickSharing = quickSharing_;
        Select(24);
    }

    public void SetQuickCollectionTitle(String quickCollectionTitle_)
    {
        quickCollectionTitle = quickCollectionTitle_;
        Select(25);
    }

    public void SetQuickCollectionEmails(String quickCollectionEmails_)
    {
        quickCollectionEmails = quickCollectionEmails_;
        Select(26);
    }

    public void SetQuickFrameRate(int quickFrameRate_)
    {
        quickFrameRate = quickFrameRate_;
        Select(27);
    }

    public void SetQuickDownloadType(int quickDownloadType_)
    {
        quickDownloadType = quickDownloadType_;
        Select(28);
    }

    public void SetClearAllTabsOnClose(boolean clearAllTabsOnClose_)
    {
        clearAllTabsOnClose = clearAllTabsOnClose_;
        Select(29);
    }

    // Property getting methods
    public int     GetCollectionMode() { return collectionMode; }
    public int     GetCollectionID() { return collectionID; }
    public int     GetSharing() { return sharing; }
    public String  GetCollectionTitle() { return collectionTitle; }
    public String  GetCollectionDescription() { return collectionDescription; }
    public boolean GetOverwriteFiles() { return overwriteFiles; }
    public String  GetKeyValue() { return keyValue; }
    public String  GetCollectionEmails() { return collectionEmails; }
    public boolean GetUploadCurrentFile() { return uploadCurrentFile; }
    public String  GetCurrentTitle() { return currentTitle; }
    public String  GetCurrentDescription() { return currentDescription; }
    public boolean GetUploadSequenceFile() { return uploadSequenceFile; }
    public String  GetSequenceTitle() { return sequenceTitle; }
    public String  GetSequenceDescription() { return sequenceDescription; }
    public boolean GetCreateVideo() { return createVideo; }
    public int     GetFrameRate() { return frameRate; }
    public String  GetOperationResult() { return operationResult; }
    public int     GetQueryAction() { return queryAction; }
    public String  GetQueryColID() { return queryColID; }
    public String  GetQueryKeyValue() { return queryKeyValue; }
    public int     GetQueryCollectionValues() { return queryCollectionValues; }
    public int     GetDownloadCollectionID() { return downloadCollectionID; }
    public int     GetDownloadType() { return downloadType; }
    public String  GetDownloadName() { return downloadName; }
    public int     GetQuickSharing() { return quickSharing; }
    public String  GetQuickCollectionTitle() { return quickCollectionTitle; }
    public String  GetQuickCollectionEmails() { return quickCollectionEmails; }
    public int     GetQuickFrameRate() { return quickFrameRate; }
    public int     GetQuickDownloadType() { return quickDownloadType; }
    public boolean GetClearAllTabsOnClose() { return clearAllTabsOnClose; }

    // Write and read methods.
    public void WriteAtts(CommunicationBuffer buf)
    {
        if(WriteSelect(0, buf))
            buf.WriteInt(collectionMode);
        if(WriteSelect(1, buf))
            buf.WriteInt(collectionID);
        if(WriteSelect(2, buf))
            buf.WriteInt(sharing);
        if(WriteSelect(3, buf))
            buf.WriteString(collectionTitle);
        if(WriteSelect(4, buf))
            buf.WriteString(collectionDescription);
        if(WriteSelect(5, buf))
            buf.WriteBool(overwriteFiles);
        if(WriteSelect(6, buf))
            buf.WriteString(keyValue);
        if(WriteSelect(7, buf))
            buf.WriteString(collectionEmails);
        if(WriteSelect(8, buf))
            buf.WriteBool(uploadCurrentFile);
        if(WriteSelect(9, buf))
            buf.WriteString(currentTitle);
        if(WriteSelect(10, buf))
            buf.WriteString(currentDescription);
        if(WriteSelect(11, buf))
            buf.WriteBool(uploadSequenceFile);
        if(WriteSelect(12, buf))
            buf.WriteString(sequenceTitle);
        if(WriteSelect(13, buf))
            buf.WriteString(sequenceDescription);
        if(WriteSelect(14, buf))
            buf.WriteBool(createVideo);
        if(WriteSelect(15, buf))
            buf.WriteInt(frameRate);
        if(WriteSelect(16, buf))
            buf.WriteString(operationResult);
        if(WriteSelect(17, buf))
            buf.WriteInt(queryAction);
        if(WriteSelect(18, buf))
            buf.WriteString(queryColID);
        if(WriteSelect(19, buf))
            buf.WriteString(queryKeyValue);
        if(WriteSelect(20, buf))
            buf.WriteInt(queryCollectionValues);
        if(WriteSelect(21, buf))
            buf.WriteInt(downloadCollectionID);
        if(WriteSelect(22, buf))
            buf.WriteInt(downloadType);
        if(WriteSelect(23, buf))
            buf.WriteString(downloadName);
        if(WriteSelect(24, buf))
            buf.WriteInt(quickSharing);
        if(WriteSelect(25, buf))
            buf.WriteString(quickCollectionTitle);
        if(WriteSelect(26, buf))
            buf.WriteString(quickCollectionEmails);
        if(WriteSelect(27, buf))
            buf.WriteInt(quickFrameRate);
        if(WriteSelect(28, buf))
            buf.WriteInt(quickDownloadType);
        if(WriteSelect(29, buf))
            buf.WriteBool(clearAllTabsOnClose);
    }

    public void ReadAtts(int index, CommunicationBuffer buf)
    {
        switch(index)
        {
        case 0:
            SetCollectionMode(buf.ReadInt());
            break;
        case 1:
            SetCollectionID(buf.ReadInt());
            break;
        case 2:
            SetSharing(buf.ReadInt());
            break;
        case 3:
            SetCollectionTitle(buf.ReadString());
            break;
        case 4:
            SetCollectionDescription(buf.ReadString());
            break;
        case 5:
            SetOverwriteFiles(buf.ReadBool());
            break;
        case 6:
            SetKeyValue(buf.ReadString());
            break;
        case 7:
            SetCollectionEmails(buf.ReadString());
            break;
        case 8:
            SetUploadCurrentFile(buf.ReadBool());
            break;
        case 9:
            SetCurrentTitle(buf.ReadString());
            break;
        case 10:
            SetCurrentDescription(buf.ReadString());
            break;
        case 11:
            SetUploadSequenceFile(buf.ReadBool());
            break;
        case 12:
            SetSequenceTitle(buf.ReadString());
            break;
        case 13:
            SetSequenceDescription(buf.ReadString());
            break;
        case 14:
            SetCreateVideo(buf.ReadBool());
            break;
        case 15:
            SetFrameRate(buf.ReadInt());
            break;
        case 16:
            SetOperationResult(buf.ReadString());
            break;
        case 17:
            SetQueryAction(buf.ReadInt());
            break;
        case 18:
            SetQueryColID(buf.ReadString());
            break;
        case 19:
            SetQueryKeyValue(buf.ReadString());
            break;
        case 20:
            SetQueryCollectionValues(buf.ReadInt());
            break;
        case 21:
            SetDownloadCollectionID(buf.ReadInt());
            break;
        case 22:
            SetDownloadType(buf.ReadInt());
            break;
        case 23:
            SetDownloadName(buf.ReadString());
            break;
        case 24:
            SetQuickSharing(buf.ReadInt());
            break;
        case 25:
            SetQuickCollectionTitle(buf.ReadString());
            break;
        case 26:
            SetQuickCollectionEmails(buf.ReadString());
            break;
        case 27:
            SetQuickFrameRate(buf.ReadInt());
            break;
        case 28:
            SetQuickDownloadType(buf.ReadInt());
            break;
        case 29:
            SetClearAllTabsOnClose(buf.ReadBool());
            break;
        }
    }

    public String toString(String indent)
    {
        String str = new String();
        str = str + indent + "collectionMode = ";
        if(collectionMode == COLLECTIONMODE_COLLECTION_NEW)
            str = str + "COLLECTIONMODE_COLLECTION_NEW";
        if(collectionMode == COLLECTIONMODE_COLLECTION_UPDATE)
            str = str + "COLLECTIONMODE_COLLECTION_UPDATE";
        str = str + "\n";
        str = str + intToString("collectionID", collectionID, indent) + "\n";
        str = str + indent + "sharing = ";
        if(sharing == SHARING_SHARING_PRIVATE)
            str = str + "SHARING_SHARING_PRIVATE";
        if(sharing == SHARING_SHARING_GROUP)
            str = str + "SHARING_SHARING_GROUP";
        if(sharing == SHARING_SHARING_PUBLIC)
            str = str + "SHARING_SHARING_PUBLIC";
        str = str + "\n";
        str = str + stringToString("collectionTitle", collectionTitle, indent) + "\n";
        str = str + stringToString("collectionDescription", collectionDescription, indent) + "\n";
        str = str + boolToString("overwriteFiles", overwriteFiles, indent) + "\n";
        str = str + stringToString("keyValue", keyValue, indent) + "\n";
        str = str + stringToString("collectionEmails", collectionEmails, indent) + "\n";
        str = str + boolToString("uploadCurrentFile", uploadCurrentFile, indent) + "\n";
        str = str + stringToString("currentTitle", currentTitle, indent) + "\n";
        str = str + stringToString("currentDescription", currentDescription, indent) + "\n";
        str = str + boolToString("uploadSequenceFile", uploadSequenceFile, indent) + "\n";
        str = str + stringToString("sequenceTitle", sequenceTitle, indent) + "\n";
        str = str + stringToString("sequenceDescription", sequenceDescription, indent) + "\n";
        str = str + boolToString("createVideo", createVideo, indent) + "\n";
        str = str + intToString("frameRate", frameRate, indent) + "\n";
        str = str + stringToString("operationResult", operationResult, indent) + "\n";
        str = str + indent + "queryAction = ";
        if(queryAction == QUERYACTION_LIST_COL)
            str = str + "QUERYACTION_LIST_COL";
        if(queryAction == QUERYACTION_FIND_COL)
            str = str + "QUERYACTION_FIND_COL";
        if(queryAction == QUERYACTION_QUERY_COL)
            str = str + "QUERYACTION_QUERY_COL";
        str = str + "\n";
        str = str + stringToString("queryColID", queryColID, indent) + "\n";
        str = str + stringToString("queryKeyValue", queryKeyValue, indent) + "\n";
        str = str + indent + "queryCollectionValues = ";
        if(queryCollectionValues == QUERYCOLLECTIONVALUES_QUERY_ALL)
            str = str + "QUERYCOLLECTIONVALUES_QUERY_ALL";
        if(queryCollectionValues == QUERYCOLLECTIONVALUES_QUERY_KEY_VALUE)
            str = str + "QUERYCOLLECTIONVALUES_QUERY_KEY_VALUE";
        if(queryCollectionValues == QUERYCOLLECTIONVALUES_QUERY_TICKER)
            str = str + "QUERYCOLLECTIONVALUES_QUERY_TICKER";
        if(queryCollectionValues == QUERYCOLLECTIONVALUES_QUERY_URL)
            str = str + "QUERYCOLLECTIONVALUES_QUERY_URL";
        str = str + "\n";
        str = str + intToString("downloadCollectionID", downloadCollectionID, indent) + "\n";
        str = str + indent + "downloadType = ";
        if(downloadType == DOWNLOAD_DOWNLOAD_ALL)
            str = str + "DOWNLOAD_DOWNLOAD_ALL";
        if(downloadType == DOWNLOAD_DOWNLOAD_VIDEO)
            str = str + "DOWNLOAD_DOWNLOAD_VIDEO";
        if(downloadType == DOWNLOAD_DOWNLOAD_NAME)
            str = str + "DOWNLOAD_DOWNLOAD_NAME";
        str = str + "\n";
        str = str + stringToString("downloadName", downloadName, indent) + "\n";
        str = str + indent + "quickSharing = ";
        if(quickSharing == SHARING_SHARING_PRIVATE)
            str = str + "SHARING_SHARING_PRIVATE";
        if(quickSharing == SHARING_SHARING_GROUP)
            str = str + "SHARING_SHARING_GROUP";
        if(quickSharing == SHARING_SHARING_PUBLIC)
            str = str + "SHARING_SHARING_PUBLIC";
        str = str + "\n";
        str = str + stringToString("quickCollectionTitle", quickCollectionTitle, indent) + "\n";
        str = str + stringToString("quickCollectionEmails", quickCollectionEmails, indent) + "\n";
        str = str + intToString("quickFrameRate", quickFrameRate, indent) + "\n";
        str = str + indent + "quickDownloadType = ";
        if(quickDownloadType == DOWNLOADQUALITY_DOWNLOAD_Q_ALL)
            str = str + "DOWNLOADQUALITY_DOWNLOAD_Q_ALL";
        if(quickDownloadType == DOWNLOADQUALITY_DOWNLOAD_Q_BEST)
            str = str + "DOWNLOADQUALITY_DOWNLOAD_Q_BEST";
        if(quickDownloadType == DOWNLOADQUALITY_DOWNLOAD_Q_HIGHEST)
            str = str + "DOWNLOADQUALITY_DOWNLOAD_Q_HIGHEST";
        if(quickDownloadType == DOWNLOADQUALITY_DOWNLOAD_Q_HIGH)
            str = str + "DOWNLOADQUALITY_DOWNLOAD_Q_HIGH";
        if(quickDownloadType == DOWNLOADQUALITY_DOWNLOAD_Q_MEDIUM)
            str = str + "DOWNLOADQUALITY_DOWNLOAD_Q_MEDIUM";
        if(quickDownloadType == DOWNLOADQUALITY_DOWNLOAD_Q_LOW)
            str = str + "DOWNLOADQUALITY_DOWNLOAD_Q_LOW";
        if(quickDownloadType == DOWNLOADQUALITY_DOWNLOAD_Q_LOWEST)
            str = str + "DOWNLOADQUALITY_DOWNLOAD_Q_LOWEST";
        str = str + "\n";
        str = str + boolToString("clearAllTabsOnClose", clearAllTabsOnClose, indent) + "\n";
        return str;
    }


    // Attributes
    private int     collectionMode;
    private int     collectionID;
    private int     sharing;
    private String  collectionTitle;
    private String  collectionDescription;
    private boolean overwriteFiles;
    private String  keyValue;
    private String  collectionEmails;
    private boolean uploadCurrentFile;
    private String  currentTitle;
    private String  currentDescription;
    private boolean uploadSequenceFile;
    private String  sequenceTitle;
    private String  sequenceDescription;
    private boolean createVideo;
    private int     frameRate;
    private String  operationResult;
    private int     queryAction;
    private String  queryColID;
    private String  queryKeyValue;
    private int     queryCollectionValues;
    private int     downloadCollectionID;
    private int     downloadType;
    private String  downloadName;
    private int     quickSharing;
    private String  quickCollectionTitle;
    private String  quickCollectionEmails;
    private int     quickFrameRate;
    private int     quickDownloadType;
    private boolean clearAllTabsOnClose;
}

