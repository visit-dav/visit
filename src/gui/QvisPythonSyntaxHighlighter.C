/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/
#include <QtGui>

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
            rule.pattern = QRegExp(pat);
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
    
    tripleSingleQuote = QRegExp("'''");       
    tripleDoubleQuote = QRegExp("\"\"\"");
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
//
// ***************************************************************************

void
QvisPythonSyntaxHighlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, rules)
    {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0)
        {
            int lenth = expression.matchedLength();
            setFormat(index, lenth, rule.format);
            index = expression.indexIn(text, index + lenth);
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
//
// ***************************************************************************

bool 
QvisPythonSyntaxHighlighter::checkOpen(const QRegExp &regx,
                                       const QString &text,
                                       bool open)
{
    int len =  0;
    int start_idx = 0;

    int index = regx.indexIn(text);
    while (index >= 0)
    {
        len = regx.matchedLength();
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

        index = regx.indexIn(text, index + len);
    }
    
    // if the entire line is open, apply fmt
    if(open)
        setFormat(start_idx, text.size() -start_idx, formats["comment"]);

    return open;
}
