// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <FontManager.h>

#include <FontFileManager.h>

#include <QFontDatabase>
#include <QString>
#include <QFont>

FontManager& FontManager::instance() {
    static FontManager instance;
    return instance;
}

const QMap<QString, QFont>& FontManager::fonts() const
{
    return _fontMap;
}

void FontManager::setupFonts(const std::map<std::string, FontFileManager::FontVariants>& fontFilesMap)
{
    if (!_fontMap.empty())
        return;

    for (auto &pair : fontFilesMap)
    {
        const FontFileManager::FontVariants &fv = pair.second;
        int const badFontId = QFontDatabase::addApplicationFont("/failed-font-file");
        int fontId = badFontId;
        if (!fv.regular.empty())
            fontId = QFontDatabase::addApplicationFont(QString::fromUtf8(fv.regular));
        if (fontId == badFontId && !fv.bold.empty())
            fontId = QFontDatabase::addApplicationFont(QString::fromUtf8(fv.bold));
        if (fontId == badFontId && !fv.italic.empty())
            fontId = QFontDatabase::addApplicationFont(QString::fromUtf8(fv.italic));
        if (fontId == badFontId && !fv.boldItalic.empty())
            fontId = QFontDatabase::addApplicationFont(QString::fromUtf8(fv.boldItalic));
        if (fontId != badFontId)
        {
            QString fontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
            _fontMap[QString::fromUtf8(pair.first)] = QFont(fontFamily);
        }
    }
}
