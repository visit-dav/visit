#include <iostream.h>
#include <algorithm>

#include <InvalidVariableException.h>
#include <DebugStream.h>
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
            debug4 << "\"" << word << "\"" << endl;
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
//   
// ****************************************************************************

void
PF3DFileFormat::Initialize()
{
    if(!initialized)
    {
        const char *mName = "PF3DFileFormat::IdentifyFormat: ";
        const char *noread = "Could not read ";

        // Read the cycle number.
        bool allFields = true;
        char *run_id_c = 0;
        const char *key = "run_id_c";
        if((allFields &= pdb->GetString(key, &run_id_c)) == false)
        {
            debug4 << mName << noread << key << endl;
        }
        else
        {
            sscanf(run_id_c, "%d", &cycle);
            delete [] run_id_c;
            debug4 << mName << key << " = " << cycle << endl;
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
        key = "nx";
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
//   
// ****************************************************************************

std::string
PF3DFileFormat::FilenameForDomain(int realDomain)
{
    std::string prefix, middle, f(filenames[0]);

    // Find the directory prefix based on the path of the first master file.
    std::string::size_type index = f.rfind(SLASH_STRING);
    if(index != std::string::npos)
    {
        prefix = f.substr(0, index - 4);
    }
//    debug4 << "prefix = " << prefix << endl;

    // Look for "/viz/" in the visnam string for the specified domain and use
    // anything to the right of it.
    std::string vizStr(SLASH_STRING);
    vizStr += "viz";
    vizStr += SLASH_STRING;
    f = std::string(master.Get_visname_for_domain(realDomain, 0));
    index = f.rfind(vizStr);
    if(index != std::string::npos)
    {
        middle = f.substr(index, f.size() - index);
    }
//    debug4 << "middle = " << middle << endl;

    // Concatenate the pieces of the filename to get the final filename.
    std::string filename(prefix + middle + 
        master.Get_visname_for_domain(realDomain, 1));

//    debug4 << "FilenameForDomain(" << dom << ") = " << filename.c_str() << endl;

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
            PDBFileObject *obj = (PDBFileObject *)(*vr);
            delete obj;
        }

        // Clear the object out of the cache somehow...
        void_ref_ptr vr2 = void_ref_ptr(0, PDBFileObject::Destruct);
        cache->CacheVoidRef(key, FILE_KEY, timestep, -1, vr2);
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
// ****************************************************************************

void
PF3DFileFormat::FreeUpResources()
{
    debug4 << "PF3DFileFormat::FreeUpResources" << endl;

    // Close the master file since we can automatically open it up again
    // if we need to.
    pdb->Close();

    for(int realDomain = 0; realDomain < master.GetNDomains(); ++realDomain)
    {
        for(int i = 0; i < glob_nams.size(); ++i)
        {
            // Store an empty BOF in the cache so the old one gets
            // deleted when we call this method.
            std::string key(GetBOFKey(realDomain, glob_nams[i].c_str()));
            void_ref_ptr vr = void_ref_ptr(0, BOF::Destruct);
            cache->CacheVoidRef(key.c_str(), BOF_KEY, timestep, realDomain, vr);
        }
    }    
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
//
// ****************************************************************************

void
PF3DFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
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
        for(int i = 0; i < glob_nams.size(); ++i)
        {
            avtScalarMetaData *smd = new avtScalarMetaData(
                glob_nams[i], "mesh", AVT_ZONECENT);

            // Set the units if there are any.
            if(glob_units[i] != "")
            {
                smd->hasUnits = true;
                smd->units = glob_units[i];
            }
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
    SetUpDomainConnectivity();
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
//   
// ****************************************************************************

void
PF3DFileFormat::GetVarMinMaxArrays(const std::string &name,
    const double **minvals, const double **maxvals)
{
    *minvals = *maxvals = 0;

    if(name == "rho")
    {
        *minvals = master.Get_rhomin_vz();
        *maxvals = master.Get_rhomax_vz();
    }
    else if(name == "e0")
    {
        *minvals = master.Get_e0min_vz();
        *maxvals = master.Get_e0max_vz();
    }
    else if(name == "e1")
    {
        *minvals = master.Get_e1min_vz();
        *maxvals = master.Get_e1max_vz();
    }
    else if(name == "deniaw")
    {
        *minvals = master.Get_iawmin_vz();
        *maxvals = master.Get_iawmax_vz();
    }
    else if(name == "e2")
    {
        *minvals = master.Get_e2min_vz();
        *maxvals = master.Get_e2max_vz();
    }
    else if(name == "denepw")
    {
        *minvals = master.Get_epwmin_vz();
        *maxvals = master.Get_epwmax_vz();
    }
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
            float range[2];
            itree = new avtIntervalTree(GetNDomains(), 1);
            for(int dom = 0; dom < GetNDomains(); ++dom)
            {
                int realDomain = GetRealDomainIndex(dom);
                range[0] = float(minvals[realDomain]);
                range[1] = float(maxvals[realDomain]);
                itree->AddDomain(dom, range);
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
            float extents[6];
            GetExtents(dom, extents);
            itree->AddDomain(dom, extents);
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
PF3DFileFormat::GetExtents(int dom, float *extents)
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
    float domainEdgeLengthX = double(size[0]) * cellx;
    float domainEdgeLengthY = double(size[1]) * celly;
    float domainEdgeLengthZ = double(size[2]) * cellz;

    float Tx = float(globalorigin[0] - 1) / float(NX);

//debug4 << "float Tx = float(domloc[0] - 1) / float(NX - 1);" << endl
//       << "float Tx = float("<<domloc[0]<<" - 1) / float(" << NX << " - 1);\n"
//       << "float Tx = " << Tx << endl;

    float Ty = float(globalorigin[1] - 1) / float(NY);

//debug4 << "float Ty = float(domloc[2] - 1) / float(NY - 1);" << endl
//       << "float Ty = float("<<domloc[2]<<" - 1) / float(" << NY << " - 1);\n"
//       << "float Ty = " << Ty << endl;

    float Tz = float(globalorigin[2] - 1) / float(NZ);
//debug4 << "float Tz = float(domloc[4] - 1) / float(NZ - 1);" << endl
//       << "float Tz = float("<<domloc[4]<<" - 1) / float(" << NZ << " - 1);\n"
//       << "float Tz = " << Tz << endl;

    // Origin of the domain.
    float domainOriginX = Tx * (lenx / 2.) + (1. - Tx) * (-lenx / 2.);
    float domainOriginY = Ty * (leny / 2.) + (1. - Ty) * (-leny / 2.);
    float domainOriginZ = Tz * lenz;

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
    float extents[6];
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
// Method: PF3DFileFormat::GetVar
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
        debug4 << mName << "Must read data" << endl;

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

            //
            // Read the data from the PDB file.
            //
            data = domainPDB->ReadValues(int_nams[varIndex].c_str(),
                                         &dataType, &nTotalElements,
                                         &dims, &nDims, 0);

            if(data != 0)
            {
                if(dataType == CHARARRAY_TYPE)
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

                            // Store the BOF in the cache.
                            void_ref_ptr vr2 = void_ref_ptr(retval,
                                BOF::Destruct);
                            cache->CacheVoidRef(key.c_str(), BOF_KEY, timestep,
                                realDomain, vr2);
                        }
                        else
                        {
                            debug4 << mName << "BOF == 0!" << endl;
                        }
                    }
                }

                // Free the data that was read from the PDB file.
                free_void_mem(data, dataType);
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
        if(debug4_real)
        {
            print_array(debug4_real, "bof->size", bof->size, 3, 8, true, false);
            print_array(debug4_real, "domainsize", domainsize, 3, 8, true, false);
            print_array(debug4_real, "start", start, 3, 8, true, false);
            print_array(debug4_real, "end", end, 3, 8, true, false);
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
    }

    return scalars;
}

///////////////////////////////////////////////////////////////////////////////

const int PF3DFileFormat::MasterInformation::visnam_size = 46;

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
//   
// ****************************************************************************

PF3DFileFormat::MasterInformation::MasterInformation()
{
    nDomains = 0;
    buffer = 0;
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
//   
// ****************************************************************************

PF3DFileFormat::MasterInformation::~MasterInformation()
{
    if(buffer != 0)
    {
        long *ptr = (long *)buffer;
        delete [] ptr;
    }
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
//   
// ****************************************************************************

bool
PF3DFileFormat::MasterInformation::Read(PDBFileObject *pdb)
{
    const char *mName = "PF3DFileFormat::MasterInformation::Read:";
    bool retval = false;

    if(pdb->GetInteger("mp_size", &nDomains))
    {
        if(buffer == 0)
        {
            int bufferSize = CalculateBufferSize();
            // Allocate the memory as longs so it is word aligned.
            int nLongs = bufferSize / sizeof(long) +
                (((bufferSize % sizeof(long)) > 0) ? 1 : 0);
            buffer = (void *) new long[nLongs];
        }

        // Read the master structure.
        retval = (PD_read(pdb->filePointer(), "__@history", buffer) == TRUE);

        if(!retval)
        {
            debug4 << mName << " Could not read __@history."
                   << PDBLIB_ERRORSTRING << endl;
        }
#ifdef DEBUG_PRINT
        else if(debug4_real)
            operator << (debug4_real);
#endif
    }
    else
    {
        debug4 << mName << "Could not read mp_size!" << endl;
    }

    return retval;
}

// ****************************************************************************
// Method: PF3DFileFormat::MasterInformation::CalculateBufferSize
//
// Purpose: 
//   Returns the buffer size needed to hold a master structure with N domains.
//
// Returns:    The number of bytes in the master struct.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 12 14:35:25 PST 2004
//
// Modifications:
//   
// ****************************************************************************

int
PF3DFileFormat::MasterInformation::CalculateBufferSize()
{
    int sum = 0;
    sum += sizeof(History);
    sum += (sizeof(double) * nDomains);    // rhomin_vz[64];
    sum += (sizeof(double) * nDomains);    // rhomax_vz[64];
    sum += (sizeof(double) * nDomains);    // e0min_vz[64];
    sum += (sizeof(double) * nDomains);    // e0max_vz[64];
    sum += (sizeof(long) * nDomains * 6);  // domloc[64][6];
    sum += (sizeof(double) * nDomains * 6);// xyzloc[64][6];
    sum += (sizeof(char) * nDomains * 2 * visnam_size); // visnams[64][2][46];
    sum += (sizeof(double) * nDomains);    // e1min_vz[64];
    sum += (sizeof(double) * nDomains);    // e1max_vz[64];
    sum += (sizeof(double) * nDomains);    // iawmin_vz[64];
    sum += (sizeof(double) * nDomains);    // iawmax_vz[64];
    sum += (sizeof(double) * nDomains);    // e2min_vz[64];
    sum += (sizeof(double) * nDomains);    // e2max_vz[64];
    sum += (sizeof(double) * nDomains);    // epwmin_vz[64];
    sum += (sizeof(double) * nDomains);    // epwmax_vz[64];
    return sum;
}

//
// Methods to return values and pointers to the various arrays in the master.
//

double
PF3DFileFormat::MasterInformation::Get_time() const
{
    History *hptr = (History *)buffer;
    return hptr->time;
}

int   
PF3DFileFormat::MasterInformation::Get_ivzsave() const
{
    History *hptr = (History *)buffer;
    return hptr->ivzsave;
}

float 
PF3DFileFormat::MasterInformation::Get_tnowps() const
{
    History *hptr = (History *)buffer;
    return hptr->tnowps;
}

long  
PF3DFileFormat::MasterInformation::Get_ncyc() const
{
    History *hptr = (History *)buffer;
    return hptr->ncyc;
}

float 
PF3DFileFormat::MasterInformation::Get_dt() const
{
    History *hptr = (History *)buffer;
    return hptr->dt;
}

const double *
PF3DFileFormat::MasterInformation::Get_rhomin_vz() const
{
    char *cptr = (char *)buffer;
    cptr += sizeof(History);
    return (const double *)cptr;
}

const double *
PF3DFileFormat::MasterInformation::Get_rhomax_vz() const
{
    const double *rhomax_vz = Get_rhomin_vz() + nDomains;
    return rhomax_vz;
}

const double *
PF3DFileFormat::MasterInformation::Get_e0min_vz() const
{
    const double *e0min_vz = Get_rhomax_vz() + nDomains;
    return e0min_vz;
}

const double *
PF3DFileFormat::MasterInformation::Get_e0max_vz() const
{
    const double *e0max_vz = Get_e0min_vz() + nDomains;
    return e0max_vz;
}

const long *
PF3DFileFormat::MasterInformation::Get_domloc() const
{
    const double *domloc = Get_e0max_vz() + nDomains;
    return (const long *)domloc;
}

const double *
PF3DFileFormat::MasterInformation::Get_xyzloc() const
{
    const long *xyzloc = Get_domloc() + (nDomains * 6);
    return (const double *)xyzloc;
}

const char *
PF3DFileFormat::MasterInformation::Get_visnams() const
{
    const double *visnams = Get_xyzloc() + (nDomains * 6);
    return (const char *)visnams;
}

const char *
PF3DFileFormat::MasterInformation::Get_visname_for_domain(int dom, int comp) const
{
    const char *visnam_base = Get_visnams();
    const char *visnames_for_domain = visnam_base + (2 * visnam_size) * dom;
    visnames_for_domain += (comp > 0) ? visnam_size : 0;
    return visnames_for_domain;
}

const double *
PF3DFileFormat::MasterInformation::Get_e1min_vz() const
{
    const char *e1min_vz = Get_visnams() + (nDomains * 2 * visnam_size);
    return (const double *)e1min_vz;
}

const double *
PF3DFileFormat::MasterInformation::Get_e1max_vz() const
{
    const double *e1min_vz = Get_e1min_vz() + nDomains;
    return (const double *)e1min_vz;
}

const double *
PF3DFileFormat::MasterInformation::Get_iawmin_vz() const
{
    const double *iawmin_vz = Get_e1max_vz() + nDomains;
    return (const double *)iawmin_vz;
}

const double *
PF3DFileFormat::MasterInformation::Get_iawmax_vz() const
{
    const double *iawmax_vz = Get_iawmin_vz() + nDomains;
    return (const double *)iawmax_vz;
}

const double *
PF3DFileFormat::MasterInformation::Get_e2min_vz() const
{
    const double *e2min_vz = Get_iawmax_vz() + nDomains;
    return (const double *)e2min_vz;
}

const double *
PF3DFileFormat::MasterInformation::Get_e2max_vz() const
{
    const double *e2max_vz = Get_e2min_vz() + nDomains;
    return (const double *)e2max_vz;
}

const double *
PF3DFileFormat::MasterInformation::Get_epwmin_vz() const
{
    const double *epwmin_vz = Get_e2max_vz() + nDomains;
    return (const double *)epwmin_vz;
}

const double *
PF3DFileFormat::MasterInformation::Get_epwmax_vz() const
{
    const double *epwmax_vz = Get_epwmin_vz() + nDomains;
    return (const double *)epwmax_vz;
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
//   
// ****************************************************************************

ostream &
PF3DFileFormat::MasterInformation::operator << (ostream &os)
{
#ifdef DEBUG_PRINT
    os << "time = " << Get_time() << endl;
    os << "ivzsave = " << Get_ivzsave() << endl;
    os << "tnowps = " << Get_tnowps() << endl;
    os << "ncyc = " << Get_ncyc() << endl;
    os << "dt = " << Get_dt() << endl;

    print_array(os, "rhomin_vz", Get_rhomin_vz(), nDomains);
    print_array(os, "rhomax_vz", Get_rhomax_vz(), nDomains);
    print_array(os, "e0min_vz", Get_e0min_vz(), nDomains);
    print_array(os, "e0max_vz", Get_e0max_vz(), nDomains);
    print_array(os, "domloc", Get_domloc(), 6 * nDomains, 6);
    print_array(os, "xyzloc", Get_xyzloc(), 6 * nDomains, 6);
    print_array(os, "visnams", Get_visnams(),
        visnam_size * nDomains, visnam_size, false);
    print_array(os, "e1min_vz", Get_e1min_vz(), nDomains);
    print_array(os, "e1max_vz", Get_e1max_vz(), nDomains);
    print_array(os, "iawmin_vz", Get_iawmin_vz(), nDomains);
    print_array(os, "iawmax_vz", Get_iawmax_vz(), nDomains);
    print_array(os, "e2min_vz", Get_e2min_vz(), nDomains);
    print_array(os, "e2max_vz", Get_e2max_vz(), nDomains);
    print_array(os, "epwmin_vz", Get_epwmin_vz(), nDomains);
    print_array(os, "epwmax_vz", Get_epwmax_vz(), nDomains);
#endif
    return os;
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
