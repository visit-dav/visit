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

static void gatherFontFileName(void* cb_data, const std::string &fontFileName, bool, bool, long)
{
    std::map<std::string, FontFileManager::FontVariants> &fontMap = *((std::map<std::string, FontFileManager::FontVariants>*) cb_data);

    std::string name = FileFunctions::Basename(fontFileName);

    if (to_lower(name).find(".ttf") == std::string::npos) return;

    std::string base = font_base(name);
    std::string key = to_lower(base);
    FontStyle style = detect_style(name);

    FontFileManager::FontVariants &fv = fontMap[key];
    if (fv.base.empty())
        fv.base = base;
    switch (style) {
        case Regular:    fv.regular = fontFileName; break;
        case Bold:       fv.bold = fontFileName; break;
        case Italic:     fv.italic = fontFileName; break;
        case BoldItalic: fv.boldItalic = fontFileName; break;
    }
}

static void scanFontFiles(std::map<std::string, FontFileManager::FontVariants>& fontMap)
{
    // gather up font file names
    FileFunctions::ReadAndProcessDirectory(GetVisItResourcesDirectory(VISIT_RESOURCES_FONTS),
        gatherFontFileName, &fontMap, false);

    // Set up GUI names for the fonts
    for (auto &pair : fontMap)
    {
        FontFileManager::FontVariants &fv = pair.second;

        if (pair.first == "dejavusans")
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

FontFileManager& FontFileManager::instance()
{
    static FontFileManager instance;
    return instance;
}

const std::map<std::string, FontFileManager::FontVariants>& FontFileManager::fonts()
{
    if (_fontMap.empty())
        scanFontFiles(_fontMap);
    return _fontMap;
}
