// ************************************************************************* //
//                             avtFileFormat.C                               //
// ************************************************************************* //

#include <limits.h> // for INT_MAX
#include <float.h> // for DBL_MAX
#include <snprintf.h>

#include <avtFileFormat.h>

#include <avtDatabaseMetaData.h>
#include <avtFileDescriptorManager.h>

#include <DebugStream.h>
#include <ImproperUseException.h>


using std::string;
using std::vector;


//
// Function Prototypes.
//

void   FileFormatCloseFileCallback(void *, int);

//
// Class statics/constants
//
const int    avtFileFormat::INVALID_CYCLE = -INT_MAX;
const double avtFileFormat::INVALID_TIME  = -DBL_MAX;

const int    avtFileFormat::FORMAT_INVALID_CYCLE = INVALID_CYCLE + 1;
const double avtFileFormat::FORMAT_INVALID_TIME  = INVALID_TIME + 1;


// ****************************************************************************
//  Method: avtFileFormat constructor
//
//  Programmer: Hank Childs
//  Creation:   October 8, 2001
//
// ****************************************************************************

avtFileFormat::avtFileFormat()
{
    cache = NULL;
    materialName = NULL;
    doMaterialSelection = false;
    canDoDynamicLoadBalancing = true;
    metadata = NULL;
    closingFile = false;
}


// ****************************************************************************
//  Method: avtFileFormat destructor
//
//  Programmer: Hank Childs
//  Creation:   October 8, 2001
//
// ****************************************************************************

avtFileFormat::~avtFileFormat()
{
    if (materialName != NULL)
    {
        delete [] materialName;
        materialName = NULL;
    }
}


// ****************************************************************************
//  Method: avtFileFormat::RegisterDatabaseMetaData
//
//  Purpose:
//      Registers the database meta-data object with the file format for future
//      reference.
//
//  Arguments:
//      md       The meta-data object.
//
//  Programmer:  Hank Childs
//  Creation:    March 12, 2002
//
// ****************************************************************************

void
avtFileFormat::RegisterDatabaseMetaData(avtDatabaseMetaData *md)
{
    metadata = md;
}


// ****************************************************************************
//  Method: avtFileFormat::TurnMaterialSelectionOff
//
//  Purpose:
//      Instructs the file format not to do any material selection.
//
//  Programmer: Hank Childs
//  Creation:   October 8, 2001
//
// ****************************************************************************

void
avtFileFormat::TurnMaterialSelectionOff(void)
{
    doMaterialSelection = false;
    if (materialName != NULL)
    {
        delete [] materialName;
        materialName = NULL;
    }
}


// ****************************************************************************
//  Method: avtFileFormatInterface::TurnMaterialSelectionOn
//
//  Purpose:
//      Instructs the file format to perform material selection for the
//      following single material.
//
//  Arguments:
//      m       A material name.
//
//  Programmer: Hank Childs
//  Creation:   October 8, 2001
//
// ****************************************************************************

void
avtFileFormat::TurnMaterialSelectionOn(const char *m)
{
    doMaterialSelection = true;
    if (materialName != NULL)
    {
        delete [] materialName;
    }
    materialName = new char[strlen(m)+1];
    strcpy(materialName, m);
}


// ****************************************************************************
//  Method: avtFileFormat::FreeUpResources
//
//  Purpose:
//      Defines an implementation of FreeUpResources that does nothing -- this
//      is for file formats where freeing resources does not make sense.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Apr  1 08:48:50 PST 2005
//    Use debug5 instead of debug1, since this isn't really a problem.
//
// ****************************************************************************

void
avtFileFormat::FreeUpResources(void)
{
    debug5 << "Asked " << GetType() << " to free up resources, but it did not "
           << "define how to do that." << endl;
}

// ****************************************************************************
//  Method: avtFileFormat::ActivateTimestep
//
//  Purpose:
//      Defines an implementation of ActivateTimestep that does nothing -- this
//      is for file formats that don't need to do anything special when a new
//      timestep is encountered. 
//
//  Programmer: Mark C. Miller 
//  Creation:   February 23, 2004
//
//  Modifications:
//
//    Hank Childs, Fri Apr  1 08:48:50 PST 2005
//    Use debug5 instead of debug1, since this isn't really a problem.
//
// ****************************************************************************

void
avtFileFormat::ActivateTimestep(void)
{
    debug5 << "Asked " << GetType() << " to activate timestep, but it did not "
           << "define how to do that." << endl;
}

// ****************************************************************************
//  Method: avtFileFormat::PopulateIOInformation
//
//  Purpose:
//      This is defined so that individual file formats will not have to
//      provide information abotu I/O if it does not make sense for that 
//      format.
//
//  Programmer: Hank Childs
//  Creation:   May 11, 2001
//
// ****************************************************************************

void
avtFileFormat::PopulateIOInformation(avtIOInformation &)
{
    debug5 << "No I/O info provided for file format " << GetType() << endl;
}


// ****************************************************************************
//  Method: avtFileFormat::SetCache
//
//  Purpose:
//      Sets the variable cache file formats can store things in.
//
//  Arguments:
//      c       The variable cache to use.
//
//  Programmer: Hank Childs
//  Creation:   September 20, 2001
//
// ****************************************************************************

void
avtFileFormat::SetCache(avtVariableCache *c)
{
    cache = c;
}


// ****************************************************************************
//  Method: avtFileFormat::AddMeshToMetaData
//
//  Purpose:
//      A convenience routine to add a mesh to a meta-data object.
//
//  Arguments:
//      md        The meta-data object to add the mesh to.
//      name      The name of the mesh.
//      type      The type of mesh - rectilinear, curvilinear, unstructured.
//      extents   The extents of the mesh. (optional)
//      blocks    The number of blocks. (optional, = 1)
//      origin    The origin of the block numbers. (optional, = 0)
//      spatial   The spatial dimension of the mesh. (optional, = 3)
//      topo      The topological dimension of the mesh. (optional, = 3)
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
//  Modifications:
//
//    Hank Childs, Tue May 28 16:53:09 PDT 2002
//    Initialized new data member for assigning titles to blocks.
//
//    Hank Childs, Sun Jun 16 19:55:52 PDT 200
//    Initialized cell origin.
//
//    Mark C. Miller, Mon Jul 18 13:41:13 PDT 2005
//    Added code to assure topo is zero if its a point mesh. VisIt has
//    subtle problems in the pipeline if it is not.
//
// ****************************************************************************

void
avtFileFormat::AddMeshToMetaData(avtDatabaseMetaData *md, string name,
                                 avtMeshType type, const double *extents,
                                 int blocks, int origin, int spatial, int topo)
{
    if (type == AVT_POINT_MESH)
        topo = 0;
    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = name;
    mesh->meshType = type;
    mesh->numBlocks = blocks;
    mesh->blockOrigin = origin;
    mesh->cellOrigin = 0;
    mesh->spatialDimension = spatial;
    mesh->topologicalDimension = topo;
    mesh->blockTitle = "blocks";
    mesh->blockPieceName = "block";
    if (extents != NULL)
    {
        mesh->SetExtents(extents);
        mesh->hasSpatialExtents = true;
    }
    else
    {
        mesh->hasSpatialExtents = false;
    }

    md->Add(mesh);
}


// ****************************************************************************
//  Method: avtFileFormat::AddScalarVarToMetaData
//
//  Purpose:
//      A convenience routine to add a scalar variable to the meta-data.
//
//  Arguments:
//      md        The meta-data object to add the scalar var to.
//      name      The name of the scalar variable.
//      mesh      The mesh the scalar var is defined on.
//      cent      The centering type - node vs cell.
//      extents   The extents of the scalar var. (optional)
//      treatAsASCII   Whether the var is 'ascii' (optional)
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
//  Modifications:
//    Kathleen Bonnell, Wed Jul 13 18:28:51 PDT 2005
//    Add optional bool 'treatAsASCII' arg. 
//
// ****************************************************************************

void
avtFileFormat::AddScalarVarToMetaData(avtDatabaseMetaData *md, string name,
                                      string mesh, avtCentering cent,
                                      const double *extents, 
                                      const bool treatAsASCII)
{
    avtScalarMetaData *scalar = new avtScalarMetaData();
    scalar->name = name;
    scalar->meshName = mesh;
    scalar->centering = cent;
    if (extents != NULL)
    {
        scalar->hasDataExtents = true;
        scalar->SetExtents(extents);
    }
    else
    {
        scalar->hasDataExtents = false;
    }
    scalar->treatAsASCII = treatAsASCII;

    md->Add(scalar);
}


// ****************************************************************************
//  Method: avtFileFormat::AddVectorVarToMetaData
//
//  Purpose:
//      A convenience routine to add a vector variable to the meta-data.
//
//  Arguments:
//      md        The meta-data object to add the vector var to.
//      name      The name of the vector variable.
//      mesh      The mesh the vector var is defined on.
//      cent      The centering type - node vs cell.
//      dim       The dimension of the vector variable. (optional = 3)
//      extents   The extents of the vector var. (optional)
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
// ****************************************************************************

void
avtFileFormat::AddVectorVarToMetaData(avtDatabaseMetaData *md, string name,
                                      string mesh, avtCentering cent,
                                      int dim, const double *extents)
{
    avtVectorMetaData *vector = new avtVectorMetaData();
    vector->name = name;
    vector->meshName = mesh;
    vector->centering = cent;
    vector->varDim = dim;
    if (extents != NULL)
    {
        vector->hasDataExtents = true;
        vector->SetExtents(extents);
    }
    else
    {
        vector->hasDataExtents = false;
    }

    md->Add(vector);
}


// ****************************************************************************
//  Method: avtFileFormat::AddTensorVarToMetaData
//
//  Purpose:
//      A convenience routine to add a tensor variable to the meta-data.
//
//  Arguments:
//      md        The meta-data object to add the tensor var to.
//      name      The name of the tensor variable.
//      mesh      The mesh the tensor var is defined on.
//      cent      The centering type - node vs cell.
//      dim       The dimension of the tensor variable. (optional = 3)
//
//  Programmer: Hank Childs
//  Creation:   September 20, 2003
//
// ****************************************************************************

void
avtFileFormat::AddTensorVarToMetaData(avtDatabaseMetaData *md, string name,
                                      string mesh, avtCentering cent, int dim)
{
    avtTensorMetaData *tensor = new avtTensorMetaData();
    tensor->name = name;
    tensor->meshName = mesh;
    tensor->centering = cent;
    tensor->dim = dim;

    md->Add(tensor);
}


// ****************************************************************************
//  Method: avtFileFormat::AddSymmetricTensorVarToMetaData
//
//  Purpose:
//      A convenience routine to add a symmetric tensor variable to the
//      meta-data.
//
//  Arguments:
//      md        The meta-data object to add the tensor var to.
//      name      The name of the tensor variable.
//      mesh      The mesh the tensor var is defined on.
//      cent      The centering type - node vs cell.
//      dim       The dimension of the tensor variable. (optional = 3)
//
//  Programmer: Hank Childs
//  Creation:   September 20, 2003
//
// ****************************************************************************

void
avtFileFormat::AddSymmetricTensorVarToMetaData(avtDatabaseMetaData *md, 
                          string name, string mesh, avtCentering cent, int dim)
{
    avtSymmetricTensorMetaData *st = new avtSymmetricTensorMetaData();
    st->name = name;
    st->meshName = mesh;
    st->centering = cent;
    st->dim = dim;

    md->Add(st);
}


// ****************************************************************************
//  Method: avtFileFormat::AddArrayVarToMetaData
//
//  Purpose:
//      A convenience routine to add a array variable to the meta-data.
//
//  Arguments:
//      md        The meta-data object to add the tensor var to.
//      name      The name of the array variable.
//      cnames    The name of the components.
//      mesh      The mesh the array var is defined on.
//      cent      The centering type - node vs cell.
//
//  Programmer: Hank Childs
//  Creation:   July 21, 2005
//
// ****************************************************************************

void
avtFileFormat::AddArrayVarToMetaData(avtDatabaseMetaData *md, string name, 
                        vector<string> &cnames, string mesh, avtCentering cent)
{
    avtArrayMetaData *st = new avtArrayMetaData();
    st->name = name;
    st->compNames = cnames;
    st->nVars = cnames.size();
    st->meshName = mesh;
    st->centering = cent;

    md->Add(st);
}


// ****************************************************************************
//  Method: avtFileFormat::AddArrayVarToMetaData
//
//  Purpose:
//      A convenience routine to add a array variable to the meta-data.
//
//  Arguments:
//      md        The meta-data object to add the tensor var to.
//      name      The name of the array variable.
//      ncomps    The number of components.
//      mesh      The mesh the array var is defined on.
//      cent      The centering type - node vs cell.
//
//  Programmer: Hank Childs
//  Creation:   July 21, 2005
//
// ****************************************************************************

void
avtFileFormat::AddArrayVarToMetaData(avtDatabaseMetaData *md, string name, 
                                     int ncomps, string mesh,avtCentering cent)
{
    avtArrayMetaData *st = new avtArrayMetaData();
    st->name = name;
    st->nVars = ncomps;
    st->compNames.resize(ncomps);
    for (int i = 0 ; i < ncomps ; i++)
    {
        char name[16];
        SNPRINTF(name, 16, "comp%02d", i);
        st->compNames[i] = name;
    }
    st->meshName = mesh;
    st->centering = cent;

    md->Add(st);
}


// ****************************************************************************
//  Method: avtFileFormat::AddMaterialToMetaData
//
//  Purpose:
//      A convenience routine to add a material to the meta-data.
//
//  Arguments:
//      md         The meta-data object to add the material to.
//      name       The name of the material.
//      mesh       The mesh the material is defined on.
//      nmats      The number of materials.
//      matnames   The names of each material.
//
//  Programmer:    Hank Childs
//  Creation:      February 23, 2001
//
// ****************************************************************************

void
avtFileFormat::AddMaterialToMetaData(avtDatabaseMetaData *md, string name,
                                     string mesh, int nmats,
                                     vector<string> matnames)
{
    avtMaterialMetaData *mat = new avtMaterialMetaData();
    mat->name = name;
    mat->meshName = mesh;
    mat->numMaterials = nmats;
    mat->materialNames = matnames;

    md->Add(mat);
}


// ****************************************************************************
//  Method: avtFileFormat::AddSpeciesToMetaData
//
//  Purpose:
//      A convenience routine to add a species to the meta-data.
//
//  Arguments:
//      md         The meta-data object to add the species to.
//      name       The name of the species.
//      mesh       The mesh the species is defined on.
//      mat        The material the species is define on.
//      nmat       The number of materials in mat.
//      nspecs     The number of species.
//      specnames  The names of each species.
//
//  Programmer:    Hank Childs
//  Creation:      February 23, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Dec 13 14:48:42 PST 2001
//    Rewrote to take advantage of the constructor's functionality.
//
// ****************************************************************************

void
avtFileFormat::AddSpeciesToMetaData(avtDatabaseMetaData *md, string name,
                                    string mesh, string mat, int nmat,
                                    vector<int> nspecs,
                                    vector<vector<string> > specnames)
{
    md->Add(new avtSpeciesMetaData(name, mesh, mat, nmat, nspecs, specnames));
}


// ****************************************************************************
//  Method: avtFileFormat::GuessCycleOrTime
//
//  Purpose:
//      Takes in a file name and tries to guess the cycle.
//
//  Arguments:
//      fname    The filename.
//
//  Programmer:  Hank Childs
//  Creation:    March 7, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Mar 18 11:06:01 PST 2002
//    Use the last set of numbers, not the first.
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Renamed to include fact that we can get cycle or time
//    Added permitDot arg to indicate whether a dot in the filename
//    should be permitted. Made it return double instead of int.
//    Made it return -DBL_MAX to indicate inability to obtain cycle/time
// ****************************************************************************

double
avtFileFormat::GuessCycleOrTime(const char *fname, bool permitDot) const
{
    //
    // Take out any of the name that comes from the directory structure.
    //
    const char *p = fname;
    const char *q = fname;
    while (p != NULL)
    {
        q = p;
        p = strstr(p+1, "/");
    }
    if (q != fname)
    {
        fname = q+1;
    }

    int len = strlen(fname);
    int lastDigit = -1;
    bool inRun = false;
    for (int i = 0 ; i < len ; i++)
    {
        if (isdigit(fname[i]) ||
            (permitDot && (fname[i] == '.')))
        {
            if (!inRun)
            {
                lastDigit = i;
            }
            inRun = true;
        }
        else
        {
            inRun = false;
        }
    }

    //
    // Example of atoi: atoi("1234str") = 1234.
    //
    if (lastDigit >= 0 && (!permitDot || fname[lastDigit] != '.' || isdigit(fname[lastDigit+1])))
    {
        return atof(fname+lastDigit);
    }

    //
    // No number in the string, so the cycle must be 0.
    //
    return -DBL_MAX;
}


// ****************************************************************************
//  Method: avtFileFormat::CloseFileDescriptor
//
//  Purpose:
//      Closes a file descriptor.  This is so that one module can manage all
//      of the open files at any given time.
//
//  Programmer: Hank Childs
//  Creation:   March 22, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Oct 10 16:00:31 PDT 2003
//    Added a more descriptive debug statement for the Exception.
//
// ****************************************************************************

void
avtFileFormat::CloseFileDescriptor(int manIndex)
{
    int index = -1;
    int nFiles = fileIndicesForDescriptorManager.size();
    for (int i = 0 ; i < nFiles ; i++)
    {
        if (fileIndicesForDescriptorManager[i] == manIndex)
        {
            index = i;
            break;
        }
    }

    if (index == -1)
    {
        debug1 << "avtFileFormat::CloseFileDescriptor cannot match up index " 
               << manIndex 
               << ".  This may be due to stale pointers from formats that "
               << "have not Unregistered their deleted files." << endl;
        EXCEPTION0(ImproperUseException);
    }

    closingFile = true;
    CloseFile(index);
    closingFile = false;
}


// ****************************************************************************
//  Method: avtFileFormat::RegisterFile
//
//  Purpose:
//      Registers a file with the file descriptor manager.
//
//  Arguments:
//      fileIndex    The index that the derived type thinks of.
//
//  Programmer: Hank Childs
//  Creation:   March 22, 2002
//
// ****************************************************************************

void
avtFileFormat::RegisterFile(int fileIndex)
{
    avtFileDescriptorManager *man = avtFileDescriptorManager::Instance();
    int manIndex = man->RegisterFile(FileFormatCloseFileCallback, this);
 
    while (fileIndex >= fileIndicesForDescriptorManager.size())
    {
        fileIndicesForDescriptorManager.push_back(-1);
    }
    fileIndicesForDescriptorManager[fileIndex] = manIndex;
}


// ****************************************************************************
//  Method: avtFileFormat::UnregisterFile
//
//  Purpose:
//      Tells the file descriptor manager that we have decided to close a file
//      descriptor on our own accord and that it should not come later and tell
//      us that we need to close it.
//
//  Arguments:
//      index   The index as far as the derived types think of it.
//
//  Programmer: Hank Childs
//  Creation:   March 22, 2002
//
// ****************************************************************************

void
avtFileFormat::UnregisterFile(int index)
{
    if (closingFile)
    {
        //
        // We told the derived type to close a file, now it is telling us that
        // it is doing it.  Just ignore this message.
        //
        return;
    }

    avtFileDescriptorManager *man = avtFileDescriptorManager::Instance();
    int manIndex = fileIndicesForDescriptorManager[index];
    if (manIndex == -1)
    {
        EXCEPTION0(ImproperUseException);
    }
    
    man->UnregisterFile(manIndex);
    fileIndicesForDescriptorManager[index] = -1;
}


// ****************************************************************************
//  Method: avtFileFormat::UsedFile
//
//  Purpose:
//      Indicates to the file descriptor manager that we have used a file
//      recently.
//
//  Arguments:
//      index   The index as far as the derived types think of it.
//
//  Programmer: Hank Childs
//  Creation:   March 22, 2002
//
// ****************************************************************************

void
avtFileFormat::UsedFile(int index)
{
    avtFileDescriptorManager *man = avtFileDescriptorManager::Instance();
    int manIndex = fileIndicesForDescriptorManager[index];
    if (manIndex == -1)
    {
        EXCEPTION0(ImproperUseException);
    }
    
    
    man->UsedFile(manIndex);
}


// ****************************************************************************
//  Method: avtFileFormat::CloseFile
//
//  Purpose:
//      Forces the file format to close a currently open file.
//
//  Arguments:
//      index   The index as far as the derived types think of it.
//
//  Programmer: Hank Childs
//  Creation:   March 22, 2002
//
// ****************************************************************************

void
avtFileFormat::CloseFile(int index)
{
    //
    // A derived type bothered to register a file with the file descriptor
    // manager, but it did define this virtual method.  That's a problem.
    // Throw an exception.
    //
    EXCEPTION0(ImproperUseException);
}


// ****************************************************************************
//  Function: FileFormatCloseFileCallback
//
//  Purpose:
//      A callback that will force avtFileFormats to close their file 
//      descriptors.
//
//  Arguments:
//      ptr     This a void * pointer to an avtFileFormat.
//      idx     An index of the file to close.  This index is relative to how
//              the file descriptor manager thinks of it.
//
//  Programmer: Hank Childs
//  Creation:   March 22, 2002
//
// ****************************************************************************

void
FileFormatCloseFileCallback(void *ptr, int idx)
{
    avtFileFormat *ff = (avtFileFormat *) ptr;
    ff->CloseFileDescriptor(idx);
}


