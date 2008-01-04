/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#include <QualifiedFilename.h>
#include <visit-config.h>
#include <Utility.h>
#include <algorithm>

// ****************************************************************************
// Method: QualifiedFilename::QualifiedFilename
//
// Purpose: 
//   Constructors for the QualifiedFilename class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 12 14:37:32 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Aug 26 17:05:06 PST 2002
//   I added the separator character.
//
//   Brad Whitlock, Fri Mar 28 12:02:46 PDT 2003
//   I added a virtual flag.
//
// ****************************************************************************

QualifiedFilename::QualifiedFilename() : host(""), path(""), filename("")
{
    separator = SLASH_CHAR;
    flag = 1;
}

QualifiedFilename::QualifiedFilename(std::string host_, std::string path_,
    std::string filename_, bool access, bool isVirtual)
{
    host = host_;
    path = path_;
    filename = filename_;
    flag = 0;
    SetAccess(access);
    SetVirtual(isVirtual);

    // Try and determine the separator to use in the filename.
    separator = DetermineSeparator(path);
}

QualifiedFilename::QualifiedFilename(const std::string &fullname)
{
    flag = 1;
    SetFromString(fullname);
}
                  
QualifiedFilename::QualifiedFilename(const QualifiedFilename &qf)
{
    host = qf.host;
    path = qf.path;
    filename = qf.filename;
    separator = qf.separator;
    flag = qf.flag;
}

// ****************************************************************************
// Method: QualifiedFilename::~QualifiedFilename
//
// Purpose: 
//   Destructor for the QualifiedFilename class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 12 14:38:01 PST 2000
//
// Modifications:
//   
// ****************************************************************************

QualifiedFilename::~QualifiedFilename()
{
    // nothing much
}

// ****************************************************************************
// Method: QualifiedFilename::operator =
//
// Purpose: 
//   Assignment operator.
//
// Arguments:
//   qf : The qualified filename that is being copied.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 12 14:38:25 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Aug 26 17:06:13 PST 2002
//   I added the separator.
//
//   Brad Whitlock, Fri Mar 28 12:25:22 PDT 2003
//   I added a flag.
//
// ****************************************************************************

void
QualifiedFilename::operator = (const QualifiedFilename &qf)
{
    host = qf.host;
    path = qf.path;
    filename = qf.filename;
    separator = qf.separator;
    flag = qf.flag;
}

// ****************************************************************************
// Method: QualifiedFilename::operator ==
//
// Purpose: 
//   Equals operator.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 12 14:38:56 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Aug 26 17:06:46 PST 2002
//   I added the separator.
//
// ****************************************************************************

bool
QualifiedFilename::operator == (const QualifiedFilename &qf) const
{
    return (host == qf.host) &&
           (path == qf.path) &&
           (filename == qf.filename);
}

// ****************************************************************************
// Method: QualifiedFilename::operator < 
//
// Purpose: 
//   Compares a filename against this filename.
//
// Arguments:
//   qf : The filename to compare against this filename.
//
// Returns:    true if this filename is less than the passed in filename.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 26 13:39:43 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Aug 26 13:39:46 PST 2003
//   Changed from using < operator for strings to using NumericStringCompare.
//
//   Brad Whitlock, Thu Sep 18 11:16:51 PDT 2003
//   I prevented the call to NumericStringCompare if the filenames are already
//   equal.
//
// ****************************************************************************

bool
QualifiedFilename::operator < (const QualifiedFilename &qf) const
{
    bool retval = false;
 
    if(!(this->operator == (qf)))
        retval = NumericStringCompare(FullName(), qf.FullName());

    return retval;
}

// ****************************************************************************
// Method: QualifiedFilename::operator >
//
// Purpose: 
//   Compares a filename against this filename.
//
// Arguments:
//   qf : The filename to compare against this filename.
//
// Returns:    true if this filename is greater than the passed in filename.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 26 13:39:43 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Aug 26 13:39:46 PST 2003
//   Changed from using > operator for strings to using NumericStringCompare.
//
//   Brad Whitlock, Thu Sep 18 11:16:51 PDT 2003
//   I prevented the call to NumericStringCompare if the filenames are already
//   equal.
//
// ****************************************************************************

bool
QualifiedFilename::operator > (const QualifiedFilename &qf) const
{
    bool retval = false;
 
    if(!(this->operator == (qf)))
        retval = NumericStringCompare(qf.FullName(), FullName());

    return retval;
}

// ****************************************************************************
// Method: QualifiedFilename::operator !=
//
// Purpose: 
//   Not-Equals operator.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 12 14:39:23 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Aug 26 17:07:14 PST 2002
//   I added the separator.
//
// ****************************************************************************

bool
QualifiedFilename::operator != (const QualifiedFilename &qf) const
{
    return (host != qf.host) ||
           (path != qf.path) ||
           (filename != qf.filename);
}

// ****************************************************************************
// Method: QualifiedFilename::Empty
//
// Purpose: 
//   Returns true if all of the filename components are empty.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 12 14:36:44 PST 2000
//
// Modifications:
//   
// ****************************************************************************

bool
QualifiedFilename::Empty() const
{
    return (host.size() == 0) &&
           (path.size() == 0) &&
           (filename.size() == 0);
}

// ****************************************************************************
// Method: QualifiedFilename::DetermineSeparator
//
// Purpose: 
//   Scans a string for common file separator characters. The first such
//   character is returned.
//
// Arguments:
//   p : The string to search.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 26 17:13:01 PST 2002
//
// Modifications:
//   
// ****************************************************************************

char
QualifiedFilename::DetermineSeparator(const std::string &p) const
{
    char sep = SLASH_CHAR;

    for(int i = 0; i < p.length(); ++i)
    {
        if(p[i] == '\\' || p[i] == '/')
        {
            sep = p[i];
            break;
        }
    }

    return sep;
}

// ****************************************************************************
// Method: QualifiedFilename::SetFromString
//
// Purpose: 
//   Sets the fields based on the filename stored in the str string.
//   This string can be of the forms: host:path/file, host:file,
//   path/file, file.
//
// Arguments:
//   str : The filename string to parse.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 12 14:33:32 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Apr 22 15:24:56 PST 2002
//   Ported to Windows.
//
//   Brad Whitlock, Thu Jun 27 14:18:28 PST 2002
//   Made the index variable an int so it works properly when there is no
//   host or path in the filename.
//
//   Brad Whitlock, Mon Aug 26 17:14:42 PST 2002
//   
// ****************************************************************************

void
QualifiedFilename::SetFromString(const std::string &str)
{
    int index;

    // Look for the hostname in the string.
    if((index = str.find(":")) != std::string::npos)
        host = str.substr(0, index);
    else
    {
        // The filename was not a qualified filename, assume it
        // is on localhost.
        host = std::string("localhost");
    }

    // Try and determine the separator to use in the filename.
    separator = DetermineSeparator(str);
    const char s[2] = {separator, '\0'};
    std::string separator_str(s);

    // Look for the last slash in the path if there is one.
    std::string pathAndFile(str.substr(index + 1));
    if((index = pathAndFile.rfind(separator_str)) > 0)
    {
        path = pathAndFile.substr(0, index);
        filename = pathAndFile.substr(index + 1);
    }
    else
    {
        // There was no path in the string.
        path = std::string("");
        filename = pathAndFile;
    }
}

// ****************************************************************************
// Method: QualifiedFilename::FullName
//
// Purpose: 
//   Returns a string of the form host:path/file.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 12 14:35:33 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Apr 22 15:24:56 PST 2002
//   Ported to Windows.
//
//   Brad Whitlock, Mon Aug 26 17:08:35 PST 2002
//   I changed the code so it can use different separators.
//
// ****************************************************************************

std::string
QualifiedFilename::FullName() const
{
    std::string temp(host);

    if(path.size() != 0 && filename.size() != 0)
        temp += ":";

    if(path.size() != 0)
    {
        const char str[2] = {separator, '\0'};
        std::string separator_str(str);

        temp += path;
        if((path.size() != 0) && (path[path.size() - 1] != separator))
            temp += separator_str;
    }

    if(filename.size() != 0)
        temp += filename;

    return temp;
}

// ****************************************************************************
// Method: QualifiedFilename::PathAndFile
//
// Purpose: 
//   Returns a string of the form: path/file.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 12 14:36:13 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Apr 22 15:24:56 PST 2002
//   Ported to Windows.
//
//   Brad Whitlock, Mon Aug 26 17:09:36 PST 2002
//   I changed the code so it can use different separators.
//
// ****************************************************************************

std::string
QualifiedFilename::PathAndFile() const
{
    std::string temp(path);
    if(path.size() != 0 && path[path.size() - 1] != separator)
    {
        const char str[2] = {separator, '\0'};
        temp += std::string(str);
    }
    temp += filename;

    return temp;
}

// ****************************************************************************
// Function: CombineQualifiedFilenameVectors
//
// Purpose: 
//   Combines two QualifiedFilenameVector objects into a single
//   QualifiedFilenameVector object while making sure that there are no
//   duplicates.
//
// Arguments:
//   a : The first filename vector.
//   b : The second filename vector.
//
// Returns:    A QualifiedFilenameVector object that contains both a,b.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 10 10:33:55 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

QualifiedFilenameVector
CombineQualifiedFilenameVectors(const QualifiedFilenameVector &a,
    const QualifiedFilenameVector &b)
{
    int i;
    QualifiedFilenameVector retval;

    // Add the elements from a if they're not in retval.
    for(i = 0; i < a.size(); ++i)
    {
        if(std::find(retval.begin(), retval.end(), a[i]) == retval.end())
            retval.push_back(a[i]);
    }

    // Add the elements from b if they're not in retval.
    for(i = 0; i < b.size(); ++i)
    {
        if(std::find(retval.begin(), retval.end(), b[i]) == retval.end())
            retval.push_back(b[i]);
    }

    // Sort the final vector of filenames.
    std::sort(retval.begin(), retval.end());

    return retval;
}
