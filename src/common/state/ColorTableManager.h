#ifndef COLORTABLE_MANAGER_H
#define COLORTABLE_MANAGER_H
#include <ConfigManager.h>
#include <ColorControlPointList.h>
#include <ColorTableAttributes.h>
#include <string>

class DataNode;

// ****************************************************************************
// Class: ColorTableManager
//
// Purpose:
//   Reads and writes ColorControlPointLists to/from files.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 3 17:49:53 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class STATE_API ColorTableManager : public ConfigManager
{
public:
    ColorTableManager();
    virtual ~ColorTableManager();

    std::string       Export(const std::string &ctName,
                             const ColorControlPointList &ccpl);
    bool              ImportColorTables(ColorTableAttributes *cta);

    virtual void      WriteConfigFile(const char *filename);
    virtual DataNode *ReadConfigFile(const char *filename);
protected:
    void              ImportColorTable(const std::string &ctFileName);
    static void       ImportHelper(void *, const std::string &,
                                   bool, bool, long);
private:
    ColorTableAttributes *ctAtts;
    ColorControlPointList ccpl;
};

#endif
