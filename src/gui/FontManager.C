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

FontManager& FontManager::instance() {
    static FontManager instance;
    return instance;
}

const QMap<QString, FontManager::QtFontInfo>& FontManager::fonts() const
{
    return _fontMap;
}

void FontManager::setupFonts()
{
    if (!_fontMap.empty())
        return;

    const std::map<std::string, FontFileManager::FontVariants>& fontFilesMap = 
        FontFileManager::instance().fonts();

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
        // to further introspect .ttf file names.
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
            _fontMap[QString::fromUtf8(pair.first)] = {
                QFont(fontFamily),
                fontFamily,
                QString::fromUtf8(pair.second.guiName),
                !fv.regular.empty(),
                !fv.bold.empty(),
                !fv.italic.empty(),
                !fv.boldItalic.empty()};
        }
    }
}

void FontManager::setupItemModel(QStandardItemModel *model)
{
    // font keys for fonts VisIt will always have as part of its installation
    static const QList<QString> standardFontKeys = {
        "dejavusans", "dejavuserif", "dejavusansmono", "libertinusmath" };

    //
    // Always put the standard fonts at the front of the list in the menus
    //
    for (auto &key : standardFontKeys) {
        auto it = _fontMap.find(key);
        if (it == _fontMap.end()) continue;
        QStandardItem *item = new QStandardItem(it.value().guiName);
        item->setFont(it.value().font);
        model->appendRow(item);
    }

    //
    // Handle whatever other font files we may have found
    //
    for (auto it = _fontMap.constBegin(); it != _fontMap.constEnd(); ++it) {
        if (standardFontKeys.contains(it.key()))
            continue;
        QStandardItem *item = new QStandardItem(it.value().guiName);
        item->setFont(it.value().font);
        model->appendRow(item);
    }
}

const FontManager::QtFontInfo& FontManager::fontInfo(QString family) const
{
    static const FontManager::QtFontInfo bad =
        {QFont("family-not-found"), "family-not-found", 0,0,0,0};

    // Try standard lookup...can fail if Qt's notion of the font's family name
    // (which it can obtain from metadata in the .ttf file) doesn't, after massaging,
    // match key we generate from .ttf file name.
    QString key = family.toLower();
    key.remove(QRegularExpression("[\\s\\-_]"));
    auto it = _fontMap.find(key);
    if (it != _fontMap.end())
        return it.value();

    // Otherwise, brute force search
    for (auto it = _fontMap.constBegin(); it != _fontMap.constEnd(); ++it) {
        if (it.value().family == family)
            return it.value();
    }

    return bad;
}
