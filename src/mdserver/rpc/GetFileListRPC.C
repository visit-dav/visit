#include <GetFileListRPC.h>
#include <GetFileListException.h>
#include <DebugStream.h>
#include <Utility.h>
#include <algorithm>

// ****************************************************************************
// Method: GetFileListRPC::GetFileListRPC
//
// Purpose: 
//   Constructor for the GetFileListRPC class.
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 10:27:15 PDT 2000
//
// Modifications:
//   Brad Whitlock, Tue Aug 29 10:27:36 PDT 2000
//   I changed the format string to an empty string since this RPC
//   sends no data when it requests the file list.
//
//   Brad Whitlock, Mon Mar 24 14:15:34 PST 2003
//   I added string and bool arguments.
//
//   Brad Whitlock, Thu Jul 29 12:16:45 PDT 2004
//   I added a bool argument.
//
// ****************************************************************************

GetFileListRPC::GetFileListRPC() : BlockingRPC("sbb", &fileList)
{
}

// ****************************************************************************
// Method: GetFileListRPC::~GetFileListRPC
//
// Purpose: 
//   Destructor for the GetFileListRPC class.
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 10:27:15 PDT 2000
//
// Modifications:
//
// ****************************************************************************

GetFileListRPC::~GetFileListRPC()
{
}

// ****************************************************************************
// Method: GetFileListRPC::operator()
//
// Purpose: 
//   This is the () operator for the GetFileListRPC class. This method
//   makes the objects of this class function objects. This method
//   executes the RPC to get the file list and returns a pointer to
//   the file list.
//
// Arguments:
//   f : The filter string to use.
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 10:33:30 PDT 2000
//
// Modifications:
//   Brad Whitlock, Tue Aug 29 18:50:08 PST 2000
//   Added code to throw an exception if the RPC had an error.
//
//   Jeremy Meredith, Fri Nov 17 16:30:39 PST 2000
//   Made output go to log file instead of cout or cerr.
//
//   Brad Whitlock, Mon Mar 24 14:16:35 PST 2003
//   I added arguments to pass to the mdserver.
//
//   Brad Whitlock, Thu Jul 29 12:17:47 PDT 2004
//   I added the smartGrouping argument.
//
// ****************************************************************************

const GetFileListRPC::FileList *
GetFileListRPC::operator()(const std::string &f, bool grouping,
    bool smartGrouping)
{
    debug3 << "Executing GetFileList(" << f.c_str()
           << (grouping?"true":"false") << ", "
           << (smartGrouping?"true":"false") << ", "
           << ") RPC\n";

    // Store the arguments.
    filter = f;
    automaticFileGrouping = grouping;
    smartFileGrouping = smartGrouping;

    // Try to execute the RPC.
    Execute();

    // If the RPC returned an error, throw an exception.
    if(GetReply()->GetStatus() == error)
    {
        EXCEPTION0(GetFileListException);
    }

    return &fileList;
}

// ****************************************************************************
// Method: GetFileListRPC::SelectAll
//
// Purpose: 
//   This method selects all of the components in the GetFileListRPC
//   before the RPC is executed. This is so RPC function parameters
//   would be communicated.
//
// Note:       
//   This RPC has no parameters so this function selects nothing.
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 10:34:55 PDT 2000
//
// Modifications:
//   Brad Whitlock, Mon Mar 24 14:17:29 PST 2003
//   I added a string and a boolean argument.
//
//   Brad Whitlock, Thu Jul 29 12:18:30 PDT 2004
//   I added smartFileGrouping.
//
// ****************************************************************************

void
GetFileListRPC::SelectAll()
{
    Select(0, (void *)&filter);
    Select(1, (void *)&automaticFileGrouping);
    Select(2, (void *)&smartFileGrouping);
}

// ****************************************************************************
// Method: GetFileListRPC::GetFilter
//
// Purpose: 
//   Returns the filter used by the RPC.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 24 14:20:54 PST 2003
//
// Modifications:
//   
// ****************************************************************************

const std::string &
GetFileListRPC::GetFilter() const
{
    return filter;
}

// ****************************************************************************
// Method: GetFileListRPC::GetAutomaticFileGrouping
//
// Purpose: 
//   Returns the automatic file grouping flag.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 27 09:56:21 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
GetFileListRPC::GetAutomaticFileGrouping() const
{
    return automaticFileGrouping;
}

// ****************************************************************************
// Method: GetFileListRPC::GetSmartFileGrouping
//
// Purpose: 
//   Returns whether smart file grouping is enabled.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 29 12:19:18 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

bool
GetFileListRPC::GetSmartFileGrouping() const
{
    return smartFileGrouping;
}

// ****************************************************************************
// Method: GetFileListRPC::FileList::FileList
//
// Purpose: 
//   Constructor for the GetFileListRPC::FileList class.
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 10:48:07 PDT 2000
//
// Modifications:
//   Brad Whitlock, Mon Mar 31 11:28:34 PDT 2003
//   I added virtualNames, numVirtualFiles.
//
// ****************************************************************************

GetFileListRPC::FileList::FileList() : AttributeSubject("s*i*l*i*s*i*"), 
    names(), types(), sizes(), access(), virtualNames(), numVirtualFiles()
{
}

GetFileListRPC::FileList::FileList(const GetFileListRPC::FileList &obj) :
    AttributeSubject("s*i*l*i*s*i*"), 
    names(obj.names), types(obj.types), sizes(obj.sizes), access(obj.access),
    virtualNames(obj.virtualNames), numVirtualFiles(obj.numVirtualFiles)
{
}

// ****************************************************************************
// Method: GetFileListRPC::FileList::~FileList
//
// Purpose: 
//   Destructor for the GetFileListRPC::FileList class.
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 10:48:07 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

GetFileListRPC::FileList::~FileList()
{
}

// ****************************************************************************
// Method: GetFileListRPC::FileList::SelectAll
//
// Purpose: 
//   Selects all the attributes in the class so they can be
//   transmitted across a connection.
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 10:48:07 PDT 2000
//
// Modifications:
//   Brad Whitlock, Mon Mar 31 11:30:52 PDT 2003
//   I added virtualNames, numVirtualFiles.
//
// ****************************************************************************

void 
GetFileListRPC::FileList::SelectAll()
{
    Select(0, (void *)&names);
    Select(1, (void *)&types);
    Select(2, (void *)&sizes);
    Select(3, (void *)&access);
    Select(4, (void *)&virtualNames);
    Select(5, (void *)&numVirtualFiles);
}

// ****************************************************************************
// Method: GetFileListRPC::FileList::Clear
//
// Purpose: 
//   Clears the file list.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 18 15:11:37 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
GetFileListRPC::FileList::Clear()
{
    names.clear();
    types.clear();
    sizes.clear();
    access.clear();

    virtualNames.clear();
    numVirtualFiles.clear();
}

// ****************************************************************************
// Class: FileListInformation
//
// Purpose:
//   Stores file list information long enough to sort it.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 14 10:18:28 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Aug 26 13:28:04 PST 2003
//   I added the name member and a < operator.
//
//   Brad Whitlock, Thu Sep 18 11:27:27 PDT 2003
//   I changed the < operator a little.
//
// ****************************************************************************

struct FileListInformation
{
    FileListInformation() : name()
    {
    }

    FileListInformation(const std::string &n, int t, long s, int a) : name(n)
    {
        type = t;
        size = s;
        access = a;
    }

    FileListInformation(const FileListInformation &obj) : name(obj.name)
    {
        type = obj.type;
        size = obj.size;
        access = obj.access;
    }

    ~FileListInformation()
    {
    }

    void operator = (const FileListInformation &obj)
    {
        name = obj.name;
        type = obj.type;
        size = obj.size;
        access = obj.access;
    }

    // Use numeric and string comparison to compare the name.
    bool operator < (const FileListInformation &obj) const
    {
        bool retval = false;

        if(name != obj.name)
            retval = NumericStringCompare(name, obj.name);

        return retval;
    }

    std::string name;
    int         type;
    int         access;
    long        size;
};

// ****************************************************************************
// Method: GetFileListRPC::FileList::Sort
//
// Purpose: 
//   Sorts the file list but does not touch the virtual file information.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 14 10:18:12 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Aug 26 13:24:44 PST 2003
//   I made it sort numerically.
//
// ****************************************************************************

void
GetFileListRPC::FileList::Sort()
{
    std::vector<FileListInformation> sortVector;

    // Fill up the map sorting it in the process.
    int i;
    for(i = 0; i < names.size(); ++i)
    {
        sortVector.push_back(
            FileListInformation(names[i], types[i], sizes[i], access[i]));
    }

    // Sort the vector.
    std::sort(sortVector.begin(), sortVector.end());

    // Iterate through the map and store the values back into the vectors.
    for(i = 0; i < sortVector.size(); ++i)
    {
        names[i]  = sortVector[i].name;
        types[i]  = sortVector[i].type;
        sizes[i]  = sortVector[i].size;
        access[i] = sortVector[i].access;
    }
}

// *******************************************************************
// Function: operator <<
//
// Purpose:
//   Prints a GetFileListRPC::FileList to an ostream.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 29 16:05:43 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Apr 14 10:35:25 PDT 2003
//   I added code to print virtual files.
//
// *******************************************************************

ostream &
operator << (ostream &os, const GetFileListRPC::FileList &fl)
{
    os << "{";
    int i;
    for(i = 0; i < fl.names.size(); ++i)
    {
        os << "{" << fl.names[i].c_str() << ", " << fl.types[i] << ", "
           << fl.sizes[i] << ", " << fl.access[i] << "}";

        if(i < fl.names.size() - 1)
            os << ", ";
    }
    os << "}" << endl;
    int index = 0;
    int nvf = 0;
    for(i = 0; i < fl.names.size(); ++i)
    {
        if(fl.types[i] == GetFileListRPC::VIRTUAL)
        {
            os << "Virtual file: " << fl.names[i].c_str() << " {" << endl;
            int start = index;
            int end = start + fl.numVirtualFiles[nvf];
            for(int j = start; j < end; ++j)
                 os << "\t" << fl.virtualNames[j].c_str() << endl;
            index += fl.numVirtualFiles[nvf];
            os << "}" << endl;
            ++nvf;
        }
    }

    return os;
}

