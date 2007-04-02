/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <stdio.h>
#include <visitstream.h>
#include <visit-config.h>
#include <string>
#include <vector>
#include <time.h>

using std::string;
using std::vector;

std::string CurrentTime()
{
    char *tstr[] = {"PDT", "PST"};
    char s1[10], s2[10], s3[10], tmpbuf[200];
    time_t t;
    char *c = NULL;
    int h,m,s,y;
    t = time(NULL);
    c = asctime(localtime(&t));
    // Read the hour.
    sscanf(c, "%s %s %s %d:%d:%d %d", s1, s2, s3, &h, &m, &s, &y);
    // Reformat the string a little.
    sprintf(tmpbuf, "%s %s %s %02d:%02d:%02d %s %d",
            s1, s2, s3, h, m, s, tstr[h > 12], y);

    return std::string(tmpbuf);
}

// ****************************************************************************
// Class: HTMLPage
//
// Purpose:
//   This class contains the title and body of an HTML page. We use it to
//   cache the content read from the MIF files until we have a complete
//   list of the HTML pages so we can create navigation links on the generated
//   pages.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 23 14:24:20 PST 2002
//
// Modifications:
//   
// ****************************************************************************

class HTMLPage
{
public:
    HTMLPage(const string &t, const string &d, const vector<string> &p,
        int l, bool c) : topic(t), doc(d), page(p)
    {
        level = l;
        chapterDocument = c;
    }

    ~HTMLPage() { };

    string         topic;
    string         doc;
    vector<string> page;
    int            level;
    bool           chapterDocument;
};

// ****************************************************************************
// Class: HelpMLFile
//
// Purpose:
//   Keeps track of entries that are added to the HelpML file which is the
//   file that the help window uses to create the help contents and index.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 23 10:43:29 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

class HelpMLFile
{
public:
    HelpMLFile();
    ~HelpMLFile();

    bool Open(const char *filename);
    void Close();
    bool AddPage(const string &topic, const string &doc,
                 const vector<string> &page, int indentLevel);
    void AddChapter(const string &name);
    void SetAllowAdds(bool val) { allowAdds = val; }

private:
    void WriteEntry(int i, bool slash);
    void CreateListDocument(const string &filename, const string &title,
                            int start, int level);
    void WritePage(int i);
    void StartHTML(FILE *fp, const string &title);
    void EndHTML(FILE *fp);
    void WriteBullets(FILE *fp, const string &title, int start, int level);
    void WriteNavigation(FILE *fp, int i);

    FILE              *helpml;
    bool               allowAdds;
    int                chapterIndex;

    vector<HTMLPage *> help;
};

// ****************************************************************************
// Method: HelpMLFile::HelpMLFile
//
// Purpose: 
//   Constructor for the HelpMLFile class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 23 10:44:50 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

HelpMLFile::HelpMLFile() : help()
{
    helpml = NULL;
    allowAdds = false;
    chapterIndex = 1;
}

// ****************************************************************************
// Method: HelpMLFile::~HelpMLFile
//
// Purpose: 
//   Destructor for the HelpMLFile class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 23 10:44:50 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

HelpMLFile::~HelpMLFile()
{
    Close();

    for(int i = 0; i < help.size(); ++i)
        delete help[i];
}

// ****************************************************************************
// Method: HelpMLFile::Open
//
// Purpose: 
//   Opens the HelpML file so it can be written.
//
// Arguments:
//   filename : The name of the file to write.
//
// Returns:    True if the file was successfully opened.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 23 10:48:08 PDT 2002
//
// Modifications:
//   Brad Whitlock, Fri Jun 18 10:51:09 PDT 2004
//   Increased the helpml version and added the current time to the
//   date created.
//
// ****************************************************************************

bool
HelpMLFile::Open(const char *filename)
{
    helpml = fopen(filename, "wb");
    if(helpml)
    {
        fprintf(helpml, "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
        fprintf(helpml, "<helpml version=\"1.1\">\n");
        fprintf(helpml, "    <head>\n");
        fprintf(helpml, "        <title>VisIt Help Contents</title>\n");
        fprintf(helpml, "        <dateCreated>%s</dateCreated>\n",
            CurrentTime().c_str());
        fprintf(helpml, "        <version>%s</version>\n", VERSION);
        fprintf(helpml, "    </head>\n");
        fprintf(helpml, "    <body>\n");
    }
    return helpml != NULL;
}

// ****************************************************************************
// Method: HelpMLFile::AddTopic
//
// Purpose: 
//   Adds a topic to the contents.
//
// Arguments:
//   topic : A string containing the help topic.
//   doc   : A string containing the HTML filename to associate with the topic.
//   indentLevel : The level at which the topic appears in the contents.
//
// Note:       There is something about the parsing that's not quite right and
//             it leads to a few HTML pages of junk before we get anything
//             valuable. That's why I don't allow ant topics to be added
//             before an "Overview" page is added.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 23 10:49:00 PDT 2002
//
// Modifications:
//   Brad Whitlock, Mon Jun 21 16:43:16 PST 2004
//   Added debugging output (commented out).
//
//   Eric Brugger, Wed Jun 23 09:09:47 PDT 2004
//   Modified the usages of strings to use the c_str method when adding its
//   contents to an iostream.
//
// ****************************************************************************

bool
HelpMLFile::AddPage(const string &topic, const string &doc,
    const vector<string> &page, int indentLevel)
{
    // Don't allow files to be added until we see an Overview page.
    if(topic == "Overview" && indentLevel == 2)
        allowAdds = true;

    if(allowAdds)
    {
//        cout << "Adding HTML page: " << topic.c_str() << endl;
//        cout << "\tdoc=" << doc.c_str() << endl;
//        cout << "\tpage.size()=" << page.size() << endl;
//        cout << "\tindentLevel=" << indentLevel << endl;

        HTMLPage *html = new HTMLPage(topic, doc, page, indentLevel, false);
        help.push_back(html);
    }
    else
    {
//        cerr << "Not allowing addition of HTML page: " << topic.c_str() << endl;
//        cerr << "\tdoc=" << doc.c_str() << endl;
//        cerr << "\tpage.size()=" << page.size() << endl;
//        cerr << "\tindentLevel=" << indentLevel << endl;
    }

    return allowAdds;
}

// ****************************************************************************
// Method: HelpMLFile::AddChapter
//
// Purpose: 
//   This is like AddTopic but it only adds top level chapter headings.
//
// Arguments:
//   name : The name of the chapter to add.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 23 10:52:14 PDT 2002
//
// Modifications:
//   Brad Whitlock, Mon Jun 21 16:43:02 PST 2004
//   Removed chapter hack.
//
// ****************************************************************************

void
HelpMLFile::AddChapter(const string &name)
{
    vector<string> empty;
    char doc[100];
    sprintf(doc, "list%04d.html", chapterIndex++);

    HTMLPage *html = new HTMLPage(name, string(doc), empty, 1, true);
    help.push_back(html);
}

// ****************************************************************************
// Method: HelpMLFile::WriteEntry
//
// Purpose: 
//   Writes a topic to the output file.
//
// Arguments:
//   i     : The index of the topic to write.
//   slash : Whether or not the XML should have a terminating slash.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 23 10:53:05 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
HelpMLFile::WriteEntry(int i, bool slash)
{
    if(slash)
    {
        fprintf(helpml, "<help topic=\"%s\" doc=\"%s\"/>\n", help[i]->topic.c_str(), help[i]->doc.c_str());
    }
    else
    {
        fprintf(helpml, "<help topic=\"%s\" doc=\"%s\">\n", help[i]->topic.c_str(), help[i]->doc.c_str());
    }
}

// ****************************************************************************
// Method: HelpMLFile::CreateListDocument
//
// Purpose: 
//   Writes a list document.
//
// Arguments:
//   filename : The name of the list document.
//   title    : The title of the list document.
//   start    : The index at which to begin searching.
//   level    : The help level.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 23 11:50:56 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
HelpMLFile::CreateListDocument(const string &filename, const string &title,
    int start, int level)
{
    FILE *fp;

    if((fp = fopen(filename.c_str(), "wb")) != NULL)
    {
        StartHTML(fp, title);
        WriteBullets(fp, title, start, level);
        EndHTML(fp);
        fclose(fp);
    }
}

// ****************************************************************************
// Method: HelpMLFile::StartHTML
//
// Purpose: 
//   Writes the header for an HTML page.
//
// Arguments:
//   fp : The file pointer used to write the page.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 23 14:11:25 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Jun 18 10:56:53 PDT 2004
//   Added generation time into an HTML comment.
//
// ****************************************************************************

void
HelpMLFile::StartHTML(FILE *fp, const string &title)
{
    fprintf(fp, "<html>\n");
    fprintf(fp, "<!-- generated by mif2help on %s -->", CurrentTime().c_str());
    fprintf(fp, "<head>\n");
    fprintf(fp, "<title>%s</title>\n", title.c_str());
    fprintf(fp, "</head>\n");
    fprintf(fp, "<body link=\"#0000FF\" vlink=\"#800080\" "
                "alink=\"#FF0000\" bgcolor=\"#FFFFFF\">\n");
}

// ****************************************************************************
// Method: HelpMLFile::EndHTML
//
// Purpose: 
//   Writes the footer for an HTML page.
//
// Arguments:
//   fp : The file pointer used to write the page.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 23 14:11:25 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
HelpMLFile::EndHTML(FILE *fp)
{
    fprintf(fp, "</body>\n");
    fprintf(fp, "</html>\n");
}

// ****************************************************************************
// Method: HelpMLFile::WriteBullets
//
// Purpose: 
//   Writes a list of bullets.
//
// Arguments:
//   fp    : The file pointer used to write the list.
//   title : The title of the list.
//   start : The starting index.
//   level : The tree level.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 23 14:12:24 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
HelpMLFile::WriteBullets(FILE *fp, const string &title, int start, int level)
{
    // Write the navigation links.
    WriteNavigation(fp, start);

    fprintf(fp, "<p><center><h1>%s</h1></center></p>\n", title.c_str());

    // Write a list of bullets.
    fprintf(fp, "<ul>\n");
    int i = start + 1;
    int nextLevel = level + 1;
    while((i < help.size()) && (help[i]->level >= nextLevel))
    {
        if(help[i]->level == nextLevel)
        {
            fprintf(fp, "  <li><a href=\"%s\">%s</a></li>\n",
                    help[i]->doc.c_str(), help[i]->topic.c_str());
        }

        ++i;
    }
    fprintf(fp, "</ul>\n");
}

// ****************************************************************************
// Method: HelpMLFile::WriteNavigation
//
// Purpose: 
//   Writes the navigation links to the HTML page.
//
// Arguments:
//   fp : The file pointer used to write the page.
//   i  : The index of the page to write.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 23 14:20:46 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
HelpMLFile::WriteNavigation(FILE *fp, int i)
{
    fprintf(fp, "<center align=\"right\">");

    if(i > 0)
        fprintf(fp, "[<a href=\"%s\">Back</a>]  ", help[i-1]->doc.c_str());

    if(i >= 0)
    {
        if(help[i]->level == 1)
        {
            fprintf(fp, "[<a href=\"list0000.html\">Up</a>]");
        }
        else
        {
            int j = i;
            while((j > 0) && (help[j]->level >= help[i]->level))
            {
                --j;
            }
            if(j >= 0)
                fprintf(fp, "[<a href=\"%s\">Up</a>]", help[j]->doc.c_str());
        }
    }

    if(i < help.size() - 1)
        fprintf(fp, "  [<a href=\"%s\">Next</a>]", help[i+1]->doc.c_str());
    fprintf(fp, "</center>\n");
}

// ****************************************************************************
// Method: HelpMLFile::WritePage
//
// Purpose: 
//   Writes out the i'th help object as an HTML page.
//
// Arguments:
//   i : The index of the page to write out.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 23 14:13:38 PST 2002
//
// Modifications:
//   Brad Whitlock, Mon Jun 21 16:44:11 PST 2004
//   Changed how page title is added.
//
// ****************************************************************************

void
HelpMLFile::WritePage(int i)
{
    FILE *htmlFile = fopen(help[i]->doc.c_str(), "wb");
    if(htmlFile)
    {
        StartHTML(htmlFile, help[i]->topic);

        // If we have a chapter document then there is no content. Write a list
        // of bullets. If we have a page, write its content.
        if(help[i]->chapterDocument)
        {
            WriteBullets(htmlFile, help[i]->topic, i, help[i]->level);
        }
        else
        {
            // Write the navigation links.
            WriteNavigation(htmlFile, i);

            // Print out the lines of text that we've accumulated.
            const vector<string> &lines = help[i]->page;
            for(int j = 0; j < lines.size(); ++j)
            {
                if(j == 0)
                    fprintf(htmlFile, "<h2>");
                fprintf(htmlFile, "%s", lines[j].c_str());
                if(j == 1)
                    fprintf(htmlFile, "</h2>");
            }
        }

        EndHTML(htmlFile);
        fclose(htmlFile);
    }
}

// ****************************************************************************
// Method: HelpMLFile::Close
//
// Purpose: 
//   Writes the HelpML contents to the file. It also writes out each HTML page.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 23 10:54:13 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
HelpMLFile::Close()
{
    if(helpml)
    {
        int i,j,k, indent = 1;
        int start, end;

        // Create a list doc for the top level.
        CreateListDocument("list0000.html", "Chapters", -1, 0);

        for(i = 0; i < help.size(); ++i)
        {
            // Write the HTML file.
            WritePage(i);

            // Indent the line
            for(j = 0; j < indent*4+4; ++j)
                fprintf(helpml, " ");

            bool backup = false;
            if(i == help.size() - 1)
            {
                // Write the last HelpML topic.
                WriteEntry(i, true);
                backup = true;
                start = help[i]->level;
                end = 1;
            }
            else
            {                    
                // Write the helpml topic. If the next topic has a larger helplevel
                // then it is a subtopic and we should increase the indentation. If 
                // the helplevel of the next
                if(help[i+1]->level > help[i]->level)
                {
                    WriteEntry(i, false);
                    ++indent;
                }
                else if (help[i+1]->level == help[i]->level)
                    WriteEntry(i, true);
                else if (help[i]->level > help[i+1]->level)
                {
                    WriteEntry(i, true);
                    backup = true;
                    start = help[i]->level;
                    end = help[i+1]->level;
                }
            }

            if(backup)
            {
                for(j = start; j > end; --j)
                {
                    for(k = 0; k < j*4; ++k)
                        fprintf(helpml, " ");

                    // Write the helpml topic
                    fprintf(helpml, "</help>\n");

                    --indent;
                    if(indent < 1) indent = 1;
                }
            }
        }

        fprintf(helpml, "    </body>\n");
        fprintf(helpml, "</helpml>\n");
        fclose(helpml);
        helpml = NULL;
    }
}

//////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Class: TableObject
//
// Purpose:
//   This class contains the information for a table so it can be added to the
//   HTML when needed.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 21 16:44:26 PST 2004
//
// Modifications:
//   
// ****************************************************************************

class TableObject
{
public:
     TableObject() : name("unnamed"), content(), columnWidths()
     {
         currentColumn = 0;
         widthsCalculated = false;
     }

     virtual ~TableObject()
     {
     }

     void BeginTable()
     {
         content += "<table width=\"90%\" border=\"1\" align=\"center\" cellpadding=\"5\" cellspacing=\"1\">\n";
     }

     void EndTable()
     {
         content += "</table>\n";
     }

     void AddColumnWidth(const string &inches)
     {
         string num(inches.substr(0, inches.length() - 1));
         float w(atof(num.c_str()));
         rawColumnWidths.push_back(w);
         //cerr << "Adding column width=" << w << endl;
     }

     void CalculateWidths()
     {
         if(!widthsCalculated && rawColumnWidths.size() > 0)
         {
             int i;
             float sum = 0.f;
             for(i = 0; i < rawColumnWidths.size(); ++i)
                 sum += rawColumnWidths[i];
             sum = (sum < 0.) ? 1. : sum;
             for(i = 0; i < rawColumnWidths.size(); ++i)
                 columnWidths.push_back(int(100.f * rawColumnWidths[i] / sum));
             widthsCalculated = true;
         }
     }

     void BeginRow()
     {
         content += "<tr>\n";
     }

     void EndRow()
     {
         content += "</tr>\n";
         currentColumn = 0;
     }

     void BeginCell()
     {
         CalculateWidths();
         if(currentColumn < columnWidths.size())
         {
             char tmp[100];
             sprintf(tmp, "<td width=\"%d%%\">", columnWidths[currentColumn]);
             content += tmp;
         }
         else
             content += "<td>";
     }

     void EndCell()
     {
         content += "</td>\n";
         ++currentColumn;
     }

     void AddCellContent(const string &s)
     {
         content += s;
     }

     string        name;
     string        content;
     bool          widthsCalculated;
     vector<float> rawColumnWidths;
     vector<int>   columnWidths;
     int           currentColumn;
};

//////////////////////////////////////////////////////////////////////////////

#define READ_TAG           0
#define READ_PARA          1
#define READ_STRING        2
#define READ_PGFTAG        3
#define READ_FTAG          4
#define READ_FANGLE        5
#define READ_FPOSITION     6
#define READ_MTYPENAME     7
#define READ_MTEXT         8
#define READ_TABLE         9
#define READ_TABLEROW      10
#define READ_TABLECOL      11
#define READ_TABLECELL     12
#define READ_TABLECOLWIDTH 13
#define READ_TABLEID       14
#define READ_INSERTTABLE   15
#define READ_TEXTFLOW      16
#define READ_IGNORE        17

// ****************************************************************************
// Class: MIF_To_HTML
//
// Purpose:
//   This class converts MIF files into a set of HTML files.
//
// Notes:      This class opens a MIF file and scans through it looking for
//             certain tags that we can convert to HTML. Each time we change
//             paragraph format to different headings, we write a new HTML page.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 23 10:55:04 PDT 2002
//
// Modifications:
//   Brad Whitlock, Wed Sep 4 14:00:41 PST 2002
//   I added superscript.
//
//   Brad Whitlock, Mon Jun 21 16:45:08 PST 2004
//   Added support for tables and subscript.
//
// ****************************************************************************

class MIF_To_HTML
{
public:
    MIF_To_HTML(HelpMLFile *ml);
    ~MIF_To_HTML();
    bool ProcessMIF(char *filename);
private:
    void Scan(FILE *fp);
    void EndTag(int, const string &);
    void BeginHTML(int level);
    void EndHTML();
    void WriteHTMLString(const char *);
    string FilterText(const string &str);

    HelpMLFile     *mlfile;
    int            paragraphLevel;
    string         paragraphFormat;
    string         paragraphString;
    bool           boldText;
    bool           italicText;
    bool           superscriptText;
    bool           subscriptText;
    bool           readingChapterNameFromMarker;
    bool           readingTextFlow;
    bool           readingTableCol;
    bool           htmlFirstString;
    int            htmlIndex;
    int            htmlLevel;
    bool           htmlInProgress;
    string         htmlName;
    string         htmlTitle;
    vector<string> htmlLines;

    vector<TableObject *>  tables;
    TableObject           *currentTable;
    vector<TableObject *>  appendTables;
};

// ****************************************************************************
// Method: MIF_To_HTML::MIF_To_HTML
//
// Purpose: 
//   Constructor for the MIF_To_HTML class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 23 10:57:46 PDT 2002
//
// Modifications:
//   Brad Whitlock, Wed Sep 4 14:00:50 PST 2002
//   I added superscript.
//
//   Brad Whitlock, Mon Jun 21 16:45:30 PST 2004
//   Added tables and subscript.
//
// ****************************************************************************

MIF_To_HTML::MIF_To_HTML(HelpMLFile *ml) : paragraphFormat(""),
    paragraphString(""), htmlName(""), htmlTitle(""), htmlLines(), tables(), appendTables()
{
    mlfile = ml;
    paragraphLevel = 0;
    boldText = false;
    italicText = false;
    superscriptText = false;
    subscriptText = false;
    htmlFirstString = false;
    htmlIndex = 0;
    htmlLevel = 0;
    htmlInProgress = false;

    readingChapterNameFromMarker = false;
    readingTextFlow = false;
    readingTableCol = false;
    htmlLines.reserve(1000);

    currentTable = 0;
}

// ****************************************************************************
// Method: MIF_To_HTML::~MIF_To_HTML
//
// Purpose: 
//   Destructor for the MIF_To_HTML class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 23 10:58:07 PDT 2002
//
// Modifications:
//   Brad Whitlock, Fri Jun 18 16:39:34 PST 2004
//   Added tables.
//
// ****************************************************************************

MIF_To_HTML::~MIF_To_HTML()
{
    for(int i = 0; i < tables.size(); ++i)
        delete tables[i];
}

// ****************************************************************************
// Method: MIF_To_HTML::FilterText
//
// Purpose: 
//   Filters out and replaces certain strings in an input string.
//
// Arguments:
//   str : The input string.
//
// Returns:    A filtered string.
//
// Note:       This is used mostly to replace certain MIF control strings with
//             the strings they represent.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 23 10:58:27 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

string
MIF_To_HTML::FilterText(const string &str)
{
    string retval(str);

    if(retval.size() >= 5)
    {
        std::string::size_type idx;

        // Replace certain strings in the input string.
        while((idx = retval.rfind("\\xd2 ")) != std::string::npos)
        {
            string left(retval.substr(0, idx));
            string right(retval.substr(idx+5, retval.length()-idx-5));
            retval = left + string("\"") + right;
        }
        while((idx = retval.rfind("\\xd3 ")) != std::string::npos)
        {
            string left(retval.substr(0, idx));
            string right(retval.substr(idx+5, retval.length()-idx-5));
            retval = left + string("\"") + right;
        }
        while((idx = retval.rfind("\\xd5 ")) != std::string::npos)
        {
            string left(retval.substr(0, idx));
            string right(retval.substr(idx+5, retval.length()-idx-5));
            retval = left + string("'") + right;
        }
    }

    return retval;
}

// ****************************************************************************
// Method: MIF_To_HTML::EndHTML
//
// Purpose: 
//   Writes the buffered HTML lines as an HTML page.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 23 11:00:02 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
MIF_To_HTML::EndHTML()
{
    if(htmlInProgress)
    {
        if(htmlLines.size() > 0)
        {
            // Add the html file to the helpml index.
            if(!mlfile->AddPage(htmlTitle, htmlName, htmlLines, htmlLevel))
            {
                --htmlIndex;
            }
        }
        else
        {
            // There are no html lines to write so let's re-use the index.
            --htmlIndex;
        }

        htmlInProgress = false;
    }
}

// ****************************************************************************
// Method: MIF_To_HTML::BeginHTML
//
// Purpose: 
//   Resets the HTML variables.
//
// Arguments:
//   level : The paragraph indentation level.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 23 11:00:42 PDT 2002
//
// Modifications:
//   Brad Whitlock, Wed Sep 4 14:01:28 PST 2002
//   I added superscript.
//
//   Brad Whitlock, Mon Jun 21 16:46:06 PST 2004
//   I added tables and subscript.
//
// ****************************************************************************

void
MIF_To_HTML::BeginHTML(int level)
{
    // If we are writing an HTML file, close it before starting a new one.
    EndHTML();

    char filename[100];
    sprintf(filename, "help%04d.html", htmlIndex++);
    htmlName = filename;
    htmlTitle = filename;
    htmlLevel = level;
    htmlInProgress = true;
    boldText = false;
    italicText = false;
    superscriptText = false;
    subscriptText = false;
    htmlFirstString = true;
    readingChapterNameFromMarker = false;
    readingTextFlow = false;
    readingTableCol = false;
    htmlLines.clear();

    currentTable = 0;
    appendTables.clear();
}

// ****************************************************************************
// Method: MIF_To_HTML::WriteHTMLString
//
// Purpose: 
//   Buffers a string into the HTML string buffer if certain conditions are met.
//
// Arguments:
//   str : The string to add.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 23 11:01:45 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
MIF_To_HTML::WriteHTMLString(const char *str)
{
    if(paragraphLevel > 0 && (paragraphFormat != "Figure"))
        htmlLines.push_back(str);
}

// ****************************************************************************
// Method: MIF_To_HTML::EndTag
//
// Purpose: 
//   This method is called by the scanner when the end of a MIF tag is read.
//   Only then do we take action and convert input to HTML.
//
// Arguments:
//   state : The state of the scanner.
//   token : The tokenized string that we're processing.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 23 11:02:35 PDT 2002
//
// Modifications:
//   Brad Whitlock, Wed Sep 4 14:06:48 PST 2002
//   I added support for superscript.
//
//   Brad Whitlock, Mon Jun 21 16:46:23 PST 2004
//   I totally changed how we add to paragraphs and I added support for
//   subscript and tables.
//
//   Eric Brugger, Wed Jun 23 09:09:47 PDT 2004
//   Modified the usages of strings to use the c_str method when adding its
//   contents to an iostream.
//
//   Brad Whitlock, Fri Oct 7 15:37:18 PST 2005
//   Added some debug printing statements.
//
// ****************************************************************************

#ifdef DEBUG_PRINT
static int indent = 0;
#endif

void
MIF_To_HTML::EndTag(int state, const string &token)
{
#ifdef DEBUG_PRINT
    for(int sp=0; sp < indent; ++sp)
        cout << "    ";
    cout << "EndTag: state=" << state << " tok=" << token << endl;
#endif

    if(state == READ_PARA)
    {
        if(paragraphString.length() > 0)
        {
            string str(FilterText(paragraphString));

            if(htmlFirstString)
            {
                // Add the chapter name to the HelpML file.
                htmlTitle = str;
                htmlFirstString = false;
            }

            if(currentTable != 0)
            {
                currentTable->AddCellContent("<p>" + str + "</p>\n");
            }
            else
            {
                htmlLines.push_back("<p>");
                htmlLines.push_back(str.c_str());
                htmlLines.push_back("</p>\n");
                if(appendTables.size() > 0)
                {
                    for(int ti = 0; ti < appendTables.size(); ++ti)
                         htmlLines.push_back(appendTables[ti]->content);
                    appendTables.clear();
                }
            }
        }

        paragraphString = "";
        --paragraphLevel;
    }
    else if(state == READ_STRING)
    {
        if(token.length() >= 2 && !readingTextFlow)
        {
            paragraphString += token.substr(1, token.length() - 2);
        }
    }
    else if(state == READ_PGFTAG)
    {
        string pFormat(token.substr(1, token.length() - 2));
         // If we have a different paragraph format, 
        if(paragraphFormat != pFormat)
        {
            if(pFormat == "1Heading" || pFormat == "1Heading-First")
            {
                BeginHTML(2);
                paragraphFormat = "1Heading";
            }
            else if(pFormat == "2Heading")
            {
                BeginHTML(3);
                paragraphFormat = pFormat;
            }
            else if(pFormat == "3Heading")
            {
                BeginHTML(4);
                paragraphFormat = pFormat;
            }
            else if(pFormat == "Body")
            {
                paragraphFormat = pFormat;
            }
            else if(pFormat == "Figure")
            {
                paragraphFormat = pFormat;
            }
            else if(pFormat == "Chapter")
            {
                paragraphFormat = pFormat;
            }
//                    else
//                    {
//                        cout << "<!-- PGFTAG = \"" << pFormat.c_str() << "\" -->" << endl;
//                    }
        }
    }
    else if(state == READ_FTAG)
    {
        if(token == "`'")
        {
            if(boldText)
            {
                boldText = false;
                paragraphString += "</b>";
            }
            else if(italicText)
            {
                italicText = false;
                paragraphString += "</i>";
            }
            else if(superscriptText)
            {
                superscriptText = false;
                paragraphString += "</sup>";
            }
            else if(subscriptText)
            {
                subscriptText = false;
                paragraphString += "</sub>";
            }
        }
        else if(token == "`GuiText'")
        {
            boldText = true;
            paragraphString += "<b>";
        }
//                else
//                    cout << "<!-- FTAG = \"" << token.c_str() << "\" -->" << endl;
    }
    else if(state == READ_FANGLE)
    {
        if(token == "`Italic'")
        {
            italicText = true;
            paragraphString += "<i>";
        }
    }
    else if(state == READ_FPOSITION)
    {
        if(token == "FSuperscript")
        {
            superscriptText = true;
            paragraphString += "<sup>";
        }
        else if(token == "FSubscript")
        {
            subscriptText = true;
            paragraphString += "<sub>";
        }
    }
    else if(state == READ_MTYPENAME)
    {
        readingChapterNameFromMarker = (token == "`Header/Footer $1'");
    }
    else if(state == READ_MTEXT)
    {
        if(readingChapterNameFromMarker && token.length() > 2)
        {
            readingChapterNameFromMarker = false;
            string str(FilterText(token.substr(1,token.length()-2)));
            mlfile->AddChapter(str);
        }
    }
    else if(state == READ_TABLE)
    {
         if(currentTable == 0)
         {
#ifdef DEBUG_PRINT
             cout << "*** Ending a table but its pointer is zero!" << endl;
#endif
         }
         else
         {
#ifdef DEBUG_PRINT
             cout << "*** Ending a table!" << endl;
#endif
             currentTable->EndTable();
             tables.push_back(currentTable);
             currentTable = 0;
         }
    }
    else if(state == READ_TABLEROW && currentTable != 0)
    {
         currentTable->EndRow();
    }
    else if(state == READ_TABLECOL)
    {
         readingTableCol = false;
    }
    else if(state == READ_TABLECELL && currentTable != 0)
    {
         currentTable->EndCell();
    }
    else if(state == READ_TABLEID && currentTable != 0)
    {
        currentTable->name = token;
    }
    else if(state == READ_TABLECOLWIDTH && currentTable != 0 &&
            !readingTableCol)
    {
        currentTable->AddColumnWidth(token);
    }
    else if(state == READ_INSERTTABLE)
    {
        int index = -1;
        for(int i = 0; i < tables.size(); ++i)
        {
            if(tables[i]->name == token)
            {
                index = i;
                break;
            }
        }

        if(index != -1)
        {
            // Add the contents of the table to the file.
            //cout << "Adding table " << tables[index]->name.c_str() << endl;
            appendTables.push_back(tables[index]);
        }
        else
            cerr << "Could not find table id=" << token.c_str() << endl;
    }
    else if(state == READ_TEXTFLOW)
    {
         readingTextFlow = false;
    }
}

// ****************************************************************************
// Method: MIF_To_HTML::Scan
//
// Purpose: 
//   This method scans through the MIF file identifying tags. As tags are
//   read, we create small web pages that typically contain 1-2 paragraphs
//   of help text.
//
// Arguments:
//   fp : The mif file pointer.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 11 12:42:42 PDT 2002
//
// Modifications:
//   Brad Whitlock, Fri Aug 30 14:26:17 PST 2002
//   I changed the token coding to use std::string again since the new
//   FrameMaker MIF format has some really large tags that are too large
//   to fit in the token buffer. This prevents zillions of error messages
//   from being printed to the console. I also added supprt for superscript.
//
//   Brad Whitlock, Mon Jun 21 16:47:24 PST 2004
//   I added support for subscript and tables.
//
//   Brad Whitlock, Fri Oct 7 15:36:06 PST 2005
//   I added code to get around slight changes in the MIF formatting due to
//   using a new version of FrameMaker.
//
// ****************************************************************************

void
MIF_To_HTML::Scan(FILE *fp)
{
    string token;
    int    state = READ_TAG;
    bool   comment = false;
    bool   escaped = false;

#define EMPTY_TOKEN()    token = ""
#define APPEND_TOKEN(c)  token += c
#define COMPARE_TOKEN(s) (token == s)

    EMPTY_TOKEN();

#ifdef DEBUG_PRINT
    for(int sp=0; sp < indent; ++sp)
        cout << "    ";
    cout << "Scan" << endl;
#endif

    while(!feof(fp))
    {
        char c = fgetc(fp);
        
        if(c == '<')
        {
            if(state == READ_STRING)
            {
                APPEND_TOKEN("&lt;");
            }
            else if(state == READ_IGNORE)
            {
                APPEND_TOKEN(c);
            }
            else
            {
#ifdef DEBUG_PRINT
                ++indent;
#endif
                Scan(fp);
            }
        }
        else if(escaped)
        {
            if(c == '>')
            {
                APPEND_TOKEN("&gt;");
            }
            else
            {
                APPEND_TOKEN('\\');
                APPEND_TOKEN(c);
            }
            escaped = false;
        }
        else if(c == '\\')
        {
            escaped = true;
        }
        else if(c == '>')
        {
            EndTag(state, token);
#ifdef DEBUG_PRINT
            --indent;
#endif
            return;
        }
        else if(c == '#')
        {
            comment = true;
            EMPTY_TOKEN();
        }
        else if(comment)
        {
            if(c == '\n')
                comment = false;
        }
        else if(c != ' ' && c != '\n')
        {
            APPEND_TOKEN(c);
        }
        else if(state == READ_STRING || state == READ_MTEXT ||
                state == READ_MTYPENAME)
        {
            APPEND_TOKEN(c);
        }
        else if(COMPARE_TOKEN("Para"))
        {
            ++paragraphLevel;
            state = READ_PARA;
            EMPTY_TOKEN();
            paragraphString = "";
        } 
        else if(COMPARE_TOKEN("String"))
        {
            state = READ_STRING;
            EMPTY_TOKEN();
        } 
        else if(COMPARE_TOKEN("PgfTag"))
        {
            state = READ_PGFTAG;
            EMPTY_TOKEN();
        } 
        else if(COMPARE_TOKEN("FTag"))
        {
            state = READ_FTAG;
            EMPTY_TOKEN();
        } 
        else if(COMPARE_TOKEN("FAngle"))
        {
            state = READ_FANGLE;
            EMPTY_TOKEN();
        }
        else if(COMPARE_TOKEN("FPosition"))
        {
            state = READ_FPOSITION;
            EMPTY_TOKEN();
        }
        else if(COMPARE_TOKEN("MTypeName"))
        {
            state = READ_MTYPENAME;
            EMPTY_TOKEN();
        }
        else if(COMPARE_TOKEN("MText"))
        {
            state = READ_MTEXT;
            EMPTY_TOKEN();
        }        
        else if(COMPARE_TOKEN("Tbl"))
        {
            state = READ_TABLE;
            EMPTY_TOKEN();

            // Hack. 
            if(currentTable != 0)
            {
#ifdef DEBUG_PRINT
                cout << "*** Ending an existing table before creating a new one." << endl;
#endif
                currentTable->EndTable();
                tables.push_back(currentTable);
            }

            currentTable = new TableObject;
            currentTable->BeginTable();

#ifdef DEBUG_PRINT
            cout << "*** Creating a new table" << endl;
#endif
        }
        else if(COMPARE_TOKEN("Row") && currentTable != 0)
        {
            //cout << "Starting a new table row" << endl;
            state = READ_TABLEROW;
            EMPTY_TOKEN();
            currentTable->BeginRow();
        }
        else if(COMPARE_TOKEN("CellContent") && currentTable != 0)
        {
            state = READ_TABLECELL;
            EMPTY_TOKEN();
            currentTable->BeginCell();
        }
        else if(COMPARE_TOKEN("TblID"))
        {
            state = READ_TABLEID;
            EMPTY_TOKEN();
        }
        else if(COMPARE_TOKEN("ATbl"))
        {
            state = READ_INSERTTABLE;
            EMPTY_TOKEN();
        }
        else if(COMPARE_TOKEN("TblColumn"))
        {
            readingTableCol = true;
            state = READ_TABLECOL;
            EMPTY_TOKEN();
        }
        else if(COMPARE_TOKEN("TblColumnWidth"))
        {
            state = READ_TABLECOLWIDTH;
            EMPTY_TOKEN();
        }
        else if(COMPARE_TOKEN("TextFlow"))
        {
            state = READ_TEXTFLOW;
            readingTextFlow = true;
            EMPTY_TOKEN();
        }
        else if(COMPARE_TOKEN("VariableDef") ||
                COMPARE_TOKEN("XRefDef") ||
                COMPARE_TOKEN("ImportObFileDI"))
        { 
            state = READ_IGNORE;
            escaped = true;
        }
    }
}

// ****************************************************************************
// Method: MIF_To_HTML::ProcessMIF
//
// Purpose: 
//   Processes a MIF file and converts it into a set of HTML files.
//
// Arguments:
//   filename : The name of the MIF file to process.
//
// Returns:    True if it was a success.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 23 11:04:59 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

bool
MIF_To_HTML::ProcessMIF(char *filename)
{
    bool success = false;
    FILE *fp;

    if((fp = fopen(filename, "rb")) != NULL)
    {
        cerr << "Processing: " << filename << endl;

        // Reinitialize some scanner variables.
        paragraphLevel = 0;
        paragraphFormat = "";
        mlfile->SetAllowAdds(false);

        Scan(fp);

        // End the html that is in progress.
        EndHTML();

        fclose(fp);
        success = true;
    }
 
    return success;   
}

// ****************************************************************************
// Function:  main
//
// Purpose:
//   The main function for the application.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 11 12:40:33 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

int
main(int argc, char *argv[])
{
    if(argc < 2)
    {
        cerr << "Usage: " << argv[0] << " mif_files ..." << endl;
        return 1;
    }

    HelpMLFile helpML;

    if(helpML.Open("visit.helpml"))
    {
        MIF_To_HTML mif2html(&helpML);

        // Process each mif file.
        for(int i = 1; i < argc; ++i)
        {
            if(!mif2html.ProcessMIF(argv[i]))
                cerr << "ERROR: Cannot open " << argv[i] << endl;
        }

        cerr << "Writing visit.helpml" << endl;
        helpML.Close();
    }
    else
        cerr << "ERROR: Cannot open visit.helpml for write." << endl;

    return 0;
}
