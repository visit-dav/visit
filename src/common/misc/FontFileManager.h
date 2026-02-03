// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef FONT_FILE_MANAGER_H
#define FONT_FILE_MANAGER_H

#include <map>
#include <string>

// ****************************************************************************
//   
// ****************************************************************************
                              
class FontFileManager {

  public:

    static FontFileManager& instance();

    enum LegacyFontEnums {
       Arial,
       Courier,
       Times,
       Unknown
    };

    static const std::string& legacyFontKey(enum FontFileManager::LegacyFontEnums);
    static const FontFileManager::LegacyFontEnums legacyFontEnum(const std::string&);

    struct FontVariants {
        std::string guiName;
        std::string base;
        std::string regular;
        std::string bold;
        std::string italic;
        std::string boldItalic;
        int index;
    };      

    // These will construct map if not already present so cannot be const
    const std::map<std::string, FontFileManager::FontVariants>& fonts();
    const std::map<int, std::string>& fontIndices();

  private:

    FontFileManager() = default;
    FontFileManager(const FontFileManager&) = delete;
    FontFileManager& operator=(const FontFileManager&) = delete;

    std::map<std::string, FontVariants> _fontMap;
    std::map<int, std::string> _indexedFonts;

};

#endif
