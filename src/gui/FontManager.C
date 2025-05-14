// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <FontManager.h>

#include <FontFileManager.h>

#include <QFont>
#include <QFontDatabase>
#include <QStandardItemModel>
#include <QString>

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

void FontManager::setupItemModel(QStandardItemModel *model,
    const std::map<std::string, FontFileManager::FontVariants>& fontFilesMap)
{
    // font keys for fonts VisIt will always have as part of its installation
    static const QList<QString> standardFontKeys = {
        "dejavusans", "dejavuserif", "dejavusansmono", "libertinusmath" };

    for (auto &key : standardFontKeys) {
        auto it = fontFilesMap.find(key.toStdString());
        if (it == fontFilesMap.end()) continue;
        const FontFileManager::FontVariants& fv = it->second;
        QStandardItem *item = new QStandardItem(QString::fromUtf8(fv.guiName));
        item->setFont(_fontMap[key]);
        model->appendRow(item);
    }
    for (auto &pair : fontFilesMap) {
        QString qfirst = QString::fromUtf8(pair.first);
        if (_fontMap.find(qfirst) == _fontMap.end())
            continue;
        if (standardFontKeys.contains(qfirst))
            continue;
        const FontFileManager::FontVariants& fv = pair.second;
        QStandardItem *item = new QStandardItem(QString::fromUtf8(fv.guiName));
        item->setFont(_fontMap[qfirst]);
        model->appendRow(item);
    }
}
