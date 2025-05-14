// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef FONT_MANAGER_H
#define FONT_MANAGER_H

// ****************************************************************************
// Class: FontManager
//
// Purpose:
//   This class manages Qt fonts associated with .ttf files from FontFileManager.
//
//   Here in the GUI, the *only* reason to manage font files and *register*
//   fonts with Qt is so that the GUI can display what the font actually looks
//   like in the pull-down selector for the font as a sort of preview of what 
//   the font actually looks like. In particular, the font manager functions
//   implimented here have NOTHING TO DO with the font the GUI uses for its
//   widgets.
//
//   When a group of related font files is registered with Qt, it apparently
//   has some smarts to know how to switch between the variants for things like
//   setBold() or setItalic(). In the viewer, with VTK, we wind up having to
//   manage all that manually.
//
//   There are some built-in fonts that should always be available in versions
//   of VisIt 3.5.0 and newer. These are the DejaVu fonts (Sans, Serif and
//   Monospace). Because most users are not converscent in font-speak, we don't
//   use the sans-serif, serif and monospace terminology. Instead, we use
//   proxies, Arial, Times and Courier, the most widely recognized fonts that
//   are sans-serif, serif and monospace, respectively. So, there is special
//   logic here to give those built-in fonts these pre-determined GUI names.
//   Otherwise, the name of the displayed font in the GUI is just whatever the
//   name of the file holding the *regular* style is without its `.ttf`
//   extension.
//
// Mark C. Miller, Fri May  9 15:15:06 PDT 2025
//
// ****************************************************************************

#include <FontFileManager.h>

#include <QFont>
#include <QMap>
#include <QStandardItemModel>
#include <QString>

class FontManager {
  public:

    struct QtFontInfo {
        QFont font;
        QString family;
        bool hasReg;
        bool hasBold;
        bool hasItalic;
        bool hasBoldItalic;
    };

    static const std::string sampleText() {
        return "Il1 O0 5S 2Z B8 G6 qg 9g 1l|!iIjJ"; }
    static FontManager& instance();
    void setupFonts(const std::map<std::string, FontFileManager::FontVariants>& fontFilesMap);
    void setupItemModel(QStandardItemModel *model,
             const std::map<std::string, FontFileManager::FontVariants>& fontFilesMap);
    const QMap<QString, QtFontInfo>& fonts() const;
    const QtFontInfo& fontInfo(QString family) const;

  private:
    FontManager() = default;
    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;

    QMap<QString, QtFontInfo> _fontMap;
};

#endif // FONTMANAGER_H
