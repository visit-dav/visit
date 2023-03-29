// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QtGui>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>

#include "QvisPythonSyntaxHighlighter.h"

// ***************************************************************************
// Method: QvisPythonSyntaxHighlighter::QvisPythonSyntaxHighlighter
//
// Purpose:
//   Constructor
//
// Programmer: Cyrus Harrison
// Creation:   Mon Feb  8 14:57:29 PST 2010
//
// Modifications:
//   Kathleen Biagas, Wed Mar 29 08:10:38 PDT 2023
//   Replaced QRegExp (which has been deprecated) with QRegularExpression.
//
// ***************************************************************************
QvisPythonSyntaxHighlighter::QvisPythonSyntaxHighlighter
(QTextDocument *parent)
: QSyntaxHighlighter(parent)
{
    setupRules();

    HighlightingRule rule;
    foreach(const QString &name,order)
    {
        rule.format = formats[name];
        foreach(const QString &pat,patterns[name])
        {
            rule.pattern = QRegularExpression(pat);
            rules.append(rule);
        }
    }
}

// ***************************************************************************
// Method: QvisPythonSyntaxHighlighter::QvisPythonSyntaxHighlighter
//
// Purpose:
//   Sets up python syntax highlighting rules.
//
// Programmer: Cyrus Harrison
// Creation:   Mon Feb  8 14:57:29 PST 2010
//
// Modifications:
//   Kathleen Biagas, Wed Mar 29 08:10:38 PDT 2023
//   Replaced QRegExp (which has been deprecated) with QRegularExpression.
//
// ***************************************************************************

void
QvisPythonSyntaxHighlighter::setupRules()
{
    /*
    full/partial support:
     keywords / operators = (0,0,255) + bold (pure blue)
     import related = (12,69,13) + bold (dark green)
     'self' = (49,132,149)
     strings =  (3,106,7)  (green)
     language const = (88,92,246) + bold (light blue)
     comments = (2,101,255) + italic  (light blue)

    future support ?:
     numbers = (0,0,205) (light blue)
     format strings = (107,6,11) + bold (dark red)
     method names = (0,0,162) + bold (dark blue)
     class / method args  =  + italic
     (multi line comments)
    */

    formats["keyword"].setForeground(QColor(0,0,255));
    formats["keyword"].setFontWeight(QFont::Bold);
    patterns["keyword"] << "\\band\\b"     << "\\bdel\\b"    << "\\bnot\\b"
                    << "\\bwhile\\b"   << "\\belif\\b"   << "\\bglobal\\b" 
                    << "\\bor\\b"      << "\\bwith\\b"   << "\\bassert\\b" 
                    << "\\belse\\b"    << "\\bif\\b"      << "\\bpass\\b"   
                    << "\\byield\\b"   << "\\bbreak\\b"   << "\\bexcept\\b"
                    << "\\bprint\\b"   << "\\bclass\\b"  << "\\bexec\\b"
                    << "\\bin\\b"      << "\\braise\\b"  << "\\bcontinue\\b"
                    << "\\bfinally\\b" << "\\bis\\b"     << "\\breturn\\b"
                    << "\\bdef\\b"     << "\\bfor\\b"    << "\\blambda\\b"
                    << "\\btry\\b";

    formats["import"].setForeground(QColor(12,69,13));
    formats["import"].setFontWeight(QFont::Bold);
    patterns["import"]  << "\\bimport\\b" << "\\bfrom\\b" << "\\bas\\b";

    formats["self"].setForeground(QColor(49,132,149));
    patterns["self"]    << "\\bself\\b";

    formats["string"].setForeground(QColor(3,106,7));
    patterns["string"]  << "\"[^\"\\\\]*(\\\\.[^\"\\\\]*)*\"" 
                        << "'[^'\\\\]*(\\\\.[^'\\\\]*)*'";
    

    formats["const"].setForeground(QColor(0,0,255));
    formats["const"].setFontWeight(QFont::Bold);
    patterns["const"]   << "\\bTrue\\b" << "\\bFalse\\b" << "\\bNone\\b";

    formats["comment"].setForeground(QColor(2,101,255));
    formats["comment"].setFontItalic(true);
    patterns["comment"] << "#[^\n]*";

    order << "keyword" << "import" << "self" << "string"
           << "const" << "comment";
    
    tripleSingleQuote = QRegularExpression("'''");       
    tripleDoubleQuote = QRegularExpression("\"\"\"");
}

// ***************************************************************************
// Method: QvisPythonSyntaxHighlighter::highlightBlock
//
// Purpose:
//   Called when text document changes to check & apply highlighting rules.
//
// Programmer: Cyrus Harrison
// Creation:   Mon Feb  8 14:57:29 PST 2010
//
// Modifications:
//   Kathleen Biagas, Wed Mar 29 08:10:38 PDT 2023
//   Replaced QRegExp (which has been deprecated) with QRegularExpression.
//
// ***************************************************************************

void
QvisPythonSyntaxHighlighter::highlightBlock(const QString &text)
{
    foreach(const HighlightingRule &rule, rules)
    {
        QRegularExpression regex(rule.pattern.pattern());
        QRegularExpressionMatchIterator iter = regex.globalMatch(text);
        while (iter.hasNext())
        {
            QRegularExpressionMatch match = iter.next();
            int length = match.capturedLength();
            int index  = match.capturedStart();
            setFormat(index, length, rule.format);
        }
    }
    
    if(previousBlockState() ==-1 || previousBlockState() ==0)
    {
        if(checkOpen(tripleSingleQuote,text,false))
            setCurrentBlockState(1);
        else
        {
            if(checkOpen(tripleDoubleQuote,text,false))
                setCurrentBlockState(2);
            else
                setCurrentBlockState(0);
        }
    }
    else if(previousBlockState() == 1)
    {
        if(!checkOpen(tripleSingleQuote,text,true))
            setCurrentBlockState(0);
        else
            setCurrentBlockState(1);
    }
    else if(previousBlockState() == 2)
    {
        if(!checkOpen(tripleDoubleQuote,text,true))
            setCurrentBlockState(0);
        else
            setCurrentBlockState(2);
    }    
    
}

// ***************************************************************************
// Method: QvisPythonSyntaxHighlighter::checkOpen
//
// Purpose:
//   Helper for handling multiline comments.
//
// Programmer: Cyrus Harrison
// Creation:   Mon Feb  8 14:57:29 PST 2010
//
// Modifications:
//   Kathleen Biagas, Wed Mar 29 08:10:38 PDT 2023
//   Replaced QRegExp (which has been deprecated) with QRegularExpression.
//
// ***************************************************************************

bool 
QvisPythonSyntaxHighlighter::checkOpen(const QRegularExpression &regx,
                                       const QString &text,
                                       bool open)
{
    int len =  0;
    int start_idx = 0;
    QRegularExpressionMatchIterator iter = regx.globalMatch(text);
    while (iter.hasNext())
    {
        QRegularExpressionMatch match = iter.next();
        len = match.capturedLength();
        int index = match.capturedStart();
        if(!open)
        {
            start_idx = index;
            open = true;
        }
        else
        {
            int full_len = (index + len) - start_idx;
            setFormat(start_idx, full_len, formats["comment"]);
            open = false;
            start_idx = 0;
        }
    }
    
    // if the entire line is open, apply fmt
    if(open)
        setFormat(start_idx, text.size() -start_idx, formats["comment"]);

    return open;
}
