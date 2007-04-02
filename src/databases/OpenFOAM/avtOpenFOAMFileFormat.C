// ************************************************************************* //
//                            avtOpenFOAMFileFormat.C                           //
// ************************************************************************* //

#include <avtOpenFOAMFileFormat.h>
#include <avtDatabaseMetaData.h>
#include <Expression.h>
#include <InvalidVariableException.h>
#include <DebugStream.h>

// ****************************************************************************
//  Method: avtOpenFOAM constructor
//
// ****************************************************************************

avtOpenFOAMFileFormat::avtOpenFOAMFileFormat(const char *filename)
    : avtMTMDFileFormat(filename)
{
  debug5<<"OpenFOAM Reader:"<<endl;
  debug5<<"Constructor"<<endl;
  Path.append(filename);
  ReadControlDict();
  debug5<<"Number of TimeSteps: "<<NumberOfTimeSteps<<endl;
  PopulatePolyMeshDirArrays();
  CreateFaces = true;
  FirstVar = true;
  FirstVectorVar = true;
  debug5<<"Constructor Completed"<<endl;
}


// ****************************************************************************
//  Method: avtEMSTDFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
// ****************************************************************************

int avtOpenFOAMFileFormat::GetNTimesteps(void)
{
  return NumberOfTimeSteps;
}

// ****************************************************************************
//  Method: avtEMSTDFileFormat::GetTimes
//
//  Purpose:
//      Adds timesteps to times which provides the visible time stamp in Visit
//
// ****************************************************************************
void avtOpenFOAMFileFormat::GetTimes(std::vector<double> &times)
{
  for (int i = 0; i < NumberOfTimeSteps; i++)
    times.push_back(Steps[i]);
}

// ****************************************************************************
//  Method: avtOpenFOAMFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
// ****************************************************************************

void avtOpenFOAMFileFormat::FreeUpResources(void)
{
  CreateFaces = true;
  FirstVar = true;
  FirstVectorVar = true;
}


// ****************************************************************************
//  Method: avtOpenFOAMFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
// ****************************************************************************

void avtOpenFOAMFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)
{
  debug5<<"Populate MetaData."<<endl;
  FirstVar = true;
  FirstVectorVar = true;
  CreateFaces = true;
  //reinitialize vectors to null
  BoundaryNames.clear();
  BoundaryNames.resize(0);
  PointZoneNames.clear();
  PointZoneNames.resize(0);
  FaceZoneNames.clear();
  FaceZoneNames.resize(0);
  CellZoneNames.clear();
  CellZoneNames.resize(0);

  //get the names of the different blocks
  BoundaryNames = GatherBlocks(std::string("boundary"), timeState);
  PointZoneNames = GatherBlocks(std::string("pointZones"), timeState);
  FaceZoneNames = GatherBlocks(std::string("faceZones"), timeState);
  CellZoneNames = GatherBlocks(std::string("cellZones"), timeState);

  //Get the number of blocks in the diff regions
  NumBoundaries = BoundaryNames.size();
  NumPointZones = PointZoneNames.size();
  NumFaceZones = FaceZoneNames.size();
  NumCellZones = CellZoneNames.size();

  //setup the metadata
  avtMeshMetaData *mesh = new avtMeshMetaData;
  mesh->name = "Mesh";
  mesh->meshType = AVT_UNSTRUCTURED_MESH;
  mesh->hasSpatialExtents = false;
  mesh->topologicalDimension = 3;
  mesh->spatialDimension = 3;
  mesh->numBlocks = 1 + NumBoundaries + NumPointZones + NumFaceZones + NumCellZones;

  //add internal mesh to meta data- always has one
  mesh->blockNames.push_back("InternalMesh");

  debug5<<"Domains:"<<endl;
  //add boundary meshes to meta data
  for(int i = 0; i < NumBoundaries; i++)
    {
    debug5<<i<<" : "<< BoundaryNames[i].c_str()<<endl;
    mesh->blockNames.push_back(BoundaryNames[i].c_str());
    }

  //add point zone meshes to meta data
  for(int i = 0; i < NumPointZones; i++)
    {
    debug5<<i<<" : "<< PointZoneNames[i].c_str()<<endl;
    mesh->blockNames.push_back(PointZoneNames[i].c_str());
    }

  //add face zone meshes to meta data
  for(int i = 0; i < NumFaceZones; i++)
    {
    debug5<<i<<" : "<< FaceZoneNames[i].c_str()<<endl;
    mesh->blockNames.push_back(FaceZoneNames[i].c_str());
    }

  //add cell zone meshes to meta data
  for(int i = 0; i < NumCellZones; i++)
    {
    debug5<<i<<" : "<< CellZoneNames[i].c_str()<<endl;
    mesh->blockNames.push_back(CellZoneNames[i].c_str());
    }

  //add the blocks information to the metadata
  md->Add(mesh);

  //Add scalars and vectors to metadata
  //create path to current time step
  std::stringstream tempPath;
  tempPath << PathPrefix;
  tempPath << Steps[timeState];

  //open the directory and get num of files
  int numSolvers;
  vtkDirectory * directory = vtkDirectory::New();
  int opened = directory->Open(tempPath.str().c_str());
  if(opened)
    numSolvers = directory->GetNumberOfFiles();
  else
    numSolvers = -1; //no dir

  //loop over all files and locate
  //volScalars and volVectors
  debug5<<"Add variables to metadata:"<<endl;
  for(int j = 0; j < numSolvers; j++)
    {
    const char * tempSolver = directory->GetFile(j);
    if(tempSolver !="polyMesh")
      {
      if(tempSolver != "." && tempSolver != "..")
        {
        if(GetDataType(tempPath.str(), std::string(tempSolver)) == "Scalar")
          {
          debug5 << "Scalar: "<<tempSolver<<endl;
          AddScalarVarToMetaData(md, tempSolver, "Mesh", AVT_ZONECENT);
          }
        else if(GetDataType(tempPath.str(), std::string(tempSolver)) == "Vector")
          {
          debug5 << "Vector: "<<tempSolver<<endl;
          AddVectorVarToMetaData(md, tempSolver, "Mesh", AVT_ZONECENT);
          }
        }
      }
    }
  directory->Delete();
  debug5<<"meta data populated"<<endl;
}


// ****************************************************************************
//  Method: avtOpenFOAMFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      timestate   The index of the timestate.  If GetNTimesteps returned
//                  'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain      The index of the domain.  If there are NDomains, this
//                  value is guaranteed to be between 0 and NDomains-1,
//                  regardless of block origin.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
// ****************************************************************************

vtkDataSet * avtOpenFOAMFileFormat::GetMesh(int timestate, int domain, const char *meshname)
{
  //CreateFaces = true;
  debug5 << "Get domain: " <<domain<<endl;
  //internal mesh
  if(domain == 0)
    {
    debug5<<"InternalMesh"<<endl;
    return MakeInternalMesh();
    }

  //boundary meshes
  else if (domain <= NumBoundaries)
    {
    debug5<<BoundaryNames[domain -1]<<endl;
    return GetBoundaryMesh(timestate, domain - 1);
    }

  //point zone meshes
  else if (domain <= NumBoundaries + NumPointZones)
    {
    debug5<<PointZoneNames[domain - 1 - NumBoundaries]<<endl;
    return GetPointZoneMesh(timestate, domain - 1 - NumBoundaries);
    }

  //face zone meshes
  else if (domain <= NumBoundaries + NumPointZones + NumFaceZones)
    {
    debug5<<FaceZoneNames[domain - 1 - NumBoundaries - NumPointZones]<<endl;
    return GetFaceZoneMesh(timestate, domain - 1 - NumBoundaries - NumPointZones);
    }

  //cell zone meshes
  else if (domain <= NumBoundaries + NumPointZones + NumFaceZones + NumCellZones)
    {
    debug5<<CellZoneNames[domain - 1 - NumBoundaries - NumPointZones - NumFaceZones]<<endl;
    return GetCellZoneMesh(timestate, domain - 1 - NumBoundaries - NumPointZones - NumFaceZones);
    }
  debug5<<"got domain: "<<domain<<endl;
}


// ****************************************************************************
//  Method: avtOpenFOAMFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
// ****************************************************************************

vtkDataArray * avtOpenFOAMFileFormat::GetVar(int timestate, int domain, const char *varname)
{
  debug5<<"Get var: "<<varname<<endl;

  //Populate the tempdata with internal mesh data
  //necessary for zero gradient boundaries
  if(FirstVar)
    {
    TempData = GetInternalVariableAtTimestep(std::string(varname), timestate);
    FirstVar = false;
    }

  //internal domain scalar data
  if(domain == 0)
    //return GetInternalVariableAtTimestep(std::string(varname), timestate);
    return TempData;

  //boundary domains scalar data
  else if (domain <= NumBoundaries)
    return GetBoundaryVariableAtTimestep(domain-1, std::string(varname), timestate);

  //no other domains have scalar data - return null
  else
    return NULL;
}


// ****************************************************************************
//  Method: avtOpenFOAMFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a std::vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: root -- generated by xml2avt
//  Creation:   Wed Jun 7 16:01:15 PST 2006
//
// ****************************************************************************

vtkDataArray *  avtOpenFOAMFileFormat::GetVectorVar(int timestate, int domain, const char *varname)
{
  debug5<<"Get Vector Var: "<<varname<<endl;

  //Populate the tempdata with internal mesh data
  //necessary for zero gradient boundaries
  if(FirstVectorVar)
    {
    TempData = GetInternalVariableAtTimestep(std::string(varname), timestate);
    FirstVectorVar = false;
    }

  //internal domain vector data  
  if(domain == 0)
    //return GetInternalVariableAtTimestep(std::string(varname), timestate);
    return TempData;

  //boundary domains vector data
  else if (domain <= NumBoundaries)
    return GetBoundaryVariableAtTimestep(domain-1, std::string(varname), timestate);

  //no other domains have vector data - return null
  else
    return NULL;
}

// ****************************************************************************
//  Method: avtOpenFOAMFileFormat::CombineOwnerNeigbor
//
//  Purpose:
//  add Owner faces to the faces of a cell and then add the neighor faces
//
// ****************************************************************************
void avtOpenFOAMFileFormat::CombineOwnerNeigbor()
{
  debug5<<"Combine owner & neighbor faces."<<endl;
  //reintialize faces of the cells
  face tempFace;
  FacesOfCell.clear();
  FacesOfCell.resize(NumCells);

  //add owner faces to cell
  for(int i = 0; i < (int)FacesOwnerCell.size(); i++)
    {
    for(int j = 0; j < (int)FacesOwnerCell[i].size(); j++)
      {
      tempFace.faceIndex = FacesOwnerCell[i][j];
      tempFace.neighborFace = false;
      FacesOfCell[i].push_back(tempFace);
      }
    }

  //add neighbor faces to cell
  for(int i = 0; i < (int)FacesNeighborCell.size(); i++)
    {
    for(int j = 0; j < (int)FacesNeighborCell[i].size(); j++)
      {
      tempFace.faceIndex = FacesNeighborCell[i][j];
      tempFace.neighborFace = true;
      FacesOfCell[i].push_back(tempFace);
      }
    }

  //clean up memory
  FacesOwnerCell.clear();
  FacesNeighborCell.clear();
}

// ****************************************************************************
//  Method: avtOpenFOAMFileFormat::MakeInternalMesh
//
//  Purpose:
//  derive cell types and create the internal mesh
//
// ****************************************************************************
vtkUnstructuredGrid * avtOpenFOAMFileFormat::MakeInternalMesh()
{
  debug5<<"Make internal mesh."<<endl;
  //initialize variables
  bool foundDup = false;
  std::vector< int > cellPoints;
  std::vector< int > tempFaces[2];
  std::vector< int > firstFace;
  int pivotPoint = 0;
  int i, j, k, l, pCount;
  int faceCount = 0;

  //Create Mesh
  vtkUnstructuredGrid *  internalMesh = vtkUnstructuredGrid::New();

  //loop through each cell, derive type and insert it into the mesh
  //hexahedron, prism, pyramid, tetrahedron, wedge&tetWedge
  for(i = 0; i < (int)FacesOfCell.size(); i++)  //each cell
    {

    //calculate the total points for the cell
    //used to derive cell type
    int totalPointCount = 0;
    for(j = 0; j < (int)FacesOfCell[i].size(); j++)  //each face
      {
      totalPointCount += 
        (int)FacePoints[FacesOfCell[i][j].faceIndex].size();
      }

    // using cell type - order points, create cell, & add to mesh

    //OFhex | vtkHexahedron
    if (totalPointCount == 24) 
      {
      faceCount = 0;

      //get first face
      for(j = 0; j <
        (int)FacePoints[FacesOfCell[i][0].faceIndex].size(); j++)
        {
        firstFace.push_back(FacePoints[
          FacesOfCell[i][0].faceIndex][j]);
        }

      //patch: if it is a neighbor face flip the points
      if(FacesOfCell[i][0].neighborFace)
        {
        int tempPop;
        for(k = 0; k < firstFace.size() - 1; k++)
          {
          tempPop = firstFace[firstFace.size()-1];
          firstFace.pop_back();
          firstFace.insert(firstFace.begin()+1+k, tempPop);
          }
        }

      //add first face to cell points
      for(j =0; j < (int)firstFace.size(); j++)
        cellPoints.push_back(firstFace[j]);

      //find the opposite face and order the points correctly
      for(int pointCount = 0; pointCount < (int)firstFace.size(); pointCount++)
        {

        //find the other 2 faces containing each point
        for(j = 1; j < (int)FacesOfCell[i].size(); j++)  //each face
          {
          for(k = 0; k < (int)FacePoints[
            FacesOfCell[i][j].faceIndex].size(); k++) //each point
            {
            if(firstFace[pointCount] == FacePoints[
              FacesOfCell[i][j].faceIndex][k])
              {
              //ANOTHER FACE WITH THE POINT
              for(l = 0; l < (int)FacePoints[
                FacesOfCell[i][j].faceIndex].size(); l++) //each point
                {
                tempFaces[faceCount].push_back(FacePoints[
                  FacesOfCell[i][j].faceIndex][l]);
                }
              faceCount++;
              }
            }
          }

        //locate the pivot point contained in faces 0 & 1
        for(j = 0; j < (int)tempFaces[0].size(); j++)
          {
          for(k = 0; k < (int)tempFaces[1].size(); k++)
            {
            if(tempFaces[0][j] == tempFaces[1][k] && tempFaces[0][j] != 
              firstFace[pointCount])
              {
              pivotPoint = tempFaces[0][j];
              break;
              }
            }
          }
        cellPoints.push_back(pivotPoint);
        tempFaces[0].clear();
        tempFaces[1].clear();
        faceCount=0;
        }

      //create the hex cell and insert it into the mesh
      vtkHexahedron * hexahedron= vtkHexahedron::New();
      for(pCount = 0; pCount < (int)cellPoints.size(); pCount++)
        {
        hexahedron->GetPointIds()->SetId(pCount, cellPoints[pCount]);
        }
      internalMesh->InsertNextCell(hexahedron->GetCellType(),
        hexahedron->GetPointIds());
      hexahedron->Delete();
      cellPoints.clear();
      firstFace.clear();
      }

    //OFprism | vtkWedge
    else if (totalPointCount == 18) 
      {
      faceCount = 0;
      int index = 0;

      //find first triangular face
      for(j = 0; j < (int)FacesOfCell[i].size(); j++)  //each face
        {
        if((int)FacePoints[FacesOfCell[i][j].faceIndex].size() == 3)
          {
          for(k = 0; k < (int)FacePoints[
            FacesOfCell[i][j].faceIndex].size(); k++)
            {
            firstFace.push_back(FacePoints[
              FacesOfCell[i][j].faceIndex][k]);
            index = j;
            }
          break;
          }
        }

      //patch: if it is a neighbor face flip the points
      if(FacesOfCell[i][0].neighborFace)
        {
        int tempPop;
        for(k = 0; k < firstFace.size() - 1; k++)
          {
          tempPop = firstFace[firstFace.size()-1];
          firstFace.pop_back();
          firstFace.insert(firstFace.begin()+1+k, tempPop);
          }
        }

      //add first face to cell points
      for(j =0; j < (int)firstFace.size(); j++)
        cellPoints.push_back(firstFace[j]);

      //find the opposite face and order the points correctly
      for(int pointCount = 0; pointCount < (int)firstFace.size(); pointCount++)
        {

        //find the 2 other faces containing each point
        for(j = 0; j < (int)FacesOfCell[i].size(); j++)  //each face
          {
          for(k = 0; k < (int)FacePoints[
            FacesOfCell[i][j].faceIndex].size(); k++) //each point
            {
            if(firstFace[pointCount] == FacePoints[
              FacesOfCell[i][j].faceIndex][k] && j != index)
              {
              //ANOTHER FACE WITH POINT
              for(l = 0; l < (int)FacePoints[
                FacesOfCell[i][j].faceIndex].size(); l++) //each point
                {
                tempFaces[faceCount].push_back(FacePoints[
                  FacesOfCell[i][j].faceIndex][l]);
                }
              faceCount++;
              }
            }
          }

        //locate the pivot point of faces 0 & 1
        for(j = 0; j < (int)tempFaces[0].size(); j++)
          {
          for(k = 0; k < (int)tempFaces[1].size(); k++)
            {
            if(tempFaces[0][j] == tempFaces[1][k] && tempFaces[0][j] !=
              firstFace[pointCount])
              {
              pivotPoint = tempFaces[0][j];
              break;
              }
            }
          }
        cellPoints.push_back(pivotPoint);
        tempFaces[0].clear();
        tempFaces[1].clear();
        faceCount=0;
        }

      //create the wedge cell and insert it into the mesh
      vtkWedge * wedge= vtkWedge::New();
      for(pCount = 0; pCount < (int)cellPoints.size(); pCount++)
        {
        wedge->GetPointIds()->SetId(pCount, cellPoints[pCount]);
        }
      internalMesh->InsertNextCell(wedge->GetCellType(),
        wedge->GetPointIds());
      cellPoints.clear();
      wedge->Delete();
      firstFace.clear();
      }

    //OFpyramid | vtkPyramid
    else if (totalPointCount == 16) 
      {
      foundDup = false;

      //find the quadratic face
      for(j = 0; j < (int)FacesOfCell[i].size(); j++)  //each face
        {
        if((int)FacePoints[FacesOfCell[i][j].faceIndex].size() == 4)
          {
          for(k = 0; k < (int)FacePoints[
            FacesOfCell[i][j].faceIndex].size(); k++)
            {
            cellPoints.push_back(FacePoints[
              FacesOfCell[i][j].faceIndex][k]);
            }
          break;
          }
        }

      //compare first face points to other faces
      for(j = 0; j < (int)cellPoints.size(); j++) //each point
        {
        for(k = 0; k < (int)FacePoints[
          FacesOfCell[i][1].faceIndex].size(); k++)
          {
          if(cellPoints[j] == FacePoints[
            FacesOfCell[i][1].faceIndex][k])
            {
            foundDup = true;
            }
          }
        if(!foundDup)
          {
          cellPoints.push_back(FacePoints[
            FacesOfCell[i][j].faceIndex][k]);
          break;
          }
        }

      //create the pyramid cell and insert it into the mesh
      vtkPyramid * pyramid = vtkPyramid::New();
      for(pCount = 0; pCount < (int)cellPoints.size(); pCount++)
        {
        pyramid->GetPointIds()->SetId(pCount, cellPoints[pCount]);
        }
      internalMesh->InsertNextCell(pyramid->GetCellType(), 
        pyramid->GetPointIds());
      cellPoints.clear();
      pyramid->Delete();
      }

    //OFtet | vtkTetrahedron
    else if (totalPointCount == 12) 
      {
      foundDup = false;

      //add first face to cell points
      for(j = 0; j < (int)FacePoints[
        FacesOfCell[i][0].faceIndex].size(); j++)
        {
        cellPoints.push_back(FacePoints[
          FacesOfCell[i][0].faceIndex][j]);
        }

      //compare first face to the points of second face
      for(j = 0; j < (int)cellPoints.size(); j++) //each point
        {
        for(k = 0; k < (int)FacePoints[
          FacesOfCell[i][1].faceIndex].size(); k++)
          {
          if(cellPoints[j] == FacePoints[
            FacesOfCell[i][1].faceIndex][k])
            {
            foundDup = true;
            }
          }
        if(!foundDup)
          {
          cellPoints.push_back(FacePoints[
            FacesOfCell[i][j].faceIndex][k]);
          break;
          }
        }

      //create the wedge cell and insert it into the mesh
      vtkTetra * tetra = vtkTetra::New();
      for(pCount = 0; pCount < (int)cellPoints.size(); pCount++)
        {
        tetra->GetPointIds()->SetId(pCount, cellPoints[pCount]);
        }
      internalMesh->InsertNextCell(tetra->GetCellType(), 
        tetra->GetPointIds());
      cellPoints.clear();
      tetra->Delete();
      }

    //OFpolyhedron || vtkConvexPointSet
    else
      {
      cout<<"Warning: Polyhedral Data is very Slow!"<<endl;
      foundDup = false;

      //get first face
      for(j = 0; j < (int)FacePoints[
        FacesOfCell[i][0].faceIndex].size(); j++)
        {
        firstFace.push_back(FacePoints[
          FacesOfCell[i][0].faceIndex][j]);
        }

      //add first face to cell points
      for(j =0; j < (int)firstFace.size(); j++)
        cellPoints.push_back(firstFace[j]);

      //loop through faces and create a list of all points
      //j = 1 skip firstFace
      for(j = 1; j < (int) FacesOfCell[i].size(); j++)
        {

        //remove duplicate points from faces
        for(k = 0; k < (int)FacePoints[
          FacesOfCell[i][j].faceIndex].size(); k++)
          {
          for(l = 0; l < (int)cellPoints.size(); l++);
            {
            if(cellPoints[l] == FacePoints[
              FacesOfCell[i][j].faceIndex][k])
              {
              foundDup = true;
              }
            }
          if(!foundDup)
            {
            cellPoints.push_back(FacePoints[
              FacesOfCell[i][j].faceIndex][k]); 
            foundDup = false;
            }
          }
        }

      //create the poly cell and insert it into the mesh
      vtkConvexPointSet * poly = vtkConvexPointSet::New();
      poly->GetPointIds()->SetNumberOfIds(cellPoints.size());
      for(pCount = 0; pCount < (int)cellPoints.size(); pCount++)
        {
        poly->GetPointIds()->SetId(pCount, cellPoints[pCount]);
        }
      internalMesh->InsertNextCell(poly->GetCellType(), 
        poly->GetPointIds());
      cellPoints.clear();
      firstFace.clear();
      poly->Delete();
      }
    }

  //set the internal mesh points
  internalMesh->SetPoints(Points);
  debug5<<"Internal mesh made."<<endl;
  return internalMesh;
}

// ****************************************************************************
//  Method: avtOpenFOAMFileFormat::ControlDictDataParser
//
//  Purpose:
//  parse out double values for controlDict entries
//  utility function
//
// ****************************************************************************
double avtOpenFOAMFileFormat::ControlDictDataParser(std::string line)
{
  double value;
  line.erase(line.begin()+line.find(";"));
  std::string token;
  std::stringstream tokenizer(line);

  //parse to the final entry - double
  while(tokenizer>>token);
  std::stringstream conversion(token);
  conversion>>value;
  return value;
}

// ****************************************************************************
//  Method: avtOpenFOAMFileFormat::ReadControlDict
//
//  Pupose:
//  reads the controlDict File
//  gather the necessary information to create a path to the data
//
// ****************************************************************************
void avtOpenFOAMFileFormat::ReadControlDict ()
{
  debug5<<"Read controlDict."<<endl;
  //create variables
  ifstream input;
  std::string temp;
  double startTime;
  double endTime;
  double deltaT;
  double writeInterval;
  double timeStepIncrement;
  std::string writeControl;
  std::string timeFormat;

  input.open(Path.c_str(),ios::in);

  //create the path to the data directory
  PathPrefix = Path;
  PathPrefix.erase(PathPrefix.begin()+
    PathPrefix.find("system"),PathPrefix.end());
  debug5<<"Path: "<<PathPrefix<<endl;

  //find Start Time
  getline(input, temp);
  while(!(temp.compare(0,8,"startTime",0,8) == 0))
    getline(input, temp);
  startTime = ControlDictDataParser(temp);
  debug5 <<"Start Time: "<< startTime<<endl;

  //find End Time
  while(!(temp.compare(0,6,"endTime",0,6) == 0))
    getline(input, temp);
  endTime = ControlDictDataParser(temp);
  debug5 <<"End Time: "<< endTime<<endl;

  //find Delta T
  while(!(temp.compare(0,5,"deltaT",0,5) == 0))
    getline(input, temp);
  deltaT = ControlDictDataParser(temp);
  debug5 <<"deltaT: "<< deltaT<<endl;

  //find write control
  while(!(temp.compare(0,11,"writeControl",0,11) == 0))
    getline(input, temp);
  temp.erase(temp.begin()+temp.find(";"));
  std::string token;
  std::stringstream tokenizer(temp);
  while(tokenizer>>token);
  writeControl = token;
  debug5 <<"Write Control: "<< writeControl<<endl;

  //find write interval
  while(!(temp.compare(0,12,"writeInterval",0,12) == 0))
    getline(input, temp);
  writeInterval = ControlDictDataParser(temp);
  debug5 <<"Write Interval: "<< writeInterval<<endl;  

  //calculate the time step increment based on type of run
  if(writeControl.compare(0,7,"timeStep",0,7) == 0)
    {
    debug5<<"Time Step Type Data"<<endl;
    timeStepIncrement = writeInterval * deltaT;
    }
  else
    {
    debug5<<"Run Time Type Data"<<endl;
    timeStepIncrement = writeInterval;
    }

  //find time format
  while(temp.find("timeFormat") == std::string::npos)
    getline(input, temp);
  timeFormat = temp;

  //calculate how many timesteps there should be
  float tempResult = ((endTime-startTime)/timeStepIncrement);
  int tempNumTimeSteps = (int)(tempResult+0.1)+1;  //+0.1 to round up

  //make sure time step dir exists
  std::vector< double > tempSteps;
  ifstream test;
  std::stringstream parser;
  double tempStep;
  for(int i = 0; i < tempNumTimeSteps; i++)
    {
    tempStep = i*timeStepIncrement + startTime;
    parser.str("");
    parser.clear();
    if(timeFormat.find("general")!= std::string::npos)
      parser << tempStep;
    else
      parser << std::scientific <<tempStep;
    test.open((PathPrefix+parser.str()).c_str(),ios::in);
    if(!test.fail())
      tempSteps.push_back(tempStep);
    test.close();
    }

  //Add the time steps that actually exist to steps
  //allows the run to be stopped short of controlDict spec
  //allows for removal of timesteps
  NumberOfTimeSteps = tempSteps.size();
  Steps = new double[NumberOfTimeSteps];
  for(int i = 0; i < NumberOfTimeSteps; i++)
    {
    Steps[i] =tempSteps[i];
    }

  input.close();
  debug5<<"controlDict read."<<endl;
}

// ****************************************************************************
//  Method: avtOpenFOAMFileFormat::GetPoints
//
//  Purpose:
//  read the points file into a vtkPoints
//
// ****************************************************************************
void avtOpenFOAMFileFormat::GetPoints (int timeState)
{
  //path to points file
  std::string pointPath = PathPrefix + PolyMeshPointsDir[timeState] + "/polyMesh/points";
  debug5<<"Read points file: "<<pointPath<<endl;

  std::string temp;
  ifstream input;
  bool binaryWriteFormat;
  input.open(pointPath.c_str(),ios::in);
  //make sure file exists
  if(input.fail())
    return;

  //determine if file is binary or ascii
  while(temp.find("format") == std::string::npos)
    getline(input, temp);
  input.close();

  //reopen file in correct format
  if(temp.find("binary") != std::string::npos)
    {
    input.open(pointPath.c_str(),ios::binary);
    binaryWriteFormat = true;
    }
  else
    {
    input.open(pointPath.c_str(),ios::in);
    binaryWriteFormat = false;
    }

  double x,y,z;
  std::stringstream tokenizer;

  //instantiate the points class
  Points = vtkPoints::New();

  //find end of header
  while(temp.compare(0,4, "// *", 0, 4) != 0)
    getline(input, temp);

  //find number of points
  getline(input, temp);
  while(temp.empty())
    getline(input, temp);

  //read number of points
  tokenizer.clear();
  tokenizer.str(temp);
  tokenizer>>NumPoints;

  //binary data
  if(binaryWriteFormat)
    {
    char paren = input.get();
    for(int i = 0; i < NumPoints; i++)
      {
      input.read((char *)&x,sizeof(double));
      input.read((char *)&y,sizeof(double));
      input.read((char *)&z,sizeof(double));
      Points->InsertPoint(i,x,y,z);
      }
    }

  //ascii data
  else
    {
    getline(input, temp); //THROW OUT "("
    for(int i = 0; i < NumPoints; i++)
      {
      getline(input, temp);
      temp.erase(temp.begin()+temp.find("("));
      temp.erase(temp.begin()+temp.find(")"));
      tokenizer.clear();
      tokenizer.str(temp);
      tokenizer>>x;
      tokenizer>>y;
      tokenizer>>z;
      Points->InsertPoint(i,x,y,z);
      }
    }

  input.close();
  debug5<<"Point file read."<<endl;
}

// ****************************************************************************
//  Method: avtOpenFOAMFileFormat::ReadFacesFile
//
//  Purpose:
//  read the faces into a std::vector
//
// ****************************************************************************
void avtOpenFOAMFileFormat::ReadFacesFile (std::string facePath)
{
  debug5<<"Read faces: "<<facePath<<endl;
  std::string temp;
  ifstream input;
  bool binaryWriteFormat;
  input.open(facePath.c_str(),ios::in);
  //make sure file exists
  if(input.fail())
    return;

  //determine if file is binary or ascii
  while(temp.find("format") == std::string::npos)
    getline(input, temp);
  input.close();

  //reopen file in correct format
  if(temp.find("binary") != std::string::npos)
    {
    input.open(facePath.c_str(),ios::binary);
    binaryWriteFormat = true;
    }
  else
    {
    input.open(facePath.c_str(),ios::in);
    binaryWriteFormat = false;
    }

  std::istringstream tokenizer;
  size_t pos;
  int numFacePoints;
  FacePoints.clear();

  //find end of header
  while(temp.compare(0,4, "// *", 0, 4) != 0)
    getline(input, temp);

  //find number of faces
  getline(input, temp);
  while(temp.empty())
    getline(input, temp);

  //read number of faces
  tokenizer.clear();
  tokenizer.str(temp);
  tokenizer>>NumFaces;

  FacePoints.resize(NumFaces);

  getline(input, temp);//THROW OUT "("

  //binary data
  if(binaryWriteFormat)
    {
    char paren;
    int tempPoint;
    for(int i = 0; i < NumFaces; i++)
      {
      getline(input, temp);//THROW OUT blankline
      getline(input, temp); //grab point count
      tokenizer.clear();
      tokenizer.str(temp);
      tokenizer >> numFacePoints;
      FacePoints[i].resize(numFacePoints);
      paren = input.get();  //grab (
      for(int j = 0; j < numFacePoints; j++)
        {
        input.read((char *) &tempPoint, sizeof(int));
        FacePoints[i][j] = tempPoint;
        }
      getline(input, temp); //throw out ) and rest of line
      }
    }

  //ascii data
  else
    {
    //create std::vector of points in each face
    for(int i = 0; i < NumFaces; i++)
      {
      getline(input, temp);
      pos = temp.find("(");
      tokenizer.clear();
      tokenizer.str(temp.substr(0, pos));
      temp.erase(0, pos+1);
      tokenizer>>numFacePoints;
      FacePoints[i].resize(numFacePoints);
      for(int j = 0; j < numFacePoints; j++)
        {
        pos = temp.find(" ");
        tokenizer.clear();
        tokenizer.str(temp.substr(0, pos));
        temp.erase(0, pos+1);
        tokenizer>>FacePoints[i][j];
        }
      }
    }

  input.close();
  debug5<<"Faces read."<<endl;
}

// ****************************************************************************
//  Method: avtOpenFOAMFileFormat::ReadOwnerFile
//
//  Purpose:
//  read the owner file into a std::vector
//
// ****************************************************************************
void avtOpenFOAMFileFormat::ReadOwnerFile(std::string ownerPath)
{
  debug5<<"Read owner file: "<<ownerPath<<endl;
  std::string temp;
  ifstream input;
  bool binaryWriteFormat;
  input.open(ownerPath.c_str(),ios::in);
  //make sure file exists
  if(input.fail())
    return;

  //determine if file is binary or ascii
  while(temp.find("format") == std::string::npos)
    getline(input, temp);
  input.close();

  //reopen file in correct format
  if(temp.find("binary") != std::string::npos)
    {
    input.open(ownerPath.c_str(),ios::binary);
    binaryWriteFormat = true;
    }
  else
    {
    input.open(ownerPath.c_str(),ios::in);
    binaryWriteFormat = false;
    }

  std::string numFacesStr;
  std::stringstream tokenizer;
  int faceValue;

  FaceOwner = vtkIntArray::New();

  tokenizer<<NumFaces;
  tokenizer>>numFacesStr;

  //find end of header & number of faces
  while(temp.compare(0,numFacesStr.size(),numFacesStr, 0, numFacesStr.size()) != 0)
    getline(input, temp);

  FaceOwner->SetNumberOfValues(NumFaces);

  //binary data
  if(binaryWriteFormat)
    {
    char paren = input.get();
    for(int i = 0; i < NumFaces; i++)
      {
      input.read((char *) &faceValue, sizeof(int));
      FaceOwner->SetValue(i, faceValue);
      }
    }

  //ascii data
  else
    {
    getline(input, temp);//throw away (
    //read face owners into int array
    for(int i = 0; i < NumFaces; i++)
      {
      getline(input, temp);
      tokenizer.clear();
      tokenizer.str(temp);
      tokenizer>>faceValue;
      FaceOwner->SetValue(i, faceValue);
      }
    }

  //find the number of cells
  double  * range;
  range = FaceOwner->GetRange();
  NumCells = (int)range[1]+1;

  //add the face number to the correct cell
  //according to owner
  FacesOwnerCell.resize(NumCells);
  int tempCellId;
  for(int j = 0; j < NumFaces; j++)
    {
    tempCellId = FaceOwner->GetValue(j);
    if(tempCellId != -1)
      FacesOwnerCell[tempCellId].push_back(j);
    }

  //FaceOwner->Delete();
  input.close();
  debug5<<"Owner file read."<<endl;
}

// ****************************************************************************
//  Method: avtOpenFOAMFileFormat::ReadNeighborFile
//
//  Purpose:
//  read the neighbor file into a std::vector
//
// ****************************************************************************
void avtOpenFOAMFileFormat::ReadNeighborFile(std::string neighborPath)
{
  debug5<<"Read neighbor file: "<<neighborPath<<endl;
  std::string temp;
  ifstream input;
  bool binaryWriteFormat;
  input.open(neighborPath.c_str(),ios::in);
  //make sure file exists
  if(input.fail())
    return;

  //determine if file is binary or ascii
  while(temp.find("format") == std::string::npos)
    getline(input, temp);
  input.close();

  //reopen file in correct format
  if(temp.find("binary") != std::string::npos)
    {
    input.open(neighborPath.c_str(),ios::binary);
    binaryWriteFormat = true;
    }
  else
    {
    input.open(neighborPath.c_str(),ios::in);
    binaryWriteFormat = false;
    }

  std::string numFacesStr;
  std::stringstream tokenizer;
  int faceValue;

  FaceNeighbor = vtkIntArray::New();

  tokenizer<<NumFaces;
  tokenizer>>numFacesStr;

  //find end of header & number of faces
  while(temp.compare(0,numFacesStr.size(),numFacesStr, 0, numFacesStr.size()) != 0)
    getline(input, temp);

  //read face owners into int array
  FaceNeighbor->SetNumberOfValues(NumFaces);

  //binary data
  if(binaryWriteFormat)
    {
    char paren = input.get();
    for(int i = 0; i < NumFaces; i++)
      {
      input.read((char *) &faceValue, sizeof(int));
      FaceNeighbor->SetValue(i, faceValue);
      }
    }

  //ascii data
  else
    {
    getline(input, temp);//throw away (
    //read face owners into int array
    for(int i = 0; i < NumFaces; i++)
      {
      getline(input, temp);
      tokenizer.clear();
      tokenizer.str(temp);
      tokenizer>>faceValue;
      FaceNeighbor->SetValue(i, faceValue);
      }
    }

////recalculate the number of cells just in case
//float * range;
//range  = FaceNeighbor->GetRange();
//NumCells = (int)range[1]+1;

  //No need to recalulate the Number of Cells
  FacesNeighborCell.resize(NumCells);

  //add face number to correct cell
  int tempCellId;
  for(int j = 0; j < NumFaces; j++)
    {
    tempCellId = FaceNeighbor->GetValue(j);
    if(tempCellId != -1)
      FacesNeighborCell[tempCellId].push_back(j);
    }

  FaceNeighbor->Delete();
  input.close();
  debug5<<"Neighbor file read."<<endl;
}

// ****************************************************************************
//  Method: avtOpenFOAMFileFormat::PopulatePolyMeshDirArrays
//
//  Purpose:
//  create a Lookup Table containing the location of the points
//  and faces files for each time steps mesh
//
// ****************************************************************************
void avtOpenFOAMFileFormat::PopulatePolyMeshDirArrays()
{
  debug5<<"Create list of points/faces file directories."<<endl;
  std::ostringstream path;
  std::stringstream timeStep;
  int numSolvers;
  bool facesFound;
  bool pointsFound;
  bool polyMeshFound;
  ifstream input;

  //intialize size to number of timesteps
  PolyMeshPointsDir.resize(NumberOfTimeSteps);
  PolyMeshFacesDir.resize(NumberOfTimeSteps);

  //loop through each timestep
  for(int i = 0; i < NumberOfTimeSteps; i++)
    {
    polyMeshFound = false;
    facesFound = false;
    pointsFound = false;

    //create the path to the timestep
    path.clear();
    path.str("");
    timeStep.clear();
    timeStep.str("");
    timeStep << Steps[i];
    path << PathPrefix <<timeStep.str() << "/";

    //get the number of files
    vtkDirectory * directory = vtkDirectory::New();
    directory->Open(path.str().c_str());
    int numFiles = directory->GetNumberOfFiles();

    //Look for polyMesh Dir
    for(int j = 0; j < numFiles; j++)
      {
      std::string tempFile = std::string(directory->GetFile(j));
      if(tempFile.find("polyMesh") != std::string::npos)
        {
        polyMeshFound = true;

        path << "polyMesh/";

        //get number of files in the polyMesh dir
        vtkDirectory * polyMeshDirectory = vtkDirectory::New();
        polyMeshDirectory->Open(path.str().c_str());
        int numPolyMeshFiles = polyMeshDirectory->GetNumberOfFiles();

        //Look for points/faces files
        for(int k = 0; k < numPolyMeshFiles; k++)
          {
          std::string tempFile2 = std::string(polyMeshDirectory->GetFile(k));
          if(tempFile2.find("points") != std::string::npos)
            {
            PolyMeshPointsDir[i] = timeStep.str();
            pointsFound = true;
            }
          else if(tempFile2.find("faces") != std::string::npos)
            {
            PolyMeshFacesDir[i] = timeStep.str();
            facesFound = true;
            }
          }

        //if there is no points or faces found in this timestep
        //set it equal to previous time step if no previous
        //set it equal to "constant" dir
        if(!pointsFound)
          {
          if(i != 0)
            PolyMeshPointsDir[i] = PolyMeshPointsDir[i-1];
          else
            PolyMeshPointsDir[i] = std::string("constant");
          }
        if(!facesFound)
          {
          if(i != 0)
            PolyMeshFacesDir[i] = PolyMeshFacesDir[i-1];
          else
            PolyMeshFacesDir[i] = std::string("constant");
          }

        polyMeshDirectory->Delete();
        break;  //found - stop looking
        }
      }

    //if there is no polyMesh dir
    //set  it equal to prev timestep
    //if no prev set to "constant" dir
    if(!polyMeshFound)
      {
      if(i != 0)
        {
        //set points/faces location to previous timesteps value
        PolyMeshPointsDir[i] = PolyMeshPointsDir[i-1];
        PolyMeshFacesDir[i] = PolyMeshFacesDir[i-1];
        }
      else
        {
        //set points/faces to constant
        PolyMeshPointsDir[i] = std::string("constant");
        PolyMeshFacesDir[i] = std::string("constant");
        }
      }
    directory->Delete();
    }

//cout<<PolyMeshPointsDir.size()<< " : "<< PolyMeshFacesDir.size()<<endl;
//for(int z= 0; z < PolyMeshPointsDir.size(); z++)
//cout<< z<<" : "<< PolyMeshPointsDir[z]<<" : "<<PolyMeshFacesDir[z]<<endl;

  debug5<<"Points/faces list created."<<endl;
}

// ****************************************************************************
//  Method: avtOpenFOAMFileFormat::GetDataType
//
//  Purpose:
//  determines whether a variable is a volume scalar, vector or neither
//  for meta data
//
// ****************************************************************************
std::string avtOpenFOAMFileFormat::GetDataType(std::string path, std::string fileName)
{
  std::string filePath = path+"/"+fileName;
  debug5<<"Get data type of: "<<filePath<<endl;
  ifstream input;
  input.open(filePath.c_str(),ios::in);
  //make sure file exists
  if(input.fail())  return "Null";

  std::string temp;
  std::string foamClass;
  std::stringstream tokenizer;
  int opened;

  //see if fileName is a file or directory
  vtkDirectory * directory = vtkDirectory::New();
  opened = directory->Open(filePath.c_str());
  if(opened)
    return "Directory";

  //find class entry
  std::getline(input, temp);
  while(temp.find("class") ==  std::string::npos && !input.eof())
    std::getline(input, temp);

  //return type
  if(!input.eof())
    {
    temp.erase(temp.begin()+temp.find(";"));
    //PARSE OUT CLASS TYPE
    tokenizer.str(temp);
    while(tokenizer>>foamClass);
    //return scalar, vector, or invalid
    if(foamClass =="volScalarField")
      return "Scalar";
    else if (foamClass =="volVectorField")
      return "Vector";
    else
      return "Invalid";
    }

  //if the file format is wrong return invalid
  else
    return "invalid";
}

// ****************************************************************************
//  Method: avtOpenFOAMFileFormat::GetInternalVariableAtTimestep
//
//  Purpose:
//  returns the values for a request variable for the internal mesh
//
// ****************************************************************************
vtkFloatArray * avtOpenFOAMFileFormat::GetInternalVariableAtTimestep( std::string varName, int timeState)
{
  std::stringstream varPath;
  varPath << PathPrefix << Steps[timeState] << "/" << varName;
  debug5 <<"Get internal variable: "<< varPath.str() << endl;

  std::string temp;
  ifstream input;
  bool binaryWriteFormat;
  input.open(varPath.str().c_str(),ios::in);
  //make sure file exists
  if(input.fail())
    return NULL;

  //determine if file is binary or ascii
  while(temp.find("format") == std::string::npos)
    getline(input, temp);
  input.close();

  //reopen file in correct format
  if(temp.find("binary") != std::string::npos)
    {
    input.open(varPath.str().c_str(),ios::binary);
    binaryWriteFormat = true;
    }
  else
    {
    input.open(varPath.str().c_str(),ios::in);
    binaryWriteFormat = false;
    }


  vtkFloatArray *data = vtkFloatArray::New();
  std::string foamClass;
  std::stringstream tokenizer;
  double value;

  //find class
  tokenizer.str("");
  tokenizer.clear();
  std::getline(input, temp);
  while(temp.find("class") == std::string::npos)
    std::getline(input, temp);
  temp.erase(temp.begin()+temp.find(";"));
  tokenizer.str(temp);
  while(tokenizer >> foamClass);
  temp.clear();
  tokenizer.str("");
  tokenizer.clear();

  //create scalar arrays
  if(foamClass =="volScalarField")
    {
    while(temp.find("internalField") == std::string::npos)
      std::getline(input, temp);
    //nonuniform
    if(!(temp.find("nonuniform") == std::string::npos))
      {
      //create an array
      std::getline(input,temp);
      tokenizer.str(temp);
      int scalarCount;
      tokenizer >> scalarCount;
      data->SetNumberOfValues(NumCells);

      //binary data
      if(binaryWriteFormat)
        {
        //add values to array
        char paren = input.get(); //throw out (
        for(int i = 0; i < scalarCount; i++)
          {
          input.read((char *) &value, sizeof(double));
          data->SetValue(i, value);
          }
        }

      //ascii data
      else
        {
        //add values to array
        std::getline(input, temp); //discard (

        for(int i = 0; i < scalarCount; i++)
          {
          tokenizer.str("");
          tokenizer.clear();
          std::getline(input,temp);
          tokenizer.str(temp);
          tokenizer >> value;
          data->SetValue(i, value);
          }
        }
      }

    //uniform
    else if(!(temp.find("uniform") == std::string::npos))
      {
      //parse out the uniform value
      std::string token;
      temp.erase(temp.begin()+temp.find(";"));
      tokenizer.str(temp);
      while(tokenizer>>token);
      tokenizer.str("");
      tokenizer.clear();
      tokenizer.str(token);
      tokenizer>>value;
      data->SetNumberOfValues(NumCells);

      //create array of uniform values
      for(int i = 0; i < NumCells; i++)
        {
        data->SetValue(i, value);
        }
      }

    //no data
    else
      return NULL;
    }

  //create vector arrays
  else if(foamClass == "volVectorField")
    {
    std::getline(input, temp);
    while(temp.find("internalField") == std::string::npos)
      std::getline(input, temp);
    if(!(temp.find("nonuniform") == std::string::npos))
      {
      //create an array
      std::getline(input,temp);
      tokenizer.str(temp);
      int vectorCount;
      tokenizer >> vectorCount;
      data->SetNumberOfComponents(3);

      //binary data
      if(binaryWriteFormat)
        {
        //add values to the array
        char paren = input.get(); //throw out (
        for(int i = 0; i < vectorCount; i++)
          {
          input.read((char *) &value, sizeof(double));
          data->InsertComponent(i, 0, value);
          input.read((char *) &value, sizeof(double));
          data->InsertComponent(i, 1, value);
          input.read((char *) &value, sizeof(double));
          data->InsertComponent(i, 2, value);
          }
        }

      //ascii data
      else
        {
        //add values to the array
        std::getline(input, temp); //discard (

        for(int i = 0; i < vectorCount; i++)
          {
          tokenizer.str("");
          tokenizer.clear();
          std::getline(input,temp);

          //REMOVE BRACKETS
          temp.erase(temp.begin()+temp.find("("));
          temp.erase(temp.begin()+temp.find(")"));

          //GRAB X,Y,&Z VALUES
          tokenizer.str(temp);
          tokenizer >> value;
          data->InsertComponent(i, 0, value);
          tokenizer >> value;
          data->InsertComponent(i, 1, value);
          tokenizer >> value;
          data->InsertComponent(i, 2, value);
          }
        }
      }
    else if(!(temp.find("uniform") == std::string::npos))
      {
      //create an array of uniform values
      double value1, value2, value3;

      //parse out the uniform values
      temp.erase(temp.begin(), temp.begin()+temp.find("(")+1);
      temp.erase(temp.begin()+temp.find(")"), temp.end());
      tokenizer.str(temp);
      tokenizer >> value1;
      tokenizer >> value2;
      tokenizer >> value3;

     data->SetNumberOfComponents(3);
      for(int i = 0; i < NumCells; i++)
        {
        data->InsertComponent(i, 0, value1);
        data->InsertComponent(i, 1, value2);
        data->InsertComponent(i, 2, value3);
        }
      }

    //no data
    else
      return NULL;
    }
  debug5<<"Internal variable data read."<<endl;
  return data;
}

// ****************************************************************************
//  Method: avtOpenFOAMFileFormat::GetBoundaryVariableAtTimestep
//
//  Purpose:
//  returns the values for a request variable for a bondary region
//
// ****************************************************************************
vtkFloatArray * avtOpenFOAMFileFormat::GetBoundaryVariableAtTimestep(int boundaryIndex, std::string varName, int timeState)
{
  std::stringstream varPath;
  varPath << PathPrefix << Steps[timeState] << "/" << varName;
  debug5 << "Get boundary variable: "<<varPath.str() << endl;

  std::string temp;
  ifstream input;
  bool binaryWriteFormat;
  input.open(varPath.str().c_str(),ios::in);
  //make sure file exists
  if(input.fail())
    return NULL;

  //determine if file is binary or ascii
  while(temp.find("format") == std::string::npos)
    getline(input, temp);
  input.close();

  //reopen file in correct format
  if(temp.find("binary") != std::string::npos)
    {
    input.open(varPath.str().c_str(),ios::binary);
    binaryWriteFormat = true;
    }
  else
    {
    input.open(varPath.str().c_str(),ios::in);
    binaryWriteFormat = false;
    }

  vtkFloatArray *data = vtkFloatArray::New();
  std::string foamClass;
  std::stringstream tokenizer;
  double value;

  //find class
  tokenizer.str("");
  tokenizer.clear();
  std::getline(input, temp);
  while(temp.find("class") == std::string::npos)
    std::getline(input, temp);
  temp.erase(temp.begin()+temp.find(";"));
  tokenizer.str(temp);
  while(tokenizer >> foamClass);
  temp.clear();
  tokenizer.str("");
  tokenizer.clear();

  //create scalar arrays
  if(foamClass =="volScalarField")
    {
    //find desired mesh
    while(temp.find(BoundaryNames[boundaryIndex]) == std::string::npos)
      std::getline(input, temp);
    //find value entry
    while(temp.find("}") == std::string::npos && temp.find("value") == std::string::npos)
        std::getline(input, temp);  //find value

      //nonuniform
    if(!(temp.find("nonuniform") == std::string::npos))
      {
      //binary data
      if(binaryWriteFormat)
        {
        //create an array
        std::getline(input,temp);
        tokenizer.str(temp);
        int scalarCount;
        tokenizer >> scalarCount;
        data->SetNumberOfValues(scalarCount);

        //assign values to the array
        char paren = input.get(); //throw out (
        for(int i = 0; i < scalarCount; i++)
          {
          input.read((char *) &value, sizeof(double));
          data->SetValue(i, value);
          }
        }

      //ascii data
      else
        {
        temp.erase(temp.begin(), temp.begin()+temp.find(">")+1);
        //ascii data with 10 or less values are on the same line
        //>10
        if(temp == std::string(" "))
          {
          //create an array of data
          std::getline(input,temp);
          tokenizer.str(temp);
          int scalarCount;
          tokenizer >> scalarCount;
          data->SetNumberOfValues(scalarCount);
          std::getline(input, temp); //discard (

          for(int i = 0; i < scalarCount; i++)
            {
            tokenizer.str("");
            tokenizer.clear();
            std::getline(input,temp);
            tokenizer.str(temp);
            tokenizer >> value;
            data->SetValue(i, value);
            }
          }
        //=<10
        else
          {
          //create an array with data
          tokenizer.str(temp);
          int scalarCount;
          tokenizer >> scalarCount;
          data->SetNumberOfValues(scalarCount);
          temp.erase(temp.begin(), temp.begin()+temp.find("(")+1);
          temp.erase(temp.begin()+temp.find(")"), temp.end());

          tokenizer.str("");
          tokenizer.clear();
          tokenizer.str(temp);
          for(int i = 0; i < scalarCount; i++)
            {
            tokenizer >> value;
            data->SetValue(i, value);
            }
          }
        }
      }

      //uniform
      else if(!(temp.find("uniform") == std::string::npos))
        {
        //create an array of uniform values
        double value1 = 0;
        temp.erase(temp.begin(), temp.begin()+temp.find("uniform")+7);
        temp.erase(temp.begin()+temp.find(";"), temp.end());
        tokenizer.str("");
        tokenizer.clear();
        tokenizer.str(temp);
        tokenizer >> value1;

        data->SetNumberOfValues(NFaces);
        for(int i = 0; i < NFaces; i++)
          data->SetValue(i, value1);
      }

    //zero gradient
    //or others without "value" entry
    else
      {
      double value1;
      int cellId;
      data->SetNumberOfValues(NFaces);
      for(int i = 0; i < NFaces; i++)
        {
        cellId = FaceOwner->GetValue(this->StartFace + i);
        data->SetValue(i, TempData->GetValue(cellId));
        }
      return data;
      }
    }

//CREATE VECTOR ARRAYS
  else if(foamClass == "volVectorField")
    {
    while(temp.find(BoundaryNames[boundaryIndex]) == std::string::npos)
      std::getline(input, temp);
    while(temp.find("}") == std::string::npos && temp.find("value") == std::string::npos)
        std::getline(input, temp);  //find value
    //nonuniform
    if(!(temp.find("nonuniform") == std::string::npos))
      {
      //create an array
      std::getline(input,temp);
      tokenizer.str(temp);
      int vectorCount;
      tokenizer >> vectorCount;
      data->SetNumberOfComponents(3);

      //binary data
      if(binaryWriteFormat)
        {
        //insert values into the array
        char paren = input.get(); //throw out (
        for(int i = 0; i < vectorCount; i++)
          {
          input.read((char *) &value, sizeof(double));
          data->InsertComponent(i, 0, value);
          input.read((char *) &value, sizeof(double));
          data->InsertComponent(i, 1, value);
          input.read((char *) &value, sizeof(double));
          data->InsertComponent(i, 2, value);
          }
        }

        //ascii data
        else
          {
          //insert values into the array
          std::getline(input, temp); //discard (
          for(int i = 0; i < vectorCount; i++)
            {
            tokenizer.str("");
            tokenizer.clear();
            std::getline(input,temp);

            //REMOVE BRACKETS
            temp.erase(temp.begin()+temp.find("("));
            temp.erase(temp.begin()+temp.find(")"));

            //GRAB X,Y,&Z VALUES
            tokenizer.str(temp);
            tokenizer >> value;
            data->InsertComponent(i, 0, value);
            tokenizer >> value;
            data->InsertComponent(i, 1, value);
            tokenizer >> value;
            data->InsertComponent(i, 2, value);
            }
          }
        }

    //uniform
    else if(!(temp.find("uniform") == std::string::npos))
      {
      //create an array of uniform values
      double value1 = 0, value2 = 0, value3 = 0;
      temp.erase(temp.begin(), temp.begin()+temp.find("(")+1);
      temp.erase(temp.begin()+temp.find(")"), temp.end());
      tokenizer.str("");
      tokenizer.clear();
      tokenizer.str(temp);
      tokenizer >> value1;
      tokenizer >> value2;
      tokenizer >> value3;

      data->SetNumberOfComponents(3);
      for(int i = 0; i < NFaces; i++)
        {
        data->InsertComponent(i, 0, value1);
        data->InsertComponent(i, 1, value2);
        data->InsertComponent(i, 2, value3);
        }
      }

    //zero gradient
    //or others without "value" entry
    else
      {
      double value1;
      int cellId;
      data->SetNumberOfComponents(3);
      for(int i = 0; i < NFaces; i++)
        {
        cellId = FaceOwner->GetValue(this->StartFace + i);
        data->InsertComponent(i, 0, TempData->GetComponent(cellId, 0));
        data->InsertComponent(i, 1, TempData->GetComponent(cellId, 1));
        data->InsertComponent(i, 2, TempData->GetComponent(cellId, 2));
        }
      return data;
      }
    }
  debug5<<"Boundary data read."<<endl;
  return data;
}

// ****************************************************************************
//  Method: avtOpenFOAMFileFormat::GatherBlocks
//
//  Purpose:
//  returns a vector of block names for a specified domain
//
// ****************************************************************************
std::vector< std::string > avtOpenFOAMFileFormat::GatherBlocks(std::string type, int timeState)
{
  std::string blockPath = PathPrefix+PolyMeshFacesDir[timeState]+"/polyMesh/"+type;
  std::vector< std::string > blocks;
  debug5<<"Get blocks: "<<blockPath<<endl;

  ifstream input;
  input.open(blockPath.c_str(), ios::in);
  //if there is no file return a null vector
  if(input.fail()) return blocks;

  std::string temp;
  std::string token;
  std::stringstream tokenizer;
  std::string tempName;

  //find end of header
  while(temp.compare(0,4,"// *",0,4)!=0)
    std::getline(input, temp);
  std::getline(input, temp); //throw out blank line
  std::getline(input, temp);

  //Number of blocks
  tokenizer.str(temp);
  tokenizer >> NumBlocks;
  blocks.resize(NumBlocks);

  //loop through each block
  for(int i = 0; i < NumBlocks; i++)
    {
    std::getline(input, temp); //throw out blank line

    //NAME
    std::getline(input, temp); //name
    tokenizer.clear();
    tokenizer.str(temp);
    tokenizer>>tempName;
    blocks[i] = tempName;
    while(temp.compare(0,1,"}",0,1)!=0)
      std::getline(input, temp);
    }

  return blocks;
}

// ****************************************************************************
//  Method: avtOpenFOAMFileFormat::GetBoundaryMesh
//
//  Purpose:
//  returns a requested boundary mesh
//
// ****************************************************************************
vtkUnstructuredGrid * avtOpenFOAMFileFormat::GetBoundaryMesh(int timeState, int boundaryIndex)
{
  vtkUnstructuredGrid * boundaryMesh = vtkUnstructuredGrid::New();
  std::string boundaryPath = PathPrefix+PolyMeshFacesDir[timeState]+"/polyMesh/boundary";
  debug5<< "Create boundary Mesh: "<<boundaryPath <<endl;

//  int nFaces;

  ifstream input;
  input.open(boundaryPath.c_str(), ios::in);
  //return a Null object
  if(input.fail()) return boundaryMesh;

  std::string temp;
  std::string token;
  std::stringstream tokenizer;

  //find desired mesh entry
  while(temp.find(BoundaryNames[boundaryIndex]) == std::string::npos)
    std::getline(input, temp);

  //get nFaces
  while(temp.find("nFaces") == std::string::npos)
    std::getline(input, temp);
  temp.erase(temp.begin()+temp.find(";")); //remove ;
  tokenizer.clear();
  tokenizer.str(temp);
  while(tokenizer >> token);
  tokenizer.clear();
  tokenizer.str(token);
  tokenizer>>NFaces;

  //get startFACE
  std::getline(input, temp);
  //look for "startFaces"
  while(temp.find("startFace") == std::string::npos)
    std::getline(input, temp);
  temp.erase(temp.begin()+temp.find(";")); //remove ;
  tokenizer.clear();
  tokenizer.str(temp);
  while(tokenizer >> token);
  tokenizer.clear();
  tokenizer.str(token);
  tokenizer>>StartFace;

  //Create the mesh
  int i, j, k;
  vtkTriangle * triangle;
  vtkQuad * quad;
  vtkPolygon * polygon;
  int endFace = StartFace + NFaces;

  //loop through each face
  for(j = StartFace; j < endFace; j++)
    {

    //triangle
    if(FacePoints[j].size() == 3)
      {
      triangle = vtkTriangle::New();
      for(k = 0; k < 3; k++) 
        triangle->GetPointIds()->SetId(k, FacePoints[j][k]);
      boundaryMesh->InsertNextCell(triangle->GetCellType(),
      triangle->GetPointIds());
      triangle->Delete();
      }

    //quad
    else if(FacePoints[j].size() == 4)
      {
      quad = vtkQuad::New();
      for(k = 0; k < 4; k++) 
        quad->GetPointIds()->SetId(k, FacePoints[j][k]);
      boundaryMesh->InsertNextCell(quad->GetCellType(),
      quad->GetPointIds());
      quad->Delete();
      }

    //polygon
    else
      {
      polygon = vtkPolygon::New();
      for(k = 0; k < (int)FacePoints[j].size(); k++) 
        polygon->GetPointIds()->InsertId(k, FacePoints[j][k]);
      boundaryMesh->InsertNextCell(polygon->GetCellType(),
      polygon->GetPointIds());
      polygon->Delete();
      }
    }

  //set points for boundary
  boundaryMesh->SetPoints(Points);
  debug5<<"Boundary mesh created."<<endl;
  return boundaryMesh;
}

// ****************************************************************************
//  Method: avtOpenFOAMFileFormat::GetPointZoneMesh
//
//  Purpose:
//  returns a requested point zone mesh
//
// ****************************************************************************
vtkUnstructuredGrid * avtOpenFOAMFileFormat::GetPointZoneMesh(int timeState, int pointZoneIndex)
{
  vtkUnstructuredGrid * pointZoneMesh = vtkUnstructuredGrid::New();
  std::string pointZonesPath = PathPrefix+PolyMeshFacesDir[timeState]+"/polyMesh/pointZones";
  debug5<<"Create point zone mesh: "<<pointZonesPath<<endl;

  std::string temp;
  ifstream input;
  bool binaryWriteFormat;
  input.open(pointZonesPath.c_str(),ios::in);
  //make sure file exists  
  if(input.fail()) 
    return pointZoneMesh;

  //determine if file is binary or ascii
  while(temp.find("format") == std::string::npos)
    getline(input, temp);
  input.close();

  //reopen file in correct format
  if(temp.find("binary") != std::string::npos)
    {
    input.open(pointZonesPath.c_str(),ios::binary);
    binaryWriteFormat = true;
    }
  else
    {
    input.open(pointZonesPath.c_str(),ios::in);
    binaryWriteFormat = false;
    }

  std::string token;
  std::stringstream tokenizer;
  vtkVertex * pointCell;
  vtkPoints * pointZonePoints = vtkPoints::New();
  int tempElement;
  std::vector< std::vector < int > > tempElementZones;
  int numElement;

  //find desired mesh entry
  while(temp.find(PointZoneNames[pointZoneIndex]) == std::string::npos)
    std::getline(input, temp);
  std::getline(input, temp); //throw out {
  std::getline(input, temp);  //type
  std::getline(input, temp);  //label
  std::getline(input, temp);  //number of elements or {

  //number of elements
  if(temp.find("}") == std::string::npos)
    {
    tokenizer.clear();
    tokenizer.str(temp);
    tokenizer>>numElement;
    if(numElement == 0)
      return NULL;

    //binary data
    if(binaryWriteFormat)
      {
      char paren = input.get();
      for(int j = 0; j < numElement; j++)
        {
        input.read((char *) &tempElement, sizeof(int));
        pointCell = vtkVertex::New();
        pointCell->GetPointIds()->SetId(0,tempElement);
        pointZoneMesh->InsertNextCell(pointCell->GetCellType(), pointCell->GetPointIds());
        pointCell->Delete();
        }
      }

    //ascii data
    else
      {
      std::getline(input, temp);//THROW OUT (

      //GET EACH ELEMENT & ADD TO VECTOR
      for(int j = 0; j < numElement; j++)
        {
        std::getline(input, temp);
        tokenizer.clear();
        tokenizer.str(temp);
        tokenizer>>tempElement;
        pointCell = vtkVertex::New();
        pointCell->GetPointIds()->SetId(0,tempElement);
        pointZoneMesh->InsertNextCell(pointCell->GetCellType(), pointCell->GetPointIds());
        pointCell->Delete();
        }
      }
    }

  //there is no entry
  else
     return NULL;

  //set point zone points
  pointZoneMesh->SetPoints(Points);
  debug5<<"Point zone mesh created."<<endl;
   return pointZoneMesh;
}

// ****************************************************************************
//  Method: avtOpenFOAMFileFormat::GetFaceZoneMesh
//
//  Purpose:
//  returns a requested face zone mesh
//
// ****************************************************************************
vtkUnstructuredGrid * avtOpenFOAMFileFormat::GetFaceZoneMesh(int timeState, int faceZoneIndex)
{
  vtkUnstructuredGrid * faceZoneMesh = vtkUnstructuredGrid::New();
  std::string faceZonesPath = PathPrefix+PolyMeshFacesDir[timeState]+"/polyMesh/faceZones";
  debug5<<"Create face zone mesh: "<<faceZonesPath<<endl;

  std::string temp;
  ifstream input;
  bool binaryWriteFormat;
  input.open(faceZonesPath.c_str(),ios::in);
  //make sure file exists  
  if(input.fail()) 
    return faceZoneMesh;

  //determine if file is binary or ascii
  while(temp.find("format") == std::string::npos)
    getline(input, temp);
  input.close();

  //reopen file in correct format
  if(temp.find("binary") != std::string::npos)
    {
    input.open(faceZonesPath.c_str(),ios::binary);
    binaryWriteFormat = true;
    }
  else
    {
    input.open(faceZonesPath.c_str(),ios::in);
    binaryWriteFormat = false;
    }

  std::string token;
  std::stringstream tokenizer;
  std::vector< int > faceZone;
  int tempElement;
  std::vector< std::vector < int > > tempElementZones;
  int numElement;

  //find desired mesh entry
  while(temp.find(FaceZoneNames[faceZoneIndex]) == std::string::npos)
    std::getline(input, temp);

    std::getline(input, temp); //throw out {
    std::getline(input, temp);  //type
    std::getline(input, temp);  //label
    std::getline(input, temp);  //number of values or flipmap

  if(temp.find("flipMap") == std::string::npos)
    {
    //number of elements
    tokenizer.clear();
    tokenizer.str(temp);
    tokenizer>>numElement;
    if(numElement == 0)
      return NULL;

    //binary
    if(binaryWriteFormat)
      {
      char paren = input.get();
      for(int j = 0; j < numElement; j++)
        {
        input.read((char *) &tempElement, sizeof(int));
        faceZone.push_back(tempElement);
        }
      }

    //ascii
    else
      {
      //THROW OUT (
      std::getline(input, temp);

      //get each element & add to vector
      for(int j = 0; j < numElement; j++)
        {
        std::getline(input, temp);
        tokenizer.clear();
        tokenizer.str(temp);
        tokenizer>>tempElement;
        faceZone.push_back(tempElement);
        }
      }
    }

  //Create the mesh
  int k;
  vtkTriangle * triangle;
  vtkQuad * quad;
  vtkPolygon * polygon;

  //LOOP THROUGH EACH FACE
  for(int j = 0; j < faceZone.size(); j++)
    {

    //Triangular Face
    if(this->FacePoints[faceZone[j]].size() == 3)
      {
      triangle = vtkTriangle::New();
      for(k = 0; k < 3; k++)
        {
        triangle->GetPointIds()->SetId(k, this->FacePoints[
        faceZone[j]][k]);
        }
      faceZoneMesh->InsertNextCell(triangle->GetCellType(),
        triangle->GetPointIds());
      triangle->Delete();
      }

    //Quadraic Face
    else if(this->FacePoints[faceZone[j]].size() == 4)
      {
      quad = vtkQuad::New();
      for(k = 0; k < 4; k++)
        {
        quad->GetPointIds()->SetId(k,
          this->FacePoints[faceZone[j]][k]);
        }
      faceZoneMesh->InsertNextCell(quad->GetCellType(),
        quad->GetPointIds());
      quad->Delete();
      }

    //Polygonal Face
    else
      {
      polygon = vtkPolygon::New();
      for(k = 0; k < (int)this->FacePoints[faceZone[j]].size(); k++)
        {
        polygon->GetPointIds()->InsertId(k, this->FacePoints[
          faceZone[j]][k]);
        }
      faceZoneMesh->InsertNextCell(polygon->GetCellType(),
        polygon->GetPointIds());
      polygon->Delete();
      }
    }

  //set the face zone points
  faceZoneMesh->SetPoints(Points);
  debug5<<"Face zone mesh created."<<endl;
  return faceZoneMesh;
}

// ****************************************************************************
//  Method: avtOpenFOAMFileFormat::GetCellZoneMesh
//
//  Purpose:
//  returns a requested cell zone mesh
//
// ****************************************************************************
vtkUnstructuredGrid * avtOpenFOAMFileFormat::GetCellZoneMesh(int timeState, int cellZoneIndex)
{
  vtkUnstructuredGrid * cellZoneMesh = vtkUnstructuredGrid::New();
  std::string cellZonesPath = PathPrefix+PolyMeshFacesDir[timeState]+"/polyMesh/cellZones";
  debug5<<"Create cell zone mesh: "<<cellZonesPath<<endl;

  std::string temp;
  ifstream input;
  bool binaryWriteFormat;
  input.open(cellZonesPath.c_str(),ios::in);
  //make sure file exists  
  if(input.fail()) 
    return cellZoneMesh;

  //determine if file is binary or ascii
  while(temp.find("format") == std::string::npos)
    getline(input, temp);
  input.close();

  //reopen file in correct format
  if(temp.find("binary") != std::string::npos)
    {
    input.open(cellZonesPath.c_str(),ios::binary);
    binaryWriteFormat = true;
    }
  else
    {
    input.open(cellZonesPath.c_str(),ios::in);
    binaryWriteFormat = false;
    }

  std::string token;
  std::stringstream tokenizer;
  std::vector< int > cellZone;
  int tempElement;
  std::vector< std::vector < int > > tempElementZones;
  int numElement;

  //find desired mesh entry
  while(temp.find(CellZoneNames[cellZoneIndex]) == std::string::npos)
    std::getline(input, temp);
  std::getline(input, temp);  //throw out {
  std::getline(input, temp);  //type
  std::getline(input, temp);  //label
  std::getline(input, temp);  

  //number of elements
  tokenizer.clear();
  tokenizer.str(temp);
  tokenizer>>numElement;

  //binary
  if(binaryWriteFormat)
    {
    char paren = input.get();
    for(int j = 0; j < numElement; j++)
      {
      input.read((char *) &tempElement, sizeof(int));
      cellZone.push_back(tempElement);
      }
    }

  //ascii
  else
    {
    std::getline(input, temp); //throw out (

    //get each element & add to vector
    for(int j = 0; j < numElement; j++)
      {
      std::getline(input, temp);
      tokenizer.clear();
      tokenizer.str(temp);
      tokenizer>>tempElement;
      cellZone.push_back(tempElement);
      }
    }

  //Create the mesh
  bool foundDup = false;
  std::vector< int > cellPoints;
  std::vector< int > tempFaces[2];
  std::vector< int > firstFace;
  int pivotPoint = 0;
  int i, j, k, l, pCount;
  int faceCount = 0;

  //Create Mesh
  for(i = 0; i < cellZone.size(); i++)  //each cell
    {
    //calculate total points for all faces of a cell
    //used to determine cell type
    int totalPointCount = 0;
    for(j = 0; j < (int)FacesOfCell[
      cellZone[i]].size(); j++)  //each face
      {
      totalPointCount += (int)FacePoints[FacesOfCell[
        cellZone[i]][j].faceIndex].size();
      }

      // using cell type - order points, create cell, add to mesh

    //OFhex | vtkHexahedron
    if (totalPointCount == 24) 
      {
      faceCount = 0;

      //get first face
      for(j = 0; j < (int)FacePoints[FacesOfCell[
        cellZone[i]][0].faceIndex].size(); j++)
        {
        firstFace.push_back(FacePoints[FacesOfCell[
          cellZone[i]][0].faceIndex][j]);
        }

      //-if it is a neighbor face flip it
      if(FacesOfCell[i][0].neighborFace)
        {
        int tempPop;
        for(k = 0; k < firstFace.size() - 1; k++)
          {
          tempPop = firstFace[firstFace.size()-1];
          firstFace.pop_back();
          firstFace.insert(firstFace.begin()+1+k, tempPop);
          }
        }

      //add first face to cell points
      for(j =0; j < (int)firstFace.size(); j++)
        cellPoints.push_back(firstFace[j]);

      for(int pointCount = 0;pointCount < (int)firstFace.size();pointCount++)
        {
        //find the 2 other faces containing each point - start with face 1
        for(j = 1; j < (int)FacesOfCell[
          cellZone[i]].size(); j++)  //each face
          {
          for(k = 0; k < (int)FacePoints[FacesOfCell[
            cellZone[i]][j].faceIndex].size(); k++) //each point
            {
            if(firstFace[pointCount] == FacePoints[FacesOfCell[
              cellZone[i]][j].faceIndex][k])
              {
              //another face with the point
              for(l = 0; l < (int)FacePoints[FacesOfCell[
                cellZone[i]][j].faceIndex].size(); l++)
                {
                tempFaces[faceCount].push_back(FacePoints[
                  FacesOfCell[cellZone[i]][j].faceIndex]
                  [l]);
                }
              faceCount++;
              }
            }
          }

        //locate the pivot point contained in faces 0 & 1
        for(j = 0; j < (int)tempFaces[0].size(); j++)
          {
          for(k = 0; k < (int)tempFaces[1].size(); k++)
            {
            if(tempFaces[0][j] == tempFaces[1][k] && tempFaces[0][j] !=
              firstFace[pointCount])
              {
              pivotPoint = tempFaces[0][j];
              break;
              }
            }
          }
        cellPoints.push_back(pivotPoint);
        tempFaces[0].clear();
        tempFaces[1].clear();
        faceCount=0;
        }

        //create the hex cell and insert it into the mesh
        vtkHexahedron * hexahedron = vtkHexahedron::New();
        for(pCount = 0; pCount < (int)cellPoints.size(); pCount++)
          {
          hexahedron->GetPointIds()->SetId(pCount, cellPoints[pCount]);
          }
        cellZoneMesh->InsertNextCell(hexahedron->GetCellType(),
          hexahedron->GetPointIds());
        hexahedron->Delete();
        cellPoints.clear();
        firstFace.clear();
        }

    //OFprism | vtkWedge
    else if (totalPointCount == 18) 
      {
      faceCount = 0;
      int index = 0;

      //find first triangular face
      for(j = 0; j < (int)FacesOfCell[
        cellZone[i]].size(); j++)  //each face
        {
        if((int)FacePoints[FacesOfCell[
          cellZone[i]][j].faceIndex].size() == 3)
          {
          for(k = 0; k < (int)FacePoints[FacesOfCell[
            cellZone[i]][j].faceIndex].size(); k++)
            {
            firstFace.push_back(FacePoints[FacesOfCell[
              cellZone[i]][j].faceIndex][k]);
            index = j;
            }
          break;
          }
        }

      //-if it is a neighbor face flip it
      if(FacesOfCell[i][0].neighborFace)
        {
        int tempPop;
        for(k = 0; k < firstFace.size() - 1; k++)
          {
          tempPop = firstFace[firstFace.size()-1];
          firstFace.pop_back();
          firstFace.insert(firstFace.begin()+1+k, tempPop);
          }
        }

      //add first face to cell points
      for(j =0; j < (int)firstFace.size(); j++)
        cellPoints.push_back(firstFace[j]);

      for(int pointCount = 0;pointCount < (int)firstFace.size();pointCount++)
        {
        //find the 2 other faces containing each point
        for(j = 0; j < (int)FacesOfCell[
          cellZone[i]].size(); j++)  //each face
          {
          for(k = 0; k < (int)FacePoints[FacesOfCell[
            cellZone[i]][j].faceIndex].size(); k++) //each point
            {
            if(firstFace[pointCount] == FacePoints[FacesOfCell[
              cellZone[i]][j].faceIndex][k] && j != index)
              {
              //another face with point
              for(l = 0; l < (int)FacePoints[FacesOfCell[
                cellZone[i]][j].faceIndex].size(); l++)
                {
                tempFaces[faceCount].push_back(FacePoints[
                  FacesOfCell[cellZone[i]][j].faceIndex]
                  [l]);
                }
              faceCount++;
              }
            }
          }

        //locate the pivot point contained in faces 0 & 1
        for(j = 0; j < (int)tempFaces[0].size(); j++)
          {
          for(k = 0; k < (int)tempFaces[1].size(); k++)
            {
            if(tempFaces[0][j] == tempFaces[1][k] && tempFaces[0][j] != 
              firstFace[pointCount])
              {
              pivotPoint = tempFaces[0][j];
              break;
              }
            }
          }
        cellPoints.push_back(pivotPoint);
        tempFaces[0].clear();
        tempFaces[1].clear();
        faceCount=0;
        }

      //create the wedge cell and insert it into the mesh
      vtkWedge * wedge = vtkWedge::New();
      for(pCount = 0; pCount < (int)cellPoints.size(); pCount++)
        {
        wedge->GetPointIds()->SetId(pCount, cellPoints[pCount]);
        }
      cellZoneMesh->InsertNextCell(wedge->GetCellType(),
        wedge->GetPointIds());
      cellPoints.clear();
      wedge->Delete(); 
      firstFace.clear();
      }

    //OFpyramid | vtkPyramid
    else if (totalPointCount == 16) 
      {
      foundDup = false;

      //find quad
      for(j = 0; j < (int)FacesOfCell[
        cellZone[i]].size(); j++)  //each face
        {
        if((int)FacePoints[FacesOfCell[
          cellZone[i]][j].faceIndex].size() == 4)
          {
          for(k = 0; k < (int)FacePoints[FacesOfCell[
            cellZone[i]][j].faceIndex].size(); k++)
            {
            cellPoints.push_back(FacePoints[FacesOfCell[
              cellZone[i]][j].faceIndex][k]);
            }
          break;
          }
        }

      //compare first face points to second faces
      for(j = 0; j < (int)cellPoints.size(); j++) //each point
        {
        for(k = 0; k < (int)FacePoints[FacesOfCell[
          cellZone[i]][1].faceIndex].size(); k++)
          {
          if(cellPoints[j] == FacePoints[FacesOfCell[
            cellZone[i]][1].faceIndex][k])
            {
            foundDup = true;
            }
          }
        if(!foundDup)
          {
          cellPoints.push_back(FacePoints[FacesOfCell[
            cellZone[i]][j].faceIndex][k]);
          break;
          }
        }

      //create the pyramid cell and insert it into the mesh
      vtkPyramid * pyramid = vtkPyramid::New();
      for(pCount = 0; pCount < (int)cellPoints.size(); pCount++)
        {
        pyramid->GetPointIds()->SetId(pCount, cellPoints[pCount]);
        }
      cellZoneMesh->InsertNextCell(pyramid->GetCellType(),
        pyramid->GetPointIds());
      cellPoints.clear();
      pyramid->Delete(); 
      }

    //OFtet | vtkTetrahedron
    else if (totalPointCount == 12) 
      {
      foundDup = false;

      //grab first face
      for(j = 0; j < (int)FacePoints[FacesOfCell[
        cellZone[i]][0].faceIndex].size(); j++)
        {
        cellPoints.push_back(FacePoints[FacesOfCell[
          cellZone[i]][0].faceIndex][j]);
        }

      //compare first face points to second faces
      for(j = 0; j < (int)cellPoints.size(); j++) //each point
        {
        for(k = 0; k < (int)FacePoints[FacesOfCell[
          cellZone[i]][1].faceIndex].size(); k++)
          {
          if(cellPoints[j] == FacePoints[FacesOfCell[
            cellZone[i]][1].faceIndex][k])
            {
            foundDup = true;
            }
          }
        if(!foundDup)
          {
          cellPoints.push_back(FacePoints[FacesOfCell[
            cellZone[i]][j].faceIndex][k]);
          break;
          }
        }

      //create the wedge cell and insert it into the mesh
      vtkTetra * tetra = vtkTetra::New();
      for(pCount = 0; pCount < (int)cellPoints.size(); pCount++)
        {
        tetra->GetPointIds()->SetId(pCount, cellPoints[pCount]);
        }
      cellZoneMesh->InsertNextCell(tetra->GetCellType(),
        tetra->GetPointIds());
      cellPoints.clear();
      tetra->Delete();
      }

    //OFpolyhedron || vtkConvexPointSet
    else
      {
      cout<<"Warning: Polyhedral Data is very Slow!"<<endl;
      foundDup = false;

      //grab face 0
      for(j = 0; j < (int)FacePoints[FacesOfCell[
        cellZone[i]][0].faceIndex].size(); j++)
        {
        firstFace.push_back(FacePoints[
          FacesOfCell[i][0].faceIndex][j]);
        }

      //ADD FIRST FACE TO CELL POINTS
      for(j =0; j < (int)firstFace.size(); j++)
        cellPoints.push_back(firstFace[j]);
      //j = 1 skip firstFace
      for(j = 1; j < (int) FacesOfCell[
        cellZone[i]].size(); j++)
        {
        //remove duplicate points from faces
        for(k = 0; k < (int)FacePoints[
          FacesOfCell[i][j].faceIndex].size(); k++)
          {
          for(l = 0; l < (int)cellPoints.size(); l++);
            {
            if(cellPoints[l] == FacePoints[FacesOfCell[
              cellZone[i]][j].faceIndex][k])
              {
              foundDup = true;
              }
            }
          if(!foundDup)
            {
            cellPoints.push_back(FacePoints[FacesOfCell[
              cellZone[i]][j].faceIndex][k]);
            foundDup = false;
            }
          }
        }

      //create the poly cell and insert it into the mesh
      vtkConvexPointSet * poly = vtkConvexPointSet::New();
      poly->GetPointIds()->SetNumberOfIds(cellPoints.size());
      for(pCount = 0; pCount < (int)cellPoints.size(); pCount++)
        {
        poly->GetPointIds()->SetId(pCount, cellPoints[pCount]);
        }
      cellZoneMesh->InsertNextCell(poly->GetCellType(),
        poly->GetPointIds());
      cellPoints.clear();
      firstFace.clear();
      poly->Delete();
      }
    }

  //set cell zone points
  cellZoneMesh->SetPoints(Points);
  debug5<<"Cell zone mesh created"<<endl;
  return cellZoneMesh;
}

void avtOpenFOAMFileFormat::ActivateTimestep(int timeState)
{
  debug5<<"activate: "<<timeState<<endl;

  //create the face info only once
  if(CreateFaces)
    {
    debug5<<"Update face data."<<endl;
    //create paths to polyMesh files
    std::string boundaryPath = PathPrefix + PolyMeshFacesDir[timeState] +"/polyMesh/boundary";
    std::string facePath = PathPrefix + PolyMeshFacesDir[timeState] + "/polyMesh/faces";
    std::string ownerPath = PathPrefix + PolyMeshFacesDir[timeState] + "/polyMesh/owner";
    std::string neighborPath = PathPrefix + PolyMeshFacesDir[timeState] + "/polyMesh/neighbour";

    //create the faces vector
    ReadFacesFile(facePath);

    //create the faces owner vector
    ReadOwnerFile(ownerPath);

    //create the faces neighbor vector
    ReadNeighborFile(neighborPath);

    //create a vector containing a faces of each cell
    CombineOwnerNeigbor();

    GetPoints(timeState); //get the points
    CreateFaces = false;
    }
}
