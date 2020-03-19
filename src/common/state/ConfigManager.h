// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef CONFIGURATION_MANAGER_H
#define CONFIGURATION_MANAGER_H
#include <state_exports.h>
#include <stdio.h>
#include <DataNode.h>
#include <MapNode.h>
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
//   Kathleen Biagas, Wed Nov 28 13:44:08 PST 2018
//   Added methods to read/write MapNode nodes.
//
// ****************************************************************************

class STATE_API ConfigManager
{
public:
    ConfigManager();
    virtual ~ConfigManager();

    virtual bool WriteConfigFile(const char *filename) = 0;
    virtual DataNode *ReadConfigFile(const char *filename) = 0;

    virtual bool WriteConfigFile(std::ostream& out) = 0;
    virtual DataNode *ReadConfigFile(std::istream& in) = 0;
protected:
    // Functions to write out the tree.
    void WriteObject(std::ostream& out, DataNode *node, int indentLevel = 0);
    void WriteData(std::ostream &out, DataNode *node);
    void WriteQuotedStringData(std::ostream& out, const std::string &str);
    void WriteEscapedString(std::ostream& out, const std::string &str);
    void WriteIndent(std::ostream &out, int indentLevel);
    void WriteBack(DataNode *root);

    // Functions to read in the tree.
    bool ReadObject(std::istream& in, DataNode *);
    bool ReadObjectHelper(std::istream &in, DataNode *, bool &);
    char ReadChar(std::istream &in);
    void PutBackChar(char c);
    void FinishTag(std::istream& in);
    bool ReadField(std::istream &in, DataNode *parentNode,
                   const std::string &tagName,
                   NodeTypeEnum tagType, int tagLength, bool noEndTag);
    DataNode *ReadFieldData(std::istream& in, const std::string &tagName,
                            NodeTypeEnum tagType, int tagLength, bool noEndTag);
    bool ReadTag(std::istream &in, std::string &tagName, NodeTypeEnum &tagType,
                 int &tagLength, bool &tagIsEndTag, bool &noEndTag);
    stringVector ReadStringVector(std::istream &in, char termChar);
    void RemoveLeadAndTailQuotes(stringVector &sv);

    void WriteMapNode(std::ostream& out, const MapNode &, int indentLevel = 0);
    void ReadMapNodeFieldData(std::istream& in, MapNode &mn,
                             const std::string &tagName, NodeTypeEnum type,
                             int tagLength, bool noEndTag);
    void ReadMapNodeFields(std::istream &in, MapNode &mn, bool &te);

private:
    // File attributes used in reading.
    bool  putback;
    char  putbackChar;
};

#endif
