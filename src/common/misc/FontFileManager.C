// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <FontFileManager.h>

#include <FileFunctions.h>
#include <InstallationFunctions.h>

#include <algorithm>
#include <dirent.h>
#include <iostream>
#include <map>
#include <regex>
#include <string> 
#include <sys/stat.h>
#include <vector>

static std::string to_lower(const std::string &s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}           
            
// *****************************************************************************
// Purpose: Handle some variation in the `.ttf` file names with respect to
// variant, "oblique" vs. "italic", "regular" vs. "", and dash, underscore or
// space separators.
//
// Mark C. Miller, Mon May 19 11:26:38 PDT 2025
// *****************************************************************************
//
static std::string font_base(const std::string &filename) {
    std::string base = filename;
    base = std::regex_replace(base, std::regex("[-_ ]?bolditalic|[-_ ]?boldoblique", std::regex::icase), "");
    base = std::regex_replace(base, std::regex("[-_ ]?italic|[-_ ]?oblique", std::regex::icase), "");
    base = std::regex_replace(base, std::regex("[-_ ]?bold", std::regex::icase), "");
    base = std::regex_replace(base, std::regex("[-_ ]?regular", std::regex::icase), "");
    base = std::regex_replace(base, std::regex("\\.ttf$", std::regex::icase), "");
    return base;
}

enum FontStyle { Regular, Bold, Italic, BoldItalic };

static FontStyle detect_style(const std::string &filename) {
    std::string lower = to_lower(filename);
    bool hasBold = lower.find("bold") != std::string::npos;
    bool hasItalic = (lower.find("italic") != std::string::npos || lower.find("oblique") != std::string::npos);
    if (hasBold && hasItalic) return BoldItalic;
    if (hasBold) return Bold;
    if (hasItalic) return Italic;
    return Regular;
}

// *****************************************************************************
// Purpose: A callback used in ReadAndProcessDirectory to build up a map using
// a normalized version of the `.ttf` file name as the key and the FontVariant
// file names as the value. The key is is all lower case with separator
// characters (dash, underscore or space) and variant removed.
//
// Mark C. Miller, Mon May 19 11:22:08 PDT 2025
// *****************************************************************************
//
static void gatherFontFileName(void* cb_data, const std::string &fontFileName, bool, bool, long)
{
    std::map<std::string, FontFileManager::FontVariants> &fontMap =
        *((std::map<std::string, FontFileManager::FontVariants>*) cb_data);

    std::string name = FileFunctions::Basename(fontFileName);

    if (to_lower(name).find(".ttf") == std::string::npos) return;

    std::string base = font_base(name);

    // key is all lower-case, no spaces, dashes or underscores and no
    // variant string.
    std::string key = to_lower(base);
    key.erase(std::remove_if(key.begin(), key.end(), [](char c) {
        return c==' '||c=='-'||c=='_'; }), key.end());

    FontStyle style = detect_style(name);

    int index = fontMap.size();
    bool hasKey = fontMap.find(key) != fontMap.end();
    FontFileManager::FontVariants &fv = fontMap[key];
    if (!hasKey)
        fv.index = index;
    if (fv.base.empty())
        fv.base = base;
    switch (style) {
        case Regular:    fv.regular = fontFileName; break;
        case Bold:       fv.bold = fontFileName; break;
        case Italic:     fv.italic = fontFileName; break;
        case BoldItalic: fv.boldItalic = fontFileName; break;
    }
}

// *****************************************************************************
// Purpose: Use ReadAndProcessDirectory to scan for all available TrueType
// font (`.ttf`) file names in the resources/font directory.
//
// There are some built-in fonts that should always be available in versions
// of VisIt 3.5.0 and newer. These are the DejaVu fonts (Sans, Serif and
// Monospace). Because most users are not converscent in font-speak, we don't
// use the sans-serif, serif and monospace terminology. Instead, we use
// proxies, Plain, Fancy and Code, respectively. So, there is special
// logic here to give those built-in fonts these pre-determined GUI names.
// Otherwise, the name of the displayed font in the GUI is just whatever the
// name of the file holding the *regular* style is without its `.ttf`
// extension. This manager does all its work based on the `.ttf` file names
// and does not attempt to interpret any metadata in `.ttf` files.
//
// Note that any `.ttf` files found in the `resources/font` directory will
// get included here. The contents of this directory on both client and
// server should be kept consistent.
//
// Mark C. Miller, Mon May 19 11:19:40 PDT 2025
// *****************************************************************************
//
static void scanFontFiles(std::map<std::string, FontFileManager::FontVariants>& fontMap,
    std::map<int, std::string>& indexedFonts)
{
    // gather up font file names
    FileFunctions::ReadAndProcessDirectory(GetVisItResourcesDirectory(VISIT_RESOURCES_FONTS),
        gatherFontFileName, &fontMap, false);

    // Set up _indexedFont map and GUI names for some built in fonts
    for (auto &pair : fontMap)
    {
        FontFileManager::FontVariants &fv = pair.second;

        indexedFonts[fv.index] = pair.first;

        if      (pair.first == "dejavusans")
            fv.guiName = "Plain";
        else if (pair.first == "dejavusansmono")
            fv.guiName = "Code";
        else if (pair.first == "dejavuserif")
            fv.guiName = "Fancy";
        else if (pair.first == "libertinusmath")
            fv.guiName = "Math";
        else 
            fv.guiName = pair.second.base;
    }
}

// *****************************************************************************
// Purpose: Enforce singleton design pattern for font manager
//
// Mark C. Miller, Mon May 19 11:20:17 PDT 2025
// *****************************************************************************
//
FontFileManager& FontFileManager::instance()
{
    static FontFileManager instance;
    return instance;
}

// *****************************************************************************
// Purpose: Maintain mapping from old, 3-font legacy font enums to the font
// manager's map of fonts.
//
// Mark C. Miller, Mon May 19 11:20:17 PDT 2025
// *****************************************************************************
//
const std::string& FontFileManager::legacyFontKey(enum LegacyFontEnums e)
{
    static const std::string arial("dejavusans");
    static const std::string courier("dejavusansmono");
    static const std::string times("dejavuserif");
    static const std::string unknown("unknown");

    switch (e)
    {
        case FontFileManager::Arial:   return arial;
        case FontFileManager::Courier: return courier;
        case FontFileManager::Times:   return times;
        default:                       return unknown;
    }

    return unknown;
}

// *****************************************************************************
// Purpose: Maintain mapping from old, 3-font legacy font enums to the font
// manager's map of fonts.
//
// Mark C. Miller, Mon May 19 11:20:17 PDT 2025
// *****************************************************************************
//
const FontFileManager::LegacyFontEnums FontFileManager::legacyFontEnum(const std::string& fontKey)
{
    if (fontKey == legacyFontKey(FontFileManager::Arial))
        return FontFileManager::Arial;
    if (fontKey == legacyFontKey(FontFileManager::Courier))
        return FontFileManager::Courier;
    if (fontKey == legacyFontKey(FontFileManager::Times))
        return FontFileManager::Times;
    return FontFileManager::Unknown;
}

// *****************************************************************************
// Purpose: Obtain FontFileManager's font map
//
// Mark C. Miller, Mon May 19 11:20:17 PDT 2025
// *****************************************************************************
//
const std::map<std::string, FontFileManager::FontVariants>& FontFileManager::fonts()
{
    if (_fontMap.empty())
        scanFontFiles(_fontMap, _indexedFonts);
    return _fontMap;
}

const std::map<int, std::string>& FontFileManager::fontIndices()
{
    fonts();
    return _indexedFonts;
}
