#ifndef AVT_COLORTABLES_H
#define AVT_COLORTABLES_H
#include <plotter_exports.h>
#include <string>

// Forward declarations
class ColorTableAttributes;

// ****************************************************************************
// Class: avtColorTables
//
// Purpose:
//   This class is a singleton that contains all of the color tables that are
//   available to avt.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 15 13:57:27 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed Nov 20 13:12:49 PST 2002
//   I made it understand discrete color tables. I also removed most of
//   the implementation in favor of using the ColorTableAttributes state
//   object.
//
//   Brad Whitlock, Tue Jul 1 17:20:38 PST 2003
//   I added methods to import and export color tables.
//
// ****************************************************************************

class PLOTTER_API avtColorTables
{
public:
    static avtColorTables *Instance();

    const std::string   &GetDefaultContinuousColorTable() const;
    void                 SetDefaultContinuousColorTable(const std::string &);
    const std::string   &GetDefaultDiscreteColorTable() const;
    void                 SetDefaultDiscreteColorTable(const std::string &);

    const unsigned char *GetColors(const std::string &ctName);

    unsigned char       *GetSampledColors(const std::string &ctName,
                                          int nColors) const;
    bool                 GetControlPointColor(const std::string &ctName,
                                              int i, unsigned char *rgb) const;
    int                  GetNumColors() const { return 256; };
    bool                 IsDiscrete(const std::string &ctName) const;
    bool                 ColorTableExists(const std::string &ctName) const;

    ColorTableAttributes   *GetColorTables() { return ctAtts; }
    void                    SetColorTables(const ColorTableAttributes &);

    std::string          ExportColorTable(const std::string &ctName);
    void                 ImportColorTables();
protected:
    avtColorTables();
    ~avtColorTables();

    ColorTableAttributes   *ctAtts;
    unsigned char           tmpColors[256*3];

    static avtColorTables  *instance;
};

#endif
