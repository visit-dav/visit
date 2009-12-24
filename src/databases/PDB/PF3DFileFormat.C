/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
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

#include <visitstream.h>
#include <algorithm>

#include <InvalidVariableException.h>
#include <DebugStream.h>
#include <FileFunctions.h>
#include <avtSTMDFileFormatInterface.h>

#include <avtDatabase.h>
#include <avtDatabaseMetaData.h>
#include <avtIntervalTree.h>
#include <avtStructuredDomainBoundaries.h>
#include <avtVariableCache.h>

#include <vtkCellData.h>
#include <vtkFloatArray.h>
#include <vtkIdList.h>
#include <vtkIntArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnsignedCharArray.h>

#include <visit-config.h>
#include <snprintf.h>
#include <math.h>
#include <float.h>
#include <bow.h>

#if !defined(_WIN32)
#include <sys/stat.h>
#endif

// This header file is last because it includes "scstd.h" (indirectly
// through "pdb.h"), which defines min and max, which conflict with
// "limits.h" on tru64 and aix systems.  On tru64 systems the conflict
// occurs with gcc-3.0.4 and on aix systems the conflict occurs with
// gcc-3.1.
#include <PF3DFileFormat.h>

#define BOF_KEY  "BOF"
#define FILE_KEY "PDBFileObject"

// Until we see nothing weird in the output.
#define DEBUG_PRINT

#ifdef DEBUG_PRINT
template <class T>
void
print_array(ostream &os, const char *name, const T *data, int nElems,
    int space=8, bool comma=true, bool donewline = true)
{
    os << name << " = {";
    if(donewline)
       os << endl;
    int ct = 0;
    bool first = true;
    for(int i = 0; i < nElems; ++i)
    {
        if(comma && !first)
            os << ", ";
        first = false;
        os << data[i];
        if(ct < space-1)
        {
            ++ct;
        }
        else
        {
            if(!comma)
                os << "***end" << endl;
            else
                os << endl;
            first = true;
            ct = 0;
        }
    }
    os << "}" << endl;
}

template <class T>
void
print_multi_dim_array(ostream &os, const char *name, const T *data, int ndims,
    const int *dims, bool doCommas)
{
    os << name << "[" << dims[0] << "][" << dims[1] << "][" << dims[2] << "] = {";
    int ct = 0;
    if(ndims == 1)
    {
        for(int i = 0; i < dims[0]; ++i)
        {
            os << data[i];
            if(doCommas && i < dims[0]-1)
                os << ", ";
        }
    }
    else if(ndims == 2)
    {
        os << endl;
        const T *ptr = data;
        for(int i = 0; i < dims[1]; ++i)
        {
            os << "{";
            for(int j = 0; j < dims[0]; ++j)
            {
                os << *ptr++;
                if(doCommas && j < dims[0]-1)
                    os << ", ";
            }
            os << "},\n";
        }
    }
    else if(ndims == 3)
    {
        os << endl;
        const T *ptr = data;
        for(int i = 0; i < dims[2]; ++i)
        {
            os << "{";
            for(int j = 0; j < dims[1]; ++j)
            {
                os << "{";
                for(int k = 0; k < dims[0]; ++k)
                {
                    os << *ptr++;
                    if(doCommas && k < dims[0]-1)
                         os << ", ";
                }
                os << "},\n";
            }
            os << "},\n";
        }
    }
    os << "}" << endl;
}
#endif

// ****************************************************************************
// Method: PF3DFileFormat::CreateInterface
//
// Purpose: 
//   Creates file format objects for all of the files and returns a file format
//   interface that works for the PF3D file format.
//
// Arguments:
//   pdb : A pointer to a PDBFileFormatObject, which has opened the first file
//         in the filenames list.
//   filenames : The list of filenames.
//   nList     : The number of filenames.
//   nBlock    : The number of filenames that make up a single time step.
//
// Returns:    A file format interface.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 11:35:20 PDT 2003
//
// Modifications:
//   Brad Whitlock, Fri Sep 19 14:39:30 PST 2003
//   I changed how the file format is identified in parallel so that only
//   one file is opened instead of having each processor trying to open
//   the first file.
//
// ****************************************************************************

avtFileFormatInterface *
PF3DFileFormat::CreateInterface(PDBFileObject *pdb, const char *const *filenames,
    int nList, int nBlock)
{
    avtFileFormatInterface *inter = 0;

    // If the file format is a PF3D file then
    if(PF3DFileFormat::StaticIdentifyFormat(pdb))
    {
        //
        // Create an array of STMD file formats since that's what the PF3D
        // file format is.
        //
        int i;
        avtSTMDFileFormat **ffl = new avtSTMDFileFormat*[nList];
        for (i = 0 ; i < nList ; i++)
            ffl[i] = 0;
        TRY
        {
            for (i = 0 ; i < nList ; ++i)
                ffl[i] = new PF3DFileFormat(filenames[i]);

            //
            // Try to create a file format interface compatible with the PF3D
            // file format.
            //
            inter = new avtSTMDFileFormatInterface(ffl, nList);
        }
        CATCH(VisItException)
        {
            for (i = 0 ; i < nList ; ++i)
                delete ffl[i];
            delete [] ffl;
            RETHROW;
        }
        ENDTRY
    }

    return inter;
}

// ****************************************************************************
// Method: PF3DFileFormat::PF3DFileFormat
//
// Purpose: 
//   Constructor for the PF3DFileFormat class.
//
// Arguments:
//   filename : The name of the file to open.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 11:38:00 PDT 2003
//
// Modifications:
//   Brad Whitlock, Fri Sep 19 14:41:23 PST 2003
//   Added the initialized flag.
//   
// ****************************************************************************

PF3DFileFormat::PF3DFileFormat(const char *filename) : PDBReader(filename),
    avtSTMDFileFormat(&filename, 1), master(), glob_nams(), int_nams(),
    glob_units(), apply_exp(), databaseComment()
{
    initialized = false;
    cycle = 0;
    nx = ny = nz = 0;
    lenx = leny = lenz = 1.;
    compression = false;
}

// ****************************************************************************
// Method: PF3DFileFormat::~PF3DFileFormat
//
// Purpose: 
//   Destructor for the PF3DFileFormat class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 11:39:47 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

PF3DFileFormat::~PF3DFileFormat()
{
    FreeUpResources();
}

// ****************************************************************************
// Method: PF3DFileFormat::IdentifyFormat
//
// Purpose: 
//   Tests the file to see if it is really PF3D.
//
// Returns:    true if the file is PF3D; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 1 17:00:45 PST 2004
//
// Modifications:
//   
// ****************************************************************************

bool
PF3DFileFormat::StaticIdentifyFormat(PDBFileObject *fileObj)
{
    bool validFile = true;
    validFile &= fileObj->SymbolExists("__@history");
    validFile &= fileObj->SymbolExists("nx");
    validFile &= fileObj->SymbolExists("ny");
    validFile &= fileObj->SymbolExists("nz");
    return validFile;
}

// ****************************************************************************
// Method: PF3DFileFormat::IdentifyFormat
//
// Purpose: 
//   Called by the Identify method to see if the file is really PF3D.
//
// Returns:    True if the file is PF3D; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 9 16:26:32 PST 2004
//
// Modifications:
//   
// ****************************************************************************

bool
PF3DFileFormat::IdentifyFormat()
{
    return StaticIdentifyFormat(pdb);
}

// ****************************************************************************
// Method: PF3DFileFormat::ReadStringVector
//
// Purpose: 
//   Reads the named array from the PDB file and creates a stringVector
//   out of the contents of that array.
//
// Arguments:
//   name   : The name of the array to read.
//   output : The stringVector used to store the resulting strings.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 20 09:27:43 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

bool
PF3DFileFormat::ReadStringVector(const char *name, stringVector &output)
{
    bool retval = true;
    TypeEnum t;
    int *dims = 0, nDims = 0, nTotalElements = 0, bufLen = 0;
    char *buf = 0;
    if(pdb->SymbolExists(name, &t, &nTotalElements, &dims, &nDims) &&
       pdb->GetString(name, &buf, &bufLen))
    {
        int nStrings, charsPerString;
        if(nDims > 1)
        {
            nStrings = dims[1];
            charsPerString = dims[0] - 1;
        }
        else
        {
            // If we have to guess.
            nStrings = 6;
            charsPerString = nTotalElements / nStrings;
        }

        char *input = buf;
        output.clear();
        debug4 << "stringVector " << name << " = {" << endl;
        for(int i = 0; i < nStrings; ++i)
        {
            std::string word;
            for(int j = 0; j < charsPerString; ++j)
            {
                char c = *input++;
                if(c > ' ')
                    word += c;
            }
            ++input;
            output.push_back(word);
            debug4 << "\"" << word.c_str() << "\"" << endl;
        }
        debug4 << "}" << endl;
        delete [] buf;
        delete [] dims;
    }
    else
    {
        debug4 << "Could not read " << name << ". "
               << PDBLIB_ERRORSTRING << endl;
        retval = false;
    }

    return retval;
}

// ****************************************************************************
// Method: PF3DFileFormat::ReadVariableInformation
//
// Purpose: 
//   Reads the information about variables from the master file.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 9 16:27:45 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
PF3DFileFormat::ReadVariableInformation()
{
    //
    // Read the global variable names that will be used in the variable list.
    //
    ReadStringVector("glob_nams", glob_nams);

    //
    // Read the internal variable names to use when accessing the domain files.
    //
    ReadStringVector("int_nams", int_nams);

    //
    // Read the variable unit names to use..
    //
    ReadStringVector("glob_units", glob_units);

    //
    // Now that we have all of the variable names, look for variables in
    // the file that tell whether exp() should be applied to the data.
    //
    char buf[100];
    for(int i = 0; i < glob_nams.size(); ++i)
    {
        // Get the right name of the _is_log array. The names stored in the
        // glob_nams array are not quite right for deniaw and denepw so
        // write the names minus the "den" part.
        const char *name = glob_nams[i].c_str();
        if(name[0] == 'd' && name[1] == 'e' && name[2] == 'n')
            name += 3;
        SNPRINTF(buf, 100, "%s_is_log", name);

        int is_log = 0;
        pdb->GetInteger(buf, &is_log);
        apply_exp.push_back(is_log);
        debug4 << "Variable " << glob_nams[i].c_str() << "_is_log=" << is_log << endl;
    }
}

// ****************************************************************************
// Method: PF3DFileFormat::Initialize
//
// Purpose: 
//   Opens the file and gets required values from it.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 19 14:42:40 PST 2003
//
// Modifications:
//   Eric Brugger, Wed May  6 11:09:23 PDT 2009
//   I replaced the code that gets the cycle number by extracting it from
//   the string run_id_c within the file, since that string doesn't
//   necessarily contain the cycle number.  The new code extracts it from
//   the last digit sequence in the filename.
//   
// ****************************************************************************

void
PF3DFileFormat::Initialize()
{
    if(!initialized)
    {
        const char *mName = "PF3DFileFormat::IdentifyFormat: ";
        const char *noread = "Could not read ";

        // Get the cycle number.
        if (pdb->GetName().length() > 0)
        {
            const char *filename = pdb->GetName().c_str();
            int iCycle = pdb->GetName().length()-1;
            while (iCycle >= 0 && !isdigit(filename[iCycle]))
                iCycle--;
            while (iCycle >= 0 && isdigit(filename[iCycle]))
                iCycle--;
            iCycle++;
            sscanf(&filename[iCycle], "%d", &cycle);
        }

#define READ_VALUE(M, K, V) \
        if(pdb-> M (K, & V)) \
        { \
            debug4 << mName << setprecision(10) << K << " = " << V << endl; \
        } \
        else \
        { \
            debug4 << mName << noread << K << endl; \
        }

        // Read in nx, ny, nx
        char *key = "nx";
        READ_VALUE(GetInteger, key, nx);
        key = "ny";
        READ_VALUE(GetInteger, key, ny);
        key = "nz";
        READ_VALUE(GetInteger, key, nz);
        // The value stored for nz is 1 smaller than it ought to be.
        ++nz;

        // Read in lenx, leny, lenz.
        key = "lenx";
        READ_VALUE(GetDouble, key, lenx);
        key = "leny";
        READ_VALUE(GetDouble, key, leny);
        key = "lenz";
        READ_VALUE(GetDouble, key, lenz);

        // See if "brick of wavelets" compression is enabled.
        int use_bow = 0, use_bowcomp = 0;
        key = "use_bow";
        READ_VALUE(GetInteger, key, use_bow);
        key = "use_bowcomp";
        READ_VALUE(GetInteger, key, use_bowcomp);
        compression = ((use_bow == 1) && (use_bowcomp == 1));

        // Get the database comment from the file.
        char *dbComment = 0;
        if(pdb->GetString("tcomment_c", &dbComment))
        {
            databaseComment = std::string(dbComment);
            delete [] dbComment;
        }

        // Read the master structure from the file so we know a little more
        // about the file.
        if(!master.Read(pdb))
        {
            EXCEPTION0(VisItException);
        }

        // Read the names of the variables and their associated data
        // arrays.
        ReadVariableInformation();

        initialized = true;
    }
}

// ****************************************************************************
// Method: PF3DFileFormat::ActivateTimestep
//
// Purpose: 
//   Activates the current time step.
//
// Note:       This method is guaranteed to be called by all processors. Here
//             we take the opportunity to initialize the reader, including
//             the master object. This prevents the reader from crashing
//             when changing to a new time state.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 20 10:51:52 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
PF3DFileFormat::ActivateTimestep(void)
{
    // Initialize the reader if we have not yet done so.
    Initialize();
}

// ****************************************************************************
// Method: PF3DFileFormat::GetCycle
//
// Purpose: 
//   Returns the cycle number for the file.
//
// Returns:    The cycle number.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 17 09:24:01 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

int
PF3DFileFormat::GetCycle(void)
{
    return cycle;
}

// ****************************************************************************
// Method: PF3DFileFormat::GetTime
//
// Purpose: 
//   Returns the time for the file.
//
// Returns:    The time for the file.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 13 15:11:48 PST 2003
//
// Modifications:
//   
// ****************************************************************************

double
PF3DFileFormat::GetTime(void)
{
    return master.Get_tnowps();
}

// ****************************************************************************
// Method: PF3DFileFormat::GetType
//
// Purpose: 
//   Return the name of the file format.
//
// Returns:    The name of the file format.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 14:59:44 PST 2003
//
// Modifications:
//   
// ****************************************************************************

const char *
PF3DFileFormat::GetType()
{
    return "PF3D File Format";
}

// ****************************************************************************
// Method: PF3DFileFormat::CanAccessFile
//
// Purpose: 
//   Determines whether we can access the specified file.
//
// Arguments:
//   filename : The name of the file that we want to access.
//
// Returns:    True if the file can be accessed; False otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 30 16:38:28 PST 2005
//
// Modifications:
//   
//    Mark C. Miller, Thu Mar 30 16:45:35 PST 2006
//    Made it use VisItStat instead of stat
//
// ****************************************************************************

bool
PF3DFileFormat::CanAccessFile(const std::string &filename) const
{
#if defined(_WIN32)
    // For now.
    return true;
#else
    // Get the userId and the groups for that user so we can check the
    // file permissions.
    gid_t gids[100];
    uid_t uid = getuid();
    int   ngids = getgroups(100, gids);

    // Get information about the file.
    VisItStat_t s;
    VisItStat(filename.c_str(), &s);

    bool isdir = S_ISDIR(s.st_mode);
    bool canaccess = false;
    bool isuser  = (s.st_uid == uid);
    bool isgroup = false;
    for (int i=0; i<ngids && !isgroup; i++)
        if (s.st_gid == gids[i])
            isgroup=true;
    
    if (isdir)
    {
        if ((s.st_mode & S_IROTH) && (s.st_mode & S_IXOTH))
            canaccess=true;
        else if (isuser && (s.st_mode & S_IRUSR) && (s.st_mode & S_IXUSR))
            canaccess=true;
        else if (isgroup && (s.st_mode & S_IRGRP) && (s.st_mode & S_IXGRP))
            canaccess=true;
    }
    else
    {
        if (s.st_mode & S_IROTH)
            canaccess=true;
        else if (isuser && (s.st_mode & S_IRUSR))
            canaccess=true;
        else if (isgroup && (s.st_mode & S_IRGRP))
            canaccess=true;
    }

    return canaccess;
#endif
}

// ****************************************************************************
// Method: PF3DFileFormat::FilenameForDomain
//
// Purpose: 
//   Returns the domain filename for the real domain.
//
// Arguments:
//   realDomain : The index of a real domain.
//
// Returns:    The name of the file that contains the data for the domain.
//
// Note:       This method uses the viz_nams information stored in the 
//             master header but fixes up the paths using the path from the
//             first opened file in case the data files were moved.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 9 16:29:15 PST 2004
//
// Modifications:
//   Brad Whitlock, Wed Nov 30 16:37:05 PST 2005
//   I made it try multiple filenames in the event that the first filename
//   that it tries is not a valid file.
//
// ****************************************************************************

std::string
PF3DFileFormat::FilenameForDomain(int realDomain)
{
    const char *mName = "PF3DFileFormat::FilenameForDomain: ";
    
    std::string prefix, middle, f(filenames[0]);
    debug4 << mName << "FilenameForDomain(" << realDomain << ") = "
           << f.c_str() << endl;

    // Find the directory prefix based on the path of the first master file.
    std::string::size_type index = f.rfind(VISIT_SLASH_STRING);
    std::string vizDir("viz");
    if(index != std::string::npos)
    {
        std::string path(f.substr(0, index));
        debug4 << mName << "path = " << path << endl;

        std::string::size_type index2 = path.rfind(VISIT_SLASH_STRING);
        if(index2 != std::string::npos)
        {
            vizDir = path.substr(index2+1);
            prefix = path.substr(0, index2);
        }
        else
            prefix = f.substr(0, index);
        debug4 << mName << "vizDir = " << vizDir << endl;
        debug4 << mName << "prefix = " << prefix << endl;
    }
    debug4 << mName << "visnam_path = "
           << master.Get_visname_for_domain(realDomain, 0) << endl;
    debug4 << mName << "visnam_file = "
           << master.Get_visname_for_domain(realDomain, 1) << endl;
    debug4 << mName << "prefix = " << prefix.c_str() << endl;

    // Look for "/viz/" in the visnam string for the specified domain and use
    // anything to the right of it.
    std::string vizStr(VISIT_SLASH_STRING);
    vizStr += "viz";
    vizStr += VISIT_SLASH_STRING;
    f = std::string(master.Get_visname_for_domain(realDomain, 0));
    index = f.rfind(vizStr);
    if(index != std::string::npos)
        middle = f.substr(index, f.size() - index);
    if(middle.size() > 1)
    {
        if(middle[0] != VISIT_SLASH_CHAR)
            middle = std::string(VISIT_SLASH_STRING) + middle;
        if(middle[middle.size()-1] != VISIT_SLASH_CHAR)
            middle += VISIT_SLASH_STRING;

        // Replace "viz" with something else, if required.
        std::string rhs(middle.substr(1,middle.size()-1));
        std::string::size_type nextSlash = rhs.find(VISIT_SLASH_STRING);
        if(rhs.substr(nextSlash) != vizDir)
        {
            middle = std::string(VISIT_SLASH_STRING) + vizDir + 
                     rhs.substr(nextSlash, rhs.size() - nextSlash);
        }
    }
    debug4 << mName << "middle = " << middle.c_str() << endl;

    // Concatenate the pieces of the filename to get the final filename.
    std::string filename(prefix);
    filename += middle;
    filename += master.Get_visname_for_domain(realDomain, 1);

    // See if the file exists.
    if(CanAccessFile(filename))
    {
        debug4 << mName << "Domain " << realDomain << "'s filename = "
               << filename.c_str() << endl;
        return filename;
    }
    else
    {
        debug4 << mName << "Domain " << realDomain << "'s filename is not: "
               << filename.c_str() << " because we cannot access that file."
               << endl;
    }

    // We can't access the filename that uses the path to the master file.
    // Try using the path stored in the master file.
    filename = std::string(master.Get_visname_for_domain(realDomain, 0)) +
               std::string(master.Get_visname_for_domain(realDomain, 1));
    if(CanAccessFile(filename))
    {
        debug4 << mName << "Domain " << realDomain << "'s filename = "
               << filename.c_str() << endl;
        return filename;
    }
    else
    {
        debug4 << mName << "Domain " << realDomain << "'s filename is not: "
               << filename.c_str() << " because we cannot access that file."
               << endl;
    }

    return filename;
}

// ****************************************************************************
// Method: PF3DFileFormat::GetDomainFileObject
//
// Purpose: 
//   Returns a pointer to the PDB file object for the specified real domain.
//
// Arguments:
//   realDomain : The index of the real domain whose file we want.
//
// Returns:    A pointer to the real domain's PDB file object.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 9 16:30:54 PST 2004
//
// Modifications:
//   
// ****************************************************************************

PDBFileObject *
PF3DFileFormat::GetDomainFileObject(int realDomain)
{
    PDBFileObject *retval = 0;
    const char *mName = "PF3DFileFormat::GetDomainFileObject: ";

    //
    // Get the filename associated with the real domain.
    //
    std::string key(FilenameForDomain(realDomain));

    //
    // See if the filename was in the variable cache.
    //
    void_ref_ptr vr = cache->GetVoidRef(key.c_str(), FILE_KEY, timestep, -1);
    if(*vr != 0)
    {
        debug4 << mName << "Found a cached file object for "
               << key.c_str() << " at: " << (*vr) << endl;
        retval = (PDBFileObject *)(*vr);
    }
    else
    {
        debug4 << mName << "Created a new PDB file object for "
               << key.c_str() << endl;

        // Not in the cache so create a new PDB file object.
        retval = new PDBFileObject(key.c_str());

        // Store it in the cache.
        void_ref_ptr vr2 = void_ref_ptr(retval, PDBFileObject::Destruct);
        cache->CacheVoidRef(key.c_str(), FILE_KEY, timestep, -1, vr2);

        // Add the file to the list of files that are considered open.
        AddFile(key.c_str());
    }

    return retval;
}

// ****************************************************************************
// Method: PF3DFileFormat::CloseFile
//
// Purpose: 
//   Closes the index'th file in the file list when the file descriptor
//   manager tells it to.
//
// Arguments:
//   index : The index of the file to close.
//
// Returns:    
//
// Note:       The file is deleted from the variable cache.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 8 15:10:42 PST 2004
//
// Modifications:
//   
//   Hank Childs, Mon Mar 14 13:03:44 PST 2005
//   Do not delete the file object, since it will be deleted by the reference
//   pointer automatically.
//
// ****************************************************************************

void
PF3DFileFormat::CloseFile(int index)
{
    const char *key = filenames[index];
    const char *mName = "PF3DFileFormat::CloseFile: ";

    if(key != 0)
    {
        debug4 << mName << "Closing: "
               << key << endl;

        void_ref_ptr vr = cache->GetVoidRef(key, FILE_KEY, timestep, -1);
        if(*vr != 0)
        {
            debug4 << mName << "Found a cached file object for "
                   << key << endl;
            // Clear the object out of the cache somehow...
            void_ref_ptr vr2 = void_ref_ptr(0, PDBFileObject::Destruct);
            cache->CacheVoidRef(key, FILE_KEY, timestep, -1, vr2);
        }
    }
}

// ****************************************************************************
// Method: PF3DFileFormat::FreeUpResources
//
// Purpose: 
//   Frees up resources for the file format.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 9 16:50:26 PST 2004
//
// Modifications:
//   
//   Hank Childs, Mon Mar 14 13:03:44 PST 2005
//   Do not create empty objects to be added to the cache -- the cache will
//   be cleared out automatically by the generic database.
//
// ****************************************************************************

void
PF3DFileFormat::FreeUpResources()
{
    debug4 << "PF3DFileFormat::FreeUpResources" << endl;

    // Close the master file since we can automatically open it up again
    // if we need to.
    pdb->Close();
}

// ****************************************************************************
// Method: PF3DReader::PopulateDatabaseMetaData
//
// Purpose: 
//   Populates the metadata with the list of variables from the file.
//
// Arguments:
//   md : The metadata object that we're going to populate.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 1 15:56:19 PST 2004
//
// Modifications:
//   Brad Whitlock, Mon Dec 5 10:18:53 PDT 2005
//   Added mdserver-specific coding to check if variables are valid.
//
//   Brad Whitlock, Thu Sep 7 16:28:34 PST 2006
//   Added more debugging log information.
//
// ****************************************************************************

void
PF3DFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    const char *mName = "PF3DFileFormat::PopulateDatabaseMetaData: ";
    debug4 << mName << "start" << endl;

    //
    // Set the database comment.
    //
    md->SetDatabaseComment(databaseComment);

    //
    // Figure out the mesh extents and create a mesh metadata object.
    //
    int   dimension = 3;
    avtMeshMetaData *mmd = new avtMeshMetaData;
    mmd->name = "mesh";
    mmd->meshType = AVT_RECTILINEAR_MESH;
    mmd->numBlocks = GetNDomains();
    mmd->blockOrigin = 0;
    mmd->cellOrigin = 1;
    mmd->spatialDimension = dimension;
    mmd->topologicalDimension = dimension;
    mmd->hasSpatialExtents = true;
    mmd->minSpatialExtents[0] = -lenx / 2.;
    mmd->minSpatialExtents[1] = -leny / 2.;
    mmd->minSpatialExtents[2] = 0.;
    mmd->maxSpatialExtents[0] = lenx / 2.;
    mmd->maxSpatialExtents[1] = leny / 2.;
    mmd->maxSpatialExtents[2] = lenz;
    mmd->blockTitle = "processors";
    mmd->blockPieceName = "processor";
    for(int i = 0; i < GetNDomains(); ++i)
    {
        char tmpName[30];
        SNPRINTF(tmpName, 30, "processor%d", i);
        mmd->blockNames.push_back(tmpName);
    }
    md->Add(mmd);

    // Add the variables to the metadata.
    if(glob_nams.size() == int_nams.size() &&
       glob_nams.size() == glob_units.size())
    {
#ifdef MDSERVER
        debug4 << mName << "Get the name of domain 0's file." << endl;

        PDBFileObject *domainPDB = GetDomainFileObject(0);

        debug4 << mName << "Domain 0 handle=" << ((void*)domainPDB);
        if(domainPDB != 0)
            debug4 << ", filename=" << domainPDB->GetName().c_str();
        debug4 << endl;
        debug4 << mName << "Start creating scalar metadata. " << endl;
#endif

        for(int i = 0; i < glob_nams.size(); ++i)
        {
            avtScalarMetaData *smd = new avtScalarMetaData(
                glob_nams[i], "mesh", AVT_ZONECENT);

            debug4 << mName << "var[" << i << "] = " << glob_nams[i].c_str();

            // Set the units if there are any.
            if(glob_units[i] != "")
            {
                smd->hasUnits = true;
                smd->units = glob_units[i];
                debug4 << ", units=" << glob_units[i].c_str() << endl;
            }

#ifdef MDSERVER
            if(domainPDB != 0)
            {
                TypeEnum varType = NO_TYPE;
                int varTotalElements = 0;
                int *vardims = 0, varndims = 0;
                smd->validVariable = false;

                // If the master file contains grp_size entries then this
                // file is a multi-domain PF3D file and we need to cd into
                // a directory before checking for the variable.
                std::string varName(int_nams[i]);
                varName = master.Get_dom_prefix_for_domain(0) + varName;

                debug4 << "Getting info for " << varName.c_str() << endl;

                if(domainPDB->SymbolExists(varName.c_str(), &varType,
                     &varTotalElements, &vardims, &varndims))
                {
                    debug4 << int_nams[i].c_str() << " dims={";
                    for(int n = 0; n < varndims; ++n)
                        debug4 << ", " << vardims[n];
                    debug4 << "}" << endl;

                    smd->validVariable = varndims == 3;
                }
                else
                {
                    debug4 << "domainPDB->SymbolExists returned false "
                              "when trying to determine the number of "
                              "dimensions in: " << int_nams[i].c_str()
                           << endl;
                }
            }
            else
            {
                debug4 << "Domain 0 handle is 0 so the var will be "
                          "marked as invalid." << endl;
            }
#endif

#if 0
            //
            // Set the variable's data extents. Note that these extrema
            // include the values from all domains.
            //
            double minval = DBL_MAX;
            double maxval = -DBL_MAX;
            const double *minvals, *maxvals;
            GetVarMinMaxArrays(glob_nams[i], &minvals, &maxvals);
            if(minvals != 0 && maxvals != 0)
            {
                for(int j = 0; j < master.GetNDomains(); ++j)
                {
                    minval = (minval < minvals[j]) ? minval : minvals[j];
                    maxval = (maxval > maxvals[j]) ? maxval : maxvals[j];
                }
                smd->hasDataExtents = true;
                smd->minDataExtents = float(minval);
                smd->maxDataExtents = float(maxval);
                debug4 << glob_nams[i].c_str() << ": min=" << minval
                       << ", max=" << maxval << endl;
            }
#endif

            md->Add(smd);
        }
    }
    else
    {
        debug4 << "glob_nams and int_nams are not the same size!" << endl;
    }

    //
    // Set up the domain connectivity arrays so this file format can
    // use VisIt's automatic ghost zone creation facilities.
    //
    debug4 << mName << "Setting up domain connectivity." << endl;
    SetUpDomainConnectivity();

    debug4 << mName << "end" << endl;
}

// ****************************************************************************
// Method: PF3DFileFormat::SetUpDomainConnectivity
//
// Purpose: 
//   Sets up the domain connectivity for ghost cell communication.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 9 17:18:32 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
PF3DFileFormat::SetUpDomainConnectivity()
{
    if(GetNDomains() < 2)
        return;

    //
    // Add the neighbors for each domain into the domain boundaries structure.
    //
    avtRectilinearDomainBoundaries *rdb = new avtRectilinearDomainBoundaries(true);
    rdb->SetNumDomains(GetNDomains());
    for(int dom = 0; dom < GetNDomains(); ++dom)
    {
        int domainOrigin[3], domainSize[3];
        GetLogicalExtents(dom, domainOrigin, domainSize);

        int extents[6];
        extents[0] = domainOrigin[0];
        extents[1] = domainOrigin[0] + domainSize[0];
        extents[2] = domainOrigin[1];
        extents[3] = domainOrigin[1] + domainSize[1];
        extents[4] = domainOrigin[2];
        extents[5] = domainOrigin[2] + domainSize[2];
        rdb->SetIndicesForRectGrid(dom, extents);
    }
    rdb->CalculateBoundaries();

    void_ref_ptr vr = void_ref_ptr(rdb,
                                   avtStructuredDomainBoundaries::Destruct);
    cache->CacheVoidRef("any_mesh",
                       AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION, -1, -1, vr);
}

// ****************************************************************************
// Method: PF3DFileFormat::GetVarMinMaxArrays
//
// Purpose: 
//   Gets the min/max arrays from the master struct for the named variable.
//
// Arguments:
//   name    : The name of the variable whose min/max arrays to return.
//   minvals : The return pointer for the min array.
//   maxvals : The return pointer for the max array.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 8 10:57:48 PDT 2004
//
// Modifications:
//   Brad Whitlock, Fri Dec 2 13:52:41 PST 2005
//   I made it general so we can support different variables.
//
// ****************************************************************************

void
PF3DFileFormat::GetVarMinMaxArrays(const std::string &name,
    const double **minvals, const double **maxvals)
{
    *minvals = master.GetMinArray(name);
    *maxvals = master.GetMaxArray(name);
}

// ****************************************************************************
// Method: PF3DFileFormat::GetAuxiliaryData
//
// Purpose: 
//   Gets auxiliary data for the database.
//
// Arguments:
//
// Returns:    Returns a pointer to the auxiliary data.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 9 17:19:57 PST 2004
//
// Modifications:
//    Kathleen Bonnell, Mon Aug 14 16:40:30 PDT 2006
//    API change for avtIntervalTree.
//
// ****************************************************************************

void *
PF3DFileFormat::GetAuxiliaryData(const char *var, int dom,
                                 const char *type, void *,
                                 DestructorFunction &df)
{
    std::string name(var);
    void *retval = 0;
    avtIntervalTree *itree = 0;

    if(strcmp(type, AUXILIARY_DATA_DATA_EXTENTS) == 0)
    {
        debug4 << "GetAuxiliaryData::GetAuxiliaryData getting DATA_EXTENTS" << endl;
        const double *minvals = 0;
        const double *maxvals = 0;
        GetVarMinMaxArrays(name, &minvals, &maxvals);

        if(minvals != 0 && maxvals != 0)
        {
            double range[2];
            itree = new avtIntervalTree(GetNDomains(), 1);
            for(int dom = 0; dom < GetNDomains(); ++dom)
            {
                int realDomain = GetRealDomainIndex(dom);
                range[0] = minvals[realDomain];
                range[1] = maxvals[realDomain];
                itree->AddElement(dom, range);
            }
            itree->Calculate(true);
            retval = (void *)itree;
            df = avtIntervalTree::Destruct;
        }
    }
    else if(strcmp(type, AUXILIARY_DATA_SPATIAL_EXTENTS) == 0)
    {
        debug4 << "avtPF3DFileFormat::GetAuxiliaryData getting SPATIAL_EXTENTS" << endl;
        itree = new avtIntervalTree(GetNDomains(), 3);
        for(int dom = 0 ; dom < GetNDomains(); ++dom)
        {
            double extents[6];
            GetExtents(dom, extents);
            itree->AddElement(dom, extents);
        }
        itree->Calculate(true);

        retval = (void *)itree;
        df = avtIntervalTree::Destruct;
    }

    return retval;
}

// ****************************************************************************
// Method: PF3DFileFormat::GetRealDomainIndex
//
// Purpose: 
//   Converts a domain index into the domain index for a real file.
//
// Arguments:
//   dom : The domain index for which we want a real file index.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 12 14:24:59 PST 2004
//
// Modifications:
//   
// ****************************************************************************

int
PF3DFileFormat::GetRealDomainIndex(int dom) const
{
    return dom;
}

// ****************************************************************************
// Method: PF3DFileFormat::GetNDomains
//
// Purpose: 
//   Return the number of domains in the problem.
//
// Returns:    The number of domains.
//
// Note:       The number of domains that gets returned does not necessarily
//             match the number of domains in the problem if we're doing
//             artifical domain chunking.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 12 14:23:45 PST 2004
//
// Modifications:
//   
// ****************************************************************************

int
PF3DFileFormat::GetNDomains() const
{
    // Fix this so it has some code to multiply the number of domains.
    return master.GetNDomains();
}

// ****************************************************************************
// Method: PF3DFileFormat::GetLogicalExtents
//
// Purpose: 
//   Returns the logical extents for a domain in the entire problem.
//
// Arguments:
//   dom          : The domain whose logical extents we want.
//   globalorigin : The global origin index of the domain.
//   size         : The number of cells in each dimension for the domain.
//
// Note:       We use this routine instead of just accessing domloc because
//             sometimes we like to artificially chunk up the domains into
//             smaller sub-domains.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 12 14:26:48 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
PF3DFileFormat::GetLogicalExtents(int dom, int *globalorigin, int *size)
{
    int realDomain = GetRealDomainIndex(dom);
    // Change this to code that can partition the domain in some way.
    const long *domloc = master.Get_domloc() + realDomain * 6;
    // Return where in the real domain the dom starts.
    if(globalorigin != 0)
    {
        globalorigin[0] = domloc[0];
        globalorigin[1] = domloc[2];
        globalorigin[2] = domloc[4];
    }
    // Return the number of cells in the dom for each dimension.
    if(size != 0)
    {
        size[0] = domloc[1] - domloc[0] + 1;
        size[1] = domloc[3] - domloc[2] + 1;
        size[2] = domloc[5] - domloc[4] + 1;
    }
}

// ****************************************************************************
// Method: PF3DFileFormat::GetExtents
//
// Purpose: 
//   Gets the spatial extents for the specified domain.
//
// Arguments:
//   dom     : The index of the domain for which we want the extents.
//   extents : An array to contain the spatial extents, which are strictly
//             the extents of the domain without ghost zones, etc.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 8 10:59:35 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
PF3DFileFormat::GetExtents(int dom, double *extents)
{
#if 0
    // Use the extents stored in the master's xyzloc array. Note that
    // this code would need to be reworked to return the extents for
    // subdomains.
    int realDomain = GetRealDomainIndex(dom);
    const double *xyzloc = master.Get_xyzloc() + (6 * realDomain);
    extents[0] = xyzloc[0];
    extents[1] = xyzloc[1];
    extents[2] = xyzloc[2];
    extents[3] = xyzloc[3];
    extents[4] = xyzloc[4];
    extents[5] = xyzloc[5];
#else
    //
    // Figure out a size for the domain based on the dom number.
    // Note this code would be smart enough for sub-domain chunking.
    //

    // Figure out the number of cells in each dimension.
    int NX = nx * 2;
    int NY = ny * 2;
    int NZ = nz;

    debug4 << "Number of nodes in each dimension for entire DB: {"
           << NX << ", "<< NY << ", " << NZ << "}" << endl;

    // Figure out the size of a single cell.
    double cellx = lenx / double(NX);
    double celly = leny / double(NY);
    double cellz = lenz / double(NZ);

    debug4 << "Size of a cell: {"
           << cellx << ", "<< celly << ", " << cellz << "}" << endl;

    // Number of cells in each dimension for domain.
    int globalorigin[3], size[3];
    GetLogicalExtents(dom, globalorigin, size);

    debug4 << "Number of cells in domain: (" << size[0] << ", "
           << size[1] << ", " << size[2] << ")\n";

    // Length of each side of the domain.
    double domainEdgeLengthX = double(size[0]) * cellx;
    double domainEdgeLengthY = double(size[1]) * celly;
    double domainEdgeLengthZ = double(size[2]) * cellz;

    double Tx = double(globalorigin[0] - 1) / double(NX);

//debug4 << "double Tx = double(domloc[0] - 1) / double(NX - 1);" << endl
//       << "double Tx = double("<<domloc[0]<<" - 1) / double(" << NX << " - 1);\n"
//       << "double Tx = " << Tx << endl;

    double Ty = double(globalorigin[1] - 1) / double(NY);

//debug4 << "double Ty = double(domloc[2] - 1) / double(NY - 1);" << endl
//       << "double Ty = double("<<domloc[2]<<" - 1) / double(" << NY << " - 1);\n"
//       << "double Ty = " << Ty << endl;

    double Tz = double(globalorigin[2] - 1) / double(NZ);
//debug4 << "double Tz = double(domloc[4] - 1) / double(NZ - 1);" << endl
//       << "double Tz = double("<<domloc[4]<<" - 1) / double(" << NZ << " - 1);\n"
//       << "double Tz = " << Tz << endl;

    // Origin of the domain.
    double domainOriginX = Tx * (lenx / 2.) + (1. - Tx) * (-lenx / 2.);
    double domainOriginY = Ty * (leny / 2.) + (1. - Ty) * (-leny / 2.);
    double domainOriginZ = Tz * lenz;

    extents[0] = domainOriginX;
    extents[1] = extents[0] + domainEdgeLengthX;
    extents[2] = domainOriginY;
    extents[3] = extents[2] + domainEdgeLengthY;
    extents[4] = domainOriginZ;
    extents[5] = extents[4] + domainEdgeLengthZ;
//    print_array(debug4_real, "extents", extents, 6);
#endif
}

// ****************************************************************************
// Method: PF3DFileFormat::GetMesh
//
// Purpose: 
//   Gets the named mesh and returns it as a vtkDataSet.
//
// Arguments:
//   meshName : The name of the mesh.
//
// Returns:    a vtkDataSet containing the mesh.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 1 15:55:37 PST 2004
//
// Modifications:
//
// ****************************************************************************

vtkDataSet *
PF3DFileFormat::GetMesh(int dom, const char *meshName)
{
    //
    // Calculate the extents of the domain.
    //
    double extents[6];
    GetExtents(dom, extents);

    //
    // The number of nodes in each dimension.
    //
    int base_index[3], size[3];
    GetLogicalExtents(dom, base_index, size);
    size[0]++;
    size[1]++;
    size[2]++;

    //
    // Populate the coordinates.
    //
    vtkFloatArray *coords[3];
    for (int i = 0 ; i < 3 ; i++)
    {
        // Default number of components for an array is 1.
        coords[i] = vtkFloatArray::New();
        coords[i]->SetNumberOfTuples(size[i]);
        int i2 = i * 2;
        int i21 = i2 + 1;
        for (int j = 0 ; j < size[i] ; j++)
        {
            float t = float(j) / float(size[i] - 1);
            float c = (1.-t)*extents[i2] + t*extents[i21];
            coords[i]->SetComponent(j, 0, c);
        }
    }
    vtkRectilinearGrid *grid = vtkRectilinearGrid::New(); 
    grid->SetDimensions(size);
    grid->SetXCoordinates(coords[0]);
    coords[0]->Delete();
    grid->SetYCoordinates(coords[1]);
    coords[1]->Delete();
    grid->SetZCoordinates(coords[2]);
    coords[2]->Delete();

    //
    // Determine the indices of the mesh within its group.  Add that to the
    // VTK dataset as field data.
    //
    vtkIntArray *arr = vtkIntArray::New();
    arr->SetNumberOfTuples(3);
    arr->SetValue(0, base_index[0]);
    arr->SetValue(1, base_index[1]);
    arr->SetValue(2, base_index[2]);
    arr->SetName("base_index");
    grid->GetFieldData()->AddArray(arr);
    arr->Delete();

    return grid;
}

// ****************************************************************************
// Method: PF3DFileFormat::GetVariableIndex
//
// Purpose: 
//   Returns the named scalar variable.
//
// Arguments:
//   varName : The variable name.
//
// Returns:    A vtkDataArray containing the variable's data.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 1 15:54:59 PST 2004
//
// Modifications:
//
// ****************************************************************************


int
PF3DFileFormat::GetVariableIndex(const std::string &name) const
{
    int retval = -1;
    for(int i = 0; i < glob_nams.size(); ++i)
    {
        if(name == glob_nams[i])
        {
            retval = i;
            break;
        }
    }

    return retval;
}

// ****************************************************************************
// Method: operator <<
//
// Purpose: 
//   Prints out the contents of a bowinfo object.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 12 14:46:42 PST 2004
//
// Modifications:
//   
// ****************************************************************************

#ifdef DEBUG_PRINT
ostream &
operator << (ostream &os, bowinfo binf)
{
    os << "bowinfo = {" << endl;
    os << "\tversion = " << binf->version << endl;
    os << "\tnumbow = " << binf->numbow << endl;
    os << "\t";
    print_array(os, "size", binf->size, binf->numbow, 8,true,false);
    os << "\t";
    print_array(os, "offset", binf->offset, binf->numbow, 8,true,false);
    os << "\t";
    print_array(os, "notran", binf->notran, binf->numbow, 8,true,false);
    os << "\t";
    print_array(os, "ti", binf->ti, binf->numbow, 8,true,false);
    os << "\t";
    print_array(os, "bi", binf->bi, binf->numbow, 8,true,false);
    os << "\t";
    print_array(os, "vi", binf->vi, binf->numbow, 8,true,false);
    os << "\t";
    print_array(os, "xs", binf->xs, binf->numbow, 8,true,false);
    os << "\t";
    print_array(os, "ys", binf->ys, binf->numbow, 8,true,false);
    os << "\t";
    print_array(os, "zs", binf->zs, binf->numbow, 8,true,false);
    os << "\t";
    print_array(os, "x0", binf->x0, binf->numbow, 8,true,false);
    os << "\t";
    print_array(os, "y0", binf->y0, binf->numbow, 8,true,false);
    os << "\t";
    print_array(os, "z0", binf->z0, binf->numbow, 8,true,false);
    os << "\t";
    print_array(os, "v0", binf->v0, binf->numbow, 8,true,false);
    os << "\t";
    print_array(os, "v1", binf->v1, binf->numbow, 8,true,false);
    os << "}" << endl;

    return os;
}
#endif

// ****************************************************************************
// Method: my_bow_alloc
//
// Purpose: 
//   Called to allocate memory for the bow library.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 12 14:46:01 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void *my_bow_alloc(void *opaque, size_t size)
{
    long nLongs = (size / sizeof(long)) + (((size % sizeof(long)) > 0) ? 1 : 0);
    void *retval = (void *)(new long[nLongs]);
//        debug4 << "my_bow_alloc2: alloc " << retval << "(" << size
//               << " bytes,  allocated " << nLongs << " longs)" << endl;

    return retval;
}

// ****************************************************************************
// Method: my_bow_free
//
// Purpose: 
//   Called to deallocate memory for the bow library.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 12 14:46:01 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void my_bow_free(void *opaque, void *ptr)
{
    if(ptr != 0) 
    {
//        debug4 << "my_bow_free: freeing " << ptr << endl;
        long *lptr =  (long*)ptr;
        delete [] lptr;
    }
}

// ****************************************************************************
// Method: PF3DFileFormat::GetBOFKey
//
// Purpose: 
//   Creates a key for the desired BOF.
//
// Arguments:
//   realDomain : The index of the domain whose BOF we want.
//   varName    : The name of the variable whose BOF we want.
//
// Returns:    A key containing the domain and varname.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 12 14:44:24 PST 2004
//
// Modifications:
//   
// ****************************************************************************

std::string
PF3DFileFormat::GetBOFKey(int realDomain, const char *varName) const
{
    // Create a key for the brick of floats.
    char key[200];
    SNPRINTF(key, 200, "BOF_%04d_%s", realDomain, varName);
    return std::string(key);
}

// ****************************************************************************
// Method: PF3DFileFormat::GetBOF
//
// Purpose: 
//   Returns a pointer to the BOF for the specified real domain.
//
// Arguments:
//   realDomain : The index of the domain whose BOF we want.
//   varName    : The name of the variable whose BOF we want.
//
// Returns:    A pointer to the BOF or 0 if we can't get it.
//
// Note:       This routine caches the BOF in the database's variable cache
//             for the current time state.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 12 14:41:16 PST 2004
//
// Modifications:
//   Brad Whitlock, Thu Dec 1 14:52:15 PST 2005
//   I added code to serve up 3D char arrays as floats. I've also disabled
//   BOF caching with the CACHE_BOF macro.
//
//   Brad Whitlock, Thu Jun 22 16:46:22 PST 2006
//   Added support for multiple domains in a file.
//
// ****************************************************************************

PF3DFileFormat::BOF *
PF3DFileFormat::GetBOF(int realDomain, const char *varName)
{
    BOF *retval = 0;
    const char *mName = "PF3DFileFormat::GetBOF: ";

    //
    // Get the cached BOF if it exists.
    //
    std::string key(GetBOFKey(realDomain, varName));
    void_ref_ptr vr = cache->GetVoidRef(key.c_str(), BOF_KEY, timestep,
                                        realDomain);

    if(*vr != 0)
    {
        debug4 << mName << "Found a cached BOF for "
               << key.c_str() << " at: " << (*vr) << endl;
        retval = (BOF *)(*vr);
    }
    else
    {
        debug4 << mName << "Must read data for " << varName << endl;

        //
        // Get the file object for the realDomain.
        //
        PDBFileObject *domainPDB = GetDomainFileObject(realDomain);
 
        if(domainPDB)
        {
            TypeEnum dataType = NO_TYPE;
            int nTotalElements = 0;
            int *dims = 0;
            int nDims = 0;
            void *data = 0;

            int varIndex = GetVariableIndex(varName);

            // If there is a domain prefix then we need to add that to
            // the variables that we want to read.
            std::string domainDir(master.Get_dom_prefix_for_domain(realDomain));
            std::string realVarName(domainDir + int_nams[varIndex]);

            //
            // Read the data from the PDB file.
            // 
            data = domainPDB->ReadValues(realVarName.c_str(),
                                         &dataType, &nTotalElements,
                                         &dims, &nDims, 0);

            debug4 << mName << "data dims = {";
            for(int i = 0; i < nDims; ++i)
                debug4 << dims[i] << ", ";
            debug4 << "}\n";

            if(data != 0)
            {
                if(dataType == CHARARRAY_TYPE)
                {
                    if(nDims == 1)
                    {
                        bowglobal bg;   // Session record for bow to use.
                        bowinfo   binf; // Information record from bow buffer.

                        // Start bow session using custom memory routines.
                        bg = bowglobal_create(my_bow_alloc, my_bow_free, 0);

                        // Get the information for the brick of wavelets.
                        binf = bow_getbowinfo(bg, (char*)data);
                        if(binf == 0)
                        {
                            debug4 << mName << "bow_getbowinfo returned 0!"
                                   << endl;
                        }
                        else
                        {
                            debug4 << mName << "Getting BOW from data"
                                   << endl;
#ifdef DEBUG_PRINT
                            debug4 << mName << binf;
#endif
    
                            //
                            // Decompress the brick of wavelets back into a
                            // brick of floats.
                            //
                            float *bof = bow2bof(bg, (char *)data, 0);

                            if(bof != 0)
                            {
                                // Create a BOF object to contain the BOF.
                                retval = new BOF;
                                retval->size[0] = binf->xs[0];
                                retval->size[1] = binf->ys[0];
                                retval->size[2] = binf->zs[0];
                                retval->data = bof;

                                // If the variable needs to have exp() applied,
                                // do that now.
                                if(apply_exp[varIndex] > 0)
                                {
                                    debug4 << mName << "Applying exp()" << endl;
                                    float *fptr = bof;
                                    int nvals = retval->size[0] *
                                        retval->size[1] * retval->size[2];
#ifdef DEBUG_PRINT
                                    float datamin = FLT_MAX, datamax = -FLT_MAX;
#endif
                                    for(int i = 0; i < nvals; ++i)
                                    {
                                        fptr[i] = exp(fptr[i]);
#ifdef DEBUG_PRINT
                                        datamin = (datamin < fptr[i]) ? datamin : fptr[i];
                                        datamax = (datamax > fptr[i]) ? datamax : fptr[i];
#endif
                                    }
#ifdef DEBUG_PRINT
                                    debug4 << "BOF " << realDomain << "'s minmax=["
                                           << datamin << ", " << datamax << "]"
                                           << endl;
#endif
                                }

#ifdef CACHE_BOF
                                // Store the BOF in the cache.
                                void_ref_ptr vr2 = void_ref_ptr(retval,
                                    BOF::Destruct);
                                cache->CacheVoidRef(key.c_str(), BOF_KEY, timestep,
                                    realDomain, vr2);
#endif
                            }
                            else
                            {
                                debug4 << mName << "BOF == 0!" << endl;
                            }
                        }
                    }
                    else if(nDims == 3)
                    {
                        long N = dims[0] * dims[1] * dims[2];
                        float *fptr = new float[N];
                        char *cptr = (char *)data;

                        retval = new BOF;
                        retval->size[0] = dims[0];
                        retval->size[1] = dims[1];
                        retval->size[2] = dims[2];
                        retval->data = fptr;

                        double vmax = 1.;
                        std::string varMax(domainDir + varName);
                        varMax += "max";
                        if(strstr(int_nams[varIndex].c_str(), "byt") != 0 &&
                           domainPDB->GetDouble(varMax.c_str(), &vmax))
                        {
                             debug4 << mName << "Converting byte-scaled data to float" << endl;

                             // Copy the data into the float array while reconstituting it.
                             for(long n = 0; n < N; ++n)
                             {
                                float cval = float(*cptr++) / 255.f;
                                *fptr++ = cval * cval * vmax;
                             }
                        }
                        else
                        {
                            debug4 << mName << "Converting char data to float" << endl;

                            // Copy the data into a float array.
                            for(long n = 0; n < N; ++n)
                                *fptr++ = float(*cptr++);
                        }

#ifdef CACHE_BOF
                        // Store the BOF in the cache.
                        void_ref_ptr vr2 = void_ref_ptr(retval,
                            BOF::Destruct);
                        cache->CacheVoidRef(key.c_str(), BOF_KEY, timestep,
                            realDomain, vr2);
#endif
                    }
                    else
                    {
                        debug4 << mName << "Unsupported array shape. nDims="
                               << nDims << endl;
                    }
                }
                else if(dataType == FLOATARRAY_TYPE && nDims == 3)
                {
                    debug4 << mName << "Using float data directly." << endl;
                    retval = new BOF;
                    retval->size[0] = dims[0];
                    retval->size[1] = dims[1];
                    retval->size[2] = dims[2];
                    retval->data = (float *)data;

                    // Make the BOF own the float data.
                    data = 0;
                    dataType = NO_TYPE;

#ifdef CACHE_BOF
                    // Store the BOF in the cache.
                    void_ref_ptr vr2 = void_ref_ptr(retval,
                        BOF::Destruct);
                    cache->CacheVoidRef(key.c_str(), BOF_KEY, timestep,
                        realDomain, vr2);
#endif
                }
                else
                {
                    debug4 << "The data was read but it is not a supported "
                           << "type (" << int(dataType) << ")" << endl;
                }

                // Free the data that was read from the PDB file.
                pdb_free_void_mem(data, dataType);
                delete [] dims;
            }
            else
            {
                debug4 << mName << "No data was read!" << endl;
            }
        }
        else
        {
            debug4 << mName << "Could not create the PDB file object needed "
                "to read domain " << realDomain << endl;
        }
    }

    return retval;
}

// ****************************************************************************
// Function: CopyBlockData
//
// Purpose: 
//   Copies a 3D rectangular block of data from one array to another array.
//
// Arguments:
//   dest  : The pointer to the destination array.
//   src   : The pointer to the source array.
//   size  : The size of the src array in X,Y,Z.
//   start : The starting index of the copying.
//   end   : The ending index of the copying.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 12 14:38:33 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
CopyBlockData(float *dest, const float *src, int *size, int *start, int *end)
{
    long xysize = size[0] * size[1];

    for(int z = start[2]; z < end[2]; ++z)
    {
        const float *xyplane = src + (xysize * z);
        for(int y = start[1]; y < end[1]; ++y)
        {
            const float *row = xyplane + y * size[0] + start[0];
            for(int x = start[0]; x < end[0]; ++x)
                *dest++ = *row++;
        }
    }
}

// ****************************************************************************
// Method: PF3DFileFormat::GetVar
//
// Purpose: 
//   Returns the named scalar variable.
//
// Arguments:
//   dom     : The domain index for which we're returning a data array.
//   varName : The variable name.
//
// Returns:    A vtkDataArray containing the variable's data.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 1 15:54:59 PST 2004
//
// Modifications:
//
// ****************************************************************************

vtkDataArray *
PF3DFileFormat::GetVar(int dom, const char *varName)
{
    vtkDataArray *scalars = 0;

    // Get the index of the real domain from dom.
    int realDomain = GetRealDomainIndex(dom);

    // Get the requested domain's brick of floats.
    BOF *bof = GetBOF(realDomain, varName);

    if(bof != 0)
    {
        //
        // Get the size of the whole real domain. 
        //
        int realglobalorigin[3], realdomainsize[3];
        GetLogicalExtents(realDomain, realglobalorigin, realdomainsize);

        //
        // Get the size of the dom.
        //
        int globalorigin[3], domainsize[3];
        GetLogicalExtents(dom, globalorigin, domainsize);

        //
        // Figure out where within the real domain's data we can find
        // the dom's data.
        //
        int start[3];
        start[0] = (bof->size[0] - realdomainsize[0]) / 2;
        start[1] = (bof->size[1] - realdomainsize[1]) / 2;
        start[2] = (bof->size[2] - realdomainsize[2]) / 2;
        int datastart[3];
        datastart[0] = globalorigin[0] - realglobalorigin[0] + start[0];
        datastart[1] = globalorigin[1] - realglobalorigin[1] + start[1];
        datastart[2] = globalorigin[2] - realglobalorigin[2] + start[2];
        int end[3];
        end[0] = datastart[0] + domainsize[0];
        end[1] = datastart[1] + domainsize[1]; 
        end[2] = datastart[2] + domainsize[2];

#ifdef DEBUG_PRINT
        debug4 << "BOF = " << (void*)bof << endl;
        debug4 << "Copying subblock of BOF into VTK object" << endl;
        if(DebugStream::Level4())
        {
            print_array(DebugStream::Stream4(), "bof->size", bof->size, 3, 8, true, false);
            print_array(DebugStream::Stream4(), "domainsize", domainsize, 3, 8, true, false);
            print_array(DebugStream::Stream4(), "start", start, 3, 8, true, false);
            print_array(DebugStream::Stream4(), "end", end, 3, 8, true, false);
        }
#endif

        //
        // Extract just the parts of the data that we care about.
        //
        int nels = domainsize[0] * domainsize[1] * domainsize[2];
        vtkFloatArray *fscalars = vtkFloatArray::New();
        fscalars->SetNumberOfTuples(nels);
        float *dest = (float *)fscalars->GetVoidPointer(0);
        CopyBlockData(dest, bof->data, bof->size, start, end);

        // It all succeeded so set the return value.
        scalars = fscalars;

#ifndef CACHE_BOF
        // Delete the BOF.
        delete bof;
#endif
    }

    return scalars;
}

// ****************************************************************************
// Method: PF3DFileFormat::PopulateIOInformation
//
// Purpose: 
//   Tells VisIt which domains are grouped together to improve load balancing.
//
// Arguments:
//   ioInfo : The I/O information that we're going to populate.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 22 17:38:37 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
PF3DFileFormat::PopulateIOInformation(avtIOInformation &ioInfo)
{
    const char *mName = "avtPF3DFileFormat::PopulateIOInformation: ";
    int nGroups = master.Get_num_grp_size();

    if(nGroups > 0)
    {
        vector<vector<int> > groups;
        for(int i = 0; i < nGroups; ++i)
        {
            int grp_size = master.Get_grp_size(i);
            const long *grp_members = master.Get_grp_members(i);
            if(grp_size > 0 && grp_members != 0)
            {
                vector<int> thisGroup;
                debug4 << mName << "Adding I/O group " << i << " {";

                for(int j = 0; j < grp_size; ++j)
                {
                    debug4 << grp_members[j] << ", ";
                    thisGroup.push_back(int(grp_members[j]));
                }
                debug4 << "}" << endl;
                if(thisGroup.size() > 0)
                    groups.push_back(thisGroup);
            }
        }

        ioInfo.SetNDomains(master.GetNDomains());
        ioInfo.AddHints(groups);
    }
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: PF3DFileFormat::MasterInformation::MasterInformation
//
// Purpose: 
//   Constructor for the MasterInformation class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 12 14:32:36 PST 2004
//
// Modifications:
//   Brad Whitlock, Wed Nov 30 16:05:37 PST 2005
//   I added the members vector.
//
// ****************************************************************************

PF3DFileFormat::MasterInformation::MasterInformation() : members()
{
    nDomains = 0;
}

// ****************************************************************************
// Method: PF3DFileFormat::MasterInformation::~MasterInformation
//
// Purpose: 
//   Destructor for the MasterInformation class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 12 14:32:53 PST 2004
//
// Modifications:
//   Brad Whitlock, Fri Dec 2 11:30:00 PDT 2005
//   Added deletion of members.
//
// ****************************************************************************

PF3DFileFormat::MasterInformation::~MasterInformation()
{
    for(int i = 0; i < members.size(); ++i)
        delete members[i];
}

// ****************************************************************************
// Method: PF3DFileFormat::MasterInformation::GetNDomains
//
// Purpose: 
//   Returns the number of domains in the master.
//
// Returns:    The number of domains.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 12 14:33:10 PST 2004
//
// Modifications:
//   
// ****************************************************************************

int
PF3DFileFormat::MasterInformation::GetNDomains() const
{
    return nDomains;
}

// ****************************************************************************
// Method: PF3DFileFormat::MasterInformation::Read
//
// Purpose: 
//   Reads the master structure from the PDB file.
//
// Arguments:
//   pdb : The PDB file object used to read the master information.
//
// Returns:    True if successful; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 12 14:33:39 PST 2004
//
// Modifications:
//   Brad Whitlock, Fri Dec 2 11:30:16 PDT 2005
//   Rewrote for dynamic master file structure.
//
//   Jeremy Meredith, Thu Aug  7 15:59:40 EDT 2008
//   Assume PDB won't modify our string literals, so cast to char* as needed.
//
//   Mark C. Miller, Tue Apr 28 11:05:17 PDT 2009
//   Fixed compiler symbol redefinition error for a HAVE_PDB_PROPER build.
//
//   Eric Brugger, Wed May  6 11:09:23 PDT 2009
//   Added code to handle a new version of the master information that is 
//   more compact than before. Steve Langer wrote the code and I debugged it.
//   Steve also removed a redundant line of code.
//
//   Eric Brugger, Thu Jun 18 14:45:48 PDT 2009
//   Doubled the size of the buffer to read __@history, to avoid an out of
//   bounds memory write deep down in a call to PD_read.  Valgrind shows it
//   as the source of the out of bounds memory write, but I couldn't find
//   the problem with with a few hours of debugging.
//
// ****************************************************************************

bool
PF3DFileFormat::MasterInformation::Read(PDBFileObject *pdb)
{
    const char *mName = "PF3DFileFormat::MasterInformation::Read:";
    bool retval = false;

    if(pdb->GetInteger("mp_size", &nDomains))
    {
        //
        // Stage 1: Get the history structure.
        //
        char s_type[100];
        defstr *ds;
#ifdef HAVE_PDB_PROPER
        syment *s = PD_query_entry(pdb->filePointer(), "__@history", NULL);
        if(s == NULL)
        {
            debug4 << mName << "Can't query __@history" << endl;
            return false;
        }

        strcpy(s_type, s->type);
        ds = PD_inquire_type(pdb->filePointer(), s_type);
#else
        // Use the name that we know it to be since Silo's PDB does not seem to
        // have any of the PDB query functions.
        strcpy(s_type, "__");
#endif
        ds = PD_inquire_type(pdb->filePointer(), s_type);
        if(ds == NULL)
        {
            debug4 << mName << "Can't get type information for " << s_type << endl;
            return false;
        }
        int bufferSize = 0;
        for(s_memdes *m = ds->members; m != NULL; m = m->next)
        {
            if(m->next == NULL)
            {
                int mSize = 1;
                for(s_dimdes *dimdes = m->dimensions;
                    dimdes != 0;
                    dimdes = dimdes->next)
                {
                    mSize *= (dimdes->index_max - dimdes->index_min + 1);
                }

                if(strcmp(m->base_type, "char") == 0)
                    bufferSize = m->member_offs + mSize * sizeof(char);
                else if(strcmp(m->base_type, "short") == 0)
                    bufferSize = m->member_offs + mSize * sizeof(short);
                else if(strcmp(m->base_type, "int") == 0 ||
                        strcmp(m->base_type, "integer") == 0)
                    bufferSize = m->member_offs + mSize * sizeof(int);
                else if(strcmp(m->base_type, "long") == 0)
                    bufferSize = m->member_offs + mSize * sizeof(long);
                else if(strcmp(m->base_type, "float") == 0)
                    bufferSize = m->member_offs + mSize * sizeof(float);
                else if(strcmp(m->base_type, "double") == 0)
                    bufferSize = m->member_offs + mSize * sizeof(double);
                else
                    debug4 << mName << "Unknown type! " << m->base_type << endl;

                // Add maximum amount for struct alignment. This does not
                // affect how the bytes are read; only how the buffer is
                // sized. We are guaranteed to over-allocate by a little
                // to ensure that the bytes that are converted on read do
                // not go off the end of the array.
                bufferSize += 8;
            }
        }
        // Multiply by 2, to avoid an out of bounds memory write deep
        // down in the call to PD_read.  Valgrid shows it as the source
        // of the out of bounds memory write, but I couldn't find the
        // problem with a few hours of debugging.
        bufferSize = bufferSize * 2;
        debug4 << mName << "Calculated a buffer size of: " << bufferSize
               << " bytes" << endl;

        //
        // Stage 2: Allocate the memory for the history structure and read it.
        //
        // 
        int nLongs = bufferSize / sizeof(long) +
            (((bufferSize % sizeof(long)) > 0) ? 1 : 0);
        void *buffer = (void *) new long[nLongs];
        retval = (PD_read(pdb->filePointer(), (char*)"__@history", buffer)
                  == TRUE);

        //
        // Stage 3: Convert the data from the history structure into items
        //          in the members vector.
        //
        if(!retval)
        {
            debug4 << mName << " Could not read __@history."
                   << PDBLIB_ERRORSTRING << endl;
        }
        else 
        {
            // Now iterate through the fields in the __@history variable type
            // And populate the member data. We'll just promote everything
            // that's not char to double and long. This will make some access
            // methods easier to write later.
            char *mptr = (char *)buffer;
            for(s_memdes *m = ds->members; m != NULL; m = m->next)
            {
                MemberData *member = new MemberData;
                member->name = m->name;
                
                debug4 << mName
                       << "name=" << m->name << ", "
                       << "type=" << m->type << ", "
                       << "base_type=" << m->base_type << ", "
                       << "member_offs=" << m->member_offs << ", "
                       << "number=" << m->number
                       << endl;

                // Save off the number of dimensions.
                int mSize = 1;
                if(m->dimensions == 0)
                    member->dims[member->ndims++] = 1;
                else
                {
                    for(s_dimdes *dimdes = m->dimensions;
                        dimdes != 0;
                        dimdes = dimdes->next)
                    {
                        int thisms = (dimdes->index_max - dimdes->index_min + 1);
                        mSize *= thisms;
                        if(member->ndims < 3)
                            member->dims[member->ndims++] = thisms;
                    }
                }

                //
                // Save off the data in the member and do conversions.
                //
                if(strcmp(m->base_type, "char") == 0)
                {
                    char *cptr = new char[mSize];
                    memcpy(cptr, mptr, mSize);
                    member->dataType = CHARARRAY_TYPE;
                    member->data = (void *)cptr;

                    mptr += mSize * sizeof(char);
                }
                else if(strcmp(m->base_type, "short") == 0)
                {
                    // Advance offset if necessary.
                    for(; ((unsigned long)(mptr)) % pdb->filePointer()->
                        align->short_alignment != 0; ++mptr);

                    short *src = (short *)mptr;
                    long *dest = new long[mSize];

                    member->dataType = LONGARRAY_TYPE;
                    member->data = (void *)dest;
                    for(int i = 0; i < mSize; ++i)
                        *dest++ = long(*src++);

                    mptr += mSize * sizeof(short);
                }
                else if(strcmp(m->base_type, "integer") == 0 ||
                        strcmp(m->base_type, "int") == 0)
                {
                    // Advance offset if necessary.
                    for(; ((unsigned long)(mptr)) % pdb->filePointer()->
                        align->int_alignment != 0; ++mptr);

                    int *src = (int *)mptr;
                    long *dest = new long[mSize];

                    member->dataType = LONGARRAY_TYPE;
                    member->data = (void *)dest;
                    for(int i = 0; i < mSize; ++i)
                        *dest++ = long(*src++);

                    mptr += mSize * sizeof(int);
                }
                else if(strcmp(m->base_type, "long") == 0)
                {
                    // Advance offset if necessary.
                    for(; ((unsigned long)(mptr)) % pdb->filePointer()->
                        align->long_alignment != 0; ++mptr);

                    long *src = (long *)mptr;
                    long *dest = new long[mSize];

                    member->dataType = LONGARRAY_TYPE;
                    member->data = (void *)dest;
                    for(int i = 0; i < mSize; ++i)
                        *dest++ = *src++;

                    mptr += mSize * sizeof(long);
                }
                else if(strcmp(m->base_type, "float") == 0)
                {
                    // Advance offset if necessary.
                    for(; ((unsigned long)(mptr)) % pdb->filePointer()->
                        align->float_alignment != 0; ++mptr);

                    float *src = (float *)mptr;
                    double *dest = new double[mSize];

                    member->dataType = DOUBLEARRAY_TYPE;
                    member->data = (void *)dest;
                    for(int i = 0; i < mSize; ++i)
                        *dest++ = double(*src++);

                    mptr += mSize * sizeof(float);
                }
                else if(strcmp(m->base_type, "double") == 0)
                {
                    // Advance offset if necessary.
                    for(; ((unsigned long)(mptr)) % pdb->filePointer()->
                        align->double_alignment != 0; ++mptr);

                    double *src = (double *)mptr;
                    double *dest = new double[mSize];

                    member->dataType = DOUBLEARRAY_TYPE;
                    member->data = (void *)dest;
                    for(int i = 0; i < mSize; ++i)
                        *dest++ = *src++;

                    mptr += mSize * sizeof(double);
                }
                else
                {
                    debug4 << mName << "Unknown type: " << m->base_type << endl;
                    mptr += mSize;
                }

                // Now that we've populated the member, add it to the list of members.
                members.push_back(member);
            }

#ifdef DEBUG_PRINT
            if(DebugStream::Level4())
                operator << (DebugStream::Stream4());
#endif            
        }

        // Free the buffer
        long *lptr = (long *)buffer;
        delete [] lptr;
    }
    else
    {
        debug4 << mName << "Could not read mp_size!" << endl;
    }

    //
    // Newer pf3d viz dumps require that some arrays be computed from
    // other data in the file. "Fix" the arrays now so that the member
    // information is in "canonical form" before returning from this function.
    //
    for (int i = 0; i < members.size(); ++i)
    {
        std::string name = members[i]->name;
        int ndims = members[i]->ndims;
        int ndom = nDomains;
        int dims[3];
        memcpy(dims, members[i]->dims, 3*sizeof(int));
        if (name == "xyzloc")
        {
            //
            // If xyzloc is a 2D array, leave it as is.  If it is a 1D
            // array it contains [dx,dx,dy,dy,dz,dz].  Create a 2D array
            // xyzloc=old_xyzloc_base*domloc and set member->data to that.
            //
            if (ndims != 2)
            {
                // Make an array of the proper size
                double *xyzloc = new double[6*ndom*sizeof(double)];
                // get a pointer to the domloc array
                long *domloc = 0;
                for (int it = 0; it < members.size(); ++it)
                {
                    std::string tname = members[it]->name;
                    if (tname == "domloc")
                    {
                        domloc= (long *) members[it]->data;
                        break;
                    }
                }
                if (domloc == NULL)
                {
                    debug4 << mName << " domloc is not present in the master file" << endl;
                    return false;
                }
                double xyzbase[6];
                memcpy(xyzbase, members[i]->data, 6*sizeof(double));
                for (int id = 0; id < ndom; ++id)
                {
                    long nn = id*6L;
                    xyzloc[nn] =   xyzbase[0]*domloc[nn];
                    xyzloc[nn+1] = xyzbase[1]*domloc[nn+1];
                    xyzloc[nn+2] = xyzbase[2]*domloc[nn+2];
                    xyzloc[nn+3] = xyzbase[3]*domloc[nn+3];
                    xyzloc[nn+4] = xyzbase[4]*domloc[nn+4];
                    xyzloc[nn+5] = xyzbase[5]*domloc[nn+5];
                }
                double *old = (double *) members[i]->data;
                members[i]->data = xyzloc;
                pdb_free_void_mem((void *)old, members[i]->dataType);
                members[i]->ndims = 2;
                members[i]->dims[0] = 6;
                members[i]->dims[1] = ndom;
            }
        }
        else if (name == "dom_prefix")
        {
            if (ndims != 2)
            {
                //
                // If dom_prefix is not a 2D array, the domain prefixes must
                // be computed.  Domain names are of the form    /domain%d/
                // Always allow for 6 digit domain numbers, and increase the
                // length if there are more than a million domains.  Leave one
                // character for a trailing zero.
                //
                int len = 15;
                int maxdom = 999999;
                while (ndom > maxdom)
                {
                    len++;
                    maxdom = 10*maxdom+9;
                }
                char *prefix = new char[len*ndom];
                char *domnam = new char[len];
                for (int j = 0; j < ndom; j++)
                {
                    sprintf(domnam, "/domain%d/", j);
                    strncpy(prefix+j*len, domnam, len);
                }
                char *old = (char *) members[i]->data;
                members[i]->data = prefix;
                pdb_free_void_mem((void *)old, members[i]->dataType);
                members[i]->ndims = 2;
                members[i]->dims[0] = len;
                members[i]->dims[1] = ndom;
            }
        }
        else if (name == "viz_nams")
        {
            if (ndims == 2)
            {
                //
                // If viz_nams is 2D, it just has the file names and all
                // files are in the directory given by common_vizdir.
                // Visit only uses the portion of the directory name
                // after "/viz/", so we will create a directory name with
                // information to satisfy that need.  Create a 3D array
                // long enough to hold the file names, create a dummy
                // directory name no longer than the file names, copy in
                // the file names, and set all the directories to the
                // dummy value.
                //
                char *oldnam = (char *) members[i]->data;
                int len = dims[0];
                if (len < 16)
                {
                    debug4 << mName << " file names are too short." << endl;
                }
                char *viz_nams = new char[len*2*ndom];
                char *vizdir = new char[len];
                //
                // Set the directory name.  We need the cycle number to
                // set this.  We are getting the cycle number from the last
                // digit sequence in the filename, since the file does not
                // appear to have that information in it.
                //
                int cycle = 0;
                if (pdb->GetName().length() > 0)
                {
                    const char *filename = pdb->GetName().c_str();
                    int iCycle = pdb->GetName().length()-1;
                    while (iCycle >= 0 && !isdigit(filename[iCycle]))
                        iCycle--;
                    while (iCycle >= 0 && isdigit(filename[iCycle]))
                        iCycle--;
                    iCycle++;
                    sscanf(&filename[iCycle], "%d", &cycle);
                }
                sprintf(vizdir, "/viz/vs%d/", cycle);
                for (int j = 0; j < ndom; j++)
                {
                    long nd = j*2*len;
                    // always use the same value for the directory
                    strncpy(viz_nams+nd, vizdir, len);
                    // file name is the second element for each domain
                    strncpy(viz_nams+nd+len, oldnam+j*len, len);
                }
                members[i]->data = (void *) viz_nams;
                pdb_free_void_mem((void *)oldnam, members[i]->dataType);
                members[i]->ndims = 3;
                members[i]->dims[0] = len;
                members[i]->dims[1] = 2;
                members[i]->dims[2] = ndom;
            }
        }
    }

    return retval;
}

// ****************************************************************************
// Method: PF3DFileFormat::MasterInformation::FindMember
//
// Purpose: 
//   Finds the struct member with the given name.
//
// Arguments:
//   name : The name of the member to find.
//
// Returns:    A pointer to the member data.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 2 13:29:59 PST 2005
//
// Modifications:
//   
// ****************************************************************************

const PF3DFileFormat::MasterInformation::MemberData *
PF3DFileFormat::MasterInformation::FindMember(const std::string &name) const
{
    for(int i = 0; i < members.size(); ++i)
    {
        if(name == members[i]->name)
            return members[i];
    }

    debug4 << "PF3DFileFormat::MasterInformation::FindMember: No member called: "
           << name.c_str() << endl;

    return 0;
}

// ****************************************************************************
// Method: PF3DFileFormat::MasterInformation::operator << 
//
// Purpose: 
//   Prints the master struct to an ostream.
//
// Arguments:
//   os : The ostream to which we want to print.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 12 14:36:37 PST 2004
//
// Modifications:
//   Brad Whitlock, Fri Dec 2 13:31:12 PST 2005
//   I rewrote the method to work with the more general structure.
//
// ****************************************************************************

ostream &
PF3DFileFormat::MasterInformation::operator << (ostream &os)
{
#ifdef DEBUG_PRINT
    for(int i = 0; i < members.size(); ++i)
        members[i]->Print(os);
#endif

    return os;
}

//
// Methods to return values and pointers to the various arrays in the master.
//

double
PF3DFileFormat::MasterInformation::Get_tnowps() const
{
    double val = 0.f;
    const MemberData *m = FindMember("tnowps");
    if(m != 0)
        val = *((double *)(m->data));

    return val;
}

const long *
PF3DFileFormat::MasterInformation::Get_domloc() const
{
    const long *val = 0;
    const MemberData *m = FindMember("domloc");
    if(m != 0)
        val = (const long *)(m->data);

    return val;
}

const double *
PF3DFileFormat::MasterInformation::Get_xyzloc() const
{
    const double *val = 0;
    const MemberData *m = FindMember("xyzloc");
    if(m != 0)
        val = (const double *)(m->data);

    return val;
}

const char *
PF3DFileFormat::MasterInformation::Get_visnams() const
{
    const char *val = 0;
    const MemberData *m = FindMember("viz_nams");
    if(m == 0)
        m = FindMember("visnams");

    if(m != 0)
        val = (const char *)(m->data);

    return val;
}

const char *
PF3DFileFormat::MasterInformation::Get_visname_for_domain(int dom, int comp) const
{
    const MemberData *m = FindMember("viz_nams");
    if(m == 0)
        m = FindMember("visnams");

    const char *visnames_for_domain = 0;
    if(m != 0)
    {
        const char *visnam_base = (const char *)m->data;
        int visnam_size = m->dims[0];

        visnames_for_domain = visnam_base + (2 * visnam_size) * dom;
        visnames_for_domain += (comp > 0) ? visnam_size : 0;
    }

    return visnames_for_domain;
}

const double *
PF3DFileFormat::MasterInformation::GetMinArray(const std::string &varName) const
{
    const double *val = 0;
    std::string minName(varName);
    minName += "min_vz";
    const MemberData *m = FindMember(minName);
    if(m != 0)
        val = (const double *)(m->data);

    return val;
}

const double *
PF3DFileFormat::MasterInformation::GetMaxArray(const std::string &varName) const
{
    const double *val = 0;
    std::string maxName(varName);
    maxName += "max_vz";
    const MemberData *m = FindMember(maxName);
    if(m != 0)
        val = (const double *)(m->data);

    return val;
}

std::string
PF3DFileFormat::MasterInformation::Get_dom_prefix_for_domain(int dom) const
{
    std::string retval;

    const MemberData *m = FindMember("dom_prefix");
    if(m != 0)
    {
        char *cptr = ((char *)m->data) + dom * m->dims[0];
        retval = std::string(cptr);
        if(retval.size() > 0)
        {
            if(retval[0] != '/')
                retval = std::string(VISIT_SLASH_STRING) + retval;
            if(retval[retval.size()-1] != '/')
                retval = retval + std::string(VISIT_SLASH_STRING);
        }
    }

    return retval;
}

int
PF3DFileFormat::MasterInformation::Get_num_grp_size() const
{
    int retval = 0;
    const MemberData *m = FindMember("grp_size");
    if(m != 0)
        retval = m->dims[0];

    return retval;
}

int 
PF3DFileFormat::MasterInformation::Get_grp_size(int grp) const
{
    int retval = 0;
    const MemberData *m = FindMember("grp_size");
    if(m != 0)
        retval = int(((const long *)(m->data))[grp]);

    return retval;
}

// ****************************************************************************
// Method: PF3DFileFormat::MasterInformation::Get_grp_members
//
// Purpose: 
//   Returns a pointer to the first member of the i'th group.
//
// Arguments:
//   grp : The group that we want.
//
// Returns:    The pointer to the first member of the group.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 7 16:26:07 PST 2006
//
// Modifications:
//   Brad Whitlock, Thu Sep 7 16:25:53 PST 2006
//   I made it use index 0 instead of index 1.
//   
// ****************************************************************************

const long *
PF3DFileFormat::MasterInformation::Get_grp_members(int grp) const
{
    const long *retval = 0;
    const MemberData *m = FindMember("grp_members");
    if(m != 0)
    {
        retval = (const long *)(m->data);
        retval += grp * m->dims[0];
    }

    return retval;
}

///////////////////////////////////////////////////////////////////////////////

PF3DFileFormat::MasterInformation::MemberData::MemberData() : name()
{
    dataType = NO_TYPE;
    ndims = 0;
    dims[0] = 0;
    dims[1] = 0;
    dims[2] = 0;
    data = 0;
}

PF3DFileFormat::MasterInformation::MemberData::~MemberData()
{
    if(data != 0)
        pdb_free_void_mem(data, dataType);
}

// ****************************************************************************
// Method: PF3DFileFormat::MasterInformation::MemberData::Print
//
// Purpose: 
//   Prints the MemberData.
//
// Arguments:
//   os : The stream to which we want to print.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 2 13:50:31 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
PF3DFileFormat::MasterInformation::MemberData::Print(ostream &os) const
{
    if(dataType == CHARARRAY_TYPE)
    {
        const char *ptr = (const char *)data;
        print_multi_dim_array(os, name.c_str(), ptr, ndims, dims, false);
    }
    else if(dataType == LONGARRAY_TYPE)
    {
        const long *ptr = (const long *)data;
        print_multi_dim_array(os, name.c_str(), ptr, ndims, dims, true);
    }
    else if(dataType == DOUBLEARRAY_TYPE)
    {
        const double *ptr = (const double *)data;
        print_multi_dim_array(os, name.c_str(), ptr, ndims, dims, true);
    }
}

///////////////////////////////////////////////////////////////////////////////


// ****************************************************************************
// Method: PF3DFileFormat::BOF::BOF
//
// Purpose: 
//   Constructor for BOF class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 12 14:30:54 PST 2004
//
// Modifications:
//   
// ****************************************************************************

PF3DFileFormat::BOF::BOF()
{
    size[0] = 0;
    size[1] = 0;
    size[2] = 0;
    data = 0;
}

// ****************************************************************************
// Method: PF3DFileFormat::BOF::BOF
//
// Purpose: 
//   Copy constructor for BOF class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 12 14:30:54 PST 2004
//
// Modifications:
//   
// ****************************************************************************

PF3DFileFormat::BOF::BOF(const BOF &obj)
{
    size[0] = obj.size[0];
    size[1] = obj.size[1];
    size[2] = obj.size[2];
    data = obj.data;
}

// ****************************************************************************
// Method: PF3DFileFormat::BOF::~BOF
//
// Purpose: 
//   Destructor for BOF class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 12 14:31:22 PST 2004
//
// Modifications:
//   
// ****************************************************************************

PF3DFileFormat::BOF::~BOF()
{
    debug4 << "Deleting BOF " << (void*)this << endl;
    delete [] data;
}

// ****************************************************************************
// Method: PF3DFileFormat::BOF::Destruct
//
// Purpose: 
//   Destruction function so the variable cache can delete a BOF.
//
// Arguments:
//   ptr : A pointer to the BOF to delete.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 12 14:31:39 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
PF3DFileFormat::BOF::Destruct(void *ptr)
{
    if(ptr != 0)
    {
        debug4 << "BOF::Destruct: ptr=" << ptr << endl;
        BOF *bof = (BOF *)ptr;
        delete bof;
    }
}
