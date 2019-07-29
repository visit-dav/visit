// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISPYTHONHIGHLIGHTER_H
#define QVISPYTHONHIGHLIGHTER_H

#include <QSyntaxHighlighter>

#include <QHash>
#include <QTextCharFormat>

class QTextDocument;

class QvisPythonSyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    QvisPythonSyntaxHighlighter(QTextDocument *parent = 0);

protected:
    void highlightBlock(const QString &text);

private:
    void setupRules();
    bool checkOpen(const QRegExp &regx,const QString &text,bool open);
    bool checkClosed(const QRegExp &regx,const QString &text);

    struct HighlightingRule
    {
        QRegExp         pattern;
        QTextCharFormat format;
    };

    QVector<HighlightingRule> rules;

    QMap<QString,QTextCharFormat> formats;
    QMap<QString,QStringList>     patterns;
    QStringList                   order;
    
    QRegExp                       tripleSingleQuote;
    QRegExp                       tripleDoubleQuote;

};

#endif
