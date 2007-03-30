#include <QualifiedFilename.h>
#include <visit-config.h>

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

bool
QualifiedFilename::operator < (const QualifiedFilename &qf) const
{
    return (FullName() < qf.FullName());
}

bool
QualifiedFilename::operator > (const QualifiedFilename &qf) const
{
    return (FullName() > qf.FullName());
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
