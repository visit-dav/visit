#ifndef CONFIGURATION_MANAGER_H
#define CONFIGURATION_MANAGER_H
#include <state_exports.h>
#include <stdio.h>
#include <DataNode.h>

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
// ****************************************************************************

class STATE_API ConfigManager
{
public:
    ConfigManager();
    virtual ~ConfigManager();

    virtual void WriteConfigFile(const char *filename) = 0;
    virtual DataNode *ReadConfigFile(const char *filename) = 0;

    char *GetDefaultConfigFile(const char *filename = 0, const char *home = 0);
    char *GetSystemConfigFile(const char *filename = 0);
protected:
    // Functions to write out the tree.
    void WriteObject(DataNode *node, int indentLevel = 0);
    void WriteData(DataNode *node);
    void WriteIndent(int indentLevel);
    void WriteBack(DataNode *root);

    // Functions to read in the tree.
    void ReadObject(DataNode *);
    char ReadChar();
    void PutBackChar(char c);
    void ReadField(DataNode *);
    DataNode *ReadFieldData(const char *, NodeTypeEnum, int);
    void FinishTag();
    int  ReadLength();
    NodeTypeEnum ReadType();
    void ReadName(char *);
    int  ReadTag();
    char **ReadStringList(int *length);
    void FreeStringList(char **strlist, int len);

    // File attributes used in reading.
    bool  putback;
    char  putbackChar;
    FILE *fp;
};

#endif
