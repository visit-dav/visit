// ************************************************************************* //
//                        avtKullLiteFileFormat.C                            //
// ************************************************************************* //

#include <avtKullLiteFileFormat.h>

#include <string>
#include <map>
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

#include <avtDatabaseMetaData.h>  
#include <avtCallback.h>

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
        char buff[4096];
        inf >> buff;
        string a(buff);
        while (inf && !inf.eof())
        {
            AddFile(a.c_str());
            inf >> buff;
            a = buff;
        }
    }
    else // We're opening a single file, it's not an index
    {
        AddFile(fname);
    }
    inf.close();

    dataset = new vtkDataSet*[nFiles];
    materials.resize(nFiles);
    for (int i = 0; i < nFiles; i++)
    {
        dataset[i] = NULL;
    }


    // We might read them in from the master file eventually, but for
    // now, we'll do it here in the constructor [so it's a straight
    // modification if we do put them in the master file]
    ReadInMaterialNames();
}


// ****************************************************************************
//  Method: avtKullLiteFileFormat destructor
//
//  Programmer: Akira Haddox
//  Creation:   June 18, 2002
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
    for (int i = 0; i < materials.size(); i++)
    {
       std::map<std::string, vtkDataSet*>::iterator im;
       for (im = materials[i].begin(); im != materials[i].end(); im++)
           (im->second)->Delete();
       materials[i].clear();
    }
    materials.clear();
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

    //Read in the Mesh
    m_kullmesh = MAKE_N(pdb_mesh, 1);
    if (PD_read(m_pdbFile, "mesh", m_kullmesh) == false)
    {
        Close();
        EXCEPTION1(InvalidDBTypeException, "Cannot be a KullLite file, does "
                   " not have a mesh.");
    }

    //Read in the tags
    m_tags = MAKE_N(pdb_taglist, 1);
    if (PD_read(m_pdbFile, "MeshTags", m_tags) == false)
    {
        // This is a valid possibility, we have to deal with it
        SFREE(m_tags);
        m_tags = NULL;
    }

    //Read in nRecvZone, used for figuring out how many 
    //zones are owned by this file
    int nRecvZones = 0; // 0, in case not multi-processor set
    PD_read(m_pdbFile, "RecvZonesSize", &nRecvZones); //This can fail,
                                                      // that's okay

    int nMaterials = 0;
    PD_read(m_pdbFile, "num_materials", &nMaterials); //This can fail,
                                                      // that's okay

    // This however, isn't okay. If there are materials, 
    // we need tags to go with
    if (nMaterials && !m_tags)
    {
        Close();
        EXCEPTION1(InvalidDBTypeException, "Cannot be a KullLite file, does "
                   " have meshtags or materials.");
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
    for( i = 0; i < nPoints; i++)
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
    for(int zone = 0; zone < nCells; zone++)
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
                faceIndex = -1-faceIndex; //one's complement to get index
            }

            //Find the nodes
            int startNodeIndex = m_kullmesh->faceToNodesIndex[faceIndex];
            int endNodeIndex = m_kullmesh->faceToNodesIndex[faceIndex+1];
            int nodesForThisFace = endNodeIndex-startNodeIndex;

            nodes[faceI-startZoneToFaceIndex].resize(nodesForThisFace);

            //Let's grab the nodeId's and store them
            if (sharedSlave) //Shared face, grab them in reverse order
            {
                for (i = nodesForThisFace-1; i >= 0; i--)
                {
                    nodes[faceI-startZoneToFaceIndex][nodesForThisFace-1-i] =
                m_kullmesh->nodeIndices[i+startNodeIndex];
                }
            }
            else //Normal face, just grab them
            {
                for (i = 0; i < nodesForThisFace; i++)
                {
                    nodes[faceI-startZoneToFaceIndex][i] =
                m_kullmesh->nodeIndices[i+startNodeIndex];
                }
            }
        }

        // Now then, we have the nodes, let's classify
        int type;
        if (numFaces == 4)
            type=VTK_TETRA;
        else if (numFaces == 6)
            type = VTK_HEXAHEDRON;
        else if (numFaces == 5)
        {


            // Cell could be Pyramid or Wedge. 
        // We assume Pyramid, then look to see if
            // there's more than one face with 4 nodes.
        // If there is, we change our
            // mind: it's a wedge.
            bool first = false;
            type = VTK_PYRAMID;
            for (i = 0; i < numFaces; i++)
            {
                if (nodes[i].size() == 4)
                {
                    if (first)
                    {   //At least two faces have 4 nodes
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
    // Let's get the points in the right order and build

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
                    // Unraveled loop, for some optimization,
            // and to avoid having
                    // a flag to know when we've found the face
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

            //You'd think we'd be done right? Wrong!
            //We have the two faces, but they aren't necessarily both aligned

            //This theory has been tested: There is a strong tendency for
            //hexahedrons not to have these two faces aligned. So,
            //unfortunately, this code is neccessary.

            //This leads to a necessary finding of the offset
            
            //Find a difference face [than face 0] that has points[0]
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
            // And two other points. We want to find the point next to points[0]
            // (which is in nodes[diffFace][whichIndex] of course)
            // That isn't points[1] or points[3], and find the index
            // of it in points[]

            int otherIndex;
            int ii = nodes[diffFace][(whichIndex+1) %4];
    
            // Start looking infront
            if ((ii == points[1] || ii == points[3]))
            { // The point is behind, re-adjust
                ii=nodes[diffFace][(whichIndex-1+4) %4];
            }

            for (otherIndex = 4; otherIndex < 8; otherIndex++)
                if (points[otherIndex] == ii)
                    break;
            
            //otherIndex should be 4. If it's not, we have to offset
            if (otherIndex !=4 )
            {
                int offSet = otherIndex-4;

                int tmp[4];
                tmp[0] = points[4];
                tmp[1] = points[5];
                tmp[2] = points[6];
                tmp[3] = points[7];

                points[4] = tmp[(offSet+0)%4];
                points[5] = tmp[(offSet+1)%4];
                points[6] = tmp[(offSet+2)%4];
                points[7] = tmp[(offSet+3)%4];
            }
            
            // Put the cell into the dataset
            cellId = datasetPtr->InsertNextCell(type, 8, points);
        }
        else if (type == VTK_TETRA)
        {
            // My personal favorite, because there's only 4 points, so order
            // doesn't matter.
            int points[4];
            //Get one face
            points[0] = nodes[0][0];
            points[1] = nodes[0][1];
            points[2] = nodes[0][2];

            //Find the other point in face 1
            if (!(nodes[1][0] == points[0] || nodes[1][0] == points[1]
                  ||  nodes[1][0] == points[2]))
                points[3] = nodes[1][0];
            else if(!(nodes[1][1] == points[0] || nodes[1][1] == points[1]
                      ||  nodes[1][1] == points[2]))
                points[3] = nodes[1][1];
            else if(!(nodes[1][2] == points[0] || nodes[1][2] == points[1]
                      ||  nodes[1][2] == points[2]))
                points[3] = nodes[1][2];

            cellId = datasetPtr->InsertNextCell(type, 4, points);
        }
        else if (type == VTK_PYRAMID)
        {
            int points[5];
            //Find the base
            int i;
            for (i = 0; i < nodes.size(); i++)
                if (nodes[i].size() == 4)
                    break;
            points[0] = nodes[i][0];
            points[1] = nodes[i][1];
            points[2] = nodes[i][2];
            points[3] = nodes[i][3];

            //Find the top point
            if (i == 0) ++i;
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
        else // Technically, should be impossible to get here but
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
                     "whose types  weren't recognized. Left them out.",
                     failedReadElements);
        avtCallback::IssueWarning(msg);
    }

    // Topology of mesh complete. Now to add in accompaning material data

    // This loop requires materials, which require tags,
    // and has been checked previously.
    // So we aren't checking to make sure m_tags isn't NULL again.
    int curTagI;
    for (curTagI = 0; curTagI < nMaterials; curTagI++)
    {
        char *name = m_tags->tags[curTagI].tagname;
        int ttype = m_tags->tags[curTagI].type;
        int tsize = m_tags->tags[curTagI].size;

        // We don't know how to deal with these in VisIt yet
        if (ttype == TAG_FACE || ttype == TAG_EDGE)
            continue; // Ignore for now

        if (ttype == TAG_NODE)
            continue; // Not supported/ not in materials


        vtkUnstructuredGrid *md;
        if (materials[fi].find(name) == materials[fi].end())
        {
            // new in this domain
            md = vtkUnstructuredGrid::New();
            materials[fi][name] = md;
            md->SetPoints(points); // We cheat a bit here
        }
        else
            md = (vtkUnstructuredGrid *)(materials[fi][name]);

        vector<int> ids(tsize);
        if (PD_read(m_pdbFile, name, &(ids[0])) == false)
        {
            EXCEPTION1(InvalidFilesException, filenames[fi]);//failed to read
        }


        if (ttype == TAG_ZONE)
        {
            for (int j = 0; j < ids.size(); j++)
            {
                // We need to check up here, since if we ask for a cell
                // from a different file,
                // and it's not shared, we could potentially crash
                if (ids[j] > nCells || ids[j] < 0)
                    continue;

                vtkCell *c = datasetPtr->GetCell(ids[j]);
                // Just a sanity check
                if (c)
                    md->InsertNextCell(c->GetCellType(), c->GetPointIds());
            }
        } 
    }

    if (m_tags) // Work with the meshTags
    {
        int nTags = m_tags->num_tags;
        for (curTagI = nMaterials; curTagI < nTags; ++curTagI)
        {
             char *name = m_tags->tags[curTagI].tagname;
             int ttype = m_tags->tags[curTagI].type;
             int tsize = m_tags->tags[curTagI].size;

             vector<int> ids(tsize);
             if (PD_read(m_pdbFile, name, &(ids[0])) == false)
             {
                 EXCEPTION1(InvalidFilesException, filenames[fi]);
             }

            if (ttype == TAG_ZONE)
            {
                vtkUnstructuredGrid *md;
                if (materials[fi].find(name) == materials[fi].end())
                {
                    // new in this domain
                    md = vtkUnstructuredGrid::New();
                    materials[fi][name] = md;
                    md->SetPoints(points); // We cheat a bit here
                }
                else
                    md = (vtkUnstructuredGrid *)(materials[fi][name]);
                for (int j = 0; j < ids.size(); j++)
                {
                    // We need to check up here, since if we ask for a
                    // cell from a different file,
                    // and it's not shared, we could potentially crash
                    if (ids[j] > nCells || ids[j] < 0)
                        continue;
    
                    vtkCell * c = datasetPtr->GetCell(ids[j]);
                    // Just a sanity check
                    if (c)
                        md->InsertNextCell(c->GetCellType(), c->GetPointIds());
                }
                continue;
            }

            if (ttype == TAG_NODE)
            {
                vtkPolyData *md;
                if (materials[fi].find(name) == materials[fi].end())
                {
                    // new in this domain
                    md = vtkPolyData::New();
                    materials[fi][name] = md;
                    md->SetPoints(points); // We cheat a bit here
                    md->Allocate(0, 1+nCells/10);
                }
                else
                    md = (vtkPolyData *)(materials[fi][name]);


                for (int nodeI = 0; nodeI < ids.size(); ++nodeI)
                {
                    md->InsertNextCell(VTK_VERTEX, 1, &(ids[nodeI]));
                }
                continue;
            }

            if (ttype == TAG_EDGE)
            {
                continue; //xyz, we don't know how to deal with this
            }

            int cellType;
            int numFacePoints;

            vtkPolyData *md;
            if (materials[fi].find(name) == materials[fi].end())
            {
                // new in this domain
                md = vtkPolyData::New();
                materials[fi][name] = md;
                md->SetPoints(points); // We cheat a bit here
                md->Allocate(0, 1+nCells/10);
            }
            else
                md = (vtkPolyData *)(materials[fi][name]);        


            for (i = 0; i < ids.size(); ++i)
            {
                int startFaceI = m_kullmesh->faceToNodesIndex[ids[i]];
                int endFaceI = m_kullmesh->faceToNodesIndex[ids[i]+1];

                numFacePoints = endFaceI-startFaceI;
                if (numFacePoints == 3)
                    cellType = VTK_TRIANGLE;
                else if (numFacePoints == 4)
                    cellType = VTK_QUAD;
                else 
                    cellType = VTK_POLYGON;

                int *nodes = new int[numFacePoints];
                for (int faceI = startFaceI; faceI < endFaceI; ++faceI)
                {
                    nodes[faceI-startFaceI] = m_kullmesh->nodeIndices[faceI];
                }
            
                md->InsertNextCell(cellType, numFacePoints, nodes);

                delete[] nodes;
            }
        }
    }

    // End reading, clean up for next read
    Close();
}



// ****************************************************************************
//  Method: avtKullLiteFileFormat::ReadInAllFiles
//
//  Purpose:
//      Read in all files [usually because a material request was made].
//
//  Programmer: Akira Haddox
//  Creation:   June 27, 2002
//
// ****************************************************************************

void avtKullLiteFileFormat::ReadInAllFiles()
{
    for (int i = 0; i < nFiles; i++)
        if (!dataset[i])
            ReadInFile(i);
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
    
    if (doMaterialSelection)
    {
        std::map<std::string, vtkDataSet*>::iterator loc =
                                            materials[dom].find(materialName);
        if (loc == materials[dom].end())
        {
            return NULL; // No such material in that domain
        }
        loc->second->Register(NULL);
        return loc->second;
    }

    std::map<std::string, vtkDataSet*>::iterator loc =
                                        materials[dom].find(mesh);
    if (loc != materials[dom].end())
    {
        loc->second->Register(NULL);
        return loc->second;
    }

    // Just because a mesh name isn't in the domain, doesn't mean it's invalid.
    // It may be a tag that isn't in that domain. So we return NULL instead of
    // raising an exception.
    const string meshNameStr(mesh);
    for (int i = 0; i < tag_names.size(); ++i)
        if (meshNameStr == tag_names[i])
            return NULL;

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
//      var      The desired varname.
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

    for (i = 0; i < materials.size(); i++)
    {
       std::map<std::string, vtkDataSet*>::iterator im;
       for (im = materials[i].begin(); im != materials[i].end(); im++)
            (im->second)->Delete();
       materials[i].clear();
    }
    Close();
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
        AddMaterialToMetaData(md,"Material",MESHNAME,
                  NumberOfMaterials(),m_names);

    for (i = 0; i < tag_names.size(); ++i)
    {
        avtMeshMetaData *mesh = new avtMeshMetaData;
        mesh->name = tag_names[i];
        mesh->meshType = AVT_UNSTRUCTURED_MESH;
        mesh->numBlocks = nFiles;
        mesh->blockOrigin = 0;
        mesh->spatialDimension = 3;
        mesh->topologicalDimension = tag_dim[i];
        mesh->blockTitle = "Files";
        mesh->blockNames = vFilenames;
        mesh->hasSpatialExtents = false;
        md->Add(mesh);
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
// ****************************************************************************

void avtKullLiteFileFormat::ReadInMaterialNames()
{
    m_names.clear(); // Just in case
    tag_names.clear();
    tag_dim.clear();
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

    //Read in the tags
    m_tags = MAKE_N(pdb_taglist, 1);
    if ((PD_read(m_pdbFile, "MeshTags", m_tags) == false) || !m_tags)
    {
        Close();
        if (nMaterials)
            EXCEPTION1(InvalidDBTypeException, "Cannot be a KullLite file, "
                        "does not have materials or mesh tags.");
        return;
    }

    // Let's get that data
    int curTagI;
    for (curTagI = 0; curTagI < nMaterials; curTagI++)
    {
        char *name = m_tags->tags[curTagI].tagname;

        int matNumber;
        for (matNumber = 0; matNumber < m_names.size(); matNumber++)
        {
            if (!strcmp(name, m_names[matNumber].c_str()))
                break;
        }
        if (matNumber == m_names.size()) // New material
        {
            m_names.push_back(name);
        }
    }


    int nTags = m_tags->num_tags;
    for (curTagI = nMaterials; curTagI < nTags; ++curTagI)
    {
        char *name = m_tags->tags[curTagI].tagname;
        switch (m_tags->tags[curTagI].type)
        {
            case TAG_NODE:
                tag_dim.push_back(0);
                break;
            case TAG_EDGE:
                tag_dim.push_back(1);
                break;
            case TAG_FACE:
                tag_dim.push_back(2);
                break;
            case TAG_ZONE:
                tag_dim.push_back(3);
                break;
        }
        
        int tagNumber;
        for (tagNumber = 0; tagNumber < tag_names.size(); tagNumber++)
        {
            if (!strcmp(name, tag_names[tagNumber].c_str()))
                break;
        }
        if (tagNumber == tag_names.size()) // New tag
        {
            tag_names.push_back(name);
        }
    }

    // We're done, clean up.
    Close();
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

    int  i, j;

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


