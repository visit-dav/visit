// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtFMSFileFormat.C                             //
// ************************************************************************* //

#include <avtFMSFileFormat.h>

#include <cstdlib>
#include <cstring>
#include <string>
#include <sstream>

#include <avtDatabaseMetaData.h>
#include <avtParallel.h>

#include <DebugStream.h>
#include <Expression.h>
#include <StringHelpers.h>
#include <FileFunctions.h>

#include <InvalidFilesException.h>
#include <InvalidVariableException.h>

#include <avtResolutionSelection.h>

#include <vtkFloatArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnsignedIntArray.h>
#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkLine.h>
#include <vtkTriangle.h>
#include <vtkHexahedron.h>
#include <vtkQuad.h>
#include <vtkTetra.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>

#include <fmsio.h>

#include <vector>
#include <map>
#include <string>

using     std::string;
using     std::ostringstream;
using     std::vector;

// Controls whether to create original cell numbers.
#define CREATE_ORIGINAL_CELL_NUMBERS

using namespace mfem;

/**
Let's make some simplifying assumptions.It looks like MFEM does not have 
multiple domains in its Mesh class. The DataCollection contains a single 
Mesh. Related domains in the FmsDataCollection all will be exposed as a 
single mesh.
*/

// ****************************************************************************
// Class: avtFMSFileFormat::Internals
//
// Purpose:
//   This class contains some reader internals that help it manage FMS/MFEM
//   conversion.
//
// Notes:    
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 10 14:03:35 PDT 2020
//
// Modifications:
//
// ****************************************************************************

class avtFMSFileFormat::Internals
{
public:
    Internals();
    ~Internals();

    bool LoadRoot(const std::string &filename);
    bool LoadFMS(const std::string &filename);

    size_t size() const { return filenames.size(); }

    int TotalDomains() const;

    mfem::DataCollection *GetDataCollection(int domain);   
    mfem::Mesh *GetDomain(const char *domain_name, int domain);
    mfem::GridFunction *GetField(const char *domain_name, int domain, const std::string &fieldName);

    const std::string &GetCoordinateFieldName() const;
    bool GetFieldUnits(const std::string &name, std::string &units) const;
    const std::string &GetDatabaseComment() const;

    void ComputeTotalDomains();

protected:
    bool ReadDomain(int domain);
    std::string FilenameForDomain(int domain) const;
    void ExamineMetaData(FmsDataCollection dc);

    std::string                                   protocol;
    std::vector<std::string>                      filenames;
    std::map<std::string, mfem::DataCollection *> mfemCache;
    std::map<std::string,std::string>             fieldUnits;
    std::string                                   coordFieldName;
    std::string                                   databaseComment;
};

// ****************************************************************************
// Method: avtFMSFileFormat::Internals::Internals
//
// Purpose:
//   Constructor
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 10 14:04:05 PDT 2020
//
// Modifications:
//
// ****************************************************************************

avtFMSFileFormat::Internals::Internals() : protocol(), filenames(), 
    mfemCache(), fieldUnits(), coordFieldName(), databaseComment()
{
}

// ****************************************************************************
// Method: avtFMSFileFormat::Internals::~Internals
//
// Purpose:
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 10 14:04:05 PDT 2020
//
// Modifications:
//
// ****************************************************************************

avtFMSFileFormat::Internals::~Internals()
{
    // Delete MFEM objects.
    for(std::map<std::string, mfem::DataCollection*>::iterator it = mfemCache.begin();
        it != mfemCache.end(); it++)
    {
        delete it->second;
    }
}

// ****************************************************************************
// Method: avtFMSFileFormat::Internals::LoadRoot
//
// Purpose:
//   Load a root file with a given name.
//
// Arguments:
//   filename : The name of the root file.
//
// Returns:    True if we read the file; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 10 14:04:39 PDT 2020
//
// Modifications:
//
// ****************************************************************************

bool
avtFMSFileFormat::Internals::LoadRoot(const std::string &filename)
{
    // Open the file.
    ifstream ifile(filename);
    if (ifile.fail())
    {
        return false;
    }

    std::string dname(FileFunctions::Dirname(filename));
    dname += VISIT_SLASH_STRING;

    char line[1024];
    while(!ifile.eof())
    {
        ifile.getline(line, 1024);

        char *comment = strstr(line, "#");
        if(comment != nullptr)
            comment[0] = '\0';

        std::string fn(line);
        StringHelpers::trim(fn);
        if(fn.find(".fms") != std::string::npos)
        {
            if(fn[0] != '/')
                fn = dname + fn;
            filenames.push_back(fn);
        }
    }

    return !filenames.empty();
}

// ****************************************************************************
// Method: avtFMSFileFormat::Internals::LoadFMS
//
// Purpose:
//   Load a single FMS file with the given name.
//
// Arguments:
//   filename : The name of the FMS file.
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 10 14:04:39 PDT 2020
//
// Modifications:
//
// ****************************************************************************

bool
avtFMSFileFormat::Internals::LoadFMS(const std::string &filename)
{
    if(!filename.empty())
        filenames.push_back(filename);
    return !filenames.empty();
}

// ****************************************************************************
// Method: avtFMSFileFormat::Internals::TotalDomains
//
// Purpose:
//   Returns the number of domains in the dataset.
//
// Returns:    The number of domains (assume 1:1 with files after MFEM conversion)
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 10 14:06:28 PDT 2020
//
// Modifications:
//
// ****************************************************************************

int
avtFMSFileFormat::Internals::TotalDomains() const
{
    return filenames.size();
}

// ****************************************************************************
// Method: avtFMSFileFormat::Internals::FilenameForDomain
//
// Purpose:
//   Returns the filename for a given domain.
//
// Arguments:
//   domain : The domain number.
//
// Returns:    The filename for the given domain.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 10 14:07:10 PDT 2020
//
// Modifications:
//
// ****************************************************************************

std::string
avtFMSFileFormat::Internals::FilenameForDomain(int domain) const
{
    return filenames[domain];
}

// ****************************************************************************
// Method: FmsMetaDataGetString
//
// Purpose:
//   Gets a string from FMS metadata.
//
// Arguments:
//   mdata : The FMS metadata object.
//   key   : The key for the value we want to obtain.
//   value : The return string value.
//
// Returns:    True if we fetched the value for the key.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 10 14:08:38 PDT 2020
//
// Modifications:
//
// ****************************************************************************

static bool
FmsMetaDataGetString(FmsMetaData mdata, const std::string &key, std::string &value)
{   
    if (!mdata) return false;

    bool retval = false;
    FmsMetaDataType type;
    FmsInt i, size;
    FmsMetaData *children = nullptr;
    const char *mdata_name = nullptr;
    const char *str_value = nullptr;

    if(FmsMetaDataGetType(mdata, &type) == 0)
    {
        switch(type)
        {
        case FMS_STRING:
            if(FmsMetaDataGetString(mdata, &mdata_name, &str_value) == 0)
            {
                if(strcasecmp(key.c_str(), mdata_name) == 0)
                {
                    retval = true;
                    value = str_value;
                }
            }
            break;
        case FMS_META_DATA:
            if(FmsMetaDataGetMetaData(mdata, &mdata_name, &size, &children) == 0)
            {
                // Recurse to look for the key we want.
                for(i = 0; i < size && !retval; i++)
                    retval = FmsMetaDataGetString(children[i], key, value);
            }
            break;
        }
    }

    return retval;
}

// ****************************************************************************
// Method: avtFMSFileFormat::Internals::ExamineMetaData
//
// Purpose:
//   Look through the FMS data collection metadata for metadata that would
//   not have been preserved via the MFEM data collection. 
//
// Arguments:
//   dc : The FMS data collection
//
// Returns:    
//
// Note:       We look at the FMS data since MFEM does not appear to provide 
//             units.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug  3 17:53:10 PDT 2020
//
// Modifications:
//
// ****************************************************************************

void
avtFMSFileFormat::Internals::ExamineMetaData(FmsDataCollection dc)
{
    // Populate the field units.
    FmsField *fields = nullptr;
    FmsInt num_fields = 0;
    if(FmsDataCollectionGetFields(dc, &fields, &num_fields) == 0)
    {
        for(FmsInt i = 0; i < num_fields; ++i)
        {
            const char *name = nullptr;
            FmsFieldGetName(fields[i], &name);

            FmsMetaData mdata;
            if(FmsFieldGetMetaData(fields[i], &mdata) == 0)
            {
                std::string units;
                if(FmsMetaDataGetString(mdata, "units", units))
                {
                    fieldUnits[name] = units;
                }
            }
        }
    }

    // Look through the components and figure out the name of the coordinate field.
    FmsMesh fms_mesh;
    if(FmsDataCollectionGetMesh(dc, &fms_mesh) == 0)
    {
        FmsInt num_comp;
        FmsMeshGetNumComponents(fms_mesh, &num_comp);
        for (FmsInt comp_id = 0; comp_id < num_comp; comp_id++)
        {
            FmsField coords = NULL;
            FmsComponent comp;
            FmsMeshGetComponent(fms_mesh, comp_id, &comp);
            FmsComponentGetCoordinates(comp, &coords);
            if(coords)
            {
                const char *name = nullptr;
                FmsFieldGetName(coords, &name);
                // save the name of the coordinate field.
                coordFieldName = name;
                break;
            }
        }
    }

    // See if there is a database comment.
    FmsMetaData mdata;
    if(FmsDataCollectionGetMetaData(dc, &mdata) == 0)
    {        
        if(!FmsMetaDataGetString(mdata, "Description", databaseComment))
            FmsMetaDataGetString(mdata, "Comment", databaseComment);
    }
}

// ****************************************************************************
// Method: avtFMSFileFormat::Internals::GetFieldUnits
//
// Purpose:
//   Returns the units for a field (if any).
//
// Arguments:
//   name : The name of the field.
//   units : The return units string for the field.
//
// Returns:    True if the field had units; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 10 14:09:56 PDT 2020
//
// Modifications:
//
// ****************************************************************************

bool
avtFMSFileFormat::Internals::GetFieldUnits(const std::string &name, std::string &units) const
{
    bool retval = false;
    const auto it = fieldUnits.find(name);
    if(it != fieldUnits.end())
    {
        units = it->second;
        retval = true;
    }
    return retval;
}

// ****************************************************************************
// Method: avtFMSFileFormat::Internals::GetCoordinateFieldName
//
// Purpose:
//   Returns the name of the coordinate field for the FMS dataset.
//
// Returns:    The name of the coordinate field.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 10 14:10:46 PDT 2020
//
// Modifications:
//
// ****************************************************************************

const std::string &
avtFMSFileFormat::Internals::GetCoordinateFieldName() const
{
    return coordFieldName;
}

// ****************************************************************************
// Method: avtFMSFileFormat::Internals::GetDatabaseComment
//
// Purpose:
//   Returns a database comment (if one exists) for the FMS dataset.
//
// Returns:    The database comment (might be empty)
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 10 14:11:17 PDT 2020
//
// Modifications:
//
// ****************************************************************************

const std::string &
avtFMSFileFormat::Internals::GetDatabaseComment() const
{
    return databaseComment;
}

// ****************************************************************************
// Method: avtFMSFileFormat::Internals::ReadDomain
//
// Purpose:
//   Read the FMS data for the domain, convert it to MFEM, and cache it locally.
//
// Arguments:
//   domain : The domain id that we want to read.
//
// Returns:    True if data were read.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 10 14:12:00 PDT 2020
//
// Modifications:
//
// ****************************************************************************

bool
avtFMSFileFormat::Internals::ReadDomain(int domain)
{
    const char *mName = "avtFMSFileFormat::Internals::ReadDomain";
    bool retval = false;
    std::string fn(FilenameForDomain(domain));
    if(!fn.empty())
    {
        if(mfemCache.find(fn) == mfemCache.end())
        {
            debug5 << mName << ": Data collection is not cached. Reading " << fn << endl;
            FmsDataCollection dc;
            if(FmsIORead(fn.c_str(), protocol.empty() ? nullptr : protocol.c_str(), &dc) == 0)
            {
                debug5 << mName << ": Data collection was read." << endl;

                TRY
                {
                    // Convert the FMS data collection to MFEM.
                    mfem::DataCollection *mdc = nullptr;
                    if(mfem::FmsDataCollectionToDataCollection(dc, &mdc) == 0)
                    {
                        debug5 << mName << ": Caching MFEM data collection for domain "
                               << domain << endl;
                        // Save the data collection.
                        mfemCache[fn] = mdc;

                        retval = true;
                    }
                    else
                    {
                        debug5 << mName << ": FAIL! MFEM data collection for domain "
                               << domain << " did not convert." << endl;
                    }
                }
                CATCH2(mfem::ErrorException, e)
                {
                    debug5 << mName << ": MFEM exception: \"" << e.what() << "\"" << endl;
                    retval = false;
                }
                CATCHALL
                {
                    debug5 << mName << ": An unspecified exception was caught." << endl;
                    retval = false;
                }
                ENDTRY

                // Look at the FMS metadata to see if there is additional 
                // metadata we can use.
                if(retval)
                    ExamineMetaData(dc);

                // For now, assume that the mfem data collection does not point 
                // to data from the FMS data collection.
                FmsDataCollectionDestroy(&dc);
            }
        }
        else
        {
            debug5 << mName << ": Data collection was already read." << endl;

            // The file has already been read.
            retval = true;
        }
    }
    return retval;
}

// ****************************************************************************
// Method: avtFMSFileFormat::Internals::GetDataCollection
//
// Purpose:
//   Get a pointer to the cached MFEM data collection, reading it from FMS
//   if needed.
//
// Arguments:
//   domain : The domain id.
//
// Returns:    A pointer to the MFEM data collection for the domain (or NULL).
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 10 14:13:36 PDT 2020
//
// Modifications:
//
// ****************************************************************************

mfem::DataCollection *
avtFMSFileFormat::Internals::GetDataCollection(int domain)
{
    mfem::DataCollection *retval = nullptr;

    if(ReadDomain(domain))
    {
        std::string fn(FilenameForDomain(domain));
        auto it = mfemCache.find(fn);
        if(it != mfemCache.end())
        {
            retval = it->second;
        }
    }

    return retval;
}

// ****************************************************************************
// Method: avtFMSFileFormat::Internals::GetDomain
//
// Purpose:
//   Get the MFEM mesh for the domain.
//
// Arguments:
//   domain_name : The mesh name (future).
//   domain : The domain id.
//
// Returns:    A pointer to the MFEM mesh for the domain.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 10 14:14:27 PDT 2020
//
// Modifications:
//
// ****************************************************************************

mfem::Mesh *
avtFMSFileFormat::Internals::GetDomain(const char *domain_name, int domain)
{
    mfem::Mesh *retval = nullptr;
    if(ReadDomain(domain))
    {
        std::string fn(FilenameForDomain(domain));
        auto it = mfemCache.find(fn);
        if(it != mfemCache.end())
        {
            // We already cached the MFEM data collection. Return its mesh.
            retval = it->second->GetMesh();
        }
    }

    return retval;
}

// ****************************************************************************
// Method: avtFMSFileFormat::Internals::GetField
//
// Purpose:
//   Get the specified field as an MFEM grid function.
//
// Arguments:
//   domain_name : The mesh name (future)
//   domain : The domain id.
//   fieldName : The name of the field to return.
//
// Returns:    An MFEM grid function or NULL.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 10 14:15:41 PDT 2020
//
// Modifications:
//
// ****************************************************************************

mfem::GridFunction *
avtFMSFileFormat::Internals::GetField(const char *domain_name, 
    int domain, const std::string &fieldName)
{
    mfem::GridFunction *retval = nullptr;
    if(ReadDomain(domain))
    {
        std::string fn(FilenameForDomain(domain));
        auto it = mfemCache.find(fn);
        if(it != mfemCache.end())
        {
            retval = it->second->GetField(fieldName);
        }
    }

    return retval;
}

const char *avtFMSFileFormat::MESH_NAME = "mesh";
const char *avtFMSFileFormat::BDR_MESH_NAME = "boundary";
const char *avtFMSFileFormat::DOF_MESH_NAME = "dofs";
const char *avtFMSFileFormat::ELEMENT_ATTRIBUTE_NAME = "element_attribute";
const char *avtFMSFileFormat::ELEMENT_COLORING_NAME = "element_coloring";

// ****************************************************************************
//  Method: avtFMSFileFormat constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Jul 27 17:30:23 PST 2020
//
// ****************************************************************************

avtFMSFileFormat::avtFMSFileFormat(const char *filename)
    : avtSTMDFileFormat(&filename, 1)
{
    selectedLOD = 0;

    // Make MFEM throw instead of aborting.
    mfem::set_error_action(MFEM_ERROR_THROW);

    d = new Internals;

    // Load the filenames into d.
    bool ok = true;
    size_t len = strlen(filename);
    const char *ext = strstr(filename, ".fms_root");
    if(ext != nullptr)
        ok = d->LoadRoot(filename);
    else
        ok = d->LoadFMS(filename);
    if(!ok)
    {
        EXCEPTION1(InvalidFilesException, filename);
    }
}

// ****************************************************************************
//  Method: avtFMSFileFormat destructor
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Jul 27 17:30:23 PST 2020
//
// ****************************************************************************

avtFMSFileFormat::~avtFMSFileFormat()
{
    delete d;
    FreeUpResources();
}


// ****************************************************************************
//  Method: avtFMSFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Jul 27 17:30:23 PST 2020
//
// ****************************************************************************

void
avtFMSFileFormat::FreeUpResources(void)
{
}

// ****************************************************************************
// Method: avtFMSFileFormat::ActivateTimestep
//
// Purpose:
//   Activates the time step.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 24 11:16:52 PDT 2020
//
// Modifications:
//
// ****************************************************************************

void
avtFMSFileFormat::ActivateTimestep(void)
{
    debug5 << "avtFMSFileFormat::ActivateTimestep" << endl;
}

// ****************************************************************************
//  Method: avtFMSFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Jul 27 17:30:23 PST 2020
//
//
//  Modifications:
//    Brad Whitlock, Wed Sep  2 17:49:59 PDT 2020
//    Up the maximum LODs to better handle the spiral cell case.
//
// ****************************************************************************
void
avtFMSFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    const char *mName = "avtFMSFileFormat::PopulateDatabaseMetaData";

    // We do not know what domains will have been read thusfar, if any.

    int nd = static_cast<int>(d->TotalDomains());
    int domain = PAR_Rank() % nd;
    const auto dc = d->GetDataCollection(domain);
    if(dc == nullptr)
    {
        debug5 << mName << ": " << GetFilename() << " could not be read." << endl;
        EXCEPTION1(InvalidFilesException, GetFilename());
    }

    md->SetDatabaseComment(d->GetDatabaseComment());

    // Get coordinate field units for use in the mesh metadata.
    std::string coordUnits;
    d->GetFieldUnits(d->GetCoordinateFieldName(), coordUnits);

    // Expose a mesh.
    auto mmd = new avtMeshMetaData;
    mmd->name = MESH_NAME;
    mmd->meshType = AVT_UNSTRUCTURED_MESH;
    mmd->spatialDimension = dc->GetMesh()->SpaceDimension();
    mmd->topologicalDimension = dc->GetMesh()->Dimension();
    mmd->cellOrigin = 0;
    mmd->nodeOrigin = 0;
    mmd->numBlocks = nd;
    mmd->blockTitle = "domains";
    mmd->blockPieceName = "domain";
    mmd->xUnits = coordUnits;
    mmd->yUnits = coordUnits;
    mmd->zUnits = coordUnits;
    mmd->xLabel = "X";
    mmd->yLabel = "Y";
    mmd->zLabel = "Z";
    mmd->LODs = 100; // The MultiresControl operator does not let us go above 1 without this.
#ifdef CREATE_ORIGINAL_CELL_NUMBERS
    mmd->containsOriginalCells = true;
#endif
    md->Add(mmd);

    // If there are boundary elements, add them as a 2nd mesh.
    if(dc->GetMesh()->GetNBE() > 0)
    {
        auto bmmd = new avtMeshMetaData;
        bmmd->name = BDR_MESH_NAME;
        bmmd->meshType = AVT_UNSTRUCTURED_MESH;
        bmmd->spatialDimension = dc->GetMesh()->SpaceDimension();
        bmmd->topologicalDimension = dc->GetMesh()->Dimension()-1;
        bmmd->cellOrigin = 0;
        bmmd->nodeOrigin = 0;
        bmmd->numBlocks = nd;
        bmmd->blockTitle = "domains";
        bmmd->blockPieceName = "domain";
        bmmd->xUnits = coordUnits;
        bmmd->yUnits = coordUnits;
        bmmd->zUnits = coordUnits;
        bmmd->xLabel = "X";
        bmmd->yLabel = "Y";
        bmmd->zLabel = "Z";
        bmmd->LODs = 50; // The MultiresControl operator does not let us go above 1 without this.
#ifdef CREATE_ORIGINAL_CELL_NUMBERS
        bmmd->containsOriginalCells = bmmd->topologicalDimension > 1;
#endif
        md->Add(bmmd);
    }

    // Add a mesh for the dofs.
    auto dmmd = new avtMeshMetaData;
    dmmd->name = DOF_MESH_NAME;
    dmmd->meshType = AVT_POINT_MESH;
    dmmd->spatialDimension = dc->GetMesh()->SpaceDimension();
    dmmd->topologicalDimension = 0;
    dmmd->cellOrigin = 0;
    dmmd->nodeOrigin = 0;
    dmmd->numBlocks = nd;
    dmmd->blockTitle = "domains";
    dmmd->blockPieceName = "domain";
    dmmd->xUnits = coordUnits;
    dmmd->yUnits = coordUnits;
    dmmd->zUnits = coordUnits;
    dmmd->xLabel = "X";
    dmmd->yLabel = "Y";
    dmmd->zLabel = "Z";
    dmmd->LODs = 0;
    md->Add(dmmd);

    // Add attribute and coloring to the main mesh.
    AddScalarVarToMetaData(md, ELEMENT_ATTRIBUTE_NAME, MESH_NAME, AVT_ZONECENT);
    AddScalarVarToMetaData(md, ELEMENT_COLORING_NAME, MESH_NAME, AVT_ZONECENT);

    // Use the cached MFEM data collection to add variables.
    auto m = dc->GetFieldMap();
    for(auto it = m.begin(); it != m.end(); it++)
    {
        avtCentering centering = AVT_NODECENT;
        if(it->second->VectorDim() == 1)
        {
            avtScalarMetaData *smd = new avtScalarMetaData;
            smd->name = it->first;
            smd->meshName = mmd->name;
            smd->centering = centering;
            d->GetFieldUnits(it->first, smd->units);
            smd->hasUnits = !smd->units.empty();
            md->Add(smd);
        }
        else
        {
            avtVectorMetaData *vmd = new avtVectorMetaData;
            vmd->name = it->first;
            vmd->meshName = mmd->name;
            vmd->centering = centering;
            d->GetFieldUnits(it->first, vmd->units);
            vmd->hasUnits = !vmd->units.empty();
            md->Add(vmd);
        }
    }
}

// ****************************************************************************
//  Method: avtFMSFileFormat::GetCycle
//
//  Purpose:
//      Returns if we have the current cycle.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Jul 30 16:03:09 PDT 2020
//
// ****************************************************************************
int
avtFMSFileFormat::GetCycle() 
{
    int nd = static_cast<int>(d->TotalDomains());
    int domain = PAR_Rank() % nd;
    auto dc = d->GetDataCollection(domain);
    if(dc != nullptr)
        return dc->GetCycle();

    return avtFileFormat::INVALID_CYCLE;
}

// ****************************************************************************
//  Method: avtFMSFileFormat::GetTime
//
//  Purpose:
//      Returns if we have the current cycle.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Jul 30 16:03:09 PDT 2020
//
// ****************************************************************************
double
avtFMSFileFormat::GetTime() 
{
    int nd = static_cast<int>(d->TotalDomains());
    int domain = PAR_Rank() % nd;
    auto dc = d->GetDataCollection(domain);
    if(dc != nullptr)
        return dc->GetTime();

    return avtFileFormat::INVALID_TIME;
}

// ****************************************************************************
// Callback functions for GetRefinedMesh.
// ****************************************************************************

static int
mesh_GetNE(mfem::Mesh *mesh)
{
    return mesh->GetNE();
}

static int
mesh_GetElementBaseGeometry(mfem::Mesh *mesh, int elem)
{
    return mesh->GetElementBaseGeometry(elem);
}

static mfem::ElementTransformation *
mesh_GetElementTransformation(mfem::Mesh *mesh, int elem)
{
    return mesh->GetElementTransformation(elem);
}

static int
mesh_GetNBE(mfem::Mesh *mesh)
{
    return mesh->GetNBE();
}

static int
mesh_GetBdrElementBaseGeometry(mfem::Mesh *mesh, int elem)
{
    return mesh->GetBdrElementBaseGeometry(elem);
}

static mfem::ElementTransformation *
mesh_GetBdrElementTransformation(mfem::Mesh *mesh, int elem)
{
    return mesh->GetBdrElementTransformation(elem);
}

// ****************************************************************************
//  Method: avtFMSFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      domain      The index of the domain.  If there are NDomains, this
//                  value is guaranteed to be between 0 and NDomains-1,
//                  regardless of block origin.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Jul 27 17:30:23 PST 2020
//
// ****************************************************************************
vtkDataSet *
avtFMSFileFormat::GetMesh(int domain, const char *meshname)
{
    vtkDataSet *ds = nullptr;
    if(strcmp(meshname, BDR_MESH_NAME) == 0)
    {
        ds = GetRefinedMesh(BDR_MESH_NAME,domain,selectedLOD+1,
                 mesh_GetNBE,
                 mesh_GetBdrElementBaseGeometry,
                 mesh_GetBdrElementTransformation);
    }
    else if(strcmp(meshname, DOF_MESH_NAME) == 0)
    {
        ds = GetDOFMesh(domain);
    }
    else
    {
        ds = GetRefinedMesh(string(meshname),domain,selectedLOD+1,
                 mesh_GetNE,
                 mesh_GetElementBaseGeometry,
                 mesh_GetElementTransformation);
    }
    return ds;
}

// ****************************************************************************
//  Method: avtFMSFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Jul 27 17:30:23 PST 2020
//
// ****************************************************************************
vtkDataArray *
avtFMSFileFormat::GetVar(int domain, const char *varname)
{
    vtkDataArray *arr = nullptr;
    if(strcmp(varname, ELEMENT_ATTRIBUTE_NAME) == 0)
        arr = GetRefinedElementAttribute(MESH_NAME, domain, selectedLOD+1);
    else if(strcmp(varname, ELEMENT_COLORING_NAME) == 0)
        arr = GetRefinedElementColoring(MESH_NAME, domain, selectedLOD+1);
    else
        arr = GetRefinedVar(string(varname),domain,selectedLOD+1);
    return arr;
}

// ****************************************************************************
//  Method: avtFMSFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Jul 27 17:30:23 PST 2020
//
// ****************************************************************************

vtkDataArray *
avtFMSFileFormat::GetVectorVar(int domain, const char *varname)
{
    return GetRefinedVar(string(varname),domain,selectedLOD+1);
}


// ****************************************************************************
//  Method: avtFMSFileFormat::FetchMesh
//
//  Purpose: Returns a new instance of the mfem mesh, given a domain id and 
//           mesh name
//
//  Arguments:
//    mesh_name: string with desired mesh name
//    domain:    domain id
//
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Jul 27 17:30:23 PST 2020
//
// Modifications:
//
// ****************************************************************************
mfem::Mesh *
avtFMSFileFormat::FetchMesh(const std::string &mesh_name, int domain)
{
    return d->GetDomain(mesh_name.c_str(), domain);
}

// ****************************************************************************
//  Method: avtFMSFileFormat::GetRefinedMesh
//
//  Purpose: 
//    Constructs a vtkUnstructuredGrid that contains a refined mfem mesh.
//
//  Arguments:
//    mesh_name: string with desired mesh name
//    domain:    domain id
//    lod:       number of refinement steps 
//
//  Programmer: Cyrus Harrison
//  Creation:   Sat Jul  5 11:38:31 PDT 2014
//
//  Modifications:
//
//    Alister Maguire, Thu Jan  2 15:23:13 MST 2020
//    Casting int to Geom::Type where appropriate. This is required after the
//    upgrade to mfem 4.0.
//
//    Brad Whitlock, Mon Jul 27 17:30:23 PST 2020
//    Adapted from MFEM reader. Added original cells code.
//
// ****************************************************************************

vtkDataSet *
avtFMSFileFormat::GetRefinedMesh(const std::string &mesh_name, int domain, int lod,
    int (*get_num_elements)(mfem::Mesh *),
    int (*get_element_base_geometry)(mfem::Mesh *, int),
    mfem::ElementTransformation *(*get_element_transformation)(mfem::Mesh *, int)
    )
{
    const char *mName = "avtFMSFileFormat::GetRefinedMesh";

    // get base mesh
    Mesh *mesh = FetchMesh(mesh_name,domain);

    if(mesh == nullptr)
    {
        debug5 << mName << ": We could not fetch the mesh." << endl;
        return nullptr;
    }

    // create output objects
    vtkUnstructuredGrid *res_ds  = vtkUnstructuredGrid::New(); 
    vtkPoints           *res_pts = vtkPoints::New();
   
    int npts=0;
    int neles=0;
    
    RefinedGeometry *refined_geo;
    DenseMatrix      pmat;
   
    //
    // find the # of output points and cells at the selected level of 
    // refinement (we may want to cache this...)
    //
    int NE = (*get_num_elements)(mesh);
    for (int i = 0; i < NE; i++)
    {
        int geom = (*get_element_base_geometry)(mesh, i);
        int ele_nverts = Geometries.GetVertices(geom)->GetNPoints();
        refined_geo = GlobGeometryRefiner.Refine((Geometry::Type)geom, lod, 1);
        npts  += refined_geo->RefPts.GetNPoints();
        neles += refined_geo->RefGeoms.Size() / ele_nverts;
    }

    // create the points for the refined topology   
    res_pts->Allocate(npts);
    res_pts->SetNumberOfPoints((vtkIdType) npts);
   
    // create the points for the refined topology
    int pt_idx=0;
    for (int i = 0; i < NE; i++)
    {
        int geom = (*get_element_base_geometry)(mesh, i);
        refined_geo = GlobGeometryRefiner.Refine((Geometry::Type)geom, lod, 1);
        // refined points
        (*get_element_transformation)(mesh,i)->Transform(refined_geo->RefPts, pmat);
        for (int j = 0; j < pmat.Width(); j++)
        {
            double pt_vals[3]={0.0,0.0,0.0};
            pt_vals[0] = pmat(0,j);
            if (pmat.Height() > 1)
                pt_vals[1] = pmat(1,j);
            if (pmat.Height() > 2)
                pt_vals[2] = pmat(2,j);
            res_pts->InsertPoint(pt_idx,pt_vals);
            pt_idx++;
        }
    }
    
    res_ds->SetPoints(res_pts);
    res_pts->Delete();  
    // create the cells for the refined topology   
    res_ds->Allocate(neles);

#ifdef CREATE_ORIGINAL_CELL_NUMBERS
    // Make some original cell ids so we can try to eliminate the mesh lines.
    std::vector<unsigned int> originalCells;
    originalCells.reserve(neles * 2);
    unsigned int udomain = static_cast<unsigned int>(domain);
    bool doOriginalCells = true;
#endif

    pt_idx=0;
    for (int i = 0; i < NE; i++)
    {
        int geom = (*get_element_base_geometry)(mesh, i);
        int ele_nverts = Geometries.GetVertices(geom)->GetNPoints();
        refined_geo    = GlobGeometryRefiner.Refine((Geometry::Type)geom, lod, 1);

        Array<int> &rg_idxs = refined_geo->RefGeoms;

        vtkCell *ele_cell = NULL;
        // rg_idxs contains all of the verts for the refined elements
        for (int j = 0; j < rg_idxs.Size(); )
        {
            switch (geom)
            {
                case Geometry::SEGMENT:
                    ele_cell = vtkLine::New();
                    doOriginalCells = false;
                    break;
                case Geometry::TRIANGLE:     ele_cell = vtkTriangle::New();   break;
                case Geometry::SQUARE:       ele_cell = vtkQuad::New();       break;
                case Geometry::TETRAHEDRON:  ele_cell = vtkTetra::New();      break;
                case Geometry::CUBE:         ele_cell = vtkHexahedron::New(); break;
            }
            // the are ele_nverts for each refined element
            for (int k = 0; k < ele_nverts; k++, j++)
                ele_cell->GetPointIds()->SetId(k,pt_idx + rg_idxs[j]);

            res_ds->InsertNextCell(ele_cell->GetCellType(),
                                   ele_cell->GetPointIds());
            ele_cell->Delete();

#ifdef CREATE_ORIGINAL_CELL_NUMBERS
            originalCells.push_back(udomain);
            originalCells.push_back(static_cast<unsigned int>(i));
#endif
        }

        pt_idx += refined_geo->RefPts.GetNPoints();
    }

#ifdef CREATE_ORIGINAL_CELL_NUMBERS
    if(doOriginalCells)
    {
        vtkUnsignedIntArray *ocn = vtkUnsignedIntArray::New();
        ocn->SetName("avtOriginalCellNumbers");
        ocn->SetNumberOfComponents(2);
        ocn->SetNumberOfTuples(originalCells.size()/2);
        memcpy(ocn->GetVoidPointer(0), &originalCells[0],
               sizeof(unsigned int) * originalCells.size());
        res_ds->GetCellData()->AddArray(ocn);
        ocn->Delete();
    }
#endif

    return res_ds;
}

// ****************************************************************************
//  Method: avtFMSFileFormat::GetRefinedVar
//
//  Purpose: 
//   Constructs a vtkDataArray that contains a refined mfem mesh field variable.
//
//  Arguments:
//   var_name:  string with desired variable name
//   domain:    domain id
//   lod:       number of refinement steps 
//
//  Programmer: Cyrus Harrison
//  Creation:   Sat Jul  5 11:38:31 PDT 2014
//
//  Modifications:
//
//    Mark C. Miller, Mon Dec 11 15:49:34 PST 2017
//    Add support for mfem_cat file
//
//    Alister Maguire, Thu Jan  2 15:23:13 MST 2020
//    Casting int to Geom::Type where appropriate. This is required after the
//    upgrade to mfem 4.0.
//
// ****************************************************************************
vtkDataArray *
avtFMSFileFormat::GetRefinedVar(const std::string &var_name,
                                 int domain,
                                 int lod)
{
    const char *mName = "avtFMSFileFormat::GetRefinedVar";

    // get base mesh
    mfem::Mesh *mesh = FetchMesh(MESH_NAME, domain);

    if(mesh == nullptr)
    {
        debug5 << mName << ": We could not fetch the mesh." << endl;
        return nullptr;
    }

    GridFunction *gf = d->GetField(MESH_NAME, domain, var_name);
    if(gf == nullptr)
    {
        EXCEPTION1(InvalidFilesException, std::string("Could not get ")+var_name);
    }

    int ncomps = gf->VectorDim();
    bool var_is_nodal = true; //(gf->FESpace()->FEColl()->GetContType() != mfem::FiniteElementCollection::DISCONTINUOUS);

    int npts=0;
    int neles=0;
    
    RefinedGeometry *refined_geo;
    Vector           scalar_vals;
    DenseMatrix      vec_vals;
    DenseMatrix      pmat;

    //
    // find the # of output points and cells at the selected level of 
    // refinement (we may want to cache this...)
    //
    for (int i = 0; i < mesh->GetNE(); i++)
    {
        int geom = mesh->GetElementBaseGeometry(i);
        int ele_nverts = Geometries.GetVertices(geom)->GetNPoints();
        refined_geo    = GlobGeometryRefiner.Refine((Geometry::Type)geom, lod, 1);
        npts  += refined_geo->RefPts.GetNPoints();
        neles += refined_geo->RefGeoms.Size() / ele_nverts;
    }
    
    vtkFloatArray *rv = vtkFloatArray::New();
    if(ncomps == 2)
        rv->SetNumberOfComponents(3);
    else
        rv->SetNumberOfComponents(ncomps);
    if(var_is_nodal)
        rv->SetNumberOfTuples(npts);
    else
        rv->SetNumberOfTuples(neles);

    double tuple_vals[9];
    int ref_idx=0;
    for (int i = 0; i <  mesh->GetNE(); i++)
    {
        int geom       = mesh->GetElementBaseGeometry(i);
        refined_geo    = GlobGeometryRefiner.Refine((Geometry::Type)geom, lod, 1);
        if(ncomps == 1)
        {
            gf->GetValues(i, refined_geo->RefPts, scalar_vals, pmat);
            for (int j = 0; j < scalar_vals.Size();j++)
            {
                tuple_vals[0] = scalar_vals(j);
                rv->SetTuple(ref_idx, tuple_vals); 
                ref_idx++;
            }
        }
        else
        {
            gf->GetVectorValues(i, refined_geo->RefPts, vec_vals, pmat);
            for (int j = 0; j < vec_vals.Width(); j++)
            {
                tuple_vals[2] = 0.0;
                tuple_vals[0] = vec_vals(0,j);
                tuple_vals[1] = vec_vals(1,j);
                if (vec_vals.Height() > 2)
                    tuple_vals[2] = vec_vals(2,j);
                rv->SetTuple(ref_idx, tuple_vals); 
                ref_idx++;
            }
        }
    }
    
    return rv;
}

// ****************************************************************************
//  Method: avtFMSFileFormat::GetRefinedElementColoring
//
//  Purpose:
//   Constructs a vtkDataArray that contains coloring that refects the orignal
//   finite elements of a mfem mesh.
//
//  Arguments:
//   var_name:  string with desired mesh name
//   domain:    domain id
//   lod:       number of refinement steps 
//
//  Programmer: Cyrus Harrison
//  Creation:   Sat Jul  5 11:38:31 PDT 2014
//
// Modifications:
//   Cyrus Harrison, Tue May 23 10:12:52 PDT 2017
//   Seed rng with domain id for predictable coloring results
//   (See: https://github.com/visit-dav/visit/issues/2747)
//
//   Alister Maguire, Thu Jan  2 15:23:13 MST 2020
//   Casting int to Geom::Type where appropriate. This is required after the
//   upgrade to mfem 4.0.
//
// ****************************************************************************
vtkDataArray *
avtFMSFileFormat::GetRefinedElementColoring(const std::string &mesh_name,
                                             int domain,
                                             int lod)
{
    //
    // fetch the base mfem mesh
    //
    Mesh *mesh = FetchMesh(mesh_name,domain);
    int npts=0;
    int neles=0;
    
    RefinedGeometry *refined_geo;
    Array<int>       coloring;
    
    //
    // find the # of output points and cells at the selected level of 
    // refinement (we may want to cache this...)
    //
    for (int i = 0; i < mesh->GetNE(); i++)
    {
        int geom = mesh->GetElementBaseGeometry(i);
        int ele_nverts = Geometries.GetVertices(geom)->GetNPoints();
        refined_geo    = GlobGeometryRefiner.Refine((Geometry::Type)geom, lod, 1);
        npts  += refined_geo->RefPts.GetNPoints();
        neles += refined_geo->RefGeoms.Size() / ele_nverts;
    }

    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfComponents(1);
    rv->SetNumberOfTuples(neles);

    //
    // Use mfem's mesh coloring alog
    //

    // seed using domain id for predictable results
    srand(domain);

    double a = double(rand()) / (double(RAND_MAX) + 1.);

    int el0 = (int)floor(a * mesh->GetNE());
    mesh->GetElementColoring(coloring, el0);
    int ref_idx=0;
    // set output array value from generated coloring
    for (int i = 0; i < mesh->GetNE(); i++)
    {
        int geom = mesh->GetElementBaseGeometry(i);
        int nv = Geometries.GetVertices(geom)->GetNPoints();
        refined_geo= GlobGeometryRefiner.Refine((Geometry::Type)geom, lod, 1);
        for (int j = 0; j < refined_geo->RefGeoms.Size(); j += nv)
        {
             rv->SetTuple1(ref_idx,coloring[i]+1);
             ref_idx++;
        }
   }

   return rv;
}


// ****************************************************************************
//  Method: avtFMSFileFormat::GetRefinedElementAttribute
//
//  Purpose:
//   Constructs a vtkDataArray that contains the refined "attribute" value 
//   for finite elements in a mfem mesh.
//
//  Arguments:
//   var_name:  string with desired mesh name
//   domain:    domain id
//   lod:       number of refinement steps 
//
//  Programmer: Cyrus Harrison
//  Creation:   Sat Jul  5 11:38:31 PDT 2014
//
//  Modifications:
//
//    Alister Maguire, Thu Jan  2 15:23:13 MST 2020
//    Casting int to Geom::Type where appropriate. This is required after the
//    upgrade to mfem 4.0.
//
// ****************************************************************************
vtkDataArray *
avtFMSFileFormat::GetRefinedElementAttribute(const std::string &mesh_name, 
                                              int domain, 
                                              int lod)
{
    //
    // fetch the base mfem mesh
    //
    Mesh *mesh = FetchMesh(mesh_name,domain);
    int npts=0;
    int neles=0;
    
    RefinedGeometry *refined_geo;
    Array<int>       coloring;
    
    //
    // find the # of output points and cells at the selected level of 
    // refinement (we may want to cache this...)
    //
    for (int i = 0; i < mesh->GetNE(); i++)
    {
        int geom = mesh->GetElementBaseGeometry(i);
        int ele_nverts = Geometries.GetVertices(geom)->GetNPoints();
        refined_geo    = GlobGeometryRefiner.Refine((Geometry::Type)geom, lod, 1);
        npts  += refined_geo->RefPts.GetNPoints();
        neles += refined_geo->RefGeoms.Size() / ele_nverts;
    }

    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfComponents(1);
    rv->SetNumberOfTuples(neles);

    // set output array value from the mfem mesh's "Attribue" field
    int ref_idx=0;
    for (int i = 0; i < mesh->GetNE(); i++)
    {
        int geom = mesh->GetElementBaseGeometry(i);
        int nv = Geometries.GetVertices(geom)->GetNPoints();
        refined_geo= GlobGeometryRefiner.Refine((Geometry::Type)geom, lod, 1);
        int attr = mesh->GetAttribute(i);
        for (int j = 0; j < refined_geo->RefGeoms.Size(); j += nv)
        {
             rv->SetTuple1(ref_idx,attr);
             ref_idx++;
        }
   }

   return rv;
}

// ****************************************************************************
// Method: avtFMSFileFormat::GetDOFMesh
//
// Purpose:
//   Make a mesh that shows the dof locations.
//
// Arguments:
//   domain : The domain for which we want dof locations.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 17 15:10:23 PDT 2020
//
// Modifications:
//
// ****************************************************************************

vtkDataSet *
avtFMSFileFormat::GetDOFMesh(int domain)
{
    vtkPolyData *pd = nullptr;
    Mesh *mesh = FetchMesh(MESH_NAME,domain);

    auto unrefined_coords = mesh->GetNodes();
    if(unrefined_coords != nullptr)
    {
        int vdim = unrefined_coords->VectorDim();
        int total_dofs = unrefined_coords->Size() / vdim;
        const auto fes = unrefined_coords->FESpace();

        // Populate the points with the raw dof locations.
        vtkPoints *pts = vtkPoints::New();
        pts->SetNumberOfPoints(total_dofs);
        for(int dof = 0; dof < total_dofs; dof++)
        {
            double pt[3]={0., 0., 0.};
            for(int v = 0; v < vdim; v++)
                pt[v] = unrefined_coords->operator()(fes->DofToVDof(dof,v));
            pts->SetPoint(dof, pt);
        }

        // Now, make the resulting polydata.
        pd = vtkPolyData::New();
        pd->SetPoints(pts);
        pts->Delete();
 
        vtkCellArray *verts = vtkCellArray::New();
        pd->SetVerts(verts);
        verts->Delete();
        for (int dof = 0 ; dof < total_dofs ; dof++)
        {
            verts->InsertNextCell(1);
            verts->InsertCellPoint(dof);
        }
    }

    return pd;
}

// ****************************************************************************
//  Method: avtFMSFileFormat::RegisterDataSelections
//
//  Purpose: 
//   Used to suport avtResolutionSelection & capture the selected lod.
//
//  Arguments:
//     sels:    data selection list from the pipeline
//     applied: pipeline handshaking for handling data selections
//
//
// ****************************************************************************
void
avtFMSFileFormat::RegisterDataSelections(const std::vector<avtDataSelection_p>& sels, 
                                          std::vector<bool>* applied)
{
    for(size_t i=0; i < sels.size(); ++i)
    {
        if(strcmp(sels[i]->GetType(), "avtResolutionSelection") == 0)
        {
            const avtResolutionSelection* sel =
                static_cast<const avtResolutionSelection*>(*sels[i]);
            this->selectedLOD = sel->resolution();
            (*applied)[i] = true;
        }
    }
}



