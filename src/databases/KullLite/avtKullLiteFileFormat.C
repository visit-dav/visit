// ************************************************************************* //
//                        avtKullLiteFileFormat.C                            //
// ************************************************************************* //

#include <avtKullLiteFileFormat.h>

#include <string>
#include <vector>
#include <fstream.h>

#include <pdb.h>
#include <KullFormatStructures.h>

#include <vtkCellType.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVertex.h>

#include <avtCallback.h>
#include <avtDatabaseMetaData.h>  
#include <avtMaterial.h>

#include <BadDomainException.h>
#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidFilesException.h>
#include <InvalidDBTypeException.h>
#include <InvalidVariableException.h>

using     std::string;
using     std::vector;

static void  OrderWedgePoints(const vector< vector<int> > &, int *);

//
// Define the static const's
//
 
const char   *avtKullLiteFileFormat::MESHNAME = "mesh";

//
// Define some functions so if PDB was compiled with a weird non-g++ compiler,
// we still have symbols that we need in order to link.
//
extern "C" void ieee_handler(int)           { }
extern "C" void standard_arithmetic(int)    { }
extern "C" void nonstandard_arithmetic(int) { }

// ****************************************************************************
//  Method: avtKullLiteFileFormat constructor
//
//  Arguments:
//      fname    The file name.
//
//  Programmer:  Akira Haddox
//  Creation:    June 18, 2002
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 27 14:09:50 PST 2003
//    Some compilers don't understand std::strings in conjuction with the
//    <iostream.h> (instead of <iostream>) headers, so I worked around it.
//
//    Akira Haddox, Tue May 20 13:51:48 PDT 2003
//    Removed data variable handling, and material dataset storage.
//    Added paths to the filenames for multi-file sets.
//
//    Brad Whitlock, Thu Oct 9 15:13:13 PST 2003
//    Added code to delete memory that was leaked when the file format can't
//    understand this type of PDB file.
//
// ****************************************************************************

avtKullLiteFileFormat::avtKullLiteFileFormat(const char *fname) 
    : avtSTMDFileFormat(NULL, 0)
{
    dataset = NULL;

    m_pdbFile = NULL;
    m_kullmesh = NULL;
    m_tags = NULL;

    // Figure out if we were given a master file list,
    // and if so, build the filename list
    ifstream inf;
    inf.open(fname);
    char a,b,c;
    inf >> a >> b >> c;
    // This is how the Master Kull Files start out
    // Other pdb files are fine, since they all start with the 
    // pdb version identifier
    if (a=='M' && b=='K' && c=='F')
    {
        string a(fname);
        char b[1024];
        string prefix = "";
        int last_slash = a.find_last_of('/');
        if (last_slash != string::npos)
            prefix = a.substr(0, last_slash + 1);
        inf >> b;
        while (inf && !inf.eof())
        {
            a = prefix + b;
            AddFile(a.c_str());
            inf >> b;
        }
    }
    else // We're opening a single file, it's not an index
    {
        AddFile(fname);
    }
    inf.close();

    dataset = new vtkDataSet*[nFiles];
    for (int i = 0; i < nFiles; i++)
        dataset[i] = NULL;

    TRY
    {
        // We might read them in from the master file eventually, but for
        // now, we'll do it here in the constructor [so it's a straight
        // modification if we do put them in the master file]
        ReadInMaterialNames();
    }
    CATCH(VisItException)
    {
        for(int j = 0; j < nFiles; ++j)
        {
            if(dataset[j] != NULL)
                dataset[j]->Delete();
        }
        delete [] dataset;
        dataset = NULL;
        RETHROW;
    }
    ENDTRY
}


// ****************************************************************************
//  Method: avtKullLiteFileFormat destructor
//
//  Programmer: Akira Haddox
//  Creation:   June 18, 2002
//
//  Modifications:
//    Akira Haddox, Tue May 20 13:51:48 PDT 2003
//    Removed data variable handling, and material dataset storage.
//
// ****************************************************************************

avtKullLiteFileFormat::~avtKullLiteFileFormat()
{
    if (dataset != NULL)
    {
        for (int i = 0; i < nFiles; i++)
            if (dataset[i])
                dataset[i]->Delete();
        delete[] dataset;
        dataset = NULL;
    }
    Close();
}

// ****************************************************************************
//  Method: avtKullLiteFileFormat::ReadInFile
//
//  Purpose:
//      Reads in the dataset from a file.
//
//  Arguments:
//    fi    The file index.
//
//  Programmer: Akira Haddox
//  Creation:  June 18, 2002
//
//  Modifications:
//
//    Hank Childs, Mon Oct 14 09:54:47 PDT 2002
//    Throw different exceptions in case of failure so that the database
//    manager can sort out overloaded exception names.
//
//    Hank Childs, Sat Mar 15 11:32:34 PST 2003 
//    The wedge was being constructed without taking care to match up the
//    two triangular faces (meaning the wedge was mangled).  I created a new
//    routine to ensure the correct order, and called that routine from here.
//
//    Akira Haddox, Tue May 20 13:51:48 PDT 2003
//    Removed data variable handling, and material dataset storage.
//    Fixed calculation of nRecvZones.
//
// ****************************************************************************

void
avtKullLiteFileFormat::ReadInFile(int fi)
{
    debug4 << "Reading in dataset from KullLite file " << filenames[fi]<< endl;

    m_pdbFile = PD_open(filenames[fi], "r");
    if (m_pdbFile == NULL)
    {
        Close();
        EXCEPTION1(InvalidDBTypeException, "Cannot be a KullLite file, this "
                   " is file not a valid PDB file.");
    }

    // Read in the Mesh
    m_kullmesh = MAKE_N(pdb_mesh, 1);
    if (PD_read(m_pdbFile, "mesh", m_kullmesh) == false)
    {
        Close();
        EXCEPTION1(InvalidDBTypeException, "Cannot be a KullLite file, does "
                   " not have a mesh.");
    }

    // Read in the tags
    m_tags = MAKE_N(pdb_taglist, 1);
    if (PD_read(m_pdbFile, "MeshTags", m_tags) == false)
    {
        // This is a valid possibility, we have to deal with it
        SFREE(m_tags);
        m_tags = NULL;
    }

    // Calculate how many zones are not used by this file.
    int nRecvZones = ReadNumberRecvZones();

    int nMaterials = 0;
    PD_read(m_pdbFile, "num_materials", &nMaterials); //This can fail,
                                                      // that's okay

    // This however, isn't okay. If there are materials, 
    // we need tags to go with them.
    if (nMaterials && !m_tags)
    {
        Close();
        EXCEPTION1(InvalidDBTypeException, "Invalid KullLite file, doesn't "
                   "have meshtags, but has materials.");
    }

    //
    // This reader only supports 3 dimensional meshes right now.
    // Fortunately, KullLite files only use 3 dimensions right now as well.
    // However, if that changes, new support will need to be coded in.
    //

    if (m_kullmesh->ndimensions != 3)
    {
        Close();
        EXCEPTION0(ImproperUseException);
    }

    // Read the points into a vtkPoints structure
    vtkPoints *points = vtkPoints::New();

    int nPoints = m_kullmesh->npnts;
    points->SetNumberOfPoints(nPoints);

    int i;
    for ( i = 0; i < nPoints; i++)
    {
        points->SetPoint(i,m_kullmesh->positions[i].x,
               m_kullmesh->positions[i].y,
                      m_kullmesh->positions[i].z);
    }

    if (dataset[fi]) // Not likely, but just in case
        dataset[fi]->Delete();

    dataset[fi] = vtkUnstructuredGrid::New();

    // I know it's an UnstructruedGrid, I made it.
    // Keep a pointer for convinience.

    vtkUnstructuredGrid *datasetPtr = (vtkUnstructuredGrid *)dataset[fi];

    datasetPtr->SetPoints(points);
    points->Delete();

    int failedReadElements = 0;

    // Let's read the cells and put them into the dataset
    // But only read the cells that belong to this file
    // The last nRecvZones are owned by someone else

    int nCells = m_kullmesh->nzones - nRecvZones;
    datasetPtr->Allocate(nCells);
    for (int zone = 0; zone < nCells; zone++)
    {
        int startZoneToFaceIndex = m_kullmesh->zoneToFacesIndex[zone];
        int endZoneToFaceIndex = m_kullmesh->zoneToFacesIndex[zone+1];

        // Find the faces for the zone
        int numFaces = endZoneToFaceIndex-startZoneToFaceIndex;

        vector<vector<int> > nodes(numFaces);
        for (int faceI = startZoneToFaceIndex; faceI < endZoneToFaceIndex; 
             faceI++)
        {
            int faceIndex = m_kullmesh->faceIndices[faceI];
            bool sharedSlave = false;
            if (faceIndex < 0) //shared face
            {
                sharedSlave = true;
                faceIndex = -1 - faceIndex; //one's complement to get index
            }

            // Find the nodes
            int startNodeIndex = m_kullmesh->faceToNodesIndex[faceIndex];
            int endNodeIndex = m_kullmesh->faceToNodesIndex[faceIndex+1];
            int nodesForThisFace = endNodeIndex-startNodeIndex;

            nodes[faceI-startZoneToFaceIndex].resize(nodesForThisFace);

            // Let's grab the nodeId's and store them
            if (sharedSlave) // Shared face, grab them in reverse order
            {
                for (i = nodesForThisFace-1; i >= 0; i--)
                {
                    int ix2 = nodesForThisFace - 1 - i;
                    nodes[faceI - startZoneToFaceIndex][ix2] =
                        m_kullmesh->nodeIndices[i + startNodeIndex];
                }
            }
            else // Normal face, just grab them
            {
                for (i = 0; i < nodesForThisFace; i++)
                {
                    nodes[faceI - startZoneToFaceIndex][i] =
                        m_kullmesh->nodeIndices[i + startNodeIndex];
                }
            }
        }

        // Now then, we have the nodes, let's classify
        int type;
        if (numFaces == 4)
            type = VTK_TETRA;
        else if (numFaces == 6)
            type = VTK_HEXAHEDRON;
        else if (numFaces == 5)
        {
            // Cell could be Pyramid or Wedge. 
            // We assume Pyramid, then look to see if
            // there's more than one face with 4 nodes.
            // If there is, we change our mind: it's a wedge.
            bool first = false;
            type = VTK_PYRAMID;
            for (i = 0; i < numFaces; i++)
            {
                if (nodes[i].size() == 4)
                {
                    if (first)
                    {   // At least two faces have 4 nodes, it's a wedge.
                        type = VTK_WEDGE;
                        break;
                    }
                    first = true;
                }
            }
        }
        else // We don't know what it is, make a note of it
        {
            failedReadElements++;
            continue;
        }


        int cellId;
        // Okay we know what it is.
        // Let's get the points in the right order and build.

        if (type == VTK_HEXAHEDRON)
        {
            int points[8];
        
            // Take one face for the first 8 nodes
            points[0] = nodes[0][0];
            points[1] = nodes[0][1];
            points[2] = nodes[0][2];
            points[3] = nodes[0][3];

            // Look for the opposite side
            for (i = 1; i < nodes.size(); i++)
            {
                int j;
                for (j = 0; j < 4; j++)
                {
                    // Unraveled loop, to avoid having a flag to know when 
                    // we've found the face.
                    if (nodes[i][0] == points[j])
                        break;
                    if (nodes[i][1] == points[j])
                        break;
                    if (nodes[i][2] == points[j])
                        break;
                    if (nodes[i][3] == points[j])
                        break;
                }
                if (j == 4)    break; //we're done
            }
        
            // The other face is face i
            // But we reverse the order so that both faces
            // are defined in same order

            points[4] = nodes[i][3];
            points[5] = nodes[i][2];
            points[6] = nodes[i][1];
            points[7] = nodes[i][0];

            // We have the two faces, but they aren't necessarily both aligned

            // This theory has been tested: There is a strong tendency for
            // hexahedrons not to have these two faces aligned. So,
            // unfortunately, this code is neccessary.

            // This leads to a necessary finding of the offset
            
            // Find a difference face [other than face 0] that has points[0]
            int diffFace;
            int whichIndex;
            for( diffFace = 1; diffFace < nodes.size(); diffFace++)
            {
                whichIndex = -1;
                if (nodes[diffFace][++whichIndex] == points[0])
                    break;
                if (nodes[diffFace][++whichIndex] == points[0])
                    break;
                if (nodes[diffFace][++whichIndex] == points[0])
                    break;
                if (nodes[diffFace][++whichIndex] == points[0])
                    break;
            }

            // This different face has points[0], (points[1] or points[3]),
            // and two other points. We want to find the point next to points[0]
            // (which is in nodes[diffFace][whichIndex]) that isn't
            // points[1] or points[3], and find the index of it in points[].

            int otherIndex;
            int ii = nodes[diffFace][(whichIndex + 1) % 4];
    
            // Start looking infront
            if ((ii == points[1] || ii == points[3]))
            { 
                // The point is behind, re-adjust
                ii = nodes[diffFace][(whichIndex - 1 + 4) % 4];
            }

            for (otherIndex = 4; otherIndex < 8; otherIndex++)
                if (points[otherIndex] == ii)
                    break;
            
            // OtherIndex should be 4. If it's not, we have to offset
            if (otherIndex != 4)
            {
                int offSet = otherIndex - 4;

                int tmp[4];
                tmp[0] = points[4];
                tmp[1] = points[5];
                tmp[2] = points[6];
                tmp[3] = points[7];

                points[4] = tmp[(offSet + 0) % 4];
                points[5] = tmp[(offSet + 1) % 4];
                points[6] = tmp[(offSet + 2) % 4];
                points[7] = tmp[(offSet + 3) % 4];
            }
            
            // Put the cell into the dataset
            cellId = datasetPtr->InsertNextCell(type, 8, points);
        }
        else if (type == VTK_TETRA)
        {
            // Because there's only 4 points, order doesn't matter.
            int points[4];
            // Get one face
            points[0] = nodes[0][0];
            points[1] = nodes[0][1];
            points[2] = nodes[0][2];

            // Find the other point in face 1
            if (!(nodes[1][0] == points[0] || nodes[1][0] == points[1]
                  ||  nodes[1][0] == points[2]))
                points[3] = nodes[1][0];
            else if (!(nodes[1][1] == points[0] || nodes[1][1] == points[1]
                      ||  nodes[1][1] == points[2]))
                points[3] = nodes[1][1];
            else if (!(nodes[1][2] == points[0] || nodes[1][2] == points[1]
                      ||  nodes[1][2] == points[2]))
                points[3] = nodes[1][2];

            cellId = datasetPtr->InsertNextCell(type, 4, points);
        }
        else if (type == VTK_PYRAMID)
        {
            int points[5];
            // Find the base
            int i;
            for (i = 0; i < nodes.size(); i++)
                if (nodes[i].size() == 4)
                    break;
            points[0] = nodes[i][0];
            points[1] = nodes[i][1];
            points[2] = nodes[i][2];
            points[3] = nodes[i][3];

            // Find the top point
            if (i == 0)
                ++i;
            if (!(nodes[i][0] == points[0] || nodes[i][0] == points[1]
                  || nodes[i][0] == points[2] || nodes[i][0] == points[3]))
                points[4] = nodes[i][0];
            else if (!(nodes[i][1] == points[0] || nodes[i][1] == points[1]
                       || nodes[i][1] == points[2] || nodes[i][1] == points[3]))
                points[4] = nodes[i][1];
            else if (!(nodes[i][2] == points[0] || nodes[i][2] == points[1]
                       || nodes[i][2] == points[2] || nodes[i][2] == points[3]))
                points[4] = nodes[i][2];

            cellId = datasetPtr->InsertNextCell(type, 5, points);
        }
        else if (type == VTK_WEDGE)
        {
            int points[6];
            OrderWedgePoints(nodes, points);

            cellId = datasetPtr->InsertNextCell(type,6,points);
        }
        else 
        {
            debug5 << "Unrecognized type in avtKullLiteFileFormat.C: "
                   << "Previous check failed." << endl;
            continue;
        }
    }

    if (failedReadElements)
    {
        char msg[128];
        sprintf(msg, "When reading in mesh, encountered %d element[s] "
                     "whose types  weren't recognized. They were left out.",
                     failedReadElements);
        avtCallback::IssueWarning(msg);
    }

    // End reading, clean up for next read
    Close();
}


// ****************************************************************************
//  Method: avtKullLiteFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh by domain.
//
//  Arguments:
//      mesh     The desired meshname.
//      dom     The domain.
//
//  Returns:     The mesh as a KullLite dataset.
//
//  Programmer: Akira Haddox
//  Creation:   June 18, 2002
//
//  Modifications:
//    Akira Haddox, Tue May 20 13:51:48 PDT 2003
//    Removed data variable handling, and material dataset storage.
//
// ****************************************************************************

vtkDataSet *
avtKullLiteFileFormat::GetMesh(int dom, const char *mesh)
{
    debug5 << "Getting mesh from KullLite file: " << filenames[dom] << endl;

    if (dom < 0 || dom >= nFiles)
        EXCEPTION2(BadDomainException, dom, nFiles);
    if (!dataset[dom])
        ReadInFile(dom);

    if (!strcmp(mesh, MESHNAME) )
    {
        dataset[dom]->Register(NULL);
        return dataset[dom];
    }
    
    EXCEPTION1(InvalidVariableException, mesh);
    return NULL;
}


// ****************************************************************************
//  Method: avtKullLiteFileFormat::GetVar
//
//  Purpose:
//      This is only in to fill a required function for its parent.
//      On use, it throws an exception.
//
//  Arguments:
//      var    The desired varname.
//      fi     The file domain.
//
//  Programmer: Akira Haddox
//  Creation:   June 18, 2002
//
// ****************************************************************************

vtkDataArray *
avtKullLiteFileFormat::GetVar(int fi, const char *var)
{
    EXCEPTION1(InvalidVariableException, var);
    return NULL;
}


// ****************************************************************************
//  Method: avtKullLiteFileFormat::FreeUpResources
//
//  Purpose:
//      Frees up resources.  Since this module does not keep an open file, that
//      only means deleting the dataset.  Since this is all reference counted,
//      there is no worry that we will be deleting something that is being
//      used. But we will call Close(), which would close any potentially open
//      files, and deallocates other memory.
//
//  Programmer: Akira Haddox
//  Creation:   June 18, 2002
//
//  Modifications:
//    Akira Haddox, Tue May 20 13:51:48 PDT 2003
//    Removed data variable handling, and material dataset storage.
//
// ****************************************************************************

void
avtKullLiteFileFormat::FreeUpResources(void)
{
    debug4 << "KullLite files " 
           << " forced to free up resources." << endl;

    // Don't delete: dataset[], or clear the main materials structure,
    // or our metadata is messed up.
    int i;
    if (dataset != NULL)
    {
        for (i = 0 ; i < nFiles; i++)
        {
            if (dataset[i])
            {
                dataset[i]->Delete();
                dataset[i] = NULL;
            }
        }    
    }

    Close();
}


// ****************************************************************************
//  Method: avtKullLiteFileFormat::GetAuxiliaryData
//
//  Purpose:
//      Gets the auxiliary data, namely the material data.
//
//  Arguments:
//      var        The variable of interest.
//      domain     The domain of interest.
//      type       The type of auxiliary data.
//      <unnamed>  The arguments for that -- not used.
//      df         The destructor for the returned data.
//
//  Returns:    The auxiliary data.
//
//  Programmer: Akira Haddox
//  Creation:   May 20, 2003
//
// ****************************************************************************

void *
avtKullLiteFileFormat::GetAuxiliaryData(const char *var, int domain,
                                        const char *type, void *,
                                        DestructorFunction &df)
{
    // We only deal with materials. If they want something else, we
    // don't have it.
    if (strcmp(type, AUXILIARY_DATA_MATERIAL))
    {
        return NULL;
    }

    // If we have no materials, we shouldn't be queried for them.
    if (NumberOfMaterials() == 0)
        EXCEPTION1(InvalidVariableException, "No materials to query.");

    if (domain < 0 || domain >= nFiles)
        EXCEPTION2(BadDomainException, domain, nFiles);

    m_pdbFile = PD_open(filenames[domain], "r");
    if (m_pdbFile == NULL)
    {
        Close();
        EXCEPTION1(InvalidDBTypeException, "Not a valid PDB file.");
    }
    
    // Read in the Mesh
    m_kullmesh = MAKE_N(pdb_mesh, 1);
    if (PD_read(m_pdbFile, "mesh", m_kullmesh) == false)
    {
        Close();
        EXCEPTION1(InvalidDBTypeException, "Cannot be a KullLite file, does "
                   " not have a mesh.");
    }
   
    // Calculate how many zones are not used by this file.
    int nRecvZones = ReadNumberRecvZones();
    
    // Now we go through to turn the Kull material into the Silo material
    // format to fit into an avtMaterial structure.

    // These variables are needed to create the material data.
    int num_materials = NumberOfMaterials();
    vector<string> &mat_names = m_names;
    int num_zones = m_kullmesh->nzones - nRecvZones;
    vector<int> material_list(num_zones);
    vector<int> mix_mat;
    vector<int> mix_next;
    vector<int> mix_zone;
    vector<float> mix_vf;

    // This matrix we'll use to calculate the fractions for all
    // zones over all materials. To ensure we correctly identify
    // pure materials, the value -1.0 will represent 0% of a material,
    // and 2.0 will represent 100% of a material.
    vector<vector<float> > values(num_materials);
    int i, j;

    // Initially, all zones are set to 0% for all materials
    // We make storage for the received zones to make computation
    // easier, but we won't use the data when it comes to making
    // the material.
    for (i = 0; i < num_materials; ++i)
    {
        values[i].resize(m_kullmesh->nzones, -1.0f);
    }
    
    // Read in the tags
    m_tags = MAKE_N(pdb_taglist, 1);
    if (PD_read(m_pdbFile, "MeshTags", m_tags) == false)
    {
        Close();
        EXCEPTION1(InvalidVariableException, "No meshtag data.");
    }

    // Now we go through the materials, and deal with them accordingly. 
    for (i = 0; i < num_materials; ++i)
    {
        string base = m_names[i];
       
        string matName = "mat_" + base + "_zones";
     
        for (j = 0; j < m_tags->num_tags; ++j)
            if (!strcmp(m_tags->tags[j].tagname, matName.c_str()))
                break;
       
        if (j == m_tags->num_tags)
            continue;           // Material is not in this domain

        int tsize = m_tags->tags[j].size;            
        vector<int> ids(tsize);

        if (PD_read(m_pdbFile, (char*)matName.c_str(), &(ids[0])) == false)
        {
            Close();
            EXCEPTION1(InvalidFilesException, filenames[domain]);
        }

        // These id's have this material. To what degree, we're not
        // yet sure, but we'll set them all to pure (100%), and find
        // later which ones are factional.
        for (j = 0; j < ids.size(); ++j)
        {
            int zone = ids[j];
            values[i][zone] = 2.0f;
        }

        string mixedName = "mat_" + base + "_mixedZones";

        for (j = 0; j < m_tags->num_tags; ++j)        
            if (!strcmp(m_tags->tags[j].tagname, mixedName.c_str()))
                break;

        if (j == m_tags->num_tags)
            continue;           // It's an exclusive material.

        tsize = m_tags->tags[j].size;
        ids.resize(tsize);

        if (PD_read(m_pdbFile, (char*)mixedName.c_str(), &(ids[0])) == false)
        {
            Close();
            EXCEPTION1(InvalidFilesException, filenames[domain]);
        }
        
        // We now have the ids of the mixed zones.
        // This implies that there is a list of fractions of the same
        // size.
        string fractionName = "mat_" + base + "_volumeFractions";
        vector<double> fractions(tsize);

        if (PD_read(m_pdbFile, (char*)fractionName.c_str(),
                    &(fractions[0])) == false)
        {
            Close();
            EXCEPTION1(InvalidFilesException, filenames[domain]);
        }

        // Now set the appropriate values for those zones
        for (j = 0; j < ids.size(); ++j)
        {
            int zone = ids[j];
            values[i][zone] = static_cast<float>(fractions[j]);
        }
    }

    // We've collected the data we need from the files.    
    Close();

    // Now to build into appropriate data structures.
    // Go through the first num_zones (which leaves out the recvzones).
    for (i = 0; i < num_zones; ++i)
    {
        bool pure = false;
        // First look for pure materials
        for (j = 0; j < num_materials; ++j)
        {
            if (values[j][i] > 1.5f)
            {
                pure = true;
                // For pure materials, the material_list value is that material.
                // All other tables are left alone.
                material_list[i] = j;
                break;                
            }
        } 
        if (pure)
            continue;
        
        // For unpure materials, we need to add entries to the tables.  
        material_list[i] = -1 * (1 + mix_zone.size());
        for (j = 0; j < num_materials; ++j)
        {
            if (values[j][i] < 0)
                continue;
            // For each material, we add an entry to each table.
            mix_zone.push_back(i);
            mix_mat.push_back(j);
            mix_vf.push_back(values[j][i]);
            mix_next.push_back(mix_zone.size() + 1);
        }
        // When we're done, the last entry we put in is a 0 in the mix_next
        mix_next[mix_next.size() - 1] = 0;
    }

    int mixed_size = mix_zone.size();
    avtMaterial * mat = new avtMaterial(num_materials, mat_names, num_zones,
                                        &(material_list[0]), mixed_size,
                                        &(mix_mat[0]), &(mix_next[0]),
                                        &(mix_zone[0]), &(mix_vf[0]));

    df = avtMaterial::Destruct;
    return (void*) mat;    
}


// ****************************************************************************
//  Method: avtKullLiteFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      Sets the database meta data.
//
//  Programmer: Akira Haddox
//  Creation:   June 18, 2002
//
//  Modifications:
//      Akira Haddox, Tue May 20 13:48:04 PDT 2003
//      Removed code that tried to deal with data variables.
// ****************************************************************************

void
avtKullLiteFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    vector<string> vFilenames(nFiles);
    int i;
    for (i = 0; i < nFiles; i++)
        vFilenames[i] = string(filenames[i]);

    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = MESHNAME;
    mesh->meshType = AVT_UNSTRUCTURED_MESH;
    mesh->numBlocks = nFiles;
    mesh->blockOrigin = 0;
    mesh->spatialDimension = 3;
    mesh->topologicalDimension = 3;
    mesh->blockTitle = "Files";
    mesh->blockNames = vFilenames;
    mesh->hasSpatialExtents = false;
    md->Add(mesh);

    if (NumberOfMaterials())
    {
        AddMaterialToMetaData(md,"Material",MESHNAME, NumberOfMaterials(),
                              m_names);
    }
}


// ****************************************************************************
//  Method: avtKullLiteFileFormat::Close
//
//  Purpose:
//      Called to clean up pdb allocated memory.
//
//  Programmer: Akira Haddox
//  Creation:   June 18, 2002
//
// ****************************************************************************


void
avtKullLiteFileFormat::Close()
{
    if (m_pdbFile)
    {
        PD_close(m_pdbFile);
        m_pdbFile = NULL;
    }
    if (m_kullmesh)
    {
        SFREE(m_kullmesh);
        m_kullmesh = NULL;
    }
    if (m_tags)
    {
        SFREE(m_tags);
        m_tags = NULL;
    }
}

// ****************************************************************************
//  Method: avtKullLiteFileFormat::ReadInMaterialNames
//
//  Purpose:
//      Reads in the material headings from all files.
//
//  Programmer: Akira Haddox
//  Creation:  July 1, 2002
//
//  Modifications:
//    Akira Haddox, Tue May 20 13:51:48 PDT 2003
//    Removed data variable handling.
//
// ****************************************************************************

void avtKullLiteFileFormat::ReadInMaterialNames()
{
    m_names.clear();
    for (int i = 0; i < nFiles; i++)
        ReadInMaterialName(i);
}


// ****************************************************************************
//  Method: avtKullLiteFileFormat::ReadInMaterialName
//
//  Purpose:
//      Reads in the material headings from a specific file.
//
//  Argument:
//    fi    The file index to read from.
//
//  Programmer: Akira Haddox
//  Creation:  July 1, 2002
//
//  Modifications:
//
//    Hank Childs, Mon Oct 14 18:00:30 PDT 2002
//    Throw different exceptions in case of failure so that the database
//    manager can sort out overloaded exception names.
//
//    Hank Childs, Thu Oct 31 07:22:34 PST 2002
//    Correct the topological dimensions for mesh tags.
//
//    Akira Haddox, Tue May 20 09:01:01 PDT 2003
//    Changed the method used to find the material names.
//
// ****************************************************************************

void avtKullLiteFileFormat::ReadInMaterialName(int fi)
{
    debug4 << "Reading in material header from KullLite file "
           << filenames[fi] << endl;    

    m_pdbFile = PD_open(filenames[fi], "r");
    if (m_pdbFile == NULL)
    {
        Close();
        EXCEPTION1(InvalidDBTypeException, "Cannot be a KullLite file, cannot "
                   " be opened as a PDB file.");
    }

    //
    // Just make sure that this is a KullLite file by making sure that it has
    // "typeofmesh" defined.
    //
    char buff[1024];
    if (PD_read(m_pdbFile, "typeofmesh", buff) == false)
    {
        Close();
        EXCEPTION1(InvalidDBTypeException, "Cannot be a KullLite file, "
                   "does not have characteristic variables.");
    }

    int nMaterials = 0;
    PD_read(m_pdbFile, "num_materials", &nMaterials);

    if (nMaterials == 0)
        return;

    // There are materials, read in the tags
    m_tags = MAKE_N(pdb_taglist, 1);
    if ((PD_read(m_pdbFile, "MeshTags", m_tags) == false) || !m_tags)
    {
        Close();
        EXCEPTION1(InvalidDBTypeException, "Cannot be a KullLite file, "
                    "does not have materials or mesh tags.");
    }

    //
    // Go through each tag in the file.
    //
    int curTagI;
    for (curTagI = 0; curTagI < m_tags->num_tags; curTagI++)
    {
        string originalName = m_tags->tags[curTagI].tagname;

        if (!IsMaterialName(originalName))      
            continue;

        // Material tag
        string name = GetMaterialName(originalName);

        int matNumber;
        for (matNumber = 0; matNumber < m_names.size(); matNumber++)
        {
            if (name == m_names[matNumber])
                break;
        }
        if (matNumber == m_names.size()) // New material
        {
            m_names.push_back(name);
        }
    }

    // We're done, clean up.
    Close();
}

// ****************************************************************************
//  Method: avtKullLiteFileFormat::ReadNumberRecvZones
//
//  Purpose:
//      Calculate the number of received zones in the currently open file.
//
//  Notes:
//      m_pdbFile must be open and valid. 
//
//  Programmer: Akira Haddox
//  Creation:  July 1, 2002
//
// ****************************************************************************


inline int
avtKullLiteFileFormat::ReadNumberRecvZones()
{
    // Read in RecvZonesSize: the number of lists of received zones.
    int nRecvZonesLists = 0;
    PD_read(m_pdbFile, "RecvZonesSize", &nRecvZonesLists); //This can fail,
                                                           // that's okay

    // Start our counter at 0 : also takes care of if there are no RecvZones.
    int nRecvZones = 0;
    
    if (nRecvZonesLists)
    {
        pdb_comm * m_recvZones = MAKE_N(pdb_comm, nRecvZonesLists);
        if (PD_read(m_pdbFile, "RecvZones", &m_recvZones) == false)
        {
            SFREE(m_recvZones);
            Close();
            EXCEPTION1(InvalidDBTypeException, "Missing RecvZones data.");
        }
        int listIndex;
        for(listIndex = 0; listIndex < nRecvZonesLists; ++listIndex)
            nRecvZones += m_recvZones[listIndex].listSize;
        SFREE(m_recvZones);
    }

    return nRecvZones;
}


// ****************************************************************************
//  Function: OrderWedgePoints
//
//  Purpose:
//      Takes a description of a wedge (4 quads, 3 tris) and comes back with
//      an ordering of those points that is suitable for VTK.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2003
//
// ****************************************************************************

static void
OrderWedgePoints(const vector< vector<int> > &nodes, int *points)
{
    const int nWedgeFaces = 5;

    int  i;

    //
    // First determine that we really have a quad.  Also determine the location
    // of the triangles and the quadrilaterals of the wedge.
    //
    if (nodes.size() != nWedgeFaces)
    {
        // A wedge has 5 faces.  This has been mis-identified.
        EXCEPTION0(ImproperUseException);
    }
    int nquads = 0;
    int ntris  = 0;
    int baseline_quad = -1;
    int tri1 = -1;
    int tri2 = -1;
    for (i = 0 ; i < nWedgeFaces ; i++)
    {
        int npts = nodes[i].size();
        if (npts == 4)
        {
            baseline_quad = (baseline_quad == -1 ? i : baseline_quad);
            nquads++;
        }
        else if (npts == 3)
        {
            if (ntris == 0)
                tri1 = i;
            else if (ntris == 1)
                tri2 = i;
            ntris++;
        }
        else
        {
            EXCEPTION0(ImproperUseException);
        }
    }
    if (ntris != 2 || nquads != 3)
    {
        EXCEPTION0(ImproperUseException);
    }

    //
    // Here is what I am going for:
    //
    //                0-----------1
    //               /|           |\
    //              4 + - - - - - + 5
    //               \|           |/
    //                3-----------2
    //
    //
    int Pt0 = nodes[baseline_quad][0];
    int Pt1 = nodes[baseline_quad][1];
    int Pt2 = nodes[baseline_quad][2];
    int Pt3 = nodes[baseline_quad][3];
    int Pt4, Pt5;

    //
    // We would like to be able to assume that Pt0 is incident to tri1.  If
    // this is not the case, just switch them.
    //
    if (nodes[tri1][0] != Pt0 && nodes[tri1][1] != Pt0 &&nodes[tri1][2] != Pt0)
    {
        int tmp = tri1;  tri1 = tri2; tri2 = tmp;
    }

    //
    // We know that Pt0 is incident to the first triangle.  Now determine
    // which of its neighboring pts (Pt1 or Pt3) is also incident to that
    // triangle.  We would like it to be Pt3.  If not, then re-order them.
    //
    if (nodes[tri1][0] == Pt1 || nodes[tri1][1] == Pt1 ||nodes[tri1][2] == Pt1)
    {
        int tmp = Pt0;  Pt0 = Pt1; Pt1 = Pt2; Pt2 = Pt3; Pt3 = tmp;
    }

    //
    // Pretty easy from here.  Pt4 is the one on tri1 that is not Pt0 and is
    // not Pt3.
    //
    if (nodes[tri1][0] != Pt0 && nodes[tri1][0] != Pt3)
        Pt4 = nodes[tri1][0];
    if (nodes[tri1][1] != Pt0 && nodes[tri1][1] != Pt3)
        Pt4 = nodes[tri1][1];
    if (nodes[tri1][2] != Pt0 && nodes[tri1][2] != Pt3)
        Pt4 = nodes[tri1][2];

    //
    // Pt5 is the one on tri2 that is not Pt1 and is not Pt2.
    //
    if (nodes[tri2][0] != Pt1 && nodes[tri2][0] != Pt2)
        Pt5 = nodes[tri2][0];
    if (nodes[tri2][1] != Pt1 && nodes[tri2][1] != Pt2)
        Pt5 = nodes[tri2][1];
    if (nodes[tri2][2] != Pt1 && nodes[tri2][2] != Pt2)
        Pt5 = nodes[tri2][2];

    //
    // The 'correct' VTK ordering for the (intuitive) orientation above is 
    // 2-5-1-3-4-0.  More details are located in the Silo reader under
    // TranslateSiloWedgeToVTKWedge.
    //
    points[0] = Pt2;
    points[1] = Pt5;
    points[2] = Pt1;
    points[3] = Pt3;
    points[4] = Pt4;
    points[5] = Pt0;
}


