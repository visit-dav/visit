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
#include <vtkCell.h>
#include <vtkLine.h>
#include <vtkTriangle.h>
#include <vtkHexahedron.h>
#include <vtkQuad.h>
#include <vtkTetra.h>
#include <vtkPoints.h>

#include <fmsio.h>

using     std::string;
using     std::ostringstream;
using     std::vector;

// TODO: transplant these to MFEM
// Define some functions for going back and forth FMS<-->MFEM
namespace mfem
{

// Converts FmsDomain to mfem::Mesh.
// Q: shall we convert all fields over too? Or, have different versions that do the fields.
mfem::Mesh *
MeshFromFmsDomain(FmsDomain domain, bool addFields)
{
    // TODO: Write me.
    return nullptr;
}

mfem::Mesh *
MeshFromFmsDomain(FmsDomain domain, const std::vector<std::string> &fieldNames)
{
    // TODO: Write me.
    return nullptr;
}

} // end namespace mfem

using namespace mfem;


class avtFMSFileFormat::Internals
{
public:
    Internals();
    ~Internals();

    bool LoadRoot(const std::string &filename);
    bool LoadFMS(const std::string &filename);

    int TotalDomains() const;
    
    mfem::Mesh *GetDomain(const char *domain_name, int domain);
    mfem::Mesh *GetDomainWithField(const char *domain_name, int domain, const std::string &fieldName);

    void ComputeTotalDomains();

protected:
    bool GlobalToLocalDomain(int dom, size_t &dbi, int &localDomain) const;
    std::string FilenameForDomain(int domain) const;
    bool ReadDomain(int domain);

    std::string                              protocol;
    std::vector<std::string>                 filenames;
    std::vector<int>                         domainCount;
    std::map<std::string, FmsDataCollection> dcCache;
    std::map<int, mfem::Mesh *>              mfemCache;
};

avtFMSFileFormat::Internals::Internals() : protocol("ascii"), filenames(), domainCount(), dcCache(), mfemCache()
{
}

avtFMSFileFormat::Internals::~Internals()
{
    // Delete FMS objects.
    for(std::map<std::string, FmsDataCollection>::iterator it = dcCache.begin();
        it != dcCache.end(); it++)
    {
        FmsDataCollectionDestroy(&(it->second));
    }

    // Delete MFEM objects.
    for(std::map<int, mfem::Mesh*>::iterator it = mfemCache.begin();
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

    return true;
}

bool
avtFMSFileFormat::Internals::LoadFMS(const std::string &filename)
{
    filenames.push_back(filename);
}

int
avtFMSFileFormat::Internals::TotalDomains() const
{
    int n = 0;
    for(size_t i = 0; i < domainCount.size(); ++i)
        n += domainCount[i];
    return n;
}

void
avtFMSFileFormat::Internals::ComputeTotalDomains()
{
    if(domainCount.empty())
        return;

    std::vector<int> mywork;
    int N = static_cast<int>(filenames.size());
    for(int i = 0; i < N; ++i)
    {
        if(i % PAR_Size() == PAR_Rank())
            mywork.push_back(i);
    }

    std::vector<int> inputDomainCount(0, filenames.size());
    domainCount.resize(filenames.size(), 0);

    // Read the files assigned in mywork so we can get their number of domains.
    for(size_t i = 0; i < mywork.size(); ++i)
    {
        FmsDataCollection dc;
        if(FmsIORead(filenames[mywork[i]].c_str(), protocol.c_str(), &dc) == 0)
        {
            // Save the data collection.
            dcCache[filenames[mywork[i]]] = dc;

            // Save its number of domains too.
            int nDomains = 0;
            FmsMesh mesh;
            if(FmsDataCollectionGetMesh(dc, &mesh) == 0)
            {
                FmsInt nDomainNames = 0;
                if(FmsMeshGetNumDomainNames(mesh, &nDomainNames) == 0)
                    nDomains = static_cast<int>(nDomainNames);
            }

            inputDomainCount[i] = nDomains;
        }
    }

    // Sum the counts across all ranks.
    SumIntArrayAcrossAllProcessors(&inputDomainCount[0], &domainCount[0], domainCount.size());
}

bool
avtFMSFileFormat::Internals::GlobalToLocalDomain(int dom, size_t &dbi,
    int &localDomain) const
{
    int start = 0;
    for(dbi = 0; dbi < domainCount.size(); ++dbi)
    {
        int nb = domainCount[dbi];
        int end = start + nb;
        if(dom >= start && dom < end)
        {
            localDomain = dom - start;
            return true;
        }
        start += nb;
    }
    return false;
}

std::string
avtFMSFileFormat::Internals::FilenameForDomain(int domain) const
{
    std::string fn;
    size_t dbi = 0;
    int localDomain = 0;
    if(GlobalToLocalDomain(domain, dbi, localDomain))
        fn = filenames[dbi];
    return fn;
}

bool
avtFMSFileFormat::Internals::ReadDomain(int domain)
{
    bool retval = false;
    std::string fn(FilenameForDomain(domain));
    if(!fn.empty())
    {
        if(dcCache.find(fn) == dcCache.end())
        {
            FmsDataCollection dc;
            if(FmsIORead(fn.c_str(), protocol.c_str(), &dc) == 0)
            {
                // Save the data collection.
                dcCache[fn] = dc;

                retval = true;
            }
        }
        else
        {
            // The file has already been read.
            retval = true;
        }
    }
    return retval;
}

mfem::Mesh *
avtFMSFileFormat::Internals::GetDomain(const char *domain_name, int domain)
{
    std::vector<std::string> fields;

    mfem::Mesh *retval = nullptr;
    if(ReadDomain(domain))
    {
        std::map<int, mfem::Mesh *>::iterator it = mfemCache.find(domain);
        if(it != mfemCache.end())
        {
            // We already cached the MFEM dataset.
            retval = it->second;
        }
        else
        {
            // Convert the MFEM dataset from FMS.
            std::string fn;
            size_t dbi = 0;
            int localDomain = 0;
            if(GlobalToLocalDomain(domain, dbi, localDomain))
                fn = filenames[dbi];

            FmsDataCollection dc = dcCache[fn];
            FmsMesh mesh;
            if(FmsDataCollectionGetMesh(dc, &mesh) == 0)
            {
                FmsInt num_domains = 0;
                FmsDomain *domains = nullptr;

                // Get all the domains in this mesh having the right name.
                if(FmsMeshGetDomainsByName(mesh, domain_name,
                                           &num_domains, &domains) == 0)
                {
                    retval = mfem::MeshFromFmsDomain(domains[localDomain], fields);

                    // Cache the object for later.
                    mfemCache[domain] = retval;
                }
            }
        }
    }

    return retval;
}

mfem::Mesh *
avtFMSFileFormat::Internals::GetDomainWithField(const char *domain_name, 
    int domain, const std::string &fieldName)
{
    std::vector<std::string> fields;
    fields.push_back(fieldName);

    mfem::Mesh *retval = nullptr;
    if(ReadDomain(domain))
    {
        // Convert the MFEM dataset from FMS.
        std::string fn;
        size_t dbi = 0;
        int localDomain = 0;
        if(GlobalToLocalDomain(domain, dbi, localDomain))
            fn = filenames[dbi];

        FmsDataCollection dc = dcCache[fn];
        FmsMesh mesh;
        if(FmsDataCollectionGetMesh(dc, &mesh) == 0)
        {
            FmsInt num_domains = 0;
            FmsDomain *domains = nullptr;

            // Get all the domains in this mesh having the right name.
            if(FmsMeshGetDomainsByName(mesh, domain_name,
                                       &num_domains, &domains) == 0)
            {
                std::map<int, mfem::Mesh *>::iterator it = mfemCache.find(domain);
                if(it == mfemCache.end())
                {
                    retval = mfem::MeshFromFmsDomain(domains[localDomain], fields);
                    // Cache the object for later.
                    mfemCache[domain] = retval;
                }

                // Look for fieldName on the MFEM object.

                // If it does not have fieldName, try to add it from
                // the FMSDomain.
            }
        }
    }

    return retval; 
}


// ****************************************************************************
//  Method: avtFMSFileFormat constructor
//
//  Programmer: harrison37 -- generated by xml2avt
//  Creation:   Fri May 23 15:16:20 PST 2014
//
// ****************************************************************************

avtFMSFileFormat::avtFMSFileFormat(const char *filename)
    : avtSTMDFileFormat(&filename, 1)
{
    selectedLOD = 0;

    d = new Internals;
}

// ****************************************************************************
//  Method: avtFMSFileFormat destructor
//
//  Programmer: harrison37 -- generated by xml2avt
//  Creation:   Fri May 23 15:16:20 PST 2014
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
//  Programmer: harrison37 -- generated by xml2avt
//  Creation:   Fri May 23 15:16:20 PST 2014
//
// ****************************************************************************

void
avtFMSFileFormat::ActivateTimestep(void)
{
    avtSTMDFileFormat::ActivateTimestep();
#if 0
    if(root)
        FreeUpResources();
    std::string root_file(GetFilename());
    root = new JSONRoot(root_file);
#endif
}


void
avtFMSFileFormat::FreeUpResources(void)
{
}

// ****************************************************************************
//  Method: avtFMSFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: harrison37 -- generated by xml2avt
//  Creation:   Fri May 23 15:16:20 PST 2014
//
//
//  Modifications:
//   Cyrus Harrison, Wed Sep 24 10:47:00 PDT 2014
//   Move abs path logic into JSONRoot.
//
//   Mark C. Miller, Tue Sep 20 18:12:29 PDT 2016
//   Add expressions
// ****************************************************************************
void
avtFMSFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
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

    vector<string>expr_names;
    root_md.Expressions(expr_names);
    for(size_t i = 0; i < expr_names.size(); i++)
    {
        JSONRootExpr &jexpr = root_md.Expression(expr_names[i]);
        
        Expression::ExprType vartype = Expression::Unknown;
        if      (!strncasecmp(jexpr.Type().c_str(), "scalar", 6))
            vartype = Expression::ScalarMeshVar;
        else if (!strncasecmp(jexpr.Type().c_str(), "vector", 6))
            vartype = Expression::VectorMeshVar;
        else if (!strncasecmp(jexpr.Type().c_str(), "tensor", 6))
            vartype = Expression::TensorMeshVar;
        else if (!strncasecmp(jexpr.Type().c_str(), "array", 5))
            vartype = Expression::ArrayMeshVar;
        else if (!strncasecmp(jexpr.Type().c_str(), "material", 8))
            vartype = Expression::Material;
        else if (!strncasecmp(jexpr.Type().c_str(), "species", 7))
            vartype = Expression::Species;
        else
        {
            debug5 << "Warning: unknown expression type \"" << jexpr.Type() << "\" for expression "
                   << "\"" << expr_names[i] << "\"...skipping it." << endl;
            continue;
        }

        Expression expr;
        expr.SetName(expr_names[i]);
        expr.SetDefinition(jexpr.Defn());
        expr.SetType(vartype);
        md->AddExpression(&expr);
    }
#endif
}

// ****************************************************************************
//  Method: avtFMSFileFormat::GetCycle
//
//  Purpose:
//      Returns if we have the current cycle.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Oct 15 10:52:22 PDT 2014
//
// ****************************************************************************
int
avtFMSFileFormat::GetCycle() 
{
#if 0
    // VisIt doesn't support diff times / cycles for meshes
    // we loop over all meshes to see if any have valid cycle info
    if(root)
    {
        std::vector<std::string> dset_names;
        root->DataSets(dset_names);
        // enumerate datasets
        bool not_found = true;
        for(size_t i=0; i<dset_names.size() && not_found ;i++)
        {
            JSONRootDataSet &dset =  root->DataSet(dset_names[i]);
            if(dset.HasCycle())
            {
                return dset.Cycle();
            }
        }
    }
#endif
    return avtFileFormat::INVALID_CYCLE;
}


// ****************************************************************************
//  Method: avtFMSFileFormat::GetTime
//
//  Purpose:
//      Returns if we have the current cycle.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Oct 15 10:52:22 PDT 2014
//
// ****************************************************************************
double
avtFMSFileFormat::GetTime() 
{
#if 0
    // VisIt doesn't support diff times / cycles for meshes
    // we loop over all meshes to see if any have valid time info
    if(root)
    {
        std::vector<std::string> dset_names;
        root->DataSets(dset_names);
        // enumerate datasets
        bool not_found = true;
        for(size_t i=0; i<dset_names.size() && not_found ;i++)
        {
            JSONRootDataSet &dset =  root->DataSet(dset_names[i]);
            if(dset.HasTime())
            {
                return dset.Time();
            }
        }
    }
#endif
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
//  Programmer: harrison37 -- generated by xml2avt
//  Creation:   Fri May 23 15:16:20 PST 2014
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
//  Programmer: harrison37 -- generated by xml2avt
//  Creation:   Fri May 23 15:16:20 PST 2014
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
//  Programmer: harrison37 -- generated by xml2avt
//  Creation:   Fri May 23 15:16:20 PST 2014
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
//  Programmer: Cyrus Harrison
//  Creation:   Sat Jul  5 11:38:31 PDT 2014
//
// Modifications:
//
// ****************************************************************************
Mesh *
avtFMSFileFormat::FetchMesh(const std::string &mesh_name,int domain)
{
#if 1
    return nullptr;
#else
    if(root == NULL)
    {
        //failed to open mesh file
        ostringstream msg;
        msg << "Failed to open MFEM mesh:"
            << " root metadata is missing!";
        EXCEPTION1(InvalidFilesException, msg.str());
    }

    string mesh_path = root->DataSet(mesh_name).Mesh().Path().Expand(domain);
    string cat_path = root->DataSet(mesh_name).CatPath().Get();

    if (cat_path != "")
    {
        std::istringstream imeshstr;
        FetchDataFromCatFile(cat_path, mesh_path, imeshstr); 

        // failed to get to mesh data from cat file
        if (imeshstr)
            return new Mesh(imeshstr, 1, 0, false);
    }

    visit_ifstream imesh(mesh_path.c_str());
    if(imesh().fail())
    {
        //failed to open mesh file
        ostringstream msg;
        msg << "Failed to open MFEM mesh:"
            << " mesh name: " << mesh_name 
            << " domain: "    << domain
            << " mesh path: \"" << mesh_path << "\"";
        if (cat_path != "")
            msg << " cat path: \"" << cat_path << "\"";
        EXCEPTION1(InvalidFilesException, msg.str());
    }
   
    return new Mesh(imesh(), 1, 0, false);
#endif
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
    // get base mesh
    Mesh *mesh = FetchMesh(mesh_name,domain);
     
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
        }

        pt_idx += refined_geo->RefPts.GetNPoints();
   }
   
   delete mesh;
       
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
#if 0
    if(root == NULL)
    {
        //failed to open mesh file
        ostringstream msg;
        msg << "Failed to fetch MFEM grid function:"
            << " root metadata is missing!";
        EXCEPTION1(InvalidFilesException, msg.str());
    }

    // find mesh for var
    string mesh_name="main";
    vector<string> dset_names;
    root->DataSets(dset_names);
    for(size_t i=0;i<dset_names.size();i++)
    {
        JSONRootDataSet &dset = root->DataSet(dset_names[i]);
        vector<string> field_names;
        dset.Fields(field_names);
        for(size_t j=0;j<field_names.size();j++)
        {
            if(field_names[j] == var_name)
                mesh_name = dset_names[i];
        }
    }

    // check for special vars
    if(var_name == "element_coloring")
        return GetRefinedElementColoring(mesh_name,domain,lod);
    else if(var_name == "element_attribute") // handle with materials in the future?
        return GetRefinedElementAttribute(mesh_name,domain,lod);

    // get base mesh
    Mesh *mesh = FetchMesh(mesh_name,domain);

    JSONRootEntry &field = root->DataSet(mesh_name).Field(var_name);
    string field_path = field.Path().Expand(domain);
    bool var_is_nodal = field.Tag("assoc") == "nodes";
    int  ncomps       = atoi(field.Tag("comps").c_str());
    string cat_path = root->DataSet(mesh_name).CatPath().Get();

    GridFunction *gf = 0;
    if (cat_path != "")
    {
        std::istringstream igfstr;
        FetchDataFromCatFile(cat_path, field_path, igfstr); 

        if (igfstr)
            gf = new GridFunction(mesh,igfstr);   
    }

    if (!gf)
    {
        visit_ifstream igf(field_path.c_str());
        if (igf().fail())
        {
            //failed to open gf file
            ostringstream msg;
            msg << "Failed to open MFEM grid function: "
                << " field name: \""       << mesh_name << "\""
                << " domain: "             << domain
                << " grid function path: \"" << field_path << "\"";
            if (cat_path != "")
                msg << " cat path: \"" << cat_path << "\"";

            EXCEPTION1(InvalidFilesException, msg.str());
        }
        gf = new GridFunction(mesh,igf());   
    }

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
    
    delete gf;
    delete mesh;

    return rv;
#endif
    return nullptr;
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



