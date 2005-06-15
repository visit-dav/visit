// ************************************************************************* //
//                        avtKullLiteFileFormat.C                            //
// ************************************************************************* //

#include <avtKullLiteFileFormat.h>

#include <algorithm>
#include <string>
#include <vector>
#include <visitstream.h>

#include <visit-config.h>

#ifdef HAVE_PDB_PROPER
#    include <pdb.h>
#else
#    include <lite_pdb.h>
#endif

#include <KullFormatStructures.h>

#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkUnsignedCharArray.h>
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
using     std::sort;

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
extern "C" void __mth_i_dpowd(void)         { }
extern "C" void __arg_reduce_(void)         { }
extern "C" void __pgdbg_stub(void)          { }

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
//    Hank Childs, Fri Oct 17 21:43:45 PDT 2003
//    Stop using constructs that come from AddFile (filenames, nFiles), since
//    they are primarily for handling file descriptors.  Used data members
//    specific to this class (my_filenames, my_filenames.size()) instead.
//
//    Hank Childs, Mon Jul 26 09:36:48 PDT 2004
//    Sort the mesh tags after we read them.
//
// ****************************************************************************

avtKullLiteFileFormat::avtKullLiteFileFormat(const char *fname) 
    : avtSTMDFileFormat(NULL, 0)
{
    dataset = NULL;

    m_pdbFile = NULL;
    m_kullmesh3d = NULL;
    m_kullmesh2d = NULL;
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
            my_filenames.push_back(a);
            inf >> b;
        }
    }
    else // We're opening a single file, it's not an index
    {
        my_filenames.push_back(fname);
    }

    // The generic database is much happier if we register at least one file 
    // as our filename.
    AddFile(fname);

    inf.close();

    dataset = new vtkUnstructuredGrid*[my_filenames.size()];
    for (int i = 0; i < my_filenames.size(); i++)
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
        for(int j = 0; j < my_filenames.size(); ++j)
        {
            if(dataset[j] != NULL)
                dataset[j]->Delete();
        }
        delete [] dataset;
        dataset = NULL;
        RETHROW;
    }
    ENDTRY

    sort(m_names.begin(), m_names.end());
    sort(zone_tags.begin(), zone_tags.end());
    sort(face_tags.begin(), face_tags.end());
    sort(edge_tags.begin(), edge_tags.end());
    sort(node_tags.begin(), node_tags.end());
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
//    Hank Childs, Fri Oct 17 21:43:45 PDT 2003
//    Stop using constructs that come from AddFile (filenames, nFiles), since
//    they are primarily for handling file descriptors.  Used data members
//    specific to this class (my_filenames, my_filenames.size()) instead.
//
// ****************************************************************************

avtKullLiteFileFormat::~avtKullLiteFileFormat()
{
    if (dataset != NULL)
    {
        for (int i = 0; i < my_filenames.size(); i++)
            if (dataset[i])
                dataset[i]->Delete();
        delete[] dataset;
        dataset = NULL;
    }
    Close();
}

// ****************************************************************************
//  Method: avtKullLiteFileFormat::ReadInPrimaryMesh
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
//    Hank Childs, Fri Oct 17 21:43:45 PDT 2003
//    Stop using constructs that come from AddFile (filenames, nFiles), since
//    they are primarily for handling file descriptors.  Used data members
//    specific to this class (my_filenames, my_filenames.size()) instead.
//
//    Hank Childs, Fri Jul 23 08:32:36 PDT 2004
//    Read in 2D files as well.
//
//    Hank Childs, Tue Jul 27 16:27:31 PDT 2004
//    Add support for ghost zones.  Also split off zone classification into
//    subroutines and renamed to ReadInPrimaryMesh (from ReadInFile), since
//    another routine now reads in "mesh tag" meshes.
//
//    Hank Childs, Fri Aug 27 16:54:45 PDT 2004
//    Rename ghost data array.
//
//    Hank Childs, Tue Jun 14 16:31:33 PDT 2005
//    Add support for RZ meshes.
//
// ****************************************************************************

void
avtKullLiteFileFormat::ReadInPrimaryMesh(int fi)
{
    int   i;

    debug4 << "Reading in dataset from KullLite file " << my_filenames[fi].c_str()
           << endl;

    m_pdbFile = PD_open((char *) my_filenames[fi].c_str(), "r");
    if (m_pdbFile == NULL)
    {
        Close();
        EXCEPTION1(InvalidDBTypeException, "Cannot be a KullLite file, this "
                   " is file not a valid PDB file.");
    }

    bool meshIs3d = ReadMeshFromFile();

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

    // Read the points into a vtkPoints structure
    vtkPoints *points = vtkPoints::New();

    int nPoints = (meshIs3d ? m_kullmesh3d->npnts : m_kullmesh2d->npnts);
    points->SetNumberOfPoints(nPoints);

    bool isRZ = IsRZ();
    for (i = 0; i < nPoints; i++)
    {
        if (meshIs3d)
        {
            points->SetPoint(i,m_kullmesh3d->positions[i].x,
                               m_kullmesh3d->positions[i].y,
                               m_kullmesh3d->positions[i].z);
        }
        else
        {
            if (isRZ)
                points->SetPoint(i,m_kullmesh2d->positions[i].y,
                                   m_kullmesh2d->positions[i].x,
                                   0.);
            else
                points->SetPoint(i,m_kullmesh2d->positions[i].x,
                                   m_kullmesh2d->positions[i].y,
                                   0.);
        }
    }

    if (dataset[fi] != NULL) // Not likely, but just in case
        dataset[fi]->Delete();

    //
    // Create a new grid and store it away with our internal bookkeeping.
    //
    vtkUnstructuredGrid *datasetPtr = vtkUnstructuredGrid::New();
    dataset[fi] = datasetPtr;

    datasetPtr->SetPoints(points);
    points->Delete();

    int failedReadElements = 0;

    // Let's read the cells and put them into the dataset
    // But only read the cells that belong to this file
    // The last nRecvZones are owned by someone else
    int mesh_nzones = (meshIs3d ? m_kullmesh3d->nzones : m_kullmesh2d->nzones);
    int nCells = mesh_nzones - nRecvZones;
    datasetPtr->Allocate(mesh_nzones);
    for (int zone = 0; zone < mesh_nzones; zone++)
    {
        if (meshIs3d)
        {
            if (!ClassifyAndAdd3DZone(m_kullmesh3d, zone, datasetPtr))
               failedReadElements++;
        }
        else
        {
            if (!ClassifyAndAdd2DZone(m_kullmesh2d, zone, datasetPtr))
               failedReadElements++;
        }
    }

    vtkUnsignedCharArray *ghosts = vtkUnsignedCharArray::New();
    ghosts->SetName("avtGhostZones");
    ghosts->SetNumberOfTuples(mesh_nzones);
    for (i = 0 ; i < nCells ; i++)
        ghosts->SetValue(i, 0);
    unsigned char v = 0;
    avtGhostData::AddGhostZoneType(v, DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
    for (i = nCells ; i < mesh_nzones ; i++)
    {
        ghosts->SetValue(i, v);
    }
    datasetPtr->GetCellData()->AddArray(ghosts);
    ghosts->Delete();
    

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
//  Method: avtKullLiteFileFormat::ClassifyAndAdd3DZone
//
//  Purpose:
//      Classifies a 3D zone and adds it to an unstructured mesh.
//
//  Note:       This code was completely taken from "ReadInFile", which was
//              written by Akira Haddox.
//
//  Programmer: Hank Childs
//  Creation:   July 28, 2004
//
// ****************************************************************************

bool
avtKullLiteFileFormat::ClassifyAndAdd3DZone(pdb_mesh3d *mesh3d, int zone,
                                            vtkUnstructuredGrid *ugrid)
{
    int   i;

    int startZoneToFaceIndex = mesh3d->zoneToFacesIndex[zone];
    int endZoneToFaceIndex = mesh3d->zoneToFacesIndex[zone+1];

    // Find the faces for the zone
    int numFaces = endZoneToFaceIndex-startZoneToFaceIndex;

    vector<vector<int> > nodes(numFaces);
    for (int faceI = startZoneToFaceIndex; faceI < endZoneToFaceIndex; 
         faceI++)
    {
        int faceIndex = mesh3d->faceIndices[faceI];
        bool sharedSlave = false;
        if (faceIndex < 0) //shared face
        {
            sharedSlave = true;
            faceIndex = -1 - faceIndex; //one's complement to get index
        }

        // Find the nodes
        int startNodeIndex = mesh3d->faceToNodesIndex[faceIndex];
        int endNodeIndex = mesh3d->faceToNodesIndex[faceIndex+1];
        int nodesForThisFace = endNodeIndex-startNodeIndex;

        nodes[faceI-startZoneToFaceIndex].resize(nodesForThisFace);

        // Let's grab the nodeId's and store them
        if (sharedSlave) // Shared face, grab them in reverse order
        {
            for (i = nodesForThisFace-1; i >= 0; i--)
            {
                int ix2 = nodesForThisFace - 1 - i;
                nodes[faceI - startZoneToFaceIndex][ix2] =
                    mesh3d->nodeIndices[i + startNodeIndex];
            }
        }
        else // Normal face, just grab them
        {
            for (i = 0; i < nodesForThisFace; i++)
            {
                nodes[faceI - startZoneToFaceIndex][i] =
                    mesh3d->nodeIndices[i + startNodeIndex];
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
        return false;
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

        // We have the two faces, but they aren't necessarily both 
        // aligned

        // This theory has been tested: There is a strong tendency for
        // hexahedrons not to have these two faces aligned. So,
        // unfortunately, this code is neccessary.

        // This leads to a necessary finding of the offset
        
        // Find different face [other than face 0] that has points[0]
        int diffFace;
        int whichIndex = -1;
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
        // and two other points. We want to find the point next to
        // points[0] (which is in nodes[diffFace][whichIndex]) that
        // isn't points[1] or points[3], and find the index of it in
        // points[].

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
        cellId = ugrid->InsertNextCell(type, 8, points);
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

        cellId = ugrid->InsertNextCell(type, 4, points);
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

        cellId = ugrid->InsertNextCell(type, 5, points);
    }
    else if (type == VTK_WEDGE)
    {
        int points[6];
        OrderWedgePoints(nodes, points);

        cellId = ugrid->InsertNextCell(type,6,points);
    }
    else 
    {
        debug5 << "Unrecognized type in avtKullLiteFileFormat.C: "
               << "Previous check failed." << endl;
        return false;
    }

    return true;
}


// ****************************************************************************
//  Method: avtKullLiteFileFormat::ClassifyAndAdd2DZone
//
//  Purpose:
//      Classifies a 2D zone and adds it to an unstructured mesh.
//
//  Programmer: Hank Childs
//  Creation:   July 28, 2004
//
// ****************************************************************************

bool
avtKullLiteFileFormat::ClassifyAndAdd2DZone(pdb_mesh2d *mesh2d, int zone,
                                            vtkUnstructuredGrid *ugrid)
{
    int firstEdge = mesh2d->zoneToEdgesIndex[zone];
    int lastEdge = mesh2d->zoneToEdgesIndex[zone+1];

    // Find the faces for the zone
    int numEdges = lastEdge-firstEdge;

    vtkIdType points[4];
    if (numEdges == 3 || numEdges == 4)
    {
        //
        // As we walk through the edges, we want to be getting the
        // "newly discovered" point for each edge.  For example, suppose
        // Ei is A,B and Ei+1 is B,C.  When we process Ei+1, we want
        // the point we get to be "C".  But that means that we had
        // to of gotten "B" when we did Ei.
        //
        // Along these lines, the first point we get has to be the
        // one that is shared with the second edge.
        //
        int edge0 = mesh2d->edgeIndices[firstEdge];
        if (edge0 < 0)
            edge0 = -edge0 - 1;
        int edge1 = mesh2d->edgeIndices[firstEdge+1];
        if (edge1 < 0)
            edge1 = -edge1 - 1;
        int pt0_0 = mesh2d->nodeIndices[2*edge0];
        int pt0_1 = mesh2d->nodeIndices[2*edge0+1];
        int pt1_0 = mesh2d->nodeIndices[2*edge1];
        int pt1_1 = mesh2d->nodeIndices[2*edge1+1];
        if (pt0_0 == pt1_0 || pt0_1 == pt1_0)
            points[0] = pt1_0;
        else if (pt0_0 == pt1_1 || pt0_1 == pt1_1)
            points[0] = pt1_1;
        else
        {
            debug1 << "Unable to find a common point between two consecutive"
                   << " edges.  Do not know how to proceed." << endl;
            return false;
        }

        for (int i = 1 ; i < numEdges ; i++)
        {
            int edge = mesh2d->edgeIndices[firstEdge+i];

            // Edges not owned by this domain stored as 1's complement
            if (edge < 0)
               edge = -edge-1; 

            int last_pt = points[i-1];
            int pt0 = mesh2d->nodeIndices[2*edge];
            int pt1 = mesh2d->nodeIndices[2*edge+1];
            if (pt0 == last_pt)
                points[i] = pt1;
            else
                points[i] = pt0;
        }
    }

    if (numEdges == 3)
    {
        ugrid->InsertNextCell(VTK_TRIANGLE, 3, points);
    }
    else if (numEdges == 4)
    {
        ugrid->InsertNextCell(VTK_QUAD, 4, points);
    }
    else
    {
        return false;
    }

    return true;
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
//    Hank Childs, Fri Oct 17 21:43:45 PDT 2003
//    Stop using constructs that come from AddFile (filenames, nFiles), since
//    they are primarily for handling file descriptors.  Used data members
//    specific to this class (my_filenames, my_filenames.size()) instead.
//
//    Hank Childs, Mon Jul 26 11:32:52 PDT 2004
//    Add support for mesh tags.
//
// ****************************************************************************

vtkDataSet *
avtKullLiteFileFormat::GetMesh(int dom, const char *mesh)
{
    //
    // Do some error checking.
    //
    debug5 << "Getting mesh from KullLite file: " << my_filenames[dom].c_str() << endl;
    if (dom < 0 || dom >= my_filenames.size())
        EXCEPTION2(BadDomainException, dom, my_filenames.size());

    //
    // Whatever we do, we will need the primary mesh (even if we construct
    // mesh tags later).
    //
    if (!dataset[dom])
        ReadInPrimaryMesh(dom);

    //
    // Sanity check.
    //
    if (dataset[dom] == NULL) 
        return NULL;

    //
    // If we wanted to get the primary mesh, then we are done.
    //
    if (strcmp(mesh, MESHNAME) == 0)
    {
        //
        // We have to increment the reference count, since the calling function
        // will believe that it owns this memory.
        //
        dataset[dom]->Register(NULL);
        return dataset[dom];
    }

    // 
    // They did not ask for the original mesh, so we should only be looking for
    // mesh tags at this point. If not, we'll catch the problem when trying
    // to create the mesh tags.
    //
    vtkDataSet *rv = CreateMeshTags(mesh, dom);

    //
    // The calling function will own the memory to this object.  Just return
    // it without freeing it up.
    //
    return rv;
}


// ****************************************************************************
//  Method: avtKullLiteFileFormat::CreateMeshTags
//
//  Purpose:
//      Creates mesh tags based on the type of mesh tag requested.
//
//  Programmer: Hank Childs
//  Creation:   July 28, 2004
//
// ****************************************************************************

vtkDataSet *
avtKullLiteFileFormat::CreateMeshTags(const char *mesh, int dom)
{
    //
    // Open the file.  We will need this to create the tags.
    //
    m_pdbFile = PD_open((char *) my_filenames[dom].c_str(), "r");
    if (m_pdbFile == NULL)
    {
        Close();
        EXCEPTION1(InvalidDBTypeException, "Cannot be a KullLite file, this "
                   " is file not a valid PDB file.");
    }

    // 
    // Construct the tags.
    //
    pdb_taglist *tags = MAKE_N(pdb_taglist, 1);
    if (PD_read(m_pdbFile, "MeshTags", tags) == false)
    {
        EXCEPTION1(InvalidVariableException, mesh);
        return NULL;
    }

    //
    // Create the unstructured grid for the objects to be read into.
    //
    vtkUnstructuredGrid *rv = vtkUnstructuredGrid::New();
    rv->SetPoints(dataset[dom]->GetPoints());

    //
    // Determine which mesh type we want and call the appropriate subroutine.
    //
    if (strcmp(mesh, "mesh_tags/nodes_mesh") == 0)
    {
        CreateNodeMeshTags(rv, tags);
    }
    else if (strcmp(mesh, "mesh_tags/edges_mesh") == 0)
    {
        pdb_mesh2d *mesh2d = MAKE_N(pdb_mesh2d, 1);
        if (PD_read(m_pdbFile, "mesh", mesh2d) == false)
        {
            Close();
            EXCEPTION1(InvalidDBTypeException, "Cannot be a KullLite file, "
                       " does not have a mesh.");
        }
        CreateEdgeMeshTags(rv, tags, mesh2d);
        SFREE(mesh2d);
    }
    else if (strcmp(mesh, "mesh_tags/faces_mesh") == 0)
    {
        pdb_mesh3d *mesh3d = MAKE_N(pdb_mesh3d, 1);
        if (PD_read(m_pdbFile, "mesh", mesh3d) == false)
        {
            Close();
            EXCEPTION1(InvalidDBTypeException, "Cannot be a KullLite file, "
                       " does not have a mesh.");
        }
        CreateFaceMeshTags(rv, tags, mesh3d);
        SFREE(mesh3d);
    }
    else if (strcmp(mesh, "mesh_tags/zones_mesh") == 0)
    {
        CreateZoneMeshTags(rv, tags, dataset[dom]);
    }
    else
    {
        EXCEPTION1(InvalidVariableException, mesh);
        Close();
        return NULL;
    }

    //
    // Clean up.
    //
    rv->Squeeze();
    SFREE(tags);
    Close();

    return rv;
}


// ****************************************************************************
//  Method: avtKullLiteFileFormat::CreateNodeMeshTags
//
//  Purpose:
//      Creates a node "mesh tags" object.
//
//  Programmer: Hank Childs
//  Creation:   July 28, 2004
//
// ****************************************************************************

void
avtKullLiteFileFormat::CreateNodeMeshTags(vtkUnstructuredGrid *ugrid,
                                          pdb_taglist *tags)
{
    int  i, j, k;

    int nnodes = 0;
    for (i = 0 ; i < tags->num_tags ; i++)
        if (tags->tags[i].type == TAG_NODE)
            nnodes += tags->tags[i].size;
    ugrid->Allocate(nnodes*2);

    for (j = 0 ; j < node_tags.size() ; j++)
        for (i = 0 ; i < tags->num_tags ; i++)
            if (node_tags[j] == tags->tags[i].tagname)
            {
                int *buff = new int[tags->tags[i].size];
                PD_read(m_pdbFile, tags->tags[i].tagname, buff);
                for (k = 0 ; k < tags->tags[i].size ; k++)
                {
                    vtkIdType pts[1] = { buff[k] };
                    ugrid->InsertNextCell(VTK_VERTEX, 1, pts);
                }
                delete [] buff;
            }
}


// ****************************************************************************
//  Method: avtKullLiteFileFormat::CreateEdgeMeshTags
//
//  Purpose:
//      Creates a edge "mesh tags" object.
//
//  Programmer: Hank Childs
//  Creation:   July 28, 2004
//
// ****************************************************************************

void
avtKullLiteFileFormat::CreateEdgeMeshTags(vtkUnstructuredGrid *ugrid,
                                          pdb_taglist *tags,pdb_mesh2d *mesh2d)
{
    int   i,j,k;

    int nedges = 0;
    for (i = 0 ; i < tags->num_tags ; i++)
        if (tags->tags[i].type == TAG_EDGE)
            nedges += tags->tags[i].size;
    ugrid->Allocate(nedges*3);

    for (j = 0 ; j < edge_tags.size() ; j++)
        for (i = 0 ; i < tags->num_tags ; i++)
            if (edge_tags[j] == tags->tags[i].tagname)
            {
                int *buff = new int[tags->tags[i].size];
                PD_read(m_pdbFile, tags->tags[i].tagname, buff);
                for (k = 0 ; k < tags->tags[i].size ; k++)
            {
                    vtkIdType pts[2];
                    pts[0] = mesh2d->nodeIndices[2*buff[k]];
                    pts[1] = mesh2d->nodeIndices[2*buff[k]+1];
                    ugrid->InsertNextCell(VTK_LINE, 2, pts);
                }
                delete [] buff;
            }
}


// ****************************************************************************
//  Method: avtKullLiteFileFormat::CreateFaceMeshTags
//
//  Purpose:
//      Creates a face "mesh tags" object.
//
//  Programmer: Hank Childs
//  Creation:   July 28, 2004
//
// ****************************************************************************

void
avtKullLiteFileFormat::CreateFaceMeshTags(vtkUnstructuredGrid *ugrid,
                                          pdb_taglist *tags,pdb_mesh3d *mesh3d)
{
    int   i, j, k, m;

    int nfaces = 0;
    for (i = 0 ; i < tags->num_tags ; i++)
        if (tags->tags[i].type == TAG_FACE)
            nfaces += tags->tags[i].size;
    ugrid->Allocate(nfaces*5);

    for (j = 0 ; j < face_tags.size() ; j++)
        for (i = 0 ; i < tags->num_tags ; i++)
            if (face_tags[j] == tags->tags[i].tagname)
            {
                int *buff = new int[tags->tags[i].size];
                PD_read(m_pdbFile, tags->tags[i].tagname, buff);
                for (k = 0 ; k < tags->tags[i].size ; k++)
                {
                    int startNodeIndex =
                                 mesh3d->faceToNodesIndex[buff[k]];
                    int endNodeIndex =
                                 mesh3d->faceToNodesIndex[buff[k]+1];
                    int nodesForThisFace = endNodeIndex-startNodeIndex;
                    if (nodesForThisFace != 3 && nodesForThisFace != 4)
                    {
                        EXCEPTION1(InvalidDBTypeException,
                              "Triangles and quads are the only supported "
                              "face types.");
                    }

                    vtkIdType pts[4];
                    for (m = 0 ; m < nodesForThisFace ; m++)
                        pts[m] = 
                               mesh3d->nodeIndices[startNodeIndex+m];
                    int cell_type = (nodesForThisFace == 3 ? VTK_TRIANGLE
                                                           : VTK_QUAD);
                    ugrid->InsertNextCell(cell_type, nodesForThisFace, pts);
                }
                delete [] buff;
            }
}


// ****************************************************************************
//  Method: avtKullLiteFileFormat::CreateZoneMeshTags
//
//  Purpose:
//      Creates a zone "mesh tags" object.
//
//  Programmer: Hank Childs
//  Creation:   July 28, 2004
//
// ****************************************************************************

void
avtKullLiteFileFormat::CreateZoneMeshTags(vtkUnstructuredGrid *ugrid,
                                          pdb_taglist *tags, vtkDataSet *ds)
{
    int  i, j, k;

    int nzones = 0;
    for (i = 0 ; i < tags->num_tags ; i++)
        if (tags->tags[i].type == TAG_ZONE)
            nzones += tags->tags[i].size;
    ugrid->Allocate(nzones*10);

    int nInputZones = ds->GetNumberOfCells();

    for (j = 0 ; j < zone_tags.size() ; j++)
        for (i = 0 ; i < tags->num_tags ; i++)
            if (zone_tags[j] == tags->tags[i].tagname)
            {
                int *buff = new int[tags->tags[i].size];
                PD_read(m_pdbFile, tags->tags[i].tagname, buff);
                vtkIdList *ptIds = vtkIdList::New();
                for (k = 0 ; k < tags->tags[i].size ; k++)
                {
                    if (buff[k] > nInputZones)
                    {
                        EXCEPTION1(InvalidDBTypeException, "The zone mesh tag "
                        "references zones that are not in the primary mesh "
                        "-- this can happen because (1) of normal indexing "
                        "errors or (2) because a zone in the primary mesh "
                        "was disregarded somehow by this reader.  If this is "
                        "the case, this warning would be preceded by others.");
                    }
                    int type = ds->GetCellType(buff[k]);
                    ds->GetCellPoints(buff[k], ptIds);
                    vtkIdType npts = ptIds->GetNumberOfIds();
                    vtkIdType *pts = ptIds->GetPointer(0);
                    ugrid->InsertNextCell(type, npts, pts);
                }
                ptIds->Delete();
            }
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
//    Hank Childs, Fri Oct 17 21:43:45 PDT 2003
//    Stop using constructs that come from AddFile (filenames, nFiles), since
//    they are primarily for handling file descriptors.  Used data members
//    specific to this class (my_filenames, my_filenames.size()) instead.
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
        for (i = 0 ; i < my_filenames.size(); i++)
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
//  Modifications:
//
//    Hank Childs, Fri Oct 17 21:43:45 PDT 2003
//    Stop using constructs that come from AddFile (filenames, nFiles), since
//    they are primarily for handling file descriptors.  Used data members
//    specific to this class (my_filenames, my_filenames.size()) instead.
//
//    Hank Childs, Fri Jul 23 09:58:35 PDT 2004
//    Add support for 2D meshes.
//
//    Hank Childs, Mon Jul 26 14:20:34 PDT 2004
//    Moved body of function to GetRealMaterial.  Add support for mesh tags.
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

    // Note: this check should come only once we are certain that we are 
    // looking for materials, since values like "-1" can be specified for
    // interval trees.
    if (domain < 0 || domain >= my_filenames.size())
        EXCEPTION2(BadDomainException, domain, my_filenames.size());

    void *rv = NULL;
    if (strcmp(var, "Material") == 0)
    {
        rv = GetRealMaterial(domain);
    }
    else
    {
        rv = GetMeshTagMaterial(var, domain);
    }

    df = avtMaterial::Destruct;
    return rv;
}


// ****************************************************************************
//  Method: avtKullLiteFileFormat::GetMeshTagMaterial
//
//  Purpose:
//      Gets the mesh tag information as a material.
//
//  Programmer: Hank Childs
//  Creation:   July 26, 2004
//
// ****************************************************************************

void *
avtKullLiteFileFormat::GetMeshTagMaterial(const char *var, int dom)
{
    int   i, j, k;

    //
    // Make sure we are really supposed to read in a mesh tag.
    //
    vector<string> *tag_list = NULL;
    int tag = TAG_BADTYPE;
    if (strcmp(var, "mesh_tags/nodes") == 0)
    {
        tag = TAG_NODE;
        tag_list = &node_tags;
    }
    else if (strcmp(var, "mesh_tags/edges") == 0)
    {
        tag = TAG_EDGE;
        tag_list = &edge_tags;
    }
    else if (strcmp(var, "mesh_tags/faces") == 0)
    {
        tag = TAG_FACE;
        tag_list = &face_tags;
    }
    else if (strcmp(var, "mesh_tags/zones") == 0)
    {
        tag = TAG_ZONE;
        tag_list = &zone_tags;
    }
  
    if (tag == TAG_BADTYPE)
    {
        return NULL;
    }

    //
    // Now open the file and read out the tag list.
    //
    m_pdbFile = PD_open((char *) my_filenames[dom].c_str(), "r");
    if (m_pdbFile == NULL)
    {
        Close();
        EXCEPTION1(InvalidDBTypeException, "Cannot be a KullLite file, this "
                   " is file not a valid PDB file.");
    }
    pdb_taglist *tags = MAKE_N(pdb_taglist, 1);
    if (PD_read(m_pdbFile, "MeshTags", tags) == false)
    {
        Close();
        EXCEPTION1(InvalidVariableException, "MeshTags");
        return NULL;
    }

    //
    // Determine how big the material will be.
    //
    int nelems = 0;
    for (j = 0 ; j < tag_list->size() ; j++)
        for (i = 0 ; i < tags->num_tags ; i++)
            if ((*tag_list)[j] == tags->tags[i].tagname)
                nelems += tags->tags[i].size;

    //
    // Now construct the avt material object based solely on sizes of mesh
    // tags.
    //
    int *ptr = new int[nelems];
    int index = 0;
    for (j = 0 ; j < tag_list->size() ; j++)
    {
        for (i = 0 ; i < tags->num_tags ; i++)
        {
            if ((*tag_list)[j] == tags->tags[i].tagname)
            {
                for (k = 0 ; k < tags->tags[i].size ; k++)
                {
                    ptr[index++] = j;
                }
            }
        }
    }
    avtMaterial *mat = new avtMaterial(tag_list->size(), *tag_list, nelems,
                                       ptr, 0, NULL, NULL, NULL, NULL);

    //
    // Clean up.
    //
    SFREE(tags);
    delete [] ptr;

    return mat;
}


// ****************************************************************************
//  Method: avtKullLiteFileFormat::GetRealMaterial
//
//  Purpose:
//      Constructs an avtMaterial object.
//
//  Arguments:
//      domain     The domain of interest.
//
//  Returns:    The auxiliary data.
//
//  Programmer: Akira Haddox
//  Creation:   May 20, 2003
//
//  Modifications:
//
//    Hank Childs, Mon Jul 26 14:20:34 PDT 2004
//    Moved function from GetAuxiliaryData.
//
//    Hank Childs, Wed Jul 28 08:22:06 PDT 2004
//    Added materials for receive zones as well.
//
// ****************************************************************************

void *
avtKullLiteFileFormat::GetRealMaterial(int domain)
{
    // If we have no materials, we shouldn't be queried for them.
    if (NumberOfMaterials() == 0)
        EXCEPTION1(InvalidVariableException, "No materials to query.");

    m_pdbFile = PD_open((char *) my_filenames[domain].c_str(), "r");
    if (m_pdbFile == NULL)
    {
        Close();
        EXCEPTION1(InvalidDBTypeException, "Not a valid PDB file.");
    }
    
    bool meshIs3d = ReadMeshFromFile();
    int total_zones = (meshIs3d ? m_kullmesh3d->nzones : m_kullmesh2d->nzones);
   
    // Calculate how many zones are not used by this file.
    int nRecvZones = ReadNumberRecvZones();
    
    // Now we go through to turn the Kull material into the Silo material
    // format to fit into an avtMaterial structure.

    // These variables are needed to create the material data.
    int num_materials = NumberOfMaterials();
    vector<string> &mat_names = m_names;
    int num_real = total_zones - nRecvZones;
    vector<int> material_list(total_zones);
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
    for (i = 0 ; i < num_materials ; i++)
    {
        values[i].resize(total_zones, -1.0f);
    }
    
    // Read in the tags
    m_tags = MAKE_N(pdb_taglist, 1);
    if (PD_read(m_pdbFile, "MeshTags", m_tags) == false)
    {
        Close();
        EXCEPTION1(InvalidVariableException, "No meshtag data.");
    }

    // Now we go through the materials, and deal with them accordingly. 
    for (i = 0; i < num_materials; i++)
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
            EXCEPTION1(InvalidFilesException, my_filenames[domain].c_str());
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
            EXCEPTION1(InvalidFilesException, my_filenames[domain].c_str());
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
            EXCEPTION1(InvalidFilesException, my_filenames[domain].c_str());
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
    // Go through the first num_real (which leaves out the recvzones).
    for (i = 0; i < num_real; ++i)
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

    // Now give all the receive zones a bogus value.
    for (i = num_real ; i < total_zones ; i++)
        material_list[i] = 0;

    int mixed_size = mix_zone.size();
    avtMaterial * mat = new avtMaterial(num_materials, mat_names, total_zones,
                                        &(material_list[0]), mixed_size,
                                        &(mix_mat[0]), &(mix_next[0]),
                                        &(mix_zone[0]), &(mix_vf[0]));

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
//
//     Hank Childs, Fri Oct 17 21:43:45 PDT 2003
//     Stop using constructs that come from AddFile (filenames, nFiles), since
//     they are primarily for handling file descriptors.  Used data members
//     specific to this class (my_filenames, my_filenames.size()) instead.
//
//     Hank Childs, Fri Jul 23 11:11:43 PDT 2004
//     Get the dimension of the mesh.
//
//     Hank Childs, Mon Jul 26 09:16:28 PDT 2004
//     Add support for mesh tags.
//
//     Kathleen Bonnell, Wed Aug 25 11:21:56 PDT 2004 
//     Set mesh-type for node_tags to AVT_POINT_MESH. 
//
//     Hank Childs, Tue Jun 14 16:25:52 PDT 2005
//     Add support for RZ meshes.
//
// ****************************************************************************

void
avtKullLiteFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    bool is3DMesh = true;
    bool isRZ = false;
    if (my_filenames.size() > 0)
    {
        m_pdbFile = PD_open((char *) my_filenames[0].c_str(), "r");
        if (m_pdbFile == NULL)
        {
            Close();
            EXCEPTION1(InvalidDBTypeException, "Not a valid PDB file.");
        }

        is3DMesh = GetMeshDimension();
        if (!is3DMesh)
            isRZ = IsRZ();
        Close();
    }

    vector<string> vFilenames(my_filenames.size());
    int i;
    for (i = 0; i < my_filenames.size(); i++)
        vFilenames[i] = my_filenames[i];

    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = MESHNAME;
    mesh->meshType = AVT_UNSTRUCTURED_MESH;
    mesh->numBlocks = my_filenames.size();
    mesh->blockOrigin = 0;
    mesh->spatialDimension = (is3DMesh ? 3 : 2);
    mesh->topologicalDimension = (is3DMesh ? 3 : 2);
    mesh->blockTitle = "Files";
    mesh->blockNames = vFilenames;
    mesh->hasSpatialExtents = false;
    if (isRZ)
    {
        mesh->xLabel = "Z-Axis";
        mesh->yLabel = "R-Axis";
    }
    md->Add(mesh);

    if (NumberOfMaterials())
    {
        AddMaterialToMetaData(md,"Material",MESHNAME, NumberOfMaterials(),
                              m_names);
    }

    if (zone_tags.size() > 0)
    {
        avtMeshMetaData *mesh = new avtMeshMetaData;
        mesh->name = "mesh_tags/zones_mesh";
        mesh->meshType = AVT_UNSTRUCTURED_MESH;
        mesh->numBlocks = my_filenames.size();
        mesh->blockOrigin = 0;
        mesh->spatialDimension = (is3DMesh ? 3 : 2);
        mesh->topologicalDimension = (is3DMesh ? 3 : 2);
        mesh->blockTitle = "Files";
        mesh->blockNames = vFilenames;
        mesh->hasSpatialExtents = false;
        md->Add(mesh);

        AddMaterialToMetaData(md, "mesh_tags/zones", "mesh_tags/zones_mesh",
                              zone_tags.size(), zone_tags);
    }
    if (face_tags.size() > 0 && is3DMesh)
    {
        avtMeshMetaData *mesh = new avtMeshMetaData;
        mesh->name = "mesh_tags/faces_mesh";
        mesh->meshType = AVT_UNSTRUCTURED_MESH;
        mesh->numBlocks = my_filenames.size();
        mesh->blockOrigin = 0;
        mesh->spatialDimension = 3;
        mesh->topologicalDimension = 2;
        mesh->blockTitle = "Files";
        mesh->blockNames = vFilenames;
        mesh->hasSpatialExtents = false;
        md->Add(mesh);

        AddMaterialToMetaData(md, "mesh_tags/faces", "mesh_tags/faces_mesh",
                              face_tags.size(), face_tags);
    }
    if (edge_tags.size() > 0 && !is3DMesh)
    {
        avtMeshMetaData *mesh = new avtMeshMetaData;
        mesh->name = "mesh_tags/edges_mesh";
        mesh->meshType = AVT_UNSTRUCTURED_MESH;
        mesh->numBlocks = my_filenames.size();
        mesh->blockOrigin = 0;
        mesh->spatialDimension = 2;
        mesh->topologicalDimension = 1;
        mesh->blockTitle = "Files";
        mesh->blockNames = vFilenames;
        mesh->hasSpatialExtents = false;
        md->Add(mesh);

        AddMaterialToMetaData(md, "mesh_tags/edges", "mesh_tags/edges_mesh",
                              edge_tags.size(), edge_tags);
    }
    if (node_tags.size() > 0)
    {
        avtMeshMetaData *mesh = new avtMeshMetaData;
        mesh->name = "mesh_tags/nodes_mesh";
        mesh->meshType = AVT_POINT_MESH;
        mesh->numBlocks = my_filenames.size();
        mesh->blockOrigin = 0;
        mesh->spatialDimension = (is3DMesh ? 3 : 2);
        mesh->topologicalDimension = 0;
        mesh->blockTitle = "Files";
        mesh->blockNames = vFilenames;
        mesh->hasSpatialExtents = false;
        md->Add(mesh);

        AddMaterialToMetaData(md, "mesh_tags/nodes", "mesh_tags/nodes_mesh",
                              node_tags.size(), node_tags);
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
//  Modifications:
//
//    Hank Childs, Fri Jul 23 09:55:55 PDT 2004
//    Clean up the 2d mesh as well.
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
    if (m_kullmesh3d)
    {
        SFREE(m_kullmesh3d);
        m_kullmesh3d = NULL;
    }
    if (m_kullmesh2d)
    {
        SFREE(m_kullmesh2d);
        m_kullmesh3d = NULL;
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
//    Hank Childs, Fri Oct 17 21:43:45 PDT 2003
//    Stop using constructs that come from AddFile (filenames, nFiles), since
//    they are primarily for handling file descriptors.  Used data members
//    specific to this class (my_filenames, my_filenames.size()) instead.
//
// ****************************************************************************

void avtKullLiteFileFormat::ReadInMaterialNames()
{
    m_names.clear();
    for (int i = 0; i < my_filenames.size(); i++)
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
//    Hank Childs, Fri Oct 17 21:43:45 PDT 2003
//    Stop using constructs that come from AddFile (filenames, nFiles), since
//    they are primarily for handling file descriptors.  Used data members
//    specific to this class (my_filenames, my_filenames.size()) instead.
//
//    Hank Childs, Fri Jul 23 16:11:55 PDT 2004
//    Add support for mesh tags as well.
//
// ****************************************************************************

void avtKullLiteFileFormat::ReadInMaterialName(int fi)
{
    debug4 << "Reading in material header from KullLite file "
           << my_filenames[fi].c_str() << endl;    

    m_pdbFile = PD_open((char *) my_filenames[fi].c_str(), "r");
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

        if (IsMaterialName(originalName))      
        {
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
        else
        {
            bool hasTag = false;
            int  i;
            switch (m_tags->tags[curTagI].type)
            {
              case TAG_ZONE:
                for (i = 0 ; i < zone_tags.size() ; i++)
                    if (zone_tags[i] == m_tags->tags[curTagI].tagname)
                        hasTag = true;
                if (!hasTag)
                    zone_tags.push_back(m_tags->tags[curTagI].tagname);
                break;
              case TAG_FACE: 
                for (i = 0 ; i < face_tags.size() ; i++)
                    if (face_tags[i] == m_tags->tags[curTagI].tagname)
                        hasTag = true;
                if (!hasTag)
                    face_tags.push_back(m_tags->tags[curTagI].tagname);
                break;
              case TAG_EDGE:
                for (i = 0 ; i < edge_tags.size() ; i++)
                    if (edge_tags[i] == m_tags->tags[curTagI].tagname)
                        hasTag = true;
                if (!hasTag)
                    edge_tags.push_back(m_tags->tags[curTagI].tagname);
                break;
              case TAG_NODE:
                for (i = 0 ; i < node_tags.size() ; i++)
                    if (node_tags[i] == m_tags->tags[curTagI].tagname)
                        hasTag = true;
                if (!hasTag)
                    node_tags.push_back(m_tags->tags[curTagI].tagname);
                break;
            }
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
    //               /|          /|
    //              4 + - - - - 5 |
    //               \|          \|
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


// ****************************************************************************
//  Method: avtKullLiteFileFormat::ReadMeshFromFile
//
//  Purpose:
//      Determines whether this is a 2D or 3D file and reads in the mesh.
//
//  Returns:    True if the mesh is 3D, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   July 23, 2004
//
// ****************************************************************************

bool
avtKullLiteFileFormat::ReadMeshFromFile(void)
{
    bool meshIs3d = GetMeshDimension();

    if (meshIs3d)
    {
        // Read in the Mesh
        m_kullmesh3d = MAKE_N(pdb_mesh3d, 1);
        if (PD_read(m_pdbFile, "mesh", m_kullmesh3d) == false)
        {
            Close();
            EXCEPTION1(InvalidDBTypeException, "Cannot be a KullLite file, "
                       " does not have a mesh.");
        }
    }
    else
    {
        // Read in the Mesh
        m_kullmesh2d = MAKE_N(pdb_mesh2d, 1);
        if (PD_read(m_pdbFile, "mesh", m_kullmesh2d) == false)
        {
            Close();
            EXCEPTION1(InvalidDBTypeException, "Cannot be a KullLite file, "
                       " does not have a mesh.");
        }
    }
    
    return meshIs3d;
}


// ****************************************************************************
//  Method: avtKullLiteFileFormat::GetMeshDimension
//
//  Purpose:
//      Gets the dimension of the mesh.
//
//  Returns:   True if its 3D, false if its 2D.
//
//  Programmer: Hank Childs
//  Creation:   July 23, 2004
//
// ****************************************************************************

bool
avtKullLiteFileFormat::GetMeshDimension(void)
{
    char *typeofmesh = MAKE_N(char, 1024);
    if (PD_read(m_pdbFile, "typeofmesh", &typeofmesh) == false)
    {
        Close();
        EXCEPTION1(InvalidDBTypeException, "Cannot be a KullLite file, does "
                   " not have \"typeofmesh\".");
    }

    bool meshIs3d = false;
    if (strstr(typeofmesh, "polyhedral") != NULL || 
        strstr(typeofmesh, "hexahedral") != NULL)
        meshIs3d = true;
    else if (strstr(typeofmesh, "polygonal") != NULL ||
             strstr(typeofmesh, "quadrilateral") != NULL)
        meshIs3d = false;
    else
    {
        Close();
        EXCEPTION1(InvalidDBTypeException, "Cannot be a KullLite file, "
                   "\"typeofmesh\" is not a recognized type.");
    }
  
    SFREE(typeofmesh);
    return meshIs3d;
}


// ****************************************************************************
//  Method: avtKullLiteFileFormat::IsRZ
//
//  Purpose:
//      Determines if a mesh is RZ.
//
//  Returns:   True if it is RZ, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   June 14, 2005
//
// ****************************************************************************

bool
avtKullLiteFileFormat::IsRZ(void)
{
    return (PD_inquire_type(m_pdbFile, "rz") ? true : false);
}


