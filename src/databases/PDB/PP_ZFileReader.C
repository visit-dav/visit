#include <MaterialEncoder.h>

#include <InvalidVariableException.h>
#include <InvalidCellTypeException.h>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkIdList.h>
#include <vtkIntArray.h>
#include <vtkMatrix4x4.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabaseMetaData.h>
#include <avtGhostData.h>
#include <avtMaterial.h>
#include <avtMixedVariable.h>
#include <avtVariableCache.h>

#include <DebugStream.h>
#include <snprintf.h>

// This header file is last because it includes "scstd.h" (indirectly
// through "pdb.h"), which defines min and max, which conflict with
// "limits.h" on tru64 and aix systems.  On tru64 systems the conflict
// occurs with gcc-3.0.4 and on aix systems the conflict occurs with
// gcc-3.1.
#include <PP_ZFileReader.h>

//
// Make the revolved mesh be 40 zones in circumference.
//
const int PP_ZFileReader::revolutionSteps = 40 + 1;

// ****************************************************************************
// Method: PP_ZFileReader::PP_ZFileReader
//
// Purpose: 
//   Constructor for the PP_ZFileReader class.
//
// Arguments:
//   pdb : The pdb file to use.
//   c   : The variable cache to use for storing auxiliary data.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 26 15:00:44 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Aug 5 17:39:10 PST 2003
//   Added nMaterials, assumeMixedMaterialsPresent, nodalVars, and the new
//   cache pointer to the argument list.
//
//   Brad Whitlock, Mon Dec 6 16:52:29 PST 2004
//   Initialize cache pointer.
//
// ****************************************************************************

PP_ZFileReader::PP_ZFileReader(const char *filename) :
    PDBReader(filename), rtVar("rt"), ztVar("zt"), materialNames(), varStorage(),
    nodalVars()
{
    kmax = lmax = 0;
    meshDimensionsKnown = false;
    unstructuredCellCount = -1;

    cycles = 0;
    nCycles = 0;
    times = 0;
    nTimes = 0;
    assumeMixedMaterialsPresent = false;

    formatIdentified = false;
    initialized = false;
    varStorageInitialized = false;
    cache = 0;
}

PP_ZFileReader::PP_ZFileReader(PDBFileObject *pdb) :
    PDBReader(pdb), rtVar("rt"), ztVar("zt"), materialNames(), varStorage(),
    nodalVars()
{
    kmax = lmax = 0;
    meshDimensionsKnown = false;
    unstructuredCellCount = -1;

    cycles = 0;
    nCycles = 0;
    times = 0;
    nTimes = 0;
    assumeMixedMaterialsPresent = false;

    formatIdentified = false;
    initialized = false;
    varStorageInitialized = false;
    cache = 0;
}

// ****************************************************************************
// Method: PP_ZFileReader::~PP_ZFileReader
//
// Purpose: 
//   Destructor for the PP_ZFileReader class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 26 15:03:54 PST 2003
//
// Modifications:
//   
// ****************************************************************************

PP_ZFileReader::~PP_ZFileReader()
{
    delete [] cycles;
    delete [] times;

    // Delete the cached VariableData objects.
    for(VariableDataMap::iterator pos = varStorage.begin();
        pos != varStorage.end(); ++pos)
    {
        delete pos->second;
    }
}

// ****************************************************************************
// Method: PP_ZFileReader::IdentifyFormat
//
// Purpose: 
//   Identifies the file as a PP Z file.
//
// Returns:    True if the PDB file is a PP Z file; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 26 15:05:44 PST 2003
//
// Modifications:
//   Brad Whitlock, Fri Aug 8 10:50:01 PDT 2003
//   Made it able to operate on files that lack kmax, lmax.
//
// ****************************************************************************

bool
PP_ZFileReader::IdentifyFormat()
{
    bool have_kmax, have_lmax, validFile;

    // Make sure the file has kmax,
    if((have_kmax = pdb->GetInteger("kmax@value", &kmax)) == false)
    {
        have_kmax = pdb->GetInteger("kmax@las", &kmax);
    }

    // Make sure the file has lmax,
    if((have_lmax = pdb->GetInteger("lmax@value", &lmax)) == false)
    {
        have_lmax = pdb->GetInteger("lmax@las", &lmax);
    }

    debug4 << "PP_ZFileReader::Identify()" << endl;
    validFile = have_kmax && have_lmax;
    if(validFile)
    {
        debug4 << "\tkmax@value=" << kmax << endl;
        debug4 << "\tlmax@value=" << lmax << endl;
        meshDimensionsKnown = true;
    }
    else
    {
        bool haveCycleVariable = pdb->SymbolExists("cycle_variable@value");
        bool haveTimeVariable = pdb->SymbolExists("time_variable@value");
        validFile = haveCycleVariable && haveTimeVariable;
        meshDimensionsKnown = false;
    }
    debug4 << "\t meshDimensionsKnown="
           << (meshDimensionsKnown?"true":"false") << endl;
    debug4 << "\t validFile=" << (validFile?"true":"false") << endl;

    formatIdentified = true;

    return validFile;
}

// ****************************************************************************
// Method: PP_ZFileReader::GetNumTimeSteps
//
// Purpose: 
//   Returns the number of time steps that are available in the file.
//
// Returns:    The number of time steps that are available in the file.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 15:34:43 PST 2003
//
// Modifications:
//   
// ****************************************************************************

int
PP_ZFileReader::GetNumTimeSteps()
{
    Initialize();
    return nTimes;
}

// ****************************************************************************
// Method: PP_ZFileReader::GetCycle
//
// Purpose: 
//   Returns a pointer to the Cycle array.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 15:39:06 PST 2003
//
// Modifications:
//   
// ****************************************************************************

const int *
PP_ZFileReader::GetCycles()
{
    Initialize();
    return cycles;
}

// ****************************************************************************
// Method: PP_ZFileReader::GetTime
//
// Purpose: 
//   Returns a pointer to the time array.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 15:41:00 PST 2003
//
// Modifications:
//   
// ****************************************************************************

const double *
PP_ZFileReader::GetTimes()
{
    Initialize();
    return times;
}

// ****************************************************************************
// Method: PP_ZFileReader::SetCache
//
// Purpose: 
//   Sets the internal cache pointer so the reader can access the generic
//   database's variable cache.
//
// Arguments:
//   c : The cache pointer.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 6 18:09:18 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
PP_ZFileReader::SetCache(avtVariableCache *c)
{
    cache = c;
}

// ****************************************************************************
// Method: PP_ZFileReader::Initialize
//
// Purpose: 
//   Reads in the cycles and times.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 26 15:06:33 PST 2003
//
// Modifications:
//   Brad Whitlock, Thu Aug 7 15:15:54 PST 2003
//   Added a list of nodal variables. Only variables in that list are
//   considered to be nodal. Added code to figure out the mesh dimensions
//   if they were never given.
//
//   Brad Whitlock, Wed Sep 17 09:48:21 PDT 2003
//   I made it able to get the cycle and time from another variable.
//
// ****************************************************************************

void
PP_ZFileReader::Initialize()
{
    if(!initialized)
    {
        if(!formatIdentified)
            IdentifyFormat();

        //
        // Read the cycles
        //
        int tempCycle;
        char *realName = 0;
        if(pdb->GetString("cycle_variable@value", &realName))
        {
            debug4 << "Cycle array name: " << realName << endl;

            if(pdb->GetIntegerArray(realName, &cycles, &nCycles))
            {
                debug4 << "Cycles = (";
                for(int i = 0; i < nCycles; ++i)
                {
                    debug4 << cycles[i];
                    if(i < nCycles - 1)
                        debug4 << ", ";
                }
                debug4 << ")" << endl;
            }

            delete [] realName;
        }
        else if(pdb->GetInteger("ncyc@las", &tempCycle))
        {
            debug4 << "Read a single cycle from ncyc@las." << endl;
            cycles = new int[1];
            cycles[0] = tempCycle;
            nCycles = 1;
        }
        else
        {
            debug4 << "Could not read the cycle array!" << endl;
            cycles = new int[1];
            cycles[0] = 0;
            nCycles = 1;
        }

        //
        // Read the time array.
        //
        double tempTime;
        if(pdb->GetString("time_variable@value", &realName))
        {
            debug4 << "Time array name: " << realName << endl;

            if(pdb->GetDoubleArray(realName, &times, &nTimes))
            {
                debug4 << "Times = (";
                for(int i = 0; i < nTimes; ++i)
                {
                    debug4 << times[i];
                    if(i < nTimes - 1)
                        debug4 << ", ";
                }
                debug4 << ")" << endl;
            }

            delete [] realName;
        }
        else if(pdb->GetDouble("time@las", &tempTime))
        {
            debug4 << "Read in a single time from time@las." << endl;
            times = new double[1];
            times[0] = tempTime;
            nTimes = 1;
        }
        else
        {
            debug4 << "Could not read the time array!" << endl;
            times = new double[1];
            times[0] = 0.;
            nTimes = 1;
        }

        //
        // Read the list of nodal variables.
        //
        char *pc_list = 0;
        int   pc_list_len = 0;
        if(pdb->GetString("pc_list@global", &pc_list, &pc_list_len))
        {
            for(int i = 0; i < pc_list_len; i += 2)
            {
                char nodalVar[3] = {0,0,0};
                nodalVar[0] = pc_list[i];
                nodalVar[1] = (pc_list[i+1] != ' ') ? pc_list[i+1] : 0;
                nodalVars.push_back(nodalVar);
                debug4 << "Added " << nodalVar << " to the list of nodal vars."
                       << endl;
            }
            free_void_mem((void *)pc_list, CHARARRAY_TYPE);
        }
        else
        {
            debug4 << "Could not read pc_list@global nodal var list." << endl;
        }

        //
        // Figure out the mesh dimensions if they were not given in the file.
        //
        if(!meshDimensionsKnown)
        {
            int *dimensions = 0;
            int nDims = 0;
            TypeEnum t = NO_TYPE;
            int nTotalElements = 0;

            if(pdb->SymbolExists("ireg@history", &t, &nTotalElements, &dimensions,
                                 &nDims))
            {
                if(nDims >= 2)
                {
                    meshDimensionsKnown = true;
                    kmax = dimensions[0];
                    lmax = dimensions[1];
                    debug4 << "kmax determined to be: " << kmax << endl;
                    debug4 << "lmax determined to be: " << lmax << endl;
                }

                delete [] dimensions;
            }
        }

        initialized = true;
    }
}

// ****************************************************************************
// Method: PP_ZFileReader::VariableIsNodal
//
// Purpose: 
//   Returns whether or not the variable is nodal.
//
// Arguments:
//   var : The name of the variable.
//
// Returns:    True if the variable is nodal; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 7 15:25:10 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool
PP_ZFileReader::VariableIsNodal(const std::string &var) const
{
    for(int i = 0; i < nodalVars.size(); ++i)
    {
        if(var == nodalVars[i])
            return true;
    }

    return false;
}

// ****************************************************************************
// Method: PP_ZFileReader::GetTimeVaryingInformation
//
// Purpose: 
//   Stores time varying information like the cycles and times in the
//   database metadata.
//
// Arguments:
//   state : The state that we're interested in.
//   md    : The database metadata that we're populating.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 26 15:09:18 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
PP_ZFileReader::GetTimeVaryingInformation(int state, avtDatabaseMetaData *md)
{
    debug4 << "PP_ZFileReader::GetTimeVaryingInformation: state=" << state << endl;

    //
    // Make sure everything is initialized.
    //
    Initialize();

    // Put the cycles into the metadata.
    if(md->GetCycles().size() < nCycles)
    {
        intVector c; c.reserve(nCycles);
        for(int i = 0; i < nCycles; ++i)
            c.push_back(cycles[i]);

        // Set all of the cycles at once.
        md->SetCycles(c);
        md->SetCyclesAreAccurate(true);
    }

    // Put the times into the metadata.
    if(md->GetTimes().size() < nTimes)
    {
        doubleVector d; d.reserve(nTimes);
        for(int i = 0; i < nTimes; ++i)
            d.push_back(times[i]);

        // Set all of the cycles at once.
        md->SetTimes(d);
        md->SetTimesAreAccurate(true);

        if(nTimes > 0)
            md->SetTemporalExtents(times[0], times[nTimes-1]);
    }
}

// ****************************************************************************
// Method: PP_ZFileReader::ReadMaterialNamesHelper
//
// Purpose: 
//   Helper function that reads a string variable from the file and splits
//   out the material names from it.
//
// Arguments:
//   namregVar : The name of the variable that contains the names of the
//               materials.
//   nmats     : The expected number of materials.
//   matNames  : The return vector for the material names.
//
// Returns:    True if successful; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 8 11:09:50 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

bool
PP_ZFileReader::ReadMaterialNamesHelper(const char *namregVar, int nmats,
    stringVector &matNames)
{
    bool     retval = false;
    TypeEnum t;
    int      nTotalElements, *dims = 0, nDims = 0;

    if(pdb->SymbolExists(namregVar, &t, &nTotalElements, &dims, &nDims))
    {
        char *namreg = 0;
        int   namregLen = 0, maxNameLength = 64;

        // Determine the maximum length for a material name.
        if(nDims > 1)
            maxNameLength = dims[0];

        if(pdb->GetString(namregVar, &namreg, &namregLen))
        {
            debug5 << "namregLen = " << namregLen << " namreg="
                   << namreg << endl;

            int nmatNames = 0;
            char *sptr = namreg;
            bool keepGoing = true;
            do
            {
                // Null terminate the string.
                char *s = sptr + maxNameLength - 1;
                for(; s > sptr && *s == ' '; --s) 
                    *s = '\0';

                // If we have a non-empty string, add it to the list of
                // material names.
                if(s > sptr)
                {
                    char tmp[100]; 
                    SNPRINTF(tmp, 100, "%d %s", nmatNames+1, sptr);
                    matNames.push_back(tmp);
                    ++nmatNames;
                }
                else
                    keepGoing = false;

                sptr += maxNameLength;
                if(sptr - namreg > namregLen)
                   keepGoing = false;
            }
            while(nmatNames < nmats && keepGoing);

            delete [] namreg;
            retval = (nmatNames == nmats);
        }

        delete [] dims;
    }

    return retval;
}

// ****************************************************************************
// Method: PP_ZFileReader::ReadMaterialNames
//
// Purpose: 
//   Reads actual material names from the file if they are present.
//
// Arguments:
//   nmats    : The expected number of materials.
//   matNames : The return stringVector for the material names.
//
// Returns:    True if successful; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 8 09:42:30 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

bool
PP_ZFileReader::ReadMaterialNames(int nmats, stringVector &matNames)
{
    // Try this variable first.
    if(ReadMaterialNamesHelper("namreg@value", nmats, matNames))
        return true;

    // Try this variable second.
    return ReadMaterialNamesHelper("namreg@las", nmats, matNames);
}

// ****************************************************************************
// Method: PP_ZFileReader::PopulateMaterialNames
//
// Purpose: 
//   Populates the materialNames object using the variables in the file.
//
// Returns:    True if materials were found.
//
// Note:       It's possible that no materials are found.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 12 13:46:52 PST 2003
//
// Modifications:
//   Brad Whitlock, Wed Mar 3 15:06:20 PST 2004
//   I fixed a bug that could cause gaps in the material range when we have
//   to read the ireg array to determine the list of materials.
//
//   Brad Whitlock, Fri Sep 3 07:26:53 PDT 2004
//   Added support for mixed materials in Flash files.
//
// ****************************************************************************

bool
PP_ZFileReader::PopulateMaterialNames()
{
    bool haveMaterials = false;

    //
    // Look for evidence of materials in the file.
    //
    int nMaterials = 0;
    int *nMatsOverTime = 0, nTimes = 0;
    if(pdb->GetInteger("nreg@las", &nMaterials))
    {
        debug4 << "We found evidence of " << nMaterials 
               << " materials in the file." << endl;

        //
        // Read any material names that might be stored in the file.
        //
        ReadMaterialNames(nMaterials, materialNames);

        //
        // Fill in any materials that were not added yet.
        //
        for(int i = materialNames.size(); i < nMaterials; ++i)
        {
            char tmp[20];
            SNPRINTF(tmp, 20, "%d", i + 1);
            materialNames.push_back(tmp);
        }
    }
    else if(pdb->GetIntegerArray("nreg@history", &nMatsOverTime, &nTimes))
    {
        nMaterials = nMatsOverTime[0];
        for(int i = 1; i < nTimes; ++i)
        {
            if(nMatsOverTime[i] != nMaterials)
            {
                debug4 << "We have a changing number of materials over time! "
                          "Let's take the largest number." << endl;
                nMaterials = (nMatsOverTime[i] > nMaterials) ? 
                    nMatsOverTime[i] : nMaterials;
            }
        }

        //
        // Read any material names that might be stored in the file.
        //
        ReadMaterialNames(nMaterials, materialNames);

        //
        // Fill in any materials that were not added yet.
        //
        for(int j = materialNames.size(); j < nMaterials; ++j)
        {
            char tmp[20];
            SNPRINTF(tmp, 20, "%d", j + 1);
            materialNames.push_back(tmp);
        }

        delete [] nMatsOverTime;
    }
    else if(varStorage.find("ireg") != varStorage.end())
    {
        debug4 << "We didn't find obvious evidence of materials in the file "
                  "but we can read the ireg array to determine the materials."
               << endl;

        TRY
        {
            ReadVariable("ireg");

            VariableData *ireg_data = varStorage["ireg"];
            if(ireg_data->dataType == INTEGERARRAY_TYPE)
            {
                const int *ireg = (const int *)ireg_data->data;
                int i;
#define MAX_MATERIALS 1000
                bool *mats = new bool[MAX_MATERIALS];
                for(i = 0; i < MAX_MATERIALS; ++i)
                    mats[i] = false;

                //
                // Search through the ireg array and look for all unique
                // material numbers.
                //
                for(i = 0; i < ireg_data->nTotalElements; ++i)
                {
                    if(ireg[i] > 0 && ireg[i] < MAX_MATERIALS)
                        mats[ireg[i]] = true;
                }

                //
                // Look through the list of material numbers and use it
                // to add to the material names vector.
                //
                int maxMat = -1;
                for(i = 1; i < MAX_MATERIALS; ++i)
                {
                    if(mats[i])
                        maxMat = (i > maxMat) ? i : maxMat;
                }

                if(maxMat > -1)
                {
                    debug4 << "We found materials by inspecting the ireg "
                              "array." << endl;

                    // Add all of the materials in the range [1,maxMat].
                    bool gaps = false;
                    for(i = 1; i <= maxMat; ++i)
                    {
                        char tmp[20];
                        sprintf(tmp, "%d", i);
                        materialNames.push_back(tmp);

                        // See if there are gapps in the range [1,maxMat].
                        gaps |= mats[i];
                    }

                    if(gaps)
                    {
                        // There were gaps. Print a message to the logs.
                        debug4 << "Unfortunately, there were gaps in the "
                                  "material list read from ireg. VisIt will "
                                  "add materials to cover the gaps so the "
                                  "SIL will be more likely to match the next "
                                  "time step as we change time states."
                               << endl;
                    }
                }

                delete [] mats;
#undef MAX_MATERIALS
            }
        }
        CATCH(InvalidVariableException)
        {
            debug4 << "We can't read the ireg array so we'll say that the "
                      "database has no materials." << endl;
        }
        ENDTRY
    }

    //
    // If we have material names, return that we have materials.
    //
    haveMaterials = (materialNames.size() > 0);
    if(haveMaterials)
    {
        debug4 << "Materials={";
        for(int i = 0; i < materialNames.size(); ++i)
            debug4 << materialNames[i].c_str() << ", ";
        debug4 << "}" << endl;
    }

    return haveMaterials;
}

// ****************************************************************************
// Method: PP_ZFileReader::InitializeVarStorage
//
// Purpose: 
//   Performs comprehensive initialization and makes sure that the varStorage
//   map contains the right list of variables.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 17 12:02:38 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
PP_ZFileReader::InitializeVarStorage()
{
    if(!varStorageInitialized)
    {
        avtDatabaseMetaData md;
        PopulateDatabaseMetaData(&md);
    }
}

// ****************************************************************************
// Method: PP_ZFileReader::PopulateDatabaseMetaData
//
// Purpose: 
//   Reads in the list of variables from the file and puts that list into
//   the database metadata.
//
// Arguments:
//   md : The database metadata that we're populating.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 26 15:11:20 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Aug 5 17:39:41 PST 2003
//   Added support for materials, a revolved mesh, and zonal variables. Made
//   it work on files that are missing some crucial information.
//
//   Brad Whitlock, Fri Jul 23 14:41:02 PST 2004
//   I added support for reading a database comment.
//
//   Brad Whitlock, Thu Jul 29 14:10:45 PST 2004
//   I added code to prevent the Windows version from saying that it has
//   a revolved mesh because it crashes on Windows and it's safer this way
//   since it's probably not too important to fix at this time.
//
//   Brad Whitlock, Fri Sep 3 07:30:28 PDT 2004
//   Changed code to account for mixed material arrays having another
//   suffix in Flash files.
//
// ****************************************************************************

void
PP_ZFileReader::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    debug4 << "PP_ZFileReader::PopulateDatabaseMetaData: start" << endl;

    //
    // Make sure that everything is initialized.
    //
    Initialize();

    //
    // Add the database comment if it is present. This won't leak memory
    // by calling GetString twice because the two keys won't ever be in the
    // same file.
    //
    char *idates = 0;
    if(pdb->GetString("idates@value", &idates) ||
       pdb->GetString("idates@las", &idates))
    {
        md->SetDatabaseComment(idates);
        delete [] idates;
    }

    int cellOrigin = 1;
    int ndims = 2;

    // Add the logical mesh
    avtMeshMetaData *mmd = new avtMeshMetaData(
        "logical_mesh", 1, 0, cellOrigin, ndims, ndims, AVT_RECTILINEAR_MESH);
    mmd->hasSpatialExtents = false;
    mmd->cellOrigin = 1;
    md->Add(mmd);

    // Add the mesh.
    mmd = new avtMeshMetaData(
        "mesh", 1, 0, cellOrigin, ndims, ndims, AVT_CURVILINEAR_MESH);
    mmd->hasSpatialExtents = false;
    mmd->cellOrigin = 1;
    md->Add(mmd);

#if !defined(_WIN32)
    // Add a revolved mesh.
    mmd = new avtMeshMetaData(
        "revolved_mesh", 1, 0, cellOrigin, 3, 3, AVT_UNSTRUCTURED_MESH);
    mmd->hasSpatialExtents = false;
    mmd->cellOrigin = 1;
    md->Add(mmd);
#endif

    // Determine the size of the problem.
    int problemSize = kmax * lmax * nCycles;
    debug4 << "problemSize = " << problemSize << endl;

    //
    // Read all variables of the specified type.
    //
    PDBfile *pdbPtr = pdb->filePointer();
    int numVars = 0;
    char **varList = PD_ls(pdbPtr, NULL /*path*/, NULL /*pattern*/, &numVars);

    //
    // If we got any variable names, see if any are the size that we think
    // the problem should be.
    //
    if(varList != NULL)
    {
        for(int j = 0; j < numVars; ++j)
        {
            bool problemSized = false;
            int length = 1;
            syment *ep = 0;

            // Check to see if the  variable is problem sized.
            if((ep = PD_inquire_entry(pdbPtr, varList[j], 0, NULL)) != NULL)
            {
                // Figure out the number of dimensions and the number of
                // elements that are in the entire array.
                dimdes *dimptr = PD_entry_dimensions(ep);
                while(dimptr != 0)
                {
                    length *= dimptr->number;
                    dimptr = dimptr->next;
                }

                problemSized = (length == problemSize);
            }

            //
            // If the variable is problem sized then consider it as a
            // candidate for being a plottable variable.
            //
            if(problemSized)
            {
                debug4 << "\t" << varList[j] << " (problem sized)";
                char *s = strstr(varList[j], "@");
                if(s != 0)
                {
                    int len = s - varList[j];
                    char *newCStr = new char[len + 1];
                    strncpy(newCStr, varList[j], len);
                    newCStr[len] = '\0';
                    std::string newStr(newCStr);

                    avtCentering centering;
                    if(VariableIsNodal(newStr))
                    {
                        centering = AVT_NODECENT;
                        debug4 << "(nodal)";
                    }
                    else
                    {
                        centering = AVT_ZONECENT;
                        debug4 << "(zonal)";
                    }

                    debug4 << " added as " << newCStr << endl;

                    // Add the variable over the logical mesh to the metadata.
                    std::string logicalMeshVar(std::string("logical_mesh/") +
                                               newStr);
                    avtScalarMetaData *smd = new avtScalarMetaData(
                        logicalMeshVar, "logical_mesh", centering);
                    md->Add(smd);

#if !defined(_WIN32)
                    // Add the variable over the revolved mesh.
                    std::string revolvedMeshVar(std::string("revolved_mesh/") +
                                                newStr);
                    smd = new avtScalarMetaData(revolvedMeshVar,
                        "revolved_mesh", centering);
                    md->Add(smd);
#endif
                    // Add the variable over the mesh to the metadata.
                    std::string meshVar(std::string("mesh/") + newStr);
                    smd = new avtScalarMetaData(meshVar, "mesh", centering);
                    md->Add(smd);

                    // Add the variable to the variable map.
                    VariableData *v = new VariableData(varList[j]);
                    varStorage[newStr] = v;

                    delete [] newCStr;
                }
            }
        }

        debug4 << endl << endl;
        SFREE(varList);
    }

    //
    // If we have material names, add materials to the metadata.
    //
    if(PopulateMaterialNames())
    {
        //
        // Add material metadata to the metadata.
        //
        avtMaterialMetaData *mmd = new avtMaterialMetaData("material",
            "mesh", materialNames.size(), materialNames);
        md->Add(mmd);
#if !defined(_WIN32)
        mmd = new avtMaterialMetaData("material2",
            "revolved_mesh", materialNames.size(), materialNames);
        md->Add(mmd);
#endif

        //
        // Look for evidence of mixed materials.
        //
        int nszmmt = 0;
        std::string matSuffix("@las");
        if(!pdb->GetInteger("nszmmt@las", &nszmmt))
        {
            matSuffix = "@history";
            pdb->GetInteger("nszmmt@history", &nszmmt);
        }

        if(nszmmt > 0)
        {
            //
            // Add the mixed material arrays to the cache so we can read them
            // later and have them be cached.
            //
            VariableData *v = new VariableData(std::string("iregmm") +
                matSuffix);
            varStorage["iregmm"] = v;
            v = new VariableData(std::string("volfmm") + matSuffix);
            varStorage["volfmm"] = v;
            v = new VariableData(std::string("ilamm") + matSuffix);
            varStorage["ilamm"] = v;
            if(varStorage.find("nummm") == varStorage.end())
            {
                v = new VariableData(std::string("nummm") + matSuffix);
                varStorage["nummm"] = v;
            }
 
            assumeMixedMaterialsPresent = true;
        }
    }

    //
    // Figure out the names that should be used for the rt,zt mesh arrays.
    //
    bool haveRt = true;
    if(varStorage.find(rtVar) == varStorage.end())
    {
        if(varStorage.find("r") != varStorage.end())
            rtVar = "r";
        else
            haveRt = false;
    }
    bool haveZt = true;
    if(varStorage.find(ztVar) == varStorage.end())
    {
        if(varStorage.find("z") != varStorage.end())
            ztVar = "z";
        else
            haveZt = false;
    }
    if(!haveRt || !haveZt)
    {
        debug4 << "We don't have r or z coordinate fields! We need to "
                   "disable most of the variables in the metadata." << endl;
    }
    else
    {
        debug4 << "Using " << rtVar.c_str() << " for r coordinate field."
               << endl;
        debug4 << "Using " << ztVar.c_str() << " for z coordinate field."
               << endl;
    }

    //
    // Add the ray mesh metadata if there are ray variables.
    //
    AddRayMetaData(md);

    // Set a flag indicating that the varStorage map has been initialized.
    varStorageInitialized = true;

    debug4 << "PP_ZFileReader::PopulateDatabaseMetaData: end" << endl;
}

// ****************************************************************************
// Method: PP_ZFileReader::AddRayMetaData
//
// Purpose: 
//   Adds ray mesh variables to the metadata.
//
// Arguments:
//   md : The metadata object to which we might add more metadata.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 26 10:37:22 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
PP_ZFileReader::AddRayMetaData(avtDatabaseMetaData *md)
{
    const int maxRayVars = 6;
    const char *vars[] = {"raypr", "raypz", "kptryp", "mxp", "raypx", "raypy"};
    const char *suffixes[] = {"@lzr", "@history"};
    const char *suffix = suffixes[0];
    stringVector realVars;

    //
    // Try and determine the variable suffix.
    //
    std::string var = std::string(vars[0]) + suffix;
    if(!pdb->SymbolExists(var.c_str()))
    {
        suffix = suffixes[1];
        var = std::string(vars[0]) + suffix;
        if(!pdb->SymbolExists(var.c_str()))
        {
            debug4 << "The first ray variable could not be found in the file so "
                   << "no ray meshes or variables will be added." << endl;
            return;
        }
    }

    //
    // Look for the arrays required to have a ray mesh.
    //
    bool haveRayVars = true;
    bool haveRay2d = false;
    bool haveRay3d = false;
    int  i;
    for(i = 0; i < maxRayVars; ++i)
    {
        realVars.push_back(std::string(vars[i]) + suffix);
        haveRayVars &= pdb->SymbolExists(realVars[i].c_str());
        if(i == 3)
            haveRay2d = haveRayVars;
        else if(i == 5)
            haveRay3d = haveRayVars;
    }

    int nRayVars = (haveRay3d ? maxRayVars : (haveRay2d ? maxRayVars-2 : 0));
    for(i = 0; i < nRayVars; ++i)
    {
        if(varStorage.find(vars[i]) == varStorage.end())
        {
            VariableData *v = new VariableData(realVars[i]);
            varStorage[vars[i]] = v;
        }
    }

    //
    // Check to see if the power array is available.
    //
    const int cellOrigin = 1;
    std::string rayPowerVar = std::string("rayppow") + suffix;
    bool haveRayPower = pdb->SymbolExists(rayPowerVar.c_str());

    //
    // If we had all of the 2d ray arrays, add a 2d ray mesh. 
    //
    if(haveRay2d)
    {
        avtMeshMetaData *mmd = new avtMeshMetaData("ray", 1, 0, cellOrigin,
            2, 1, AVT_UNSTRUCTURED_MESH);
        mmd->hasSpatialExtents = false;
        mmd->cellOrigin = cellOrigin;
        md->Add(mmd);

        // Add the variables to the metadata and create varStorage
        // entries for them.
        if(haveRayPower)
        {
            md->Add(new avtScalarMetaData("ray/power", "ray", AVT_NODECENT));
            varStorage["power"] = new VariableData(rayPowerVar);
            md->Add(new avtScalarMetaData("ray/rel_power", "ray", AVT_NODECENT));
            varStorage["rel_power"] = new VariableData(rayPowerVar);
        }
    }

    //
    // If we had all of the 3d ray arrays, add a 3d ray mesh. 
    //
    if(haveRay3d)
    {
        avtMeshMetaData *mmd = new avtMeshMetaData("ray3d", 1, 0, cellOrigin,
            3, 1, AVT_UNSTRUCTURED_MESH);
        mmd->hasSpatialExtents = false;
        mmd->cellOrigin = cellOrigin;
        md->Add(mmd);

        // Add the variables to the metadata.
        if(haveRayPower)
        {
            md->Add(new avtScalarMetaData("ray3d/power", "ray3d", AVT_NODECENT));
            md->Add(new avtScalarMetaData("ray3d/rel_power", "ray3d", AVT_NODECENT));
        }
    }
}

// ****************************************************************************
// Method: PP_ZFileReader::FreeUpResources
//
// Purpose: 
//   Clears the cached data arrays from the varStorage map and closes the 
//   PDB file object to free up the file descriptor.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 2 00:02:13 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
PP_ZFileReader::FreeUpResources()
{
    // Free all of the data arrays that we've read into the varStorage cache.
    for(VariableDataMap::iterator pos = varStorage.begin();
        pos != varStorage.end(); ++pos)
    {
        pos->second->FreeData();
    }

    // Close the PDB file to free the file descriptor. If we need it again,
    // we'll open it up transparently.
    pdb->Close();
}

// ****************************************************************************
// Method: PP_ZFileReader::CreateGhostZones
//
// Purpose: 
//   Marks all of the cells in the input mesh where ireg <= 0 as ghsot cells.
//
// Arguments:
//   ireg : The ireg array used to determine if cells are ghost cells.
//   ds   : The input dataset.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 16 16:11:08 PST 2003
//
// Modifications:
//   Kathleen Bonnell, Tue Apr 27 13:29:46 PDT 2004
//   For avtRealDims array, size should not be larger (but can be smaller)
//   than the Dimensions of the vtkRectilinearGrid or vtkStructuredGrid.
//   
//   Hank Childs, Fri Aug 27 17:18:37 PDT 2004
//   Rename ghost data array.  Also set appropriate ghost type using new
//   convention.
//
// ****************************************************************************

void
PP_ZFileReader::CreateGhostZones(const int *ireg, vtkDataSet *ds)
{
    if(ireg)
    {
        unsigned char realVal = 0, ghost = 0;
        avtGhostData::AddGhostZoneType(ghost, ZONE_NOT_APPLICABLE_TO_PROBLEM);
        int nCells = ds->GetNumberOfCells();
        vtkIdList *ptIds = vtkIdList::New();
        vtkUnsignedCharArray *ghostCells = vtkUnsignedCharArray::New();
        ghostCells->SetName("avtGhostZones");
        ghostCells->Allocate(nCells);

        for(int i = 0; i < nCells; ++i)
        {
            ds->GetCellPoints(i, ptIds);
            int cellRow = i / (kmax - 1);
            int cellCol = i % (kmax - 1);
            // Get the node index for the upper right node in the cell.
            int nodeIndex = (cellRow + 1) * kmax + cellCol + 1;

            if(ireg[nodeIndex] <= 0)
            {
                ghostCells->InsertNextValue(ghost);
            }
            else
                ghostCells->InsertNextValue(realVal);
        }

        ds->GetCellData()->AddArray(ghostCells);
        ghostCells->Delete();
        ptIds->Delete();

        vtkIntArray *realDims = vtkIntArray::New();
        realDims->SetName("avtRealDims");
        realDims->Allocate(6);
        realDims->SetValue(0, 0);
        realDims->SetValue(1, kmax-1);
        realDims->SetValue(2, 0);
        realDims->SetValue(3, lmax-1);
        realDims->SetValue(4, 0);
        realDims->SetValue(5, 0);
        ds->GetFieldData()->AddArray(realDims);
        ds->GetFieldData()->CopyFieldOn("avtRealDims");
        realDims->Delete();
        ds->SetUpdateGhostLevel(0);
    }
}

// ****************************************************************************
// Method: PP_ZFileReader::GetIreg
//
// Purpose: 
//   Returns the ireg variable, which is commonly used to identify ghost cells
//   and material numbers.
//
// Arguments:
//   state : The state to use when returning ireg.
//
// Returns:    A const pointer to the ireg array at the specified state.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 8 16:16:38 PST 2003
//
// Modifications:
//   
// ****************************************************************************

const int *
PP_ZFileReader::GetIreg(int state)
{
    const int *ireg = 0;
    int nnodes = kmax * lmax;

    TRY
    {
        ReadVariable("ireg");

        VariableData *iregData = varStorage["ireg"];
        if(iregData->dataType == INTEGERARRAY_TYPE)
        {
            //
            // Go to the right time state in ireg
            //
            ireg = (const int *)iregData->data;
            ireg += ((state < nCycles) ? (state * nnodes) : 0);
        }
    }
    CATCH(InvalidVariableException)
    {
        debug4 << "ireg could not be read in." << endl;
    }
    ENDTRY


    return ireg;
}

// ****************************************************************************
// Method: GetMesh_StoreMeshPoints
//
// Purpose: 
//   Template function to help store points from the rt,zt arrays into the 
//   coordinate arrays for the VTK mesh. It's a template function because
//   the types of the rt,zt arrays can be different depending on the file.
//
// Arguments:
//   ptr    : Float array that contains the mesh points.
//   rt     : The original array for X coordinates.
//   zt     : The original array for Y coordinates.
//   kmax   : The number of nodes in the horizontal direction.
//   lmax   : The number of nodes in the vertical direction.
//   nnodes : The total number of nodes in the mesh.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 7 09:04:47 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

template <class T>
inline void
GetMesh_StoreMeshPoints(float *ptr, const T *rt, const T *zt, const int kmax,
    const int lmax, const int nnodes)
{
    //
    // The minimum value for R is way off the charts so we should take care
    // to not actually use it as a node coordinate since it would make the
    // extents be wrong. Find the maximum R value and use that everywhere we
    // get the minR value.
    //
    const T *rt2 = rt;
    const T *zt2 = zt;
    const T minR = -1e8;
    float maxR = float(minR);
    for(int k = 0; k < nnodes; ++k, ++rt2)
        if(*rt2 > maxR) maxR = *rt2;
    rt2 = rt;

    //
    // Store the points for the mesh.
    //
    for(int j = 0; j < lmax; ++j)
    { 
        for(int i = 0; i < kmax; ++i)
        {
            *ptr++ = float(*zt2++);
            if(*rt2 == minR)
                *ptr++ = float(maxR);
            else
                *ptr++ = float(*rt2);
            ++rt2;
            *ptr++ = 0.f;
        }
    }    
}

// ****************************************************************************
// Method: PP_ZFileReader::GetMesh
//
// Purpose: 
//   Returns the mesh.
//
// Arguments:
//   state : The time state for which we want the mesh.
//   var   : The name of the mesh that we want.
//
// Returns:    A vtkDataSet object containing the mesh.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 26 15:58:50 PST 2003
//
// Modifications:
//   Brad Whitlock, Wed Jul 16 16:14:12 PST 2003
//   I added a curvilinear mesh, which is what they really want to look at,
//   and I made cells be marked as ghost zones according to the ireg array.
//
//   Brad Whitlock, Wed Aug 6 11:50:54 PDT 2003
//   I added a revolved (unstructured) version of their curvilinear mesh.
//
//   Brad Whitlock, Wed Sep 17 12:03:32 PDT 2003
//   I made it call InitializeVarStorage.
//
//   Brad Whitlock, Fri Sep 26 11:04:27 PDT 2003
//   Added support for a ray mesh.
//
//   Brad Whitlock, Mon Jun 7 09:07:41 PDT 2004
//   Templatized the code for storing the mesh points for the "mesh" variable
//   so we can support double and float coordinate fields.
//
// ****************************************************************************

vtkDataSet *
PP_ZFileReader::GetMesh(int state, const char *var)
{
    debug4 << "PP_ZFileReader::GetMesh: state=" << state
           << ", var=" << var << endl;

    //
    // Make sure that everything is initialized.
    //
    InitializeVarStorage();

    //
    // Try getting the ray mesh.
    //
    vtkDataSet *retval = 0;
    if((retval = GetRayMesh(state, var)) != 0)
        return retval;
   
    //
    // Read the ireg variable, which tells us which are ghost zones.
    //
    int i, j;
    int nnodes = kmax * lmax;
    const int *ireg = GetIreg(state);

    //
    // If we're asking for the logical mesh, return it.
    //
    if(strcmp(var, "logical_mesh") == 0)
    {
        //
        // Populate the coordinates.  Put in 3D points with z=0 if the mesh is 2D.
        //
        vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New(); 
        int dims[3] = {kmax, lmax, 1};
        int ndims = 2;
        vtkFloatArray *coords[3];

        for (i = 0 ; i < 3 ; i++)
        {
            // Default number of components for an array is 1.
            coords[i] = vtkFloatArray::New();

            if (i < ndims)
            {
                coords[i]->SetNumberOfTuples(dims[i]);
                for (int j = 0; j < dims[i]; j++)
                    coords[i]->SetComponent(j, 0, j);
            }
            else
            {
                dims[i] = 1;
                coords[i]->SetNumberOfTuples(1);
                coords[i]->SetComponent(0, 0, 0.);
            }
        }

        rgrid->SetDimensions(dims);
        rgrid->SetXCoordinates(coords[0]);
        coords[0]->Delete();
        rgrid->SetYCoordinates(coords[1]);
        coords[1]->Delete();
        rgrid->SetZCoordinates(coords[2]);
        coords[2]->Delete();

        //
        // Mark some cells as ghost zones so they don't show up.
        //    
        CreateGhostZones(ireg, rgrid);

        return rgrid;
    }

    //
    // Read the variables that give the positions of the nodes.
    //
    TRY
    {
        ReadVariable(rtVar);
        ReadVariable(ztVar);
    }
    CATCH(InvalidVariableException)
    {
        EXCEPTION1(InvalidVariableException, var);
    }
    ENDTRY

    //
    // Both the unstructured grid and the structured grid need the points
    // populated in the same way so do it beforehand.
    //
    vtkPoints *points = vtkPoints::New();
    points->SetNumberOfPoints(nnodes);
    unsigned int adjustment = ((state < nCycles) ? (state * nnodes) : 0);

    VariableData *rtData = varStorage[rtVar];
    VariableData *ztData = varStorage[ztVar];
    if(rtData->dataType == DOUBLEARRAY_TYPE &&
       ztData->dataType == DOUBLEARRAY_TYPE)
    {
        const double *rt = (const double *)rtData->data;
        const double *zt = (const double *)ztData->data;
        rt += adjustment; zt += adjustment;
        GetMesh_StoreMeshPoints((float *)points->GetVoidPointer(0), rt, zt,
            kmax, lmax, nnodes);
    }
    else if(rtData->dataType == FLOATARRAY_TYPE &&
            ztData->dataType == FLOATARRAY_TYPE)
    {
        const float *rt = (const float *)rtData->data;
        const float *zt = (const float *)ztData->data;
        rt += adjustment; zt += adjustment;
        GetMesh_StoreMeshPoints((float *)points->GetVoidPointer(0), rt, zt,
            kmax, lmax, nnodes);
    }
    else
    {
        points->Delete();
        EXCEPTION1(InvalidVariableException, var);
    }

    if(strcmp(var, "revolved_mesh") == 0)
    {
        //
        // We want a revolved mesh so create an unstructured grid and
        // revolve it. We can't just revolve the structured grid because
        // it contains so many ghost zones that we don't get all of the
        // faces that we need.
        //
        vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
        ugrid->SetPoints(points);
        points->Delete();
        int nzones = (kmax - 1) * (lmax - 1);
        ugrid->Allocate(nzones);

        //
        // Add the zones that are not ghost zones.
        //
        for(j = 1; j < lmax; ++j)
        {
            const int *ireg_row = ireg + j * kmax;
            for(i = 1; i < kmax; ++i)
            {
                // Add it if it's a real zone.
                if(ireg_row[i] > 0)
                {
                    vtkIdType quad[4];
                    quad[0] = j * kmax + i;
                    quad[1] = j * kmax + i - 1;
                    quad[2] = (j-1) * kmax + i - 1;
                    quad[3] = (j-1) * kmax + i;
                    ugrid->InsertNextCell(VTK_QUAD, 4, quad);
                }
            }
        }

        //
        // Revolve the unstructured grid.
        //
        double axis[3] = {1., 0., 0.};
        retval = RevolveDataSet(ugrid, axis, 0., 360., revolutionSteps, true);
        ugrid->Delete();
    }
    else
    {
        //
        // Create a structured grid that uses the points array.
        //
        vtkStructuredGrid *sgrid = vtkStructuredGrid::New();
        sgrid->SetPoints(points);
        points->Delete();

        //
        // Tell the grid what its dimensions are.
        //
        int dims[3];
        dims[0] = kmax;
        dims[1] = lmax;
        dims[2] = 1;
        sgrid->SetDimensions(dims);

        //
        // Mark some cells as ghost zones so they don't show up.
        //    
        CreateGhostZones(ireg, sgrid);
        retval = sgrid;
    }

    return retval;
}

// ****************************************************************************
// Method: PP_ZFileReader::GetRayMesh
//
// Purpose: 
//   Returns a pointer to the ray mesh or 0 if the ray mesh was not requested.
//
// Arguments:
//   mesh : The name of the ray mesh to return.
//
// Returns:    A pointer to the requested ray mesh.
//
// Note:       Recurses for the revolved ray mesh.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 26 11:00:41 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet *
PP_ZFileReader::GetRayMesh(int state, const char *mesh)
{
    vtkDataSet *retval = 0;

    if(strcmp(mesh, "ray") == 0)
        retval = ConstructRayMesh(state, false);
    else if(strcmp(mesh, "ray3d") == 0)
        retval = ConstructRayMesh(state, true);
    
    return retval;
}

// ****************************************************************************
// Method: ConstructRayMesh_CreateMesh_3D
//
// Purpose: 
//   Creates a vtkPolyData object that contains 3D rays.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 7 09:44:13 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

template <class T>
inline vtkPolyData *
ConstructRayMesh_CreateMesh_3D(const int *mxp, const int *kptryp,
    const T *raypx, const T *raypy, const T *raypz,
    const int npts, const int nRays)
{
    //
    // Populate the coordinates for the ray mesh.
    //
    vtkPoints *points = vtkPoints::New();
    points->SetNumberOfPoints(npts);
    vtkPolyData *pd = vtkPolyData::New();
    pd->SetPoints(points);
    pd->Allocate(npts);
    points->Delete();
    float *pts = (float *)points->GetVoidPointer(0);

    float *tmp = pts;
    vtkIdType vertices[2];
    int pointIndex = 0;

    for(int nrr = 0; nrr < nRays; ++nrr)
    {
        if(mxp[nrr] != 0)
        {
            int i1 = kptryp[nrr];
            int i2 = kptryp[nrr] + mxp[nrr];

            for(int i = i1; i < i2; ++i)
            {
                *tmp++ = float(raypz[i]);
                *tmp++ = float(raypx[i]);
                *tmp++ = float(raypy[i]);

                if(i > i1)
                {
                    vertices[0] = pointIndex-1;
                    vertices[1] = pointIndex;
                    pd->InsertNextCell(VTK_LINE, 2, vertices);
                }

                ++pointIndex;
            }
        }
    }

    return pd;
}

// ****************************************************************************
// Method: ConstructRayMesh_CreateMesh_2D
//
// Purpose: 
//   Creates a vtkPolyData object that contains 2D rays.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 7 09:44:13 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

template <class T>
inline vtkPolyData *
ConstructRayMesh_CreateMesh_2D(const int *mxp,
    const int *kptryp, const T *raypz, const T *raypr, const int npts,
    const int nRays)
{
    //
    // Populate the coordinates for the ray mesh.
    //
    vtkPoints *points = vtkPoints::New();
    points->SetNumberOfPoints(npts);
    vtkPolyData *pd = vtkPolyData::New();
    pd->SetPoints(points);
    pd->Allocate(npts);
    points->Delete();
    float *pts = (float *)points->GetVoidPointer(0);

    float *tmp = pts;
    vtkIdType vertices[2];
    int pointIndex = 0;

    for(int nrr = 0; nrr < nRays; ++nrr)
    {
        if(mxp[nrr] != 0)
        {
            int i1 = kptryp[nrr];
            int i2 = kptryp[nrr] + mxp[nrr];

            for(int i = i1; i < i2; ++i)
            {
                *tmp++ = float(raypz[i]);
                *tmp++ = float(raypr[i]);
                *tmp++ = 0.f;

                if(i > i1)
                {
                    vertices[0] = pointIndex-1;
                    vertices[1] = pointIndex;
                    pd->InsertNextCell(VTK_LINE, 2, vertices);
                }

                ++pointIndex;
            }
        }
    }

    return pd;
}

// ****************************************************************************
// Method: PP_ZFileReader::ConstructRayMesh
//
// Purpose: 
//   Returns a 2d or 3d ray mesh.
//
// Arguments:
//   state : The time state for which we're returning the ray mesh.
//   is3d  : Whether we want a 3d mesh or a 2d mesh.
//
// Returns:    vtkPolyData containing the mesh.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 6 17:50:51 PST 2003
//
// Modifications:
//   Brad Whitlock, Fri Mar 5 10:50:03 PDT 2004
//   Fixed for Windows compiler.
//
//   Brad Whitlock, Mon Jun 7 09:12:34 PDT 2004
//   Added support for float coordinate arrays.
//
// ****************************************************************************

vtkDataSet *
PP_ZFileReader::ConstructRayMesh(int state, bool is3d)
{
    vtkDataSet *retval = 0;

    TRY
    {
        if(is3d)
        {
            ReadVariable("raypx");
            ReadVariable("raypy");
        }
        else
            ReadVariable("raypr");
        ReadVariable("raypz");
        ReadVariable("kptryp");
        ReadVariable("mxp");

        VariableData *raypr_data = 0;
        VariableData *raypx_data = 0;
        VariableData *raypy_data = 0;
        VariableData *raypz_data = 0;
        VariableData *kptryp_data = varStorage["kptryp"];
        VariableData *mxp_data = varStorage["mxp"];

        bool haveVars = false;
        bool doubleVars = false;
        if(is3d)
        {
            raypx_data = varStorage["raypx"];
            raypy_data = varStorage["raypy"];
            raypz_data = varStorage["raypz"];
            doubleVars = raypx_data->dataType == DOUBLEARRAY_TYPE &&
                         raypy_data->dataType == DOUBLEARRAY_TYPE &&
                         raypz_data->dataType == DOUBLEARRAY_TYPE;
            bool floatVars = raypx_data->dataType == FLOATARRAY_TYPE &&
                             raypy_data->dataType == FLOATARRAY_TYPE &&
                             raypz_data->dataType == FLOATARRAY_TYPE;
            haveVars = (doubleVars || floatVars) &&
                       kptryp_data->dataType == INTEGERARRAY_TYPE &&
                       mxp_data->dataType == INTEGERARRAY_TYPE;
        }
        else
        {
            raypr_data = varStorage["raypr"];
            raypz_data = varStorage["raypz"];
            doubleVars = raypr_data->dataType == DOUBLEARRAY_TYPE &&
                         raypz_data->dataType == DOUBLEARRAY_TYPE;
            bool floatVars = raypr_data->dataType == FLOATARRAY_TYPE &&
                             raypz_data->dataType == FLOATARRAY_TYPE;
            haveVars = (doubleVars || floatVars) &&
                       kptryp_data->dataType == INTEGERARRAY_TYPE &&
                       mxp_data->dataType == INTEGERARRAY_TYPE;
        }

        if(haveVars)
        {
            const int *kptryp = (const int *)kptryp_data->data;
            const int *mxp = (const int *)mxp_data->data;

            // Add time offsets to the arrays so we're looking at the right
            // time state.
#define ADD_TIME_OFFSET(P, ObjPtr) \
            P += (state < nCycles) ? (state * ObjPtr->nTotalElements / nCycles) : 0;

            ADD_TIME_OFFSET(kptryp, kptryp_data);
            ADD_TIME_OFFSET(mxp, mxp_data);
            int npts = 0;
            int nRays = mxp_data->nTotalElements / nCycles;
            for(int nrr = 0; nrr < nRays; ++nrr)
                npts += mxp[nrr];

            if(doubleVars)
            {
                const double *raypr = 0;
                const double *raypx = 0;
                const double *raypy = 0;
                const double *raypz = 0;

                if(is3d)
                {
                    raypx = (const double *)raypx_data->data;
                    raypy = (const double *)raypy_data->data;
                    raypz = (const double *)raypz_data->data;

                    ADD_TIME_OFFSET(raypx, raypx_data);
                    ADD_TIME_OFFSET(raypy, raypy_data);
                    ADD_TIME_OFFSET(raypz, raypz_data);
                    retval = ConstructRayMesh_CreateMesh_3D(mxp, kptryp, raypx,
                        raypy, raypz, npts, nRays);
                }
                else
                {
                    raypr = (const double *)raypr_data->data;
                    raypz = (const double *)raypz_data->data;
                    ADD_TIME_OFFSET(raypr, raypr_data);
                    ADD_TIME_OFFSET(raypz, raypz_data);
                    retval = ConstructRayMesh_CreateMesh_2D(mxp, kptryp, raypz,
                        raypr, npts, nRays);
                }
            }
            else
            {
                const float *raypr = 0;
                const float *raypx = 0;
                const float *raypy = 0;
                const float *raypz = 0;

                if(is3d)
                {
                    raypx = (const float *)raypx_data->data;
                    raypy = (const float *)raypy_data->data;
                    raypz = (const float *)raypz_data->data;

                    ADD_TIME_OFFSET(raypx, raypx_data);
                    ADD_TIME_OFFSET(raypy, raypy_data);
                    ADD_TIME_OFFSET(raypz, raypz_data);
                    retval = ConstructRayMesh_CreateMesh_3D(mxp, kptryp, raypx,
                        raypy, raypz, npts, nRays);
                }
                else
                {
                    raypr = (const float *)raypr_data->data;
                    raypz = (const float *)raypz_data->data;
                    ADD_TIME_OFFSET(raypr, raypr_data);
                    ADD_TIME_OFFSET(raypz, raypz_data);
                    retval = ConstructRayMesh_CreateMesh_2D(mxp, kptryp, raypz,
                        raypr, npts, nRays);
                }
            }
        }
        else
        {
            debug4 << "Some arrays were not the expected types!" << endl;
        }
    }
    CATCH(InvalidVariableException)
    {
        debug4 << "Could not read one of the required ray arrays." << endl;
    }
    ENDTRY

    return retval;
}

// ****************************************************************************
// Method: PP_ZFileReader::ReadVariable
//
// Purpose: 
//   Reads in a variable and caches it in the varStorage map.
//
// Arguments:
//   varName : The name of the variable to be read.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 26 16:06:40 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Dec 7 16:14:04 PST 2004
//   I put in code to convert double data to float to avoid trouble in more
//   frail pieces of the VisIt pipeline. Undo this change if we ever handle
//   the data using whatever type the user provided.
//
// ****************************************************************************

void
PP_ZFileReader::ReadVariable(const std::string &varStr)
{
    VariableDataMap::iterator pos;

    if((pos = varStorage.find(varStr)) != varStorage.end())
    {
        if(!pos->second->ReadValues(pdb))
        {
            delete pos->second;
            varStorage.erase(pos);
            EXCEPTION1(InvalidVariableException, varStr);
        }

#if 1
        //
        // This plugin handles doubles but many parts of VisIt do not
        // due to some "optimizations". To prevent this plugin from
        // causing VisIt to mess up later, we're now converting double
        // data to float. This will temporarily render some of the
        // explicit double coding in this plugin dead but it may be
        // possible to remove this code here someday.
        //
        if(pos->second->dataType == DOUBLEARRAY_TYPE)
        {
            int n = pos->second->nTotalElements;
            float *fdata = new float[n];
            float *dest = fdata;
            const double *src = (const double *)pos->second->data;
            for(int i = 0; i < n; ++i)
                *dest++ = float(*src++);
            free_void_mem(pos->second->data, DOUBLEARRAY_TYPE);
            pos->second->data = (void *)fdata;
            pos->second->dataType = FLOATARRAY_TYPE;
        }
#endif

    }
    else
    {
        EXCEPTION1(InvalidVariableException, varStr);
    }
}

// ****************************************************************************
// Function: GetMixArray
//
// Purpose: 
//   This is a template helper function for the ReadMixvarAndCache method. It
//   traverses the mixvar data just as the volume fractions array would be
//   traversed so that we create an array that is the same "shape" as the
//   volume fraction array.
//
// Arguments:
//   ireg   : Material numbers for the mesh.
//   ilamm  : Mixed material indexing array,
//   nummm  : Number of materials per cell for the mesh.
//   varmm  : Mixvar array.
//   kmax   : Cells in x.
//   lmax   : Cells in y;
//   mixlen : The length of the returned array.
//
// Returns:    Float array containing the mixvar data.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 6 18:02:28 PST 2004
//
// Modifications:
//   
// ****************************************************************************

template <class T>
float *
GetMixArray(const int *ireg, const int *ilamm, const int *nummm,
    const T *varmm, int kmax, int lmax, int &mixlen)
{
    int arraySize = kmax * lmax / 2;
    float *mixout = new float[arraySize];
    mixlen = 0;

    for(int j = 1; j < lmax; ++j)
    {
        const int *nummm_row = nummm + j * kmax;
        const int *ireg_row = ireg + j * kmax;
        const int *ilamm_row = ilamm + j * 2 * kmax;
        for(int i = 1; i < kmax; ++i)
        {
            int nMatsInCell = nummm_row[i];
            if(ireg_row[i] >= 1 && nMatsInCell != 0)
            {
                // Resize the mixout array.
                if(mixlen + nMatsInCell >= arraySize)
                {
                    int newSize = arraySize * 3 / 2; 
                    float *newData = new float[newSize];
                    float *src = mixout, *dest = newData;
                    for(int index = 0; index < mixlen; ++index)
                        *dest++ = *src++;
                    delete [] mixout;
                    mixout = newData;
                    arraySize = newSize;
                }

                // Copy the data into the mixout array.
                int minIndex = ilamm_row[2 * i] - 1;
                const T *src = varmm + minIndex;
                for(int index = 0; index < nMatsInCell; ++index)
                    mixout[mixlen++] = float(*src++);
            }
        }
    }

    return mixout;
}

// ****************************************************************************
// Method: PP_ZFileReader::ReadMixvarAndCache
//
// Purpose: 
//   Reads in a mixvar for the specified variable and adds it to the generic
//   database's variable cache.
//
// Arguments:
//   varStr  : The name of the variable for which we want a mixed var array.
//             The variable name is the name of the variable in the file
//             without the suffix.
//   realVar : The advertised name of the variable (as in the metadata).
//   state   : The timestate that we want.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 7 16:41:51 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Jun 7 09:47:10 PDT 2004
//   Added support for float arrays.
//
//   Brad Whitlock, Mon Dec 6 17:49:05 PST 2004
//   Changed the code so it works and removed the conditional compilation.
//
// ****************************************************************************

void
PP_ZFileReader::ReadMixvarAndCache(const std::string &varStr,
    const std::string &realVar, int state)
{
    const char *mName = "PP_ZFileReader::ReadMixvarAndCache: ";
    if(!assumeMixedMaterialsPresent)
    {
        debug4 << mName << "Returning because there is no evidence "
               << "of mixed materials." << endl;
    }

    VariableData *data = varStorage[varStr];
    std::string suffix(data->varName.substr(varStr.size()));
    std::string mixedVar(varStr + "mm");
    std::string mixedVarWithSuffix(mixedVar + suffix);

    if(pdb->SymbolExists(mixedVarWithSuffix.c_str()))
    {
        //
        // If the mixedVar is not cached, add an empty cache entry.
        //
        if(varStorage.find(mixedVar) == varStorage.end())
        {
            VariableData *v = new VariableData(mixedVarWithSuffix);
            varStorage[mixedVar] = v;
            debug4 << mName << "Added an empty entry in varStorage for "
                   << mixedVar << " since it was not already in the map."
                   << endl;
        }

        //
        // Try and read in required material vars.
        //
        TRY
        {
            ReadVariable("nummm");
            ReadVariable("ilamm");
            ReadVariable("ireg");
        }
        CATCH(InvalidVariableException)
        {
            debug4 << "Could not read required mixed material arrays." << endl;
            assumeMixedMaterialsPresent = false;
            CATCH_RETURN();
        }
        ENDTRY

        TRY
        {
            //
            // Try and read the mixed variable array.
            //
            ReadVariable(mixedVar);

            VariableData *ilamm_data = varStorage["ilamm"];
            VariableData *nummm_data = varStorage["nummm"];
            VariableData *ireg_data = varStorage["ireg"];
            
            if(ilamm_data->dataType == INTEGERARRAY_TYPE &&
               nummm_data->dataType == INTEGERARRAY_TYPE &&
               ireg_data->dataType == INTEGERARRAY_TYPE)
            {
                // Get a pointers to data at the right time state.
                int nnodes = kmax * lmax;
                const int *ireg = (const int *)data->data;
                ireg += ((state < nCycles) ? (state * nnodes) : 0);

                const int *nummm = (const int *)nummm_data->data;
                nummm += ((state < nCycles) ? (state * nnodes) : 0);

                const int *ilamm = (const int *)ilamm_data->data;
                ilamm += ((state < nCycles) ? (state * 2 * nnodes) : 0);

                //
                // Get the mixvar array.
                //
                int mixlen = 0;
                float *mixvar = 0;
                VariableData *varmm_data = varStorage[mixedVar];
                int mixOffset = 0;
                if(varmm_data->nDims > 1)
                {
                    mixOffset = varmm_data->dims[0];
                    debug4 << mixedVar.c_str() << "'s dims={";
                    for(int q = 0; q < varmm_data->nDims; ++q)
                        debug4 << varmm_data->dims[q] << ", ";
                    debug4 << "}" << endl;
                    debug4 << "mixOffset for " << mixedVar.c_str()
                           << "to be: " << mixOffset << endl;
                }
                else
                {
                    mixOffset = varmm_data->nTotalElements / nCycles;
                    debug4 << "Guessed mixOffset for " << mixedVar.c_str()
                           << "to be: " << mixOffset << endl;
                }

                if(varmm_data->dataType == FLOATARRAY_TYPE)
                {
                    const float *varmm = (const float *)varmm_data->data;
                    varmm += ((state < nCycles) ? (state * mixOffset) : 0);
                    mixvar = GetMixArray(ireg, ilamm, nummm, varmm,
                        kmax, lmax, mixlen);
                }
                else if(varmm_data->dataType == DOUBLEARRAY_TYPE)
                {
                    const double *varmm = (const double *)varmm_data->data;
                    varmm += ((state < nCycles) ? (state * mixOffset) : 0);
                    mixvar = GetMixArray(ireg, ilamm, nummm, varmm,
                        kmax, lmax, mixlen);
                }
                else if(varmm_data->dataType == INTEGERARRAY_TYPE)
                {
                    const int *varmm = (const int *)varmm_data->data;
                    varmm += ((state < nCycles) ? (state * mixOffset) : 0);
                    mixvar = GetMixArray(ireg, ilamm, nummm, varmm,
                        kmax, lmax, mixlen);
                }
                else
                {
                    debug4 << mName << "mixvar data type unsupported." << endl;
                }

                //
                // If we have a mixed variable array by this point, add it to
                // the generic database's cache.
                //
                if(mixvar)
                {
                    avtMixedVariable *mv = new avtMixedVariable(mixvar,
                        mixlen, realVar);
                    void_ref_ptr vr = void_ref_ptr(mv,
                        avtMixedVariable::Destruct);
                    cache->CacheVoidRef(realVar.c_str(),
                        AUXILIARY_DATA_MIXED_VARIABLE, state, 0, vr);
                    delete [] mixvar;
                }
            }
            else
            {
                debug4 << mName << "ilamm, ireg, or nummm was not int[]\n";
            }
        }
        CATCH(InvalidVariableException)
        {
            debug4 << mName << "The mixvar array: "
                   << mixedVarWithSuffix.c_str() << " could "
                   << "not be read!" << endl;
        }
        ENDTRY
    }
    else
    {
        debug4 << mName << "The mixvar array: " << mixedVarWithSuffix.c_str()
               << " was not found in the file." << endl;
    }
}

// ****************************************************************************
// Function: CopyVariableData
//
// Purpose:
//   This template function copies data from one destination array to memory
//   that is owned by a VTK object.
//
// Arguments:
//   ptr         : The destination array.
//   dataPointer : The source data array.
//   centering   : The variable centering, which indicates how it should be
//                 copied from the src to the destination array.
//   kmax        : Number of nodes in X.
//   lmax        : Number of nodes in Y.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 8 16:20:24 PST 2003
//
// Modifications:
//   
// ****************************************************************************

template <class T>
void
CopyVariableData(T *ptr, const T *dataPointer, avtCentering centering,
    int kmax, int lmax)
{
    int nnodes = kmax * lmax;

    if(centering == AVT_NODECENT)
    {
        // Nodal copy
        memcpy(ptr, dataPointer, sizeof(T) * nnodes);
    }
    else
    {
        // Zonal copy
        for(int j = 1; j < lmax; ++j)
        {
            const T *var_row = dataPointer + j * kmax;
            for(int i = 1; i < kmax; ++i)
                *ptr++ = var_row[i];
        }
    }
}

// ****************************************************************************
// Function: CopyRevolvedVariableData
//
// Purpose:
//   This template function copies data from one destination array to memory
//   that is owned by a VTK object.
//
// Arguments:
//   ptr         : The destination array.
//   dataPointer : The source data array.
//   ireg        : The file's ireg array.
//   centering   : The variable centering, which indicates how it should be
//                 copied from the src to the destination array.
//   kmax        : Number of nodes in X.
//   lmax        : Number of nodes in Y.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 8 16:20:24 PST 2003
//
// Modifications:
//   
// ****************************************************************************

template <class T>
void
CopyRevolvedVariableData(T *ptr, const T *dataPointer, const int *ireg,
    avtCentering centering, int kmax, int lmax, int revolutionSteps)
{
    int nnodes = kmax * lmax;

    if(centering == AVT_NODECENT)
    {
        // Nodal copy
        for(int k = 0; k < revolutionSteps; ++k)
        {
            memcpy(ptr, dataPointer, sizeof(T) * nnodes);
            ptr += nnodes;
        }
    }
    else
    {
        // Zonal copy
        for(int j = 1; j < lmax; ++j)
        {
            const int *ireg_row = ireg + j * kmax;
            const T *var_row = dataPointer + j * kmax;
            for(int i = 1; i < kmax; ++i)
            {
                if(ireg_row[i] > 0)
                {
                    for(int k = 0; k < revolutionSteps - 1; ++k)
                        *ptr++ = var_row[i];
                }
            }
        }
    }
}

// ****************************************************************************
// Method: PP_ZFileReader::GetUnstructuredCellCount
//
// Purpose: 
//   Returns the number of cells that would be in an unstructured version of
//   the mesh.
//
// Returns:    The number of cells that would be in an unstructured version of
//             the mesh.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 8 16:30:55 PST 2003
//
// Modifications:
//   
// ****************************************************************************

int
PP_ZFileReader::GetUnstructuredCellCount()
{
    if(unstructuredCellCount < 0)
    {
        const int *ireg = GetIreg(0);
        if(ireg)
        {
            unstructuredCellCount = 0;
            for(int j = 1; j < lmax; ++j)
            {
                const int *ireg_row = ireg + j * kmax;
                for(int i = 1; i < kmax; ++i)
                {
                    if(ireg_row[i] > 0)
                        ++unstructuredCellCount;
                }
            }
        }
    }

    return unstructuredCellCount;
}

// ****************************************************************************
// Method: GetRayVar_StoreRayData
//
// Purpose: 
//   Stores ray data into an input data array.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 7 09:54:27 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

template <class T>
inline void
GetRayVar_StoreRayData(T *ptr, const T *dataPointer, const int *mxp,
    const int *kptryp, const int nRays, bool relative)
{
    T *dest = ptr;

    if(relative)
    {
        for(int nrr = 0; nrr < nRays; ++nrr)
        {
            if(mxp[nrr] != 0)
            {
                int i1 = kptryp[nrr];
                int i2 = kptryp[nrr] + mxp[nrr];

                T firstVal = dataPointer[i1];
                if(firstVal == 0.)
                    firstVal = 1.;
                for(int i = i1; i < i2; ++i)
                    *dest++ = (dataPointer[i] / firstVal);
            }
        }
    }
    else
    {
        for(int nrr = 0; nrr < nRays; ++nrr)
        {
            if(mxp[nrr] != 0)
            {
                int i1 = kptryp[nrr];
                int i2 = kptryp[nrr] + mxp[nrr];

                for(int i = i1; i < i2; ++i)
                    *dest++ = dataPointer[i];
            }
        }
    }
}

// ****************************************************************************
// Method: PP_ZFileReader::GetRayVar
//
// Purpose: 
//   Gets a variable that is defined on a ray mesh.
//
// Arguments:
//   state  : The time state for which we want the data.
//   varStr : The variable to return.
//
// Returns:    A vtkDataArray object containing the variable data.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 6 18:00:19 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Jun 7 09:58:13 PDT 2004
//   I added support for float arrays.
//
// ****************************************************************************

vtkDataArray *
PP_ZFileReader::GetRayVar(int state, const std::string &varStr)
{
    vtkDataArray *retval = 0;

    //
    // Read in the the required variables.
    //
    ReadVariable(varStr);
    ReadVariable("kptryp");
    ReadVariable("mxp");

    VariableData *kptryp_data = varStorage["kptryp"];
    VariableData *mxp_data = varStorage["mxp"];
    VariableData *var_data = varStorage[varStr];

    if(kptryp_data->dataType == INTEGERARRAY_TYPE &&
       mxp_data->dataType == INTEGERARRAY_TYPE)
    {
        const int *kptryp = (const int *)kptryp_data->data;
        const int *mxp = (const int *)mxp_data->data;
        int nrr, npts = 0;

        // Add an offset the pointers so they are at the desired time state.
        ADD_TIME_OFFSET(kptryp, kptryp_data);
        ADD_TIME_OFFSET(mxp, mxp_data);

        // Determine the number of points in the rays.
        int nRays = mxp_data->nTotalElements / nCycles;
        for(nrr = 0; nrr < nRays; ++nrr)
            npts += mxp[nrr];

        //
        // Create the data array
        //
        bool relative = (varStr == "rel_power");
        if(var_data->dataType == DOUBLEARRAY_TYPE)
        {
            vtkDoubleArray *dscalars = vtkDoubleArray::New();
            dscalars->SetNumberOfTuples(npts);
            retval = dscalars;
            double *ptr = (double *)dscalars->GetVoidPointer(0);

            // Add an offset the data pointer so they are at the desired
            // time state.
            const double *dataPointer = (const double *)var_data->data;
            ADD_TIME_OFFSET(dataPointer, var_data);

            GetRayVar_StoreRayData(ptr, dataPointer, mxp, kptryp, 
                nRays, relative);
        }
        else if(var_data->dataType == FLOATARRAY_TYPE)
        {
            vtkFloatArray *fscalars = vtkFloatArray::New();
            fscalars->SetNumberOfTuples(npts);
            retval = fscalars;
            float *ptr = (float *)fscalars->GetVoidPointer(0);

            // Add an offset the data pointer so they are at the desired
            // time state.
            const float *dataPointer = (const float *)var_data->data;
            ADD_TIME_OFFSET(dataPointer, var_data);

            GetRayVar_StoreRayData(ptr, dataPointer, mxp, kptryp, 
                nRays, relative);
        }
        else
        {
            debug4 << "The ray data is not of a supported type." << endl;
        }
    }
    else
    {
        debug4 << "Some variables were not of the right type!" << endl;
    }
    
    return retval;
}

// ****************************************************************************
// Method: PP_ZFileReader::GetVar
//
// Purpose: 
//   Returns data for the specified variable and time state.
//
// Arguments:
//   state : The time state.
//   var   : The variable that we want to read.
//
// Returns:    A data array containing the data that we want.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 26 16:01:18 PST 2003
//
// Modifications:
//   Brad Whitlock, Fri Aug 8 16:08:58 PST 2003
//   Added support for zonal variables and variables defined on the new
//   revolved mesh.
//
//   Brad Whitlock, Wed Sep 17 12:03:50 PDT 2003
//   I made it call InitializeVarStorage.
//
//   Brad Whitlock, Mon Oct 6 17:54:30 PST 2003
//   I added support for ray variables.
//
//   Brad Whitlock, Mon Jun 7 10:00:04 PDT 2004
//   I added support for float arrays.
//
//   Brad Whitlock, Mon Dec 6 16:54:18 PST 2004
//   Passed the real variable name to ReadMixvarAndCache. I also made that
//   routine only be called on non-revolved meshes since it's not worth it
//   to revolve that information.
//
// ****************************************************************************

vtkDataArray *
PP_ZFileReader::GetVar(int state, const char *var)
{
    debug4 << "PP_ZFileReader::GetVar: state=" << state
           << ", var=" << var << endl;

    //
    // Make sure that everything is initialized.
    //
    InitializeVarStorage();

    //
    // If we're asking for the variable over the logical mesh, it's really
    // the same variable so strip off the logical_mesh part of the name.
    //
    std::string varStr(var);
    std::string meshName("mesh/");
    std::string logical("logical_mesh/");
    std::string revolved("revolved_mesh/");
    std::string ray("ray/");
    std::string ray3d("ray3d/");
    bool wantRevolvedMesh = false;
    if(varStr.substr(0, meshName.size()) == meshName)
        varStr = varStr.substr(meshName.size());
    else if(varStr.substr(0, logical.size()) == logical)
        varStr = varStr.substr(logical.size());
    else if(varStr.substr(0, revolved.size()) == revolved)
    {
        varStr = varStr.substr(revolved.size());
        wantRevolvedMesh = true;
    }
    else if(varStr.substr(0, ray.size()) == ray)
    {
        varStr = varStr.substr(ray.size());
        return GetRayVar(state, varStr);
    }
    else if(varStr.substr(0, ray3d.size()) == ray3d)
    {
        varStr = varStr.substr(ray3d.size());
        return GetRayVar(state, varStr);
    }

    //
    // Read in the data for the variable if it hasn't been read yet.
    //
    ReadVariable(varStr);

    //
    // Determine the variable centering and use that to figure out how
    // large to allocate the data array.
    //
    int nels;
    int nnodes = kmax * lmax;
    int nzones = (kmax - 1) * (lmax - 1);
    avtCentering centering;
    const int *ireg = 0;
    if(VariableIsNodal(varStr))
    {
        nels = wantRevolvedMesh ? (nnodes * revolutionSteps) : nnodes;
        centering = AVT_NODECENT;
    }
    else
    {
        centering = AVT_ZONECENT;
        if(wantRevolvedMesh)
        {
            int size = GetUnstructuredCellCount();
            if(size < 0)
                EXCEPTION1(InvalidVariableException, varStr);

            ireg = GetIreg(state);
            nels = size * (revolutionSteps - 1);
        }
        else
            nels = nzones;
    }

    //
    // Now that the variable is in memory, put it in a VTK data structure.
    //
    vtkDataArray *scalars = 0;
    VariableData *data = varStorage[varStr];

    if(data->dataType == INTEGERARRAY_TYPE)
    {
        vtkIntArray *iscalars = vtkIntArray::New();
        iscalars->SetNumberOfTuples(nels);
        scalars = iscalars;
        int *ptr = (int *) iscalars->GetVoidPointer(0);

        // Get a pointer to the start of the VariableData object's data array.
        const int *dataPointer = (const int *)data->data;
        dataPointer += ((state < nCycles) ? (state * nnodes) : 0);

        //
        // Copy the data appropriately.
        //
        if(wantRevolvedMesh)
        {
            CopyRevolvedVariableData(ptr, dataPointer, ireg, centering,
                                     kmax, lmax, revolutionSteps);
        }
        else
        {
            CopyVariableData(ptr, dataPointer, centering, kmax, lmax);
            ReadMixvarAndCache(varStr, var, state);
        }
    }
    else if(data->dataType == DOUBLEARRAY_TYPE)
    {
        vtkDoubleArray *dscalars = vtkDoubleArray::New();
        dscalars->SetNumberOfTuples(nels);
        scalars = dscalars;
        double *ptr = (double *) dscalars->GetVoidPointer(0);

        // Get a pointer to the start of the VariableData object's data array.
        const double *dataPointer = (const double *)data->data;
        dataPointer += ((state < nCycles) ? (state * nnodes) : 0);

        //
        // Copy the data appropriately.
        //
        if(wantRevolvedMesh)
        {
            CopyRevolvedVariableData(ptr, dataPointer, ireg, centering,
                                     kmax, lmax, revolutionSteps);
        }
        else
        {
            CopyVariableData(ptr, dataPointer, centering, kmax, lmax);
            ReadMixvarAndCache(varStr, var, state);
        }
    }
    else if(data->dataType == FLOATARRAY_TYPE)
    {
        vtkFloatArray *fscalars = vtkFloatArray::New();
        fscalars->SetNumberOfTuples(nels);
        scalars = fscalars;
        float *ptr = (float *) fscalars->GetVoidPointer(0);

        // Get a pointer to the start of the VariableData object's data array.
        const float *dataPointer = (const float *)data->data;
        dataPointer += ((state < nCycles) ? (state * nnodes) : 0);

        //
        // Copy the data appropriately.
        //
        if(wantRevolvedMesh)
        {
            CopyRevolvedVariableData(ptr, dataPointer, ireg, centering,
                                     kmax, lmax, revolutionSteps);
        }
        else
        {
            CopyVariableData(ptr, dataPointer, centering, kmax, lmax);
            ReadMixvarAndCache(varStr, var, state);
        }
    }
    else
    {
        debug4 << "GetVar: Unsupported data type: "
               << int(data->dataType) << endl;
    }

    return scalars;
}

// ****************************************************************************
// Function: FindGhostMaterial
//
// Purpose: 
//   Finds the first non-ghost material value so we can use it for the
//   material in ghost cells so VisIt does not yell at us for using an
//   invalid material.
//
// Returns:    The material number to use in ghost cells.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 8 12:02:03 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

int
FindGhostMaterial(const int *ireg, int kmax, int lmax)
{
    int ghostMaterial = 1;
    bool keepLooking = true;
    for(int j = 1; j < lmax && keepLooking; ++j)
    {
         const int *ireg_row = ireg + j * kmax;
         for(int i = 1; i < kmax && keepLooking; ++i)
         {
             if(ireg_row[i] > 0)
             {
                 ghostMaterial = ireg_row[i];
                 keepLooking = false;
             }
         }
    }
    debug4 << "Using material " << ghostMaterial << " for ghost cells."
           << endl;

    return ghostMaterial;
}

// ****************************************************************************
// Function: AddCleanMaterials
//
// Purpose:
//   Adds clean materials to the material encoder for the regular 2d mesh or
//   for the revolved mesh.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 11 13:56:37 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Sep 7 16:55:20 PST 2004
//   I prevented invalid material ids from being used in ghost zones since
//   it was causing VisIt to complain.
//
// ****************************************************************************

static void
AddCleanMaterials(MaterialEncoder &mats, const int *ireg, int kmax, int lmax,
    bool wantRevolvedMaterial, int nSteps)
{
    //
    // Add materials for all zones in a way that matches the mesh.
    //
    if(wantRevolvedMaterial)
    {
        int zoneId = 0;
        for(int j = 1; j < lmax; ++j)
        {
            const int *ireg_row = ireg + j * kmax;
            for(int i = 1; i < kmax; ++i)
            {
                if(ireg_row[i] > 0)
                {
                    for(int k = 0; k < nSteps; ++k)
                        mats.AddClean(zoneId++, ireg_row[i]);
                }
            }
        }
    }
    else
    {
        //
        // Look for a valid material that we can use when we have to
        // add a material for a ghost zone.
        //
        int ghostMaterial = FindGhostMaterial(ireg, kmax, lmax);

        int zoneId = 0;
        for(int j = 1; j < lmax; ++j)
        {
            const int *ireg_row = ireg + j * kmax;
            for(int i = 1; i < kmax; ++i, ++zoneId)
            {
                int matno = (ireg_row[i] > 0) ? ireg_row[i] : ghostMaterial;
                mats.AddClean(zoneId, matno);
            }
        }
    }
}

// ****************************************************************************
// Function: AddMixedMaterials
//
// Purpose:
//   Adds mixed materials to the material encoder for the regular 2d mesh or
//   for the revolved mesh.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 11 13:56:37 PST 2003
//
// Modifications:
//   Brad Whitlock, Thu Sep 18 17:15:21 PST 2003
//   Fixed ilamm offset so it uses 0 origin array.
//
//   Brad Whitlock, Mon Jun 7 10:02:11 PDT 2004
//   Removed stray debugging statements.
//
//   Brad Whitlock, Tue Sep 7 16:57:28 PST 2004
//   Added code to make sure that VisIt does not use an invalid material
//   number in ghost zones.
//
//   Brad Whitlock, Tue Dec 7 16:20:23 PST 2004
//   I changed the volfmm argument to float.
//
// ****************************************************************************

static void
AddMixedMaterials(MaterialEncoder &mats, const int *ireg, const int *iregmm,
    const int *nummm, const int *ilamm, const float *volfmm, int kmax,
    int lmax, bool wantRevolvedMaterial, int nSteps)
{
    //
    // Add materials for all zones in a way that matches the mesh.
    //
    if(wantRevolvedMaterial)
    {
        int zoneId = 0;
        for(int j = 1; j < lmax; ++j)
        {
             const int *nummm_row = nummm + j * kmax;
             const int *ireg_row = ireg + j * kmax;
             const int *ilamm_row = ilamm + j * 2 * kmax;
             for(int i = 1; i < kmax; ++i)
             {
                 if(ireg_row[i] > 0)
                 {
                     if(nummm_row[i] == 0)
                     {
                         for(int k = 0; k < nSteps; ++k)
                             mats.AddClean(zoneId++, ireg_row[i]);
                     }
                     else
                     {
                         int minIndex = ilamm_row[2 * i] - 1;
                         for(int k = 0; k < nSteps; ++k)
                             mats.AddMixed(zoneId++,
                                           iregmm + minIndex,
                                           volfmm + minIndex,
                                           nummm_row[i]);
                     }
                 }
             }
        }
    }
    else
    {
        //
        // Look for a valid material that we can use when we have to
        // add a material for a ghost zone.
        //
        int ghostMaterial = FindGhostMaterial(ireg, kmax, lmax);

        int zoneId = 0;
        for(int j = 1; j < lmax; ++j)
        {
             const int *nummm_row = nummm + j * kmax;
             const int *ireg_row = ireg + j * kmax;
             const int *ilamm_row = ilamm + j * 2 * kmax;
             for(int i = 1; i < kmax; ++i, ++zoneId)
             {
                 if(ireg_row[i] < 1)
                 {
                     // This is a ghost zone. To make VisIt be quiet about
                     // "invalid material 0", stick a valid material number
                     // in the ghost zone.
                     mats.AddClean(zoneId, ghostMaterial);
                 }
                 else if(nummm_row[i] == 0)
                     mats.AddClean(zoneId, ireg_row[i]);
                 else
                 {
                     int minIndex = ilamm_row[2 * i] - 1;
                     mats.AddMixed(zoneId,
                                   iregmm + minIndex,
                                   volfmm + minIndex,
                                   nummm_row[i]);
                 }
             }
        }
    }
}

// ****************************************************************************
// Method: PP_ZFileReader::GetAuxiliaryData
//
// Purpose: 
//   Returns auxiliary data such as materials.
//
// Arguments:
//   state : The time state that we're interested in.
//   var   : The name of the variable for which to return auxiliary data.
//   type  : The type of aux data that we want.
//   args  : ?
//   df    : The function to use when destroying the returned data.
//
// Returns:    An object that contains auxiliary data.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 5 17:35:49 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Jun 7 11:20:56 PDT 2004
//   Added support for float arrays.
//
//   Brad Whitlock, Wed Sep 8 13:39:33 PST 2004
//   Fixed mixed materials over time so they work correctly with Flash files.
//
//   Brad Whitlock, Tue Dec 7 16:19:09 PST 2004
//   Changed some float->double code to double->float and disabled it since
//   we're converting doubles to float at read time in ReadVariable.
//
// ****************************************************************************

void  *
PP_ZFileReader::GetAuxiliaryData(int state, const char *var, const char *type,
    void *args, DestructorFunction &df)
{
    avtMaterial *retval = 0;
    if(strcmp(type, AUXILIARY_DATA_MATERIAL) == 0)
    {
        df = avtMaterial::Destruct;

        //
        // Initialize some variables based on the variable name.
        //
        int  nnodes = kmax * lmax;
        int  nzones = (kmax-1)*(lmax-1);
        bool validVariable = false;
        bool wantRevolvedMaterial = false;
        int  nSteps = 1;
        int  nCleanZones = nzones;
        if(strcmp(var, "material") == 0)
        {
            validVariable = true;
        }
        else if(strcmp(var, "material2") == 0)
        {
            wantRevolvedMaterial = true;
            validVariable = true;
            nSteps = revolutionSteps - 1;
            nCleanZones = GetUnstructuredCellCount() * nSteps;
        }
           
        if(validVariable)
        {
            std::string varStr("ireg");
            VariableData *data = varStorage[varStr];
            const int *ireg = 0;
            if(data->dataType == INTEGERARRAY_TYPE)
            {
                // Get a pointer to ireg's data at the right time state.
                ireg = (const int *)data->data;
                ireg += ((state < nCycles) ? (state * nnodes) : 0);

                //
                // Try and read the mixed material variables.
                //
                if(assumeMixedMaterialsPresent)
                {
                    TRY
                    {
                        ReadVariable("nummm");
                        ReadVariable("ilamm");
                        ReadVariable("volfmm");
                        ReadVariable("iregmm");
                    }
                    CATCH(InvalidVariableException)
                    {
                        assumeMixedMaterialsPresent = false; 
                    }
                    ENDTRY
                }

                //
                // Create a MaterialEncoder object and define materials.
                //
                MaterialEncoder mats;
                mats.AllocClean(nCleanZones);
                for(int i = 0; i < materialNames.size(); ++i)
                    mats.AddMaterial(materialNames[i]);

                //
                // Populate the MaterialEncoder object and define materials.
                //
                if(assumeMixedMaterialsPresent)
                {
                    VariableData *nummm_data  = varStorage["nummm"];
                    VariableData *ilamm_data  = varStorage["ilamm"];
                    VariableData *volfmm_data = varStorage["volfmm"];
                    VariableData *iregmm_data = varStorage["iregmm"];

                    if(nummm_data->dataType == INTEGERARRAY_TYPE &&
                       ilamm_data->dataType == INTEGERARRAY_TYPE &&
                       (volfmm_data->dataType == DOUBLEARRAY_TYPE ||
                        volfmm_data->dataType == FLOATARRAY_TYPE) &&
                       iregmm_data->dataType == INTEGERARRAY_TYPE)
                    {
                        // Figure out the size of each time step for the
                        // iregmm, volfmm arrays.
                        int mixOffset = 0;
                        if(iregmm_data->nDims > 1)
                            mixOffset = iregmm_data->dims[0];
                        else
                            mixOffset = iregmm_data->nTotalElements / nCycles;

                        const int *nummm = (const int *)nummm_data->data;
                        nummm += ((state < nCycles) ? (state * nnodes) : 0);

                        const int *ilamm = (const int *)ilamm_data->data;
                        ilamm += ((state < nCycles) ? (state * 2 * nnodes) : 0);

                        const int *iregmm = (const int *)iregmm_data->data;
                        iregmm += ((state < nCycles) ? (state * mixOffset) : 0);
#if 0
                        if(volfmm_data->dataType == DOUBLEARRAY_TYPE)
                        {
                            // Convert the double data to float.
                            float *f = new float[volfmm_data->nTotalElements];
                            float *fptr = f;
                            const double *src = (const double *)volfmm_data->data;
                            for(int i = 0; i < volfmm_data->nTotalElements; ++i)
                                *fptr++ = float(*src++);
                            
                            // Free the old data.
                            free_void_mem(volfmm_data->data, DOUBLEARRAY_TYPE);

                            // Store the converted float data.
                            volfmm_data->data = (void *)f;
                            volfmm_data->dataType = FLOATARRAY_TYPE;
                        }
#endif
                        //
                        // Add materials for all of the zones.
                        //
                        const float *volfmm = (const float *)volfmm_data->data;
                        volfmm += ((state < nCycles) ? (state * mixOffset) : 0);
                        AddMixedMaterials(mats, ireg, iregmm, nummm, ilamm,
                                          volfmm, kmax, lmax,
                                          wantRevolvedMaterial, nSteps);
                    }
                    else
                    {
                        debug4 << "PP_ZFileReader::GetAuxiliaryData: "
                               << "Some of the mixed material arrays had the "
                               << "wrong types. Making all zones clean."
                               << endl;

                        //
                        // Add materials for all of the zones but make them
                        // all clean.
                        //
                        AddCleanMaterials(mats, ireg, kmax, lmax,
                                          wantRevolvedMaterial, nSteps);
                    }
                }
                else
                {
                    //
                    // Add materials for all of the zones but make them
                    // all clean.
                    //
                    AddCleanMaterials(mats, ireg, kmax, lmax,
                                      wantRevolvedMaterial, nSteps);
                }

                //
                // Create an avtMaterial using the MaterialEncoder object.
                //
                if(wantRevolvedMaterial)
                {
                    int dims[3] = {nCleanZones, 1, 1};
                    retval = mats.CreateMaterial(dims, 3);
                }
                else
                {
                    int dims[2] = {kmax-1, lmax-1};
                    retval = mats.CreateMaterial(dims, 2);
                }
            }
        }
    }

    return retval;
}

// ****************************************************************************
//  Function: GetRotationMatrix
//
//  Purpose:
//      Given an angle and an axis, this creates a matrix that will rotate a
//      point around that axis.
//
//  Programmer: Hank Childs
//  Creation:   December 11, 2002
//
//  Modifications:
//    Brad Whitlock, Wed Aug 6 12:16:01 PDT 2003
//    I stole this code from avtRevolveFilter in the Revolve operator.
//
// ****************************************************************************

static void
GetRotationMatrix(double angle, const double axis[3], vtkMatrix4x4 *mat)
{
    //
    // The game plan is to transform into a coordinate space that we are
    // familiar with, perform the rotation there, and then rotate back.
    //

    //
    // First rotate to the yz plane.  We will do this by rotating by theta
    // around the y-axis, where theta is arctan(axis[0] / axis[2]).
    //
    vtkMatrix4x4 *rot1 = vtkMatrix4x4::New();
    rot1->Identity();
    vtkMatrix4x4 *rot5 = vtkMatrix4x4::New();
    rot5->Identity();
    if (axis[0] != 0.)
    {
        double theta = atan2(axis[0], axis[2]);
        double cos_theta = cos(theta);
        double sin_theta = sin(theta);
        rot1->SetElement(0, 0, cos_theta);
        rot1->SetElement(2, 0, -sin_theta);
        rot1->SetElement(0, 2, sin_theta);
        rot1->SetElement(2, 2, cos_theta);

        //
        // Rot 5 will be a rotation around -theta.
        //
        double cos_minus_theta = cos_theta;
        double sin_minus_theta = -sin_theta;
        rot5->SetElement(0, 0, cos_minus_theta);
        rot5->SetElement(2, 0, -sin_minus_theta);
        rot5->SetElement(0, 2, sin_minus_theta);
        rot5->SetElement(2, 2, cos_minus_theta);
    }

    //
    // Now rotate around the x-axis until we get the to the z-axis.
    //
    vtkMatrix4x4 *rot2 = vtkMatrix4x4::New();
    rot2->Identity();
    vtkMatrix4x4 *rot4 = vtkMatrix4x4::New();
    rot4->Identity();
    if (axis[1] != 0.)
    {
        double theta = atan2(axis[1], sqrt(axis[0]*axis[0] + axis[2]*axis[2]));
        double cos_theta = cos(theta);
        double sin_theta = sin(theta);
        rot2->SetElement(1, 1, cos_theta);
        rot2->SetElement(2, 1, sin_theta);
        rot2->SetElement(1, 2, -sin_theta);
        rot2->SetElement(2, 2, cos_theta);

        //
        // Rot 4 will be a rotation around -theta.
        //
        double cos_minus_theta = cos_theta;
        double sin_minus_theta = -sin_theta;
        rot4->SetElement(1, 1, cos_minus_theta);
        rot4->SetElement(2, 1, sin_minus_theta);
        rot4->SetElement(1, 2, -sin_minus_theta);
        rot4->SetElement(2, 2, cos_minus_theta);
    }

    //
    // Now we can do the easy rotation around the z-axis.
    //
#ifdef _WIN32
    double angle_rad = (angle / 360. * 2. * 3.14159265358979323846);
#else
    double angle_rad = (angle / 360. * 2. * M_PI);
#endif
    vtkMatrix4x4 *rot3 = vtkMatrix4x4::New();
    rot3->Identity();
    double cos_angle = cos(angle_rad);
    double sin_angle = sin(angle_rad);
    rot3->SetElement(0, 0, cos_angle);
    rot3->SetElement(1, 0, sin_angle);
    rot3->SetElement(0, 1, -sin_angle);
    rot3->SetElement(1, 1, cos_angle);

    //
    // Now set up our matrix.
    //
    vtkMatrix4x4 *tmp  = vtkMatrix4x4::New();
    vtkMatrix4x4 *tmp2 = vtkMatrix4x4::New();
    vtkMatrix4x4::Multiply4x4(rot5, rot4, tmp);
    vtkMatrix4x4::Multiply4x4(tmp, rot3, tmp2);
    vtkMatrix4x4::Multiply4x4(tmp2, rot2, tmp);
    vtkMatrix4x4::Multiply4x4(tmp, rot1, mat);

    tmp->Delete();
    tmp2->Delete();
    rot1->Delete();
    rot2->Delete();
    rot3->Delete();
    rot4->Delete();
    rot5->Delete();
}

// ****************************************************************************
//  Function: RevolveDataSetHelper
//
//  Purpose:
//      Sends the specified input and output through the Revolve filter.
//
//  Arguments:
//      in_ds      The input dataset.
//      <unused>   The domain number.
//      <unused>   The label.
//
//  Returns:       The output dataset.
//
//  Programmer: childs -- generated by xml2info
//  Creation:   Wed Dec 11 11:31:52 PDT 2002
//
//  Modifications:
//     Brad Whitlock, Wed Aug 6 12:14:51 PDT 2003
//     I stole this code from avtRevolveFilter in the Revolve operator.
//
//     Brad Whitlock, Fri Sep 26 11:04:48 PDT 2003
//     I turned it into a template function and added support for
//     revolving lines. I changed the code a little so there can never
//     be uninitialized points to interfere with computing the plot bounds.
//
// ****************************************************************************

template <class T>
static vtkDataSet *
RevolveDataSetHelper(T *ugrid, vtkDataSet *in_ds,
    const double *axis, double start_angle, double stop_angle, int nsteps)
{
    int   i, j;

    //
    // Set up our VTK structures.
    //
    vtkMatrix4x4 *mat = vtkMatrix4x4::New();
    vtkPoints *pts = vtkPoints::New();
    int npts = in_ds->GetNumberOfPoints();
    int ncells = in_ds->GetNumberOfCells();
    int niter = (fabs(stop_angle-start_angle-360.) < 0.001 ? nsteps : nsteps+1);
    int n_out_pts = npts * niter;
    pts->SetNumberOfPoints(n_out_pts);
    ugrid->SetPoints(pts);

    //
    // Create the points for each timestep.
    //
    float *ptr = (float *) pts->GetVoidPointer(0);
    for (i = 0 ; i < niter ; i++)
    {
        double angle = ((stop_angle-start_angle)*i)/(niter-1) + start_angle;
        GetRotationMatrix(angle, axis, mat);
        for (j = 0 ; j < npts ; j++)
        {
            float pt[4];
            in_ds->GetPoint(j, pt);
            pt[3] = 1.;
            float outpt[4];
            mat->MultiplyPoint(pt, outpt);
            *ptr++ = outpt[0];
            *ptr++ = outpt[1];
            *ptr++ = outpt[2];
        }
    }

    //
    // Now set up the connectivity.  The output will consist of revolved
    // quads (-> hexes) and revolved triangles (-> wedges).  No special care is
    // given to the case where an edge of a cell lies directly on the axis of
    // revolution (ie: you get a degenerate hex, not a wedge).
    //
    int n_out_cells = ncells*(niter-1);
    ugrid->Allocate(8*n_out_cells);
    bool overlap_ends = (fabs(stop_angle-start_angle-360.) < 0.001);
    for (i = 0 ; i < ncells ; i++)
    {
         vtkCell *cell = in_ds->GetCell(i);
         int c = cell->GetCellType();
         if (c != VTK_QUAD && c != VTK_TRIANGLE && c != VTK_PIXEL &&
             c != VTK_LINE)
         {
             EXCEPTION1(InvalidCellTypeException, "anything but quads and"
                                                  " tris.");
         }
         vtkIdList *list = cell->GetPointIds();

         if (c == VTK_TRIANGLE)
         {
             int pt0 = list->GetId(0);
             int pt1 = list->GetId(1);
             int pt2 = list->GetId(2);
             int npts_times_j = 0;
             int npts_times_j1 = npts;
             for (j = 0 ; j < niter-1 ; j++)
             {
                 vtkIdType wedge[6];
                 wedge[0] = npts_times_j + pt0;
                 wedge[1] = npts_times_j + pt1;
                 wedge[2] = npts_times_j + pt2;
                 wedge[3] = npts_times_j1 + pt0;
                 wedge[4] = npts_times_j1 + pt1;
                 wedge[5] = npts_times_j1 + pt2;
                 if (j == niter-2 && overlap_ends)
                 {
                     wedge[3] = pt0;
                     wedge[4] = pt1;
                     wedge[5] = pt2;
                 }
                 ugrid->InsertNextCell(VTK_WEDGE, 6, wedge);
                 npts_times_j += npts;
                 npts_times_j1 += npts;
             }
         }
         else if(c == VTK_LINE)
         {
             // We revolve the line to create more lines; not a surface.
             int pt0 = list->GetId(0);
             int pt1 = list->GetId(1);
             int npts_times_j = 0;
             for(j = 0; j < niter - 1; ++j)
             {
                 vtkIdType segment[2];
                 segment[0] = npts_times_j + pt0;
                 segment[1] = npts_times_j + pt1;
                 ugrid->InsertNextCell(VTK_LINE, 2, segment);
                 npts_times_j += npts;
             }
         }
         else
         {
             int pt0 = list->GetId(0);
             int pt1 = list->GetId(1);
             int pt2 = list->GetId(2);
             int pt3 = list->GetId(3);
             if (c == VTK_PIXEL)
             {
                 pt2 = list->GetId(3);
                 pt3 = list->GetId(2);
             }
             int npts_times_j = 0;
             int npts_times_j1 = npts;
             for (j = 0 ; j < niter-1 ; j++)
             {
                 vtkIdType hex[6];
                 hex[0] = npts_times_j + pt0;
                 hex[1] = npts_times_j + pt1;
                 hex[2] = npts_times_j + pt2;
                 hex[3] = npts_times_j + pt3;
                 hex[4] = npts_times_j1 + pt0;
                 hex[5] = npts_times_j1 + pt1;
                 hex[6] = npts_times_j1 + pt2;
                 hex[7] = npts_times_j1 + pt3;
                 if (j == niter-2 && overlap_ends)
                 {
                     hex[4] = pt0;
                     hex[5] = pt1;
                     hex[6] = pt2;
                     hex[7] = pt3;
                 }
                 ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, hex);
                 npts_times_j += npts;
                 npts_times_j1 += npts;
             }
         }
    }

    vtkCellData *incd   = in_ds->GetCellData();
    vtkCellData *outcd  = ugrid->GetCellData();
    outcd->CopyAllocate(incd, n_out_cells);
    for (i = 0 ; i < n_out_cells ; i++)
    {
        outcd->CopyData(incd, i/(niter-1), i);
    }
    
    vtkPointData *inpd  = in_ds->GetPointData();
    vtkPointData *outpd = ugrid->GetPointData();
    outpd->CopyAllocate(inpd, n_out_pts);
    for (i = 0 ; i < n_out_pts ; i++)
    {
        outpd->CopyData(inpd, i%npts, i);
    }
    
    //
    // Clean up.
    //
    mat->Delete();
    pts->Delete();

    return ugrid;
}

// ****************************************************************************
// Method: PP_ZFileReader::RevolveDataSet
//
// Purpose: 
//   Revolves a VTK dataset.
//
// Arguments:
//
// Returns:    A pointer to the revolved dataset.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 26 11:16:15 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet *
PP_ZFileReader::RevolveDataSet(vtkDataSet *in_ds, const double *axis,
    double start_angle, double stop_angle, int nsteps, bool extrude)
{
    vtkDataSet *retval = 0;

    if(extrude)
    {
        vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
        retval = RevolveDataSetHelper(ugrid, in_ds, axis, start_angle,
                                      stop_angle, nsteps);
    }
    else
    {
        vtkPolyData *pd = vtkPolyData::New();
        retval = RevolveDataSetHelper(pd, in_ds, axis, start_angle,
                                      stop_angle, nsteps);
    }

    return retval;
}



