#ifndef CONFIGURATION_MANAGER_H
#define CONFIGURATION_MANAGER_H
#include <state_exports.h>
#include <stdio.h>
#include <DataNode.h>
#include <string>

// ****************************************************************************
// Class: ConfigManager
//
// Purpose:
//   This class contains logic to read and write config files.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 28 10:29:28 PDT 2000
//
// Modifications:
//   Brad Whitlock, Thu Mar 29 17:21:51 PST 2001
//   Modified the GetDefaultConfigFile method so it accepts an argument.
//
//   Brad Whitlock, Tue Feb 19 12:29:17 PDT 2002
//   Added a method to get the name of the system config file.
//
//   Brad Whitlock, Thu Jul 3 16:14:04 PST 2003
//   Rewrote the methods that read in the file.
//
//   Brad Whitlock, Thu Jul 3 17:43:47 PST 2003
//   I added a convenience method to get the user's home .visit dir.
//
//   Brad Whitlock, Fri Oct 3 16:11:27 PST 2003
//   Added internal methods to handle string vectors better.
//
//   Brad Whitlock, Wed Feb 16 09:13:21 PDT 2005
//   Removed GetUserVisItDirectory, GetDefaultConfigFile, GetSystemConfigFile
//   methods and made them part of the utility library (Utility.h).
//
//   Brad Whitlock, Thu Feb 17 15:45:19 PST 2005
//   I made WriteConfigFile return a bool.
//
//   Jeremy Meredith, Tue Aug  2 16:05:09 PDT 2005
//   Added WriteEscapedString method.
//
// ****************************************************************************

class STATE_API ConfigManager
{
public:
    ConfigManager();
    virtual ~ConfigManager();

    virtual bool WriteConfigFile(const char *filename) = 0;
    virtual DataNode *ReadConfigFile(const char *filename) = 0;
protected:
    // Functions to write out the tree.
    void WriteObject(DataNode *node, int indentLevel = 0);
    void WriteData(DataNode *node);
    void WriteQuotedStringData(const std::string &str);
    void WriteEscapedString(const std::string &str);
    void WriteIndent(int indentLevel);
    void WriteBack(DataNode *root);

    // Functions to read in the tree.
    bool ReadObject(DataNode *);
    bool ReadObjectHelper(DataNode *, bool &);
    char ReadChar();
    void PutBackChar(char c);
    void FinishTag();
    bool ReadField(DataNode *parentNode, const std::string &tagName,
                   NodeTypeEnum tagType, int tagLength);
    DataNode *ReadFieldData(const std::string &tagName, NodeTypeEnum tagType,
                            int tagLength);
    bool ReadTag(std::string &tagName, NodeTypeEnum &tagType,
                   int &tagLength, bool &tagIsReturnTag);
    stringVector ReadStringVector(char termChar);
    void RemoveLeadAndTailQuotes(stringVector &sv);

    // File attributes used in reading.
    bool  putback;
    char  putbackChar;
    FILE *fp;
};

#endif
