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

#include <fmsio.h>

#include <vector>
#include <map>
#include <string>

using     std::string;
using     std::ostringstream;
using     std::vector;

// Controls whether to create original cell ids.
#define CREATE_ORIGINAL_CELL_IDS

//#define MAKE_DATA_INSTEAD_OF_READING_IT
#ifdef MAKE_DATA_INSTEAD_OF_READING_IT
void TestMakeFmsHexMesh(FmsDataCollection *dc_ptr) {
  FmsMesh mesh;
  FmsMeshConstruct(&mesh);

  FmsDomain *domains;
  FmsMeshAddDomains(mesh, "domain", 1, &domains);
  FmsDomainSetNumVertices(domains[0], 12);

  FmsDomainSetNumEntities(domains[0], FMS_EDGE, FMS_INT32, 20);
  const int edge_vert[] = {
    0,1,  // 1
    1,2,  // 2
    0,3,  // 3
    1,4,  // 4
    2,5,  // 5
    3,4,  // 6
    4,5,  // 7
    0,6,  // 8
    1,7,  // 9
    2,8,  // 10
    3,9,  // 11
    4,10, // 12
    5,11, // 13
    6,7,  // 14
    7,8,  // 15
    6,9,  // 16
    7,10, // 17
    8,11, // 18
    9,10, // 19
    10,11 // 20
  };
  FmsDomainAddEntities(domains[0], FMS_EDGE, NULL, FMS_INT32, edge_vert, 20);

  FmsDomainSetNumEntities(domains[0], FMS_QUADRILATERAL, FMS_INT32, 11);
  const int face_edge[] = {
    0,3,5,2,     // 1
    1,4,6,3,     // 2
    7,2,10,15,   // 3
    8,3,11,16,   // 4
    9,17,12,4,   // 5
    13,15,18,16, // 6
    14,16,19,17, // 7
    0,8,13,7,    // 8
    1,9,14,8,    // 9
    5,11,18,10,  // 10
    6,12,19,11   // 11
  };
  FmsDomainAddEntities(domains[0], FMS_QUADRILATERAL, NULL, FMS_INT32,
                       face_edge, 11);

  FmsDomainSetNumEntities(domains[0], FMS_HEXAHEDRON, FMS_INT32, 2);
  const int hex_face[] = {
    7,9,0,5,2,3,
    8,10,1,6,3,4,
  };
  FmsDomainAddEntities(domains[0], FMS_HEXAHEDRON, NULL, FMS_INT32, hex_face,
                       2);

  FmsComponent volume, boundary;
  FmsMeshAddComponent(mesh, "volume", &volume);
  FmsComponentAddDomain(volume, domains[0]);

  FmsMeshAddComponent(mesh, "boundary", &boundary);
  FmsInt part_id;
  FmsComponentAddPart(boundary, domains[0], &part_id);
  const int bdr_face[] = {0,1,2,4,5,6,7,8,9,10};
  FmsComponentAddPartEntities(boundary, part_id, FMS_QUADRILATERAL, FMS_INT32,
                              FMS_INT32, FMS_INT32, NULL, bdr_face, NULL, 10);

  FmsComponentAddRelation(volume, 1); // 1 = index of "boundary" component

  FmsTag attributes, bdr_attributes;
  FmsMeshAddTag(mesh, "attributes", &attributes);
  FmsTagSetComponent(attributes, volume);
  const int hex_attr[] = {1,2};
  FmsTagSet(attributes, FMS_UINT8, FMS_INT32, hex_attr, 2);
  const int described_attr[] = {1,2};
  const char *attr_descr[] = {"material 1", "material 2"};
  FmsTagAddDescriptions(attributes, FMS_INT32, described_attr, attr_descr, 2);

  FmsMeshAddTag(mesh, "bdr_attributes", &bdr_attributes);
  FmsTagSetComponent(bdr_attributes, boundary);
  const int bdr_attr[] = {1,1,2,3,4,4,5,5,6,6};
  FmsTagSet(bdr_attributes, FMS_UINT8, FMS_INT32, bdr_attr, 10);
  const int bdr_described_attr[] = {1,2,3,4,5,6};
  const char *bdr_attr_descr[] = {
    "face 1", "face 2", "face 3", "face 4", "face 5", "face 6"
  };
  FmsTagAddDescriptions(bdr_attributes, FMS_INT32, bdr_described_attr,
                        bdr_attr_descr, 6);

  FmsMeshFinalize(mesh);
  FmsMeshValidate(mesh);

  FmsDataCollectionCreate(mesh, "data collection", dc_ptr);
  FmsDataCollection dc = *dc_ptr;

  FmsFieldDescriptor coords_fd;
  FmsDataCollectionAddFieldDescriptor(dc, "coords descriptor", &coords_fd);
  FmsFieldDescriptorSetComponent(coords_fd, volume);
  FmsInt order = 1;
  FmsFieldDescriptorSetFixedOrder(coords_fd, FMS_CONTINUOUS,
                                  FMS_NODAL_GAUSS_CLOSED, order);

  FmsField coords;
  FmsDataCollectionAddField(dc, "coords", &coords);
  const double coords_data[] = {
    0.,0.,0.,
    1.,0.,0.,
    2.,0.,0.,
    0.,0.,1.,
    1.,0.,1.,
    2.,0.,1.,
    0.,1.,0.,
    1.,1.,0.,
    2.,1.,0.,
    0.,1.,1.,
    1.,1.,1.,
    2.,1.,1.
  };
  FmsFieldSet(coords, coords_fd, 3, FMS_BY_VDIM, FMS_DOUBLE, coords_data);

  FmsComponentSetCoordinates(volume, coords);

#if 1
   // BJW: Add some fields.
   
  FmsFieldDescriptor scalar_fd;
  FmsDataCollectionAddFieldDescriptor(dc, "scalar", &scalar_fd);
  FmsFieldDescriptorSetComponent(scalar_fd, volume);
  FmsInt sorder = 1;
  FmsFieldDescriptorSetFixedOrder(scalar_fd, FMS_CONTINUOUS,
                                  FMS_NODAL_GAUSS_CLOSED, sorder);
  FmsField scalar;
  FmsDataCollectionAddField(dc, "scalar", &scalar);
  const double scalar_data[] = {
    0.,1.,2.,3.,4.,5.,6.,7.,8.,9.,10.,11.
  };
  FmsFieldSet(scalar, scalar_fd, 1, FMS_BY_VDIM, FMS_DOUBLE, scalar_data);

   // BJW: Add some metadata.

   // NOTE: We can add fmsMetaData to fields too.
   //       I would add units and extents.

   /** @brief Make sure the meta-data structure associated with a field is
       allocated and return it in @a mdata.
   int FmsFieldAttachMetaData(FmsField field, FmsMetaData *mdata);
   */

  // Make a metadata on the data collection.
  FmsMetaData md;
  FmsDataCollectionAttachMetaData(*dc_ptr, &md);

  double time[] = {1.23456};
  FmsMetaDataSetScalars(md, "time", FMS_DOUBLE, 1, (void **)&time);
  int cycle [] = {123456};
  FmsMetaDataSetIntegers(md, "cycle", FMS_INT32, 1, (void **)&cycle);

  // Eh, maybe this would be better to add to the coordinates field.
  FmsMetaDataSetString(md, "units", "cm");
 
#endif
}

#endif
using namespace mfem;


/**
Let's make some simplifying assumptions.It looks like MFEM does not have 
multiple domains in its Mesh class. The DataCollection contains a single 
Mesh. Related domains in the FmsDataCollection all will be exposed as a 
single mesh.
*/

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

avtFMSFileFormat::Internals::Internals() : protocol("yaml"), filenames(), 
    mfemCache(), fieldUnits(), coordFieldName(), databaseComment()
{
}

avtFMSFileFormat::Internals::~Internals()
{
    // Delete MFEM objects.
    for(std::map<std::string, mfem::DataCollection*>::iterator it = mfemCache.begin();
        it != mfemCache.end(); it++)
    {
        delete it->second;
    }
}

bool
avtFMSFileFormat::Internals::LoadRoot(const std::string &filename)
{
    // Open the file.
    ifstream ifile(filename);
    if (ifile.fail())
    {
        return false;
    }

    char line[1024];
    while(!ifile.eof())
    {
        ifile.getline(line, 1024);

        char *comment = strstr(line, "#");
        if(comment != nullptr)
            comment[0] = '\0';

        std::string fn(line);
        StringHelpers::trim(fn);

        filenames.push_back(fn);
    }

    return !filenames.empty();
}

bool
avtFMSFileFormat::Internals::LoadFMS(const std::string &filename)
{
    if(!filename.empty())
        filenames.push_back(filename);
    return !filenames.empty();
}

int
avtFMSFileFormat::Internals::TotalDomains() const
{
    return filenames.size();
}

std::string
avtFMSFileFormat::Internals::FilenameForDomain(int domain) const
{
    return filenames[domain];
}

//---------------------------------------------------------------------------
static bool
FmsMetaDataGetString(FmsMetaData mdata, const std::string &key, std::string &value)
{   
    if (!mdata) false;

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

const std::string &
avtFMSFileFormat::Internals::GetCoordinateFieldName() const
{
    return coordFieldName;
}

const std::string &
avtFMSFileFormat::Internals::GetDatabaseComment() const
{
    return databaseComment;
}

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
#ifdef MAKE_DATA_INSTEAD_OF_READING_IT
            TestMakeFmsHexMesh(&dc);
#else
            if(FmsIORead(fn.c_str(), protocol.c_str(), &dc) == 0)
#endif
            {
debug5 << mName << ": Data collection was read." << endl;

                TRY
                {
                    // Convert the FMS data collection to MFEM.
                    mfem::DataCollection *mdc = nullptr;
                    if(mfem::FmsDataCollectionToDataCollection(dc, &mdc) == 0)
                    {
debug5 << mName << ": Caching MFEM data collection for domain " << domain << endl;
                        // Save the data collection.
                        mfemCache[fn] = mdc;

                        retval = true;
                    }
                    else
                    {
debug5 << mName << ": FAIL! MFEM data collection for domain " << domain << " did not convert." << endl;
                    }
                }
                CATCH2(mfem::ErrorException, e)
                {
                    debug5 << mName << ": MFEM exception: \"" << e.what() << "\"" << endl;
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
        EXCEPTION1(InvalidFilesException, GetFilename());
    }

    md->SetDatabaseComment(d->GetDatabaseComment());

    // Q: Do we need to segregate the domains/dataset into different meshes?

    std::string coordUnits;
    d->GetFieldUnits(d->GetCoordinateFieldName(), coordUnits);

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
    mmd->LODs = 50; // The MultiresControl operator does not let us go above 1 without this.
#ifdef CREATE_ORIGINAL_CELL_IDS
    mmd->containsOriginalCells = true;
#endif
    md->Add(mmd);

    // Use the cached MFEM data collection to add variables.
    auto m = dc->GetFieldMap();
    for(auto it = m.begin(); it != m.end(); it++)
    {
        // NOTE: assume node centered except when we get a DISCONTINUOUS
        // FE collection. This does not account for TANGENTIAL,NORMAL cases.
        avtCentering centering = AVT_NODECENT;
        if(it->second->FESpace()->FEColl()->GetContType() ==
            FiniteElementCollection::DISCONTINUOUS)
        {
            centering = AVT_ZONECENT;
        }

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

#if 0
// Interrogate the FMS data collection to add scalars.

    std::vector<std::string> scalars, vectors;
    FmsField *fields = nullptr;
    FmsInt num_fields = 0;
    if(FmsDataCollectionGetFields(dc, &fields, &num_fields) == 0)
    {
        debug5 << mName << ": num_fields = " << num_fields << endl;

        for(FmsInt i = 0; i < num_fields; ++i)
        {
            FmsFieldDescriptor fd;
            FmsInt num_vec_comp = 0;
            FmsLayoutType layout_type;
            FmsScalarType data_type;
            const void *data = nullptr;

            if(FmsFieldGet(fields[i], &fd, &num_vec_comp, &layout_type, &data_type, &data) == 0)
            {
                const char *fd_name = nullptr;
                FmsFieldDescriptorGetName(fd, &fd_name);

                debug5 << mName << ": We got field \"" << fd_name << "\"" << endl;

                FmsFieldType field_type;
                FmsBasisType basis_type;
                FmsInt order = 0;
                int err = FmsFieldDescriptorGetFixedOrder(fd, &field_type, &basis_type, &order);
                debug5 << "\terr="<<err<<endl;
                if(fd_name != nullptr && err == 0)
                {
// TODO: read up on these field types. Are they centerings plus other info?
                    avtCentering centering = AVT_NODECENT;
                    if(field_type == FMS_DISCONTINUOUS ||
                       field_type == FMS_DISCONTINUOUS_WEIGHTED)
                    {
                        centering = AVT_ZONECENT;
                    }

                    if(data_type == FMS_FLOAT || data_type == FMS_DOUBLE)
                    {

// Q: Are there any min/max,units, etc in the metadata?

                        if(num_vec_comp == 1)
                        {
                            avtScalarMetaData *smd = new avtScalarMetaData;
                            smd->name = fd_name;
                            smd->meshName = mmd->name;
                            smd->centering = centering;
                            md->Add(smd);
                        }
                        else if(num_vec_comp == 2 || num_vec_comp == 3)
                        {
                            avtVectorMetaData *vmd = new avtVectorMetaData;
                            vmd->name = fd_name;
                            vmd->meshName = mmd->name;
                            vmd->centering = centering;
                            md->Add(vmd);
                        }
                        else
                        {
                            debug5 << "Unsupported number of components: " << num_vec_comp << endl;
                        }
                    }
                    else if(data_type == FMS_COMPLEX_FLOAT || data_type == FMS_COMPLEX_DOUBLE)
                    {
                        if(num_vec_comp == 1)
                        {
                            // Expose the data as "real" and "imaginary" parts.

                            avtScalarMetaData *smd = new avtScalarMetaData;
                            smd->name = std::string(fd_name) + ".real";
                            smd->meshName = mmd->name;
                            smd->centering = centering;
                            md->Add(smd);

                            avtScalarMetaData *smdi = new avtScalarMetaData;
                            smdi->name = std::string(fd_name) + ".imag";
                            smdi->meshName = mmd->name;
                            smdi->centering = centering;
                            md->Add(smdi);
                        }
                        else
                        {
                            debug5 << mName << ": Unsupported number of components "
                                   << num_vec_comp << " on complex data." << endl;
                        }
                    }
                    else
                    {
                        debug5 << mName << ": Unsupported field type." << endl;
                    }
                }
                else
                {
                    debug5 << mName << ": Could not get field descriptor information." << endl;
                }
            }
            else
            {
                debug5 << mName << ": FmsFieldGet failed." << endl;
            }
        }
    }
    else
    {
        debug5 << mName << ": Could not get the fields." << endl;
    }
#endif

#if 0


    ///
    /// Open the root file
    ///
    std::string root_file(GetFilename());

    JSONRoot root_md(root_file);
    vector<string>dset_names;
    root_md.DataSets(dset_names);

    // enumerate datasets)
    selectedLOD = 0;
    for(int i=0;i<(int)dset_names.size();i++)
    {
        JSONRootDataSet &dset =  root_md.DataSet(dset_names[i]);
        int nblocks      = dset.NumberOfDomains();
        int spatial_dim  = atoi(dset.Mesh().Tag("spatial_dim").c_str());
        int topo_dim     = atoi(dset.Mesh().Tag("topo_dim").c_str());
        int block_origin = 0;
        double *extents = NULL;

        //
        // Add the meta-data object for the current mesh:
        //
        AddMeshToMetaData(md,
                          dset_names[i].c_str(),
                          AVT_UNSTRUCTURED_MESH,
                          extents,
                          nblocks,
                          block_origin,
                          spatial_dim, topo_dim);

        md->GetMeshes(i).LODs = atoi(dset.Mesh().Tag("max_lods").c_str());

        // Add builtin mfem fields related to the mesh:

        AddScalarVarToMetaData(md,
                               "element_coloring",
                               dset_names[i].c_str(),
                               AVT_ZONECENT);
        AddScalarVarToMetaData(md,
                               "element_attribute",
                               dset_names[i].c_str(),
                               AVT_ZONECENT);
    
    
        /// add mesh variables
        vector<string>field_names;
        dset.Fields(field_names);
        for(size_t j=0;j<field_names.size();j++)
        {
            JSONRootEntry &field = dset.Field(field_names[j]);
            std::string slod = field.Tag("lod");
            int ilod = std::min(md->GetMeshes(i).LODs,atoi(slod.c_str()));
            selectedLOD = std::max(selectedLOD,ilod);
            std::string f_assoc = field.Tag("assoc");

            if(f_assoc == "elements")
            {
                if(!field.HasTag("comps") || field.Tag("comps") == "1")
                {
                    AddScalarVarToMetaData(md,
                                           field_names[j].c_str(),
                                           dset_names[i].c_str(),
                                           AVT_ZONECENT);
                }
                else if(field.Tag("comps") == "2")
                {
                    AddVectorVarToMetaData(md,
                                          field_names[j].c_str(),
                                          dset_names[i].c_str(),
                                          AVT_ZONECENT,2);
                }
                else if(field.Tag("comps") == "3")
                {
                    AddVectorVarToMetaData(md,
                                          field_names[j].c_str(),
                                          dset_names[i].c_str(),
                                          AVT_ZONECENT,3);
                }


            }
            else if(f_assoc == "nodes")
            {
                if(field.Tag("comps") == "1")
                {
                    AddScalarVarToMetaData(md,
                                           field_names[j].c_str(),
                                           dset_names[i].c_str(),
                                           AVT_NODECENT);
                }
                else if(field.Tag("comps") == "2")
                {
                    AddVectorVarToMetaData(md,
                                          field_names[j].c_str(),
                                          dset_names[i].c_str(),
                                          AVT_NODECENT,2);
                }
                else if(field.Tag("comps") == "3")
                {
                    AddVectorVarToMetaData(md,
                                          field_names[j].c_str(),
                                          dset_names[i].c_str(),
                                          AVT_NODECENT,3);
                }
            }
        }
    }
#endif
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
    return GetRefinedMesh(string(meshname),domain,selectedLOD+1);
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
   return GetRefinedVar(string(varname),domain,selectedLOD+1);
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
// ****************************************************************************
vtkDataSet *
avtFMSFileFormat::GetRefinedMesh(const std::string &mesh_name, int domain, int lod)
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
    for (int i = 0; i < mesh->GetNE(); i++)
    {
        int geom = mesh->GetElementBaseGeometry(i);
        int ele_nverts = Geometries.GetVertices(geom)->GetNPoints();
        refined_geo = GlobGeometryRefiner.Refine((Geometry::Type)geom, lod, 1);
        npts  += refined_geo->RefPts.GetNPoints();
        neles += refined_geo->RefGeoms.Size() / ele_nverts;
    }

    // create the points for the refined topoloy   
    res_pts->Allocate(npts);
    res_pts->SetNumberOfPoints((vtkIdType) npts);
   
    // create the points for the refined topoloy
    int pt_idx=0;
    for (int i = 0; i < mesh->GetNE(); i++)
    {
        int geom = mesh->GetElementBaseGeometry(i);
        refined_geo = GlobGeometryRefiner.Refine((Geometry::Type)geom, lod, 1);
        // refined points
        mesh->GetElementTransformation(i)->Transform(refined_geo->RefPts, pmat);
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

#ifdef CREATE_ORIGINAL_CELL_IDS
    // Make some original cell ids so we can try to eliminate the mesh lines.
    std::vector<unsigned int> originalCells;
    originalCells.reserve(neles);
    unsigned int udomain = static_cast<unsigned int>(domain);
#endif

    pt_idx=0;
    for (int i = 0; i <  mesh->GetNE(); i++)
    {
        int geom       = mesh->GetElementBaseGeometry(i);
        int ele_nverts = Geometries.GetVertices(geom)->GetNPoints();
        refined_geo    = GlobGeometryRefiner.Refine((Geometry::Type)geom, lod, 1);

        Array<int> &rg_idxs = refined_geo->RefGeoms;

        vtkCell *ele_cell = NULL;
        // rg_idxs contains all of the verts for the refined elements
        for (int j = 0; j < rg_idxs.Size(); )
        {
            switch (geom)
            {
                case Geometry::SEGMENT:      ele_cell = vtkLine::New();       break;
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

#ifdef CREATE_ORIGINAL_CELL_IDS
            originalCells.push_back(udomain);
            originalCells.push_back(static_cast<unsigned int>(i));
#endif
        }

        pt_idx += refined_geo->RefPts.GetNPoints();
   }

#ifdef CREATE_ORIGINAL_CELL_IDS
    vtkUnsignedIntArray *origZones = vtkUnsignedIntArray::New();
    origZones->SetName("avtOriginalCellNumbers");
    origZones->SetNumberOfComponents(2);
    origZones->SetNumberOfTuples(originalCells.size()/2);
    memcpy(origZones->GetVoidPointer(0), &originalCells[0],
           sizeof(unsigned int) * originalCells.size());
    res_ds->GetCellData()->AddArray(origZones);
    origZones->Delete();
#endif

//   delete mesh;
       
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
    bool var_is_nodal = (gf->FESpace()->FEColl()->GetContType() != mfem::FiniteElementCollection::DISCONTINUOUS);

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
//   (See: http://visitbugs.ornl.gov/issues/2747)
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
   
   delete mesh;
   
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
   
   delete mesh;
   
   return rv;
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



