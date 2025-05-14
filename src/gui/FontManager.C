// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <FontManager.h>

#include <FontFileManager.h>

#include <QFont>
#include <QFontDatabase>
#include <QRegularExpression>
#include <QStandardItemModel>
#include <QString>

#warning FIXME
#include <iostream>

FontManager& FontManager::instance() {
    static FontManager instance;
    return instance;
}

const QMap<QString, FontManager::QtFontInfo>& FontManager::fonts() const
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

        //
        // Generally, even if each style of a font is in its own .ttf file, Qt has the ability
        // to either read metadata from those files or somehow figure out that the other .ttf
        // files in a group of related files are all for the same basic font. So, typically,
        // only the font id for the "regular" style's id needs to be used to acquire the 
        // "family" name and then things like setBold and setItalic just work without having
        // to further manipulate .ttf file names.
        //
        if (!fv.regular.empty())
        {
            int fid = QFontDatabase::addApplicationFont(QString::fromUtf8(fv.regular));
            if (fid != badFontId) fontId = fid;
        }
        if (!fv.bold.empty())
        {
            int fid = QFontDatabase::addApplicationFont(QString::fromUtf8(fv.bold));
            if (fid != badFontId) fontId = fid;
        }
        if (!fv.italic.empty())
        {
            int fid = QFontDatabase::addApplicationFont(QString::fromUtf8(fv.italic));
            if (fid != badFontId) fontId = fid;
        }
        if (!fv.boldItalic.empty())
        {
            int fid = QFontDatabase::addApplicationFont(QString::fromUtf8(fv.boldItalic));
            if (fid != badFontId) fontId = fid;
        }

        if (fontId != badFontId)
        {
            QString fontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
std::cerr << "key = \"" << pair.first << "\", family = \"" << fontFamily.toStdString() << "\"" << std::endl;
            _fontMap[QString::fromUtf8(pair.first)] = {
                QFont(fontFamily),
                fontFamily,
                !fv.regular.empty(),
                !fv.bold.empty(),
                !fv.italic.empty(),
                !fv.boldItalic.empty()};
        }
    }
}

void FontManager::setupItemModel(QStandardItemModel *model,
    const std::map<std::string, FontFileManager::FontVariants>& fontFilesMap)
{
    // font keys for fonts VisIt will always have as part of its installation
    static const QList<QString> standardFontKeys = {
        "dejavusans", "dejavuserif", "dejavusansmono", "libertinusmath" };

    //
    // Always put the standard fonts at the front of the list in the menus
    //
    for (auto &key : standardFontKeys) {
        auto it = fontFilesMap.find(key.toStdString());
        if (it == fontFilesMap.end()) continue;
        const FontFileManager::FontVariants& fv = it->second;
        QStandardItem *item = new QStandardItem(QString::fromUtf8(fv.guiName));
        item->setFont(_fontMap[key].font);
        model->appendRow(item);
    }

    //
    // Handle whatever other font files we may have found
    //
    for (auto &pair : fontFilesMap) {
        QString qfirst = QString::fromUtf8(pair.first);
        if (_fontMap.find(qfirst) == _fontMap.end())
            continue;
        if (standardFontKeys.contains(qfirst))
            continue;
        const FontFileManager::FontVariants& fv = pair.second;
        QStandardItem *item = new QStandardItem(QString::fromUtf8(fv.guiName));
        item->setFont(_fontMap[qfirst].font);
        model->appendRow(item);
    }
}

const FontManager::QtFontInfo& FontManager::fontInfo(QString family) const
{
    static const FontManager::QtFontInfo bad =
        {QFont("family-not-found"), "family-not-found", 0,0,0,0};

    // First, try standard lookup
    QString key = family.toLower();
    key.remove(QRegularExpression("[\\s\\-_]"));
    auto it = _fontMap.find(key);
    if (it != _fontMap.end())
        return it.value();

    std::cerr << "Hitting bad case with family = \"" << family.toStdString() << "\"\n";
    // Otherwise, brute force search
    return bad;
}
