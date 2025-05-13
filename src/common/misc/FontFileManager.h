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

    struct FontVariants {
        std::string guiName;
        std::string base;
        std::string regular;
        std::string bold;
        std::string italic;
        std::string boldItalic;
    };      

    const std::map<std::string, FontFileManager::FontVariants>& fonts();

  private:

    FontFileManager() = default;
    FontFileManager(const FontFileManager&) = delete;
    FontFileManager& operator=(const FontFileManager&) = delete;

    std::map<std::string, FontVariants> _fontMap;

};

#endif
