/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                            avtGTCFileFormat.C                           //
// ************************************************************************* //

#include <avtMTSDFileFormatInterface.h>
#include <avtGTCFileFormat.h>
#include <ADIOS2HelperFuncs.h>

#include <string>
#include <map>

#include <vtkFloatArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkGenericCell.h>

#include <avtDatabaseMetaData.h>
#include <DBOptionsAttributes.h>
#include <Expression.h>
#include <Utility.h>
#include <VisItStreamUtil.h>
#include <InvalidVariableException.h>

using namespace std;

bool
avtGTCFileFormat::Identify(const char *fname)
{
    bool retval = false;
    string engineName = ADIOS2Helper_GetEngineName(fname);
    string fileName   = ADIOS2Helper_GetFileName(fname);
    bool stagingMode  = ADIOS2Helper_IsStagingEngine(engineName);

    adios2::ADIOS adios;
    adios2::IO io(adios.DeclareIO("ReadBP"));
    io.SetEngine(engineName);
    adios2::Engine reader = io.Open(fileName, adios2::Mode::Read);
    adios2::StepStatus status =
        reader.BeginStep(adios2::StepMode::Read, -1.0f);
    if (status == adios2::StepStatus::OK)
    {
        //std::cout<<" Identifier for GTC received streaming step = "<<reader.CurrentStep()<<endl;
        std::map<std::string, adios2::Params> variables, attributes;
        variables = io.AvailableVariables();
        attributes = io.AvailableAttributes();

        int vfind = 0;
        vector<string> reqVars = {"coordinates", "potential",
                                  "igrid", "index-shift"};
        for (auto vi = variables.begin(); vi != variables.end(); vi++)
            if (std::find(reqVars.begin(), reqVars.end(), vi->first) != reqVars.end())
                vfind++;

        retval = (vfind == reqVars.size());
        reader.EndStep();
    }
    reader.Close();
    return retval;
}

bool avtGTCFileFormat::IdentifyADIOS2(
                    std::map<std::string, adios2::Params> &variables,
                    std::map<std::string, adios2::Params> &attributes)
{
    int vfind = 0;
    vector<string> reqVars = {"coordinates", "potential",
                              "igrid", "index-shift"};
    for (auto vi = variables.begin(); vi != variables.end(); vi++)
        if (std::find(reqVars.begin(), reqVars.end(), vi->first) != reqVars.end())
            vfind++;

    return (vfind == reqVars.size());
}

avtFileFormatInterface *
avtGTCFileFormat::CreateInterface(const char *const *list,
                                  int nList,
                                  int nBlock)
{
    int nTimestepGroups = nList / nBlock;
    avtMTSDFileFormat ***ffl = new avtMTSDFileFormat**[nTimestepGroups];
    for (int i = 0; i < nTimestepGroups; i++)
    {
        ffl[i] =  new avtMTSDFileFormat*[nBlock];
        for (int j = 0; j < nBlock; j++)
            ffl[i][j] =  new avtGTCFileFormat(list[i*nBlock +j]);
    }
    return new avtMTSDFileFormatInterface(ffl, nTimestepGroups, nBlock);
}

avtFileFormatInterface *
avtGTCFileFormat::CreateInterfaceADIOS2(
        const char *const *list,
        int nList,
        int nBlock,
        std::shared_ptr<adios2::ADIOS> adios,
        adios2::Engine &reader,
        adios2::IO &io,
        std::map<std::string, adios2::Params> &variables,
        std::map<std::string, adios2::Params> &attributes
        )
{
    int nTimestepGroups = nList / nBlock;
    avtMTSDFileFormat ***ffl = new avtMTSDFileFormat**[nTimestepGroups];
    for (int i = 0; i < nTimestepGroups; i++)
    {
        ffl[i] =  new avtMTSDFileFormat*[nBlock];
        for (int j = 0; j < nBlock; j++)
            if (!i && !j)
            {
                ffl[i][j] =  new avtGTCFileFormat(adios, reader, io, variables, attributes, list[i*nBlock +j]);
            }
            else
            {
                ffl[i][j] =  new avtGTCFileFormat(list[i*nBlock +j]);
            }
    }
    return new avtMTSDFileFormatInterface(ffl, nTimestepGroups, nBlock);
}

// ****************************************************************************
//  Method: avtGTCFileFormat constructor
//
//  Programmer: pugmire -- generated by xml2avt
//  Creation:   Thu Apr 12 08:39:47 PDT 2018
//
// ****************************************************************************

avtGTCFileFormat::avtGTCFileFormat(const char *filename)
    : adios(std::make_shared<adios2::ADIOS>(adios2::DebugON)),
      io(adios->DeclareIO("ReadBP")),
      numTimeSteps(1),
      avtMTSDFileFormat(&filename, 1),
      grid(NULL),
      cylGrid(NULL),
      ptGrid(NULL)
{
    reader = io.Open(filename, adios2::Mode::Read);
    if (!reader)
        EXCEPTION1(ImproperUseException, "Invalid file");

    variables = io.AvailableVariables();
    attributes = io.AvailableAttributes();

    //Determine how many steps we have.
    if (variables.find("potential") != variables.end())
        numTimeSteps = std::stoi(variables["potential"]["AvailableStepsCount"]);

    /*
    cout<<"Attrs: "<<endl;
    for (auto ai = attributes.begin(); ai != attributes.end(); ai++)
        cout<<ai->first<<" "<<ai->second<<endl;
    cout<<"Vars:"<<endl;
    for (auto vi = variables.begin(); vi != variables.end(); vi++)
        cout<<vi->first<<" "<<vi->second<<endl;
    */
}


avtGTCFileFormat::avtGTCFileFormat(std::shared_ptr<adios2::ADIOS> adios,
        adios2::Engine &reader,
        adios2::IO &io,
        std::map<std::string, adios2::Params> &variables,
        std::map<std::string, adios2::Params> &attributes,
        const char *filename)
    : adios(adios),
      reader(reader),
      io(io),
      numTimeSteps(1),
      avtMTSDFileFormat(&filename, 1),
      variables(variables),
      attributes(attributes),
      grid(NULL),
      cylGrid(NULL),
      ptGrid(NULL)
{
    //Determine how many steps we have.
    if (variables.find("potential") != variables.end())
        numTimeSteps = std::stoi(variables["potential"]["AvailableStepsCount"]);
}

// ****************************************************************************
//  Method: avtGTCFileFormat destructor
//
//  Programmer: pugmire -- generated by xml2avt
//  Creation:   Thu Apr 12 08:39:47 PDT 2018
//
// ****************************************************************************

avtGTCFileFormat::~avtGTCFileFormat()
{
    if (grid)
        grid->Delete();
    if (cylGrid)
        cylGrid->Delete();
    if (ptGrid)
        ptGrid->Delete();

    grid = NULL;
    cylGrid = NULL;
    ptGrid = NULL;
}


// ****************************************************************************
//  Method: avtGTCFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: pugmire -- generated by xml2avt
//  Creation:   Thu Apr 12 08:39:47 PDT 2018
//
// ****************************************************************************

int
avtGTCFileFormat::GetNTimesteps()
{
    return numTimeSteps;
}

void
avtGTCFileFormat::GetCycles(std::vector<int> &c)
{
    c.resize(numTimeSteps);
    for (int i = 0; i < numTimeSteps; i++)
        c[i] = i;
}

void
avtGTCFileFormat::GetTimes(std::vector<double> &t)
{
    t.resize(numTimeSteps);
    for (int i = 0; i < numTimeSteps; i++)
        t[i] = (double)i;
}

// ****************************************************************************
//  Method: avtGTCFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: pugmire -- generated by xml2avt
//  Creation:   Thu Apr 12 08:39:47 PDT 2018
//
// ****************************************************************************

void
avtGTCFileFormat::FreeUpResources()
{
    grid->Delete();
    grid = NULL;
}


// ****************************************************************************
//  Method: avtGTCFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: pugmire -- generated by xml2avt
//  Creation:   Thu Apr 12 08:39:47 PDT 2018
//
// ****************************************************************************

void
avtGTCFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)
{
    AddMeshToMetaData(md, "mesh", AVT_UNSTRUCTURED_MESH, NULL, 1, 0, 3, 3);
    AddMeshToMetaData(md, "mesh_pt", AVT_POINT_MESH, NULL, 1, 0, 3, 3);
    AddMeshToMetaData(md, "mesh_cyl", AVT_UNSTRUCTURED_MESH, NULL, 1, 0, 3, 3);

    AddScalarVarToMetaData(md, "potential", "mesh", AVT_NODECENT);
    AddScalarVarToMetaData(md, "potential_pt", "mesh_pt", AVT_NODECENT);
    AddScalarVarToMetaData(md, "potential_cyl", "mesh_cyl", AVT_NODECENT);
}


// ****************************************************************************
//  Method: avtGTCFileFormat::GetPtMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      timestate   The index of the timestate.  If GetNTimesteps returned
//                  'N' time steps, this is guaranteed to be between 0 and N-1.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: pugmire -- generated by xml2avt
//  Creation:   Thu Apr 12 08:39:47 PDT 2018
//
// ****************************************************************************

vtkDataSet *
avtGTCFileFormat::GetPtMesh(int timestate, const char *meshname)
{
    if (ptGrid != NULL)
    {
        ptGrid->Register(NULL);
        return ptGrid;
    }

    adios2::Variable<float> cVar = io.InquireVariable<float>("coordinates");
    if (!cVar)
        return NULL;

    //dims: nPlanes, 3, nPts
    auto cDims = cVar.Shape();

    //cout<<"DIMS: "<<cDims<<endl;
    vector<float> xbuff, ybuff, zbuff;

    cVar.SetSelection(adios2::Box<adios2::Dims>({0,0,0}, {cDims[0], 1, cDims[2]}));
    reader.Get(cVar, xbuff, adios2::Mode::Sync);
    cVar.SetSelection(adios2::Box<adios2::Dims>({0,1,0}, {cDims[0], 1, cDims[2]}));
    reader.Get(cVar, ybuff, adios2::Mode::Sync);
    cVar.SetSelection(adios2::Box<adios2::Dims>({0,2,0}, {cDims[0], 1, cDims[2]}));
    reader.Get(cVar, zbuff, adios2::Mode::Sync);

    ptGrid = vtkUnstructuredGrid::New();
    vtkPoints *pts = vtkPoints::New();
    pts->SetNumberOfPoints(xbuff.size());

    ptGrid->SetPoints(pts);
    pts->Delete();
    vtkIdType idx = 0;
    for (int i = 0; i < xbuff.size(); i++)
    {
        pts->SetPoint(i, xbuff[i], ybuff[i], zbuff[i]);
        ptGrid->InsertNextCell(VTK_VERTEX, 1, &idx);
        idx++;
    }

    ptGrid->Register(NULL);
    return ptGrid;
}

///DRP
///insert code
#define TRIANGLES 0
#define PRISMS 1
#define HEXAHEDRALS 0

//
// Helper function for comparing two points.
//
float diff( float x0, float y0, float z0, float x1, float y1, float z1 )
{
  return sqrt( (x0-x1)*(x0-x1) + (y0-y1)*(y0-y1) + (z0-z1)*(z0-z1) );
}

//
// Create the potential mesh vertex list from the points.
//
void create_potential_mesh_vertex_list( //float **data_in,
                                        float *X_in,
                                        float *Y_in,
                                        float *Z_in,
                                        unsigned int nPoloidalPlanes,
                                        unsigned int nNodes,
                                        int **vertex_list,
                                        int &numElements,
                                        int &numVertices)
{
    float *data_in[3] = {X_in, Y_in, Z_in};

    vector<int> nPoloidalNodes[nPoloidalPlanes];   // Number of nodes in each poloidal contour
    vector<int> poloidalIndex[nPoloidalPlanes];    // Starting node index of each poloidal contour

    vector<int> vertices[nPoloidalPlanes]; // Plane vertex connections

    vector< vector< int > > degeneracies[nPoloidalPlanes]; // degenerate indices

    // Index of the closest node on the neighboring contour.
    int **neighborIndex = new int*[nPoloidalPlanes];

    for( int p=0; p<nPoloidalPlanes; ++p )
    {
      neighborIndex[p] = new int[nNodes];

      float *x_in_ptr = (float *) &(data_in[0][p*nNodes]);
      float *y_in_ptr = (float *) &(data_in[1][p*nNodes]);
      float *z_in_ptr = (float *) &(data_in[2][p*nNodes]);

      float basePtX = *x_in_ptr; ++x_in_ptr;
      float basePtY = *y_in_ptr; ++y_in_ptr;
      float basePtZ = *z_in_ptr; ++z_in_ptr;

      int cc = 1;  // Temporary counter of the nodes in each poloidal contour

      poloidalIndex[p].push_back(0);

      for (int i=1; i<nNodes; i++)
      {
        float tmpPtX = *x_in_ptr; ++x_in_ptr;
        float tmpPtY = *y_in_ptr; ++y_in_ptr;
        float tmpPtZ = *z_in_ptr; ++z_in_ptr;

        ++cc;

        // In each contour the first and last node point are the same.
        // Well almost - a bit of rounding.
        if( diff( basePtX, basePtY, basePtZ, tmpPtX, tmpPtY, tmpPtZ ) < 1.0e-6 )
        {
          nPoloidalNodes[p].push_back(cc);

          // Restart for the next poloidal contour.
          if( i < nNodes-1 )
          {
            basePtX = *x_in_ptr; ++x_in_ptr;
            basePtY = *y_in_ptr; ++y_in_ptr;
            basePtZ = *z_in_ptr; ++z_in_ptr;

            ++i;

            poloidalIndex[p].push_back(i);

            cc = 1;
          }

          // Last node so quit.
          else
          {
            break;
          }
        }
      }


      /*
        cerr << "Plane " << p
             << " found " << nPoloidalNodes[p].size() << " Flux Surfaces" << endl;
      */

      if( poloidalIndex[p].size() != nPoloidalNodes[p].size() )
      {
        cerr << "Could not find matching start and stop points for plane "
             << p << endl;
        return;
      }

      if( poloidalIndex[p].size() != poloidalIndex[0].size() )
      {
        cerr << "poloidalIndex does match for plane " << p << endl;
        return;
      }

      if( nPoloidalNodes[p].size() != nPoloidalNodes[0].size() )
      {
        cerr << "nPoloidalNodes does match for plane " << p << "  "
             << nPoloidalNodes[p].size() << "  " << nPoloidalNodes[0].size() << endl;
        return;
      }

      for (int j=0; j<poloidalIndex[p].size(); j++ ) {
        if( poloidalIndex[p][j] != poloidalIndex[0][j] )
        {
          cerr << "poloidalIndex " << j << " does match for plane " << p << "  "
               << poloidalIndex[p][j] << "  " << poloidalIndex[0][j]
               << endl;
          return;
        }

        if( nPoloidalNodes[p][j] != nPoloidalNodes[0][j] )
        {
          cerr << "nPoloidalNodes " << j << " does match for plane " << p << "  "
               << nPoloidalNodes[p][j] << "  " << nPoloidalNodes[0][j] << endl;
          return;
        }
      }

      // For each point get the nearest neighbor on the flux surface.

      // Work from the outside to the inside because there are more nodes
      // on the outside. As such, neighbors will get used multiple times
      // thus allowing for degenerate connections to be found.
      for (int k=nPoloidalNodes[p].size()-1; k>0; --k)
      {
        for (int j=0; j<nPoloidalNodes[p][k]-1; ++j )
        {
          // Index of the working node.
          int l = poloidalIndex[p][k] + j;

          basePtX = data_in[0][p*nNodes+l];
          basePtY = data_in[1][p*nNodes+l];
          basePtZ = data_in[2][p*nNodes+l];

          float rmin = 1.0e8;

          // Find the node on the adjacent contour that is the closest to
          // the working node. Brute force search.

          // Never search the last node because it is the same as the
          // first node.
          for (int i=0; i<nPoloidalNodes[p][k-1]-1; i++ )
          {
            // Index of the test node.
            int m = p*nNodes+poloidalIndex[p][k-1] + i;

            float tmpPtX = data_in[0][m];
            float tmpPtY = data_in[1][m];
            float tmpPtZ = data_in[2][m];

            float rdiff =
              diff( basePtX, basePtY, basePtZ, tmpPtX, tmpPtY, tmpPtZ );

            if (rdiff <= rmin )
            {
              neighborIndex[p][l] = m;
              rmin = rdiff;
            }
          }
        }
      }

      // Now check for degeneracies.
      degeneracies[p].resize(nPoloidalNodes[p].size());

      // Work from the outside to the inside because there are more nodes
      // on the outside. As such, neighbors will get used multiple times
      // thus allowing for degenerate connections to be found.
      for (int k=nPoloidalNodes[p].size()-1; k>0; k--)
      {
        for (int j=0; j<nPoloidalNodes[p][k]-1; j++ )
        {
          int l  = poloidalIndex[p][k] + j;
          int l1 = (l + 1);

          // Never use the last node cause it is the same as the first
          // node.
          if (l1 == poloidalIndex[p][k] + nPoloidalNodes[p][k] - 1)
            l1 = poloidalIndex[p][k];

          // Check for a degeneracy.
          if( neighborIndex[p][l] == neighborIndex[p][l1] )
            degeneracies[p][k].push_back(j);
        }

        // The number of degenerate connections should always equal the
        // difference in the number of nodes between two countors.
        if( degeneracies[p][k].size() !=
            nPoloidalNodes[p][k] - nPoloidalNodes[p][k-1] )
        {
          cerr << "The number of degenerate connections, "
               << degeneracies[p][k].size() << " does not equal the " << endl
               << "difference in the number of nodes between two countors, "
               << nPoloidalNodes[p][k] - nPoloidalNodes[p][k-1] << "."
               << "  " << nPoloidalNodes[p].size() << "  " << k
               << "  " << nPoloidalNodes[p][k] << "  " << nPoloidalNodes[p][k-1]
               << endl;
          return;
        }

        if( degeneracies[p][k].size() != degeneracies[0][k].size() )
        {
          cerr << "For plane " << p
               << " on flux surface " << k
               << " the number of degeneracies, " << degeneracies[p][k].size()
               << " does match the base, "
               << degeneracies[0][k].size() << endl;

          return;
        }
      }
    }

    // Total number of elements.
    numElements = 0;

    // If only one slice then create connections for trisurf.
    if( nPoloidalPlanes == 1 || TRIANGLES )
    {
      // Create a local array of axis sizes (Nx3)
      numVertices = 3;
      *vertex_list =
        (int *) new int*[ (nPoloidalPlanes==1 ? 1 : nPoloidalPlanes-1) *
                          nNodes * 2 * numVertices];
      //DRP
      *vertex_list =
        (int *) new int*[ (nPoloidalPlanes==1 ? 1 : nPoloidalPlanes) *
                          nNodes * 2 * numVertices];

      int *vertex_list_ptr = *vertex_list;
      //DRP
      //for( int p=0; p<(nPoloidalPlanes==1 ? 1 : nPoloidalPlanes-1); ++p )
      for( int p=0; p<nPoloidalPlanes; ++p )
      {
        // Work from the outside to the inside because there are more nodes
        // on the outside. As such, neighbors will get used multiple times
        // thus allowing for degenerate connections to be found.
        for (int k=nPoloidalNodes[p].size()-1; k>0; k--)
        {
          for (int j=0; j<nPoloidalNodes[p][k]-1; j++ )
          {
            int l  = poloidalIndex[p][k] + j;
            int l1 = (l + 1);

            // Never use the last node cause it is the same as the first
            // node.
            if (l1 == poloidalIndex[p][k] + nPoloidalNodes[p][k] - 1)
              l1 = poloidalIndex[p][k];

            // Check for a degenerate triangle
            if( neighborIndex[p][l] == neighborIndex[p][l1] )
            {
              vertex_list_ptr[0] = p*nNodes+l;
              vertex_list_ptr[1] = p*nNodes+l1;
              vertex_list_ptr[2] = neighborIndex[p][l];

              vertex_list_ptr += numVertices;
              ++numElements;
            }

            else
            {
              vertex_list_ptr[0] = p*nNodes+l;
              vertex_list_ptr[1] = p*nNodes+l1;
              vertex_list_ptr[2] = neighborIndex[p][l];

              vertex_list_ptr += numVertices;
              ++numElements;


              vertex_list_ptr[0] = p*nNodes+l1;
              vertex_list_ptr[1] = neighborIndex[p][l1];
              vertex_list_ptr[2] = neighborIndex[p][l];

              vertex_list_ptr += numVertices;
              ++numElements;
            }
          }
        }
      }
    }

    // Multiple slices so create connections for prisms.

    // This version connects the prisms without any enforcement of
    // continuity from one prism to another across a plane.
    else if ( 0 && PRISMS )
    {
      // Create a local array of axis sizes (Nx6)
      numVertices = 6;

      // This
      *vertex_list =
        (int *) new int*[ (nPoloidalPlanes-1) * nNodes * 2 * numVertices ];

      int *vertex_list_ptr = *vertex_list;
      vector<int> numElemPlane;

      //  Connect along the toriodal direction.
      for( int p=0, q=1; p<nPoloidalPlanes-1; ++p, ++q )
      {
        bool degenerate = false;

        // Work from the outside to the inside because there are more nodes
        // on the outside. As such, neighbors will get used multiple times
        // thus allowing for degenerate connections to be found.
        for (int k=nPoloidalNodes[p].size()-1; k>0; k--)
        {
          for (int j=0; j<nPoloidalNodes[p][k]-1; j++ )
          {
            int l  = poloidalIndex[p][k] + j;
            int l1 = (l + 1);

            int m  = poloidalIndex[q][k] + j;
            int m1 = (m + 1);

            if( l != m || l1 != m1 )
              cerr << l << "  " << l1 << "  " << m << "  " << m1 << endl;

            // Never use the last node cause it is the same as the first
            // node.
            if (l1 == poloidalIndex[p][k] + nPoloidalNodes[p][k] - 1)
              l1 = poloidalIndex[p][k];

            if (m1 == poloidalIndex[q][k] + nPoloidalNodes[q][k] - 1)
              m1 = poloidalIndex[q][k];

            //  Connect along the poliodal plane Prism 1.
            vertex_list_ptr[0] = p*nNodes+l;
            vertex_list_ptr[1] = p*nNodes+l1;
            vertex_list_ptr[2] = neighborIndex[p][l];

            vertex_list_ptr[3] = q*nNodes+l;
            vertex_list_ptr[4] = q*nNodes+l1;
            vertex_list_ptr[5] = neighborIndex[p][l] + nNodes;

            vertex_list_ptr += numVertices;
            ++numElements;

            // Second prism has a degeneracy
            if( neighborIndex[p][l] != neighborIndex[p][l1] )
            {
              //  Connect along the poliodal plane Prism 2.
              vertex_list_ptr[0] = p*nNodes+l1;
              vertex_list_ptr[1] = neighborIndex[p][l1];
              vertex_list_ptr[2] = neighborIndex[p][l];

              vertex_list_ptr[3] = q*nNodes+l1;
              vertex_list_ptr[4] = neighborIndex[p][l1] + nNodes;
              vertex_list_ptr[5] = neighborIndex[p][l] + nNodes;

              vertex_list_ptr += numVertices;
              ++numElements;

              degenerate = true;
            }
          }
        }
        numElemPlane.push_back(numElements);
      }
    }

    // Multiple slices so create connections for prisms.

    // This version connects the prisms while trying to enforce
    // continuity from one prism to another across a plane.
    else if ( PRISMS )
    {
      // Create a local array of axis sizes (Nx6)
      numVertices = 6;

      *vertex_list =
        (int *) new int*[ (nPoloidalPlanes-1) * nNodes * 2 * numVertices ];

      int *vertex_list_ptr = *vertex_list;

      //  Connect along the toriodal direction.
      vector<int> numElemPlane;
      for( int p=0, q=1; p<nPoloidalPlanes-1; ++p, ++q )
      {
        bool degenerate = false;
        int offset;

        // Work from the outside to the inside because there are more nodes
        // on the outside. As such, neighbors will get used multiple times
        // thus allowing for degenerate connections to be found.
        for (int k=nPoloidalNodes[p].size()-1; k>0; k--)
        {
          // Find the degeneracy pairs. Typically they are one to one
          // but wrap through zero for soem flux surface.
          int nd = degeneracies[p][k].size();

          // The degeracy pairs are typically only a few tens of pairs
          // of neighbor points. However, it is possible to for a set
          // of degenercies to start near the end and wrap around past
          // the first the node (aka through zero).
          float average = nPoloidalNodes[p][k];
          unsigned int index = 0;

          // Determine if the pairs go through zero. Normally if
          // perfectly aligned the pairs (i.e. the difference from the
          // degenercy on one plane to the other) last only a few dozen
          // elements. If not aligned then the degeneracy goes through
          // zero so shift the pairs by one in both directions.

          for( int j=-1; j<=1; ++j)
          {
            float tmp = 0;

            for( int i=0; i<nd; ++i)
            {
              if( j == -1 && i == 0 )
                offset = 1-nPoloidalNodes[p][k];

              else if ( j == 1 && i == 5 )
                offset = nPoloidalNodes[p][k]-1;

              else
                offset = 0;

              tmp += fabs(offset +
                          degeneracies[q][k][(i+j+nd)%nd] -
                          degeneracies[q][k][i]);
            }

            if( average > tmp / (float) nd )
            {
              index = j;
              average = tmp / (float) nd;
            }
          }

          if( index )
          {
            for( int i=0; i<nd; ++i)
              cerr << p << "  " << k << "  " << index << "  " << i << "  "
                   << degeneracies[p][k][i] << "  " << degeneracies[q][k][i] << endl;
          }

          // Sort the degeneracy locations after the index
          // shifting. These values are used only for getting an
          // offset for when the degeneracy goes through
          // zero. Otherwise they are just for debugging.
          vector< int > p_degeneracies;
          vector< int > n_degeneracies;

          p_degeneracies.resize( nd );
          n_degeneracies.resize( nd );

          for( int i=0; i<nd; ++i)
            n_degeneracies[i] = degeneracies[q][k][i];

          if( index == -1 )
          {
            p_degeneracies[0] =
              degeneracies[p][k][nd-1] + (1-nPoloidalNodes[p][k]);

            for( int i=nd-1; i>0; --i)
              p_degeneracies[i] = degeneracies[p][k][i-1];
          }
          else if( index == 0 )
          {
            for( int i=0; i<nd; ++i)
              p_degeneracies[i] = degeneracies[p][k][i];
          }
          else if( index == 1 )
          {
            p_degeneracies[nd-1] =
              degeneracies[p][k][0] + (nPoloidalNodes[p][k]-1);

            for( int i=0; i<nd-1; ++i)
              p_degeneracies[i] = degeneracies[p][k][i+1];
          }

          // Start in a non degenerate area.
          if( index && p_degeneracies[0] < n_degeneracies[0] )
            offset = n_degeneracies[0] + 1;

          else if( index && p_degeneracies[0] > n_degeneracies[0] )
            offset = p_degeneracies[0] + 1;

          else
            offset = 0;

          int nFluxNodes = nPoloidalNodes[p][k] - 1;

          if( offset )
          {
            cerr << endl;

            for( int i=0; i<nd; ++i)
              cerr << p << "  " << k << "  " << index << "  " << i << "  "
                   << p_degeneracies[i] << "  " << n_degeneracies[i] << endl;
          }

          // Find the first degeneracy location that is different.
          index = 0;

          while( index < nd-1 &&
                 p_degeneracies[index] != n_degeneracies[index] )
            ++index;

          // If the degeneracy pair starts on the neighbor plane
          // use the neighbor info from the primary plane.
          // i.e. the primary plane the drives the connections
          if( p_degeneracies[index] >= n_degeneracies[index] )
          {
            for (int j=0; j<nPoloidalNodes[p][k]-1; j++ )
            {
              int l  = poloidalIndex[p][k] + (j+offset) % nFluxNodes;
              int l1 = (l + 1);

              int m  = poloidalIndex[q][k] + (j+offset) % nFluxNodes;
              int m1 = (m + 1);

              if( l != m || l1 != m1 )
                cerr << l << "  " << l1 << "  " << m << "  " << m1 << endl;

              // Never use the last node cause it is the same as the first
              // node.
              if (l1 == poloidalIndex[p][k] + nPoloidalNodes[p][k] - 1)
                l1 = poloidalIndex[p][k];

              if (m1 == poloidalIndex[q][k] + nPoloidalNodes[q][k] - 1)
                m1 = poloidalIndex[q][k];

              // Degeneracy previously found on neighbor plane and the
              // second prism on the primary plane has a degeneracy
              if( (degenerate &&
                   neighborIndex[p][l] == neighborIndex[p][l1]) ||
                  // Degeneracy is in the same location.
                  (neighborIndex[p][l] == neighborIndex[p][l1] &&
                   neighborIndex[q][l] == neighborIndex[q][l1]) )
              {
                //  Connect along the poliodal plane Prism 1 using the
                //  primary plane for the neighbor connections.
                vertex_list_ptr[0] = p*nNodes+l;
                vertex_list_ptr[1] = p*nNodes+l1;
                vertex_list_ptr[2] = neighborIndex[p][l];

                vertex_list_ptr[3] = q*nNodes+m;
                vertex_list_ptr[4] = q*nNodes+m1;
                vertex_list_ptr[5] = neighborIndex[p][m] + nNodes;

                vertex_list_ptr += numVertices;
                ++numElements;

                degenerate = false;

                if( offset )
                  cerr << "off " << (j+offset) % nFluxNodes << " ";
              }

              // Standard arbitrary ordering using the primary plane
              // for the neighbor connections.
              else
              {
                //  Connect along the poliodal plane Prism 1.
                vertex_list_ptr[0] = p*nNodes+l;
                vertex_list_ptr[1] = p*nNodes+l1;
                vertex_list_ptr[2] = neighborIndex[p][l];

                vertex_list_ptr[3] = q*nNodes+m;
                vertex_list_ptr[4] = q*nNodes+m1;
                vertex_list_ptr[5] = neighborIndex[p][m] + nNodes;

                vertex_list_ptr += numVertices;
                ++numElements;

                //  Connect along the poliodal plane Prism 2.
                vertex_list_ptr[0] = p*nNodes+l1;
                vertex_list_ptr[1] = neighborIndex[p][l1];
                vertex_list_ptr[2] = neighborIndex[p][l];

                vertex_list_ptr[3] = q*nNodes+m1;
                vertex_list_ptr[4] = neighborIndex[p][m1] + nNodes;
                vertex_list_ptr[5] = neighborIndex[p][m] + nNodes;

                vertex_list_ptr += numVertices;
                ++numElements;

                // Second prism on the neighbor plane has a degeneracy
                if( neighborIndex[q][m] == neighborIndex[q][m1] )
                {
                  degenerate = true;

                  if( offset )
                    cerr << "on " << (j+offset) % nFluxNodes << " ";
                }
              }
            }

            if( offset )
              cerr << endl;
          }

          // If the degeneracy pair starts on the primary plane then for
          // simplicity use the neighbor info from the neighbor plane.
          // i.e. the neighbor plane is the drives the connections
          else // if( p_degeneracies[index] < n_degeneracies[index] )
          {
            for (int j=0; j<nPoloidalNodes[p][k]-1; j++ )
            {
              int l  = poloidalIndex[p][k] + (j+offset) % nFluxNodes;
              int l1 = (l + 1);

              int m  = poloidalIndex[q][k] + (j+offset) % nFluxNodes;
              int m1 = (m + 1);

              if( l != m || l1 != m1 )
                cerr << l << "  " << l1 << "  " << m << "  " << m1 << endl;

              // Never use the last node cause it is the same as the first
              // node.
              if (l1 == poloidalIndex[p][k] + nPoloidalNodes[p][k] - 1)
                l1 = poloidalIndex[p][k];

              if (m1 == poloidalIndex[q][k] + nPoloidalNodes[q][k] - 1)
                m1 = poloidalIndex[q][k];

              // Degeneracy previously found on the primary plane and
              // the second prism on the neighbor plane has a degeneracy
              if( (degenerate &&
                   neighborIndex[q][l] == neighborIndex[q][l1]) ||
                  // Degeneracy is in the same location.
                  (neighborIndex[p][l] == neighborIndex[p][l1] &&
                   neighborIndex[q][l] == neighborIndex[q][l1]) )
              {
                //  Connect along the poliodal plane Prism 1 using the
                //  neighbor plane for the neighbor connections.
                vertex_list_ptr[0] = p*nNodes+l;
                vertex_list_ptr[1] = p*nNodes+l1;
                vertex_list_ptr[2] = neighborIndex[q][l] - nNodes;

                vertex_list_ptr[3] = q*nNodes+m;
                vertex_list_ptr[4] = q*nNodes+m1;
                vertex_list_ptr[5] = neighborIndex[q][m];

                vertex_list_ptr += numVertices;
                ++numElements;

                degenerate = false;

                if( offset )
                  cerr << "off " << (j+offset) % nFluxNodes << " ";
              }

              // Standard arbitrary ordering using the neighbor plane
              // for the neighbor connections.
              else
              {
                //  Connect along the poliodal plane Prism 1.
                vertex_list_ptr[0] = p*nNodes+l;
                vertex_list_ptr[1] = p*nNodes+l1;
                vertex_list_ptr[2] = neighborIndex[q][l] - nNodes;

                vertex_list_ptr[3] = q*nNodes+m;
                vertex_list_ptr[4] = q*nNodes+m1;
                vertex_list_ptr[5] = neighborIndex[q][m];

                vertex_list_ptr += numVertices;
                ++numElements;

                //  Connect along the poliodal plane Prism 2.
                vertex_list_ptr[0] = p*nNodes+l1;
                vertex_list_ptr[1] = neighborIndex[q][l1] - nNodes;
                vertex_list_ptr[2] = neighborIndex[q][l] - nNodes;

                vertex_list_ptr[3] = q*nNodes+m1;
                vertex_list_ptr[4] = neighborIndex[q][m1];
                vertex_list_ptr[5] = neighborIndex[q][m];

                vertex_list_ptr += numVertices;
                ++numElements;

                // Second prism on the primary plane has a degeneracy
                if( neighborIndex[p][m] == neighborIndex[p][m1] )
                {
                  degenerate = true;

                  if( offset )
                    cerr << "on " << (j+offset) % nFluxNodes << " ";
                }
              }
            }

            if( offset )
              cerr << endl;
          }
        }
      }
    }

    // Multiple slices so create connections for hexahedrals.
    else if( HEXAHEDRALS )
    {
      // Create a local array of axis sizes (Nx6)
      numVertices = 8;

      *vertex_list =
        (int *) new int* [(nPoloidalPlanes-1) * nNodes * numVertices];

      int *vertex_list_ptr = *vertex_list;

      //  Connect along the toriodal direction.
      for( int p=0, q=1; p<nPoloidalPlanes-1; ++p, ++q )
      {
        // Work from the outside to the inside because there are more nodes
        // on the outside. As such, neighbors will get used multiple times
        // thus allowing for degenerate connections to be found.
        for (int k=nPoloidalNodes[p].size()-1; k>0; k--)
        {
          for (int j=0; j<nPoloidalNodes[p][k]-1; j++ )
          {
            int l  = poloidalIndex[p][k] + j;
            int l1 = (l + 1);

            int m  = poloidalIndex[q][k] + j;
            int m1 = (m + 1);

            if( l != m || l1 != m1 )
              cerr << l << "  " << l1 << "  " << m << "  " << m1 << endl;

            // Never use the last node cause it is the same as the first
            // node.
            if (l1 == poloidalIndex[p][k] + nPoloidalNodes[p][k] - 1)
              l1 = poloidalIndex[p][k];

            if (m1 == poloidalIndex[q][k] + nPoloidalNodes[q][k] - 1)
              m1 = poloidalIndex[q][k];

            //  Connect along the poliodal plane.
            vertex_list_ptr[0] = p*nNodes+l;
            vertex_list_ptr[1] = p*nNodes+l1;
            vertex_list_ptr[2] = neighborIndex[p][l1];
            vertex_list_ptr[3] = neighborIndex[p][l];

            vertex_list_ptr[4] = q*nNodes+m;
            vertex_list_ptr[5] = q*nNodes+m1;
            vertex_list_ptr[6] = neighborIndex[q][m1];
            vertex_list_ptr[7] = neighborIndex[q][m];

            vertex_list_ptr += numVertices;
            ++numElements;
          }
        }
      }
    }

    for( int p=0; p<nPoloidalPlanes; ++p )
      delete[] neighborIndex[p];
}

static void
createMapping(const vector<int> &igrid,
              const vector<int> &indexShift,
              vector<int> &pn)
{
    for (int i = 0; i < pn.size(); i++)
        pn[i] = -1;

    /*
    cout<<"CreateMapping: "<<igrid.size()<<" "<<indexShift.size()<<endl;
    cout<<"igrid: "<<igrid<<endl;
    cout<<"shift: "<<indexShift<<endl;
    */

    int n = igrid.size();
    for (int gi = 0; gi < n-1; gi++)
    {
        int n0 = igrid[gi], nn = igrid[gi+1]-1;
        int shift = indexShift[gi];
        //cout<<gi<<": Create Mapping: "<<n0<<" "<<nn<<"  shift= "<<shift<<endl;
        for (int i = 0; i < (nn-n0); i++)
        {
            int i0 = i;
            int i1 = i-shift;
            if (i1 < 0)
                i1 = i1+(nn-n0);
            pn[n0 + i0] = n0+i1;
        }
    }

    /*
    int cnt = 0;
    for (int i = 0; i < pn.size(); i++)
        if (pn[i] < 0) cnt++;
    cout<<" ******** no mapping count: "<<cnt<<" sz: "<<pn.size()<<endl;
    */
}

// ****************************************************************************
//  Method: avtGTCFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      timestate   The index of the timestate.  If GetNTimesteps returned
//                  'N' time steps, this is guaranteed to be between 0 and N-1.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: pugmire -- generated by xml2avt
//  Creation:   Thu Apr 12 08:39:47 PDT 2018
//
// ****************************************************************************

vtkUnstructuredGrid *
avtGTCFileFormat::CreateMesh(bool xyzMesh)
{
    adios2::Variable<float> cVar = io.InquireVariable<float>("coordinates");
    adios2::Variable<int> igridVar = io.InquireVariable<int>("igrid");
    adios2::Variable<int> indexShiftVar = io.InquireVariable<int>("index-shift");

    if (!cVar || !igridVar || !indexShiftVar)
        return NULL;

    vtkUnstructuredGrid *mesh = vtkUnstructuredGrid::New();

    auto cDims = cVar.Shape();

    vector<float> xbuff, ybuff, zbuff;
    cVar.SetSelection(adios2::Box<adios2::Dims>({0,0,0}, {cDims[0], 1, cDims[2]}));
    reader.Get(cVar, xbuff, adios2::Mode::Sync);
    cVar.SetSelection(adios2::Box<adios2::Dims>({0,1,0}, {cDims[0], 1, cDims[2]}));
    reader.Get(cVar, ybuff, adios2::Mode::Sync);
    cVar.SetSelection(adios2::Box<adios2::Dims>({0,2,0}, {cDims[0], 1, cDims[2]}));
    reader.Get(cVar, zbuff, adios2::Mode::Sync);

    auto igridDims = igridVar.Shape();
    auto indexShiftDims = indexShiftVar.Shape();
    vector<int> igrid, indexShift;
    igridVar.SetSelection(adios2::Box<adios2::Dims>({0,0}, {1,igridDims[1]}));
    reader.Get(igridVar, igrid, adios2::Mode::Sync);
    indexShiftVar.SetSelection(adios2::Box<adios2::Dims>({0,0}, {1,indexShiftDims[1]}));
    reader.Get(indexShiftVar, indexShift, adios2::Mode::Sync);

    std::swap(cDims[0], cDims[2]);
    int numPlanes = cDims[2];
    int ptsPerPlane = cDims[0];

    if (!xyzMesh)
    {
        float phi = 0.0f;
        float dPhi = (2.0*M_PI)/(numPlanes-1);
        for (int i = 0; i < numPlanes-1; i++)
        {
            //cout<<"PHI= "<<phi*(180.0/M_PI)<<endl;
            for (int j = 0; j < ptsPerPlane; j++)
            {
                int idx = i*ptsPerPlane+j;
                float x = xbuff[idx];
                float y = ybuff[idx];
                float z = zbuff[idx];
                float r = sqrt(x*x + y*y);

                xbuff[idx] = r;
                ybuff[idx] = phi;
                zbuff[idx] = z;
            }
            phi += dPhi;
        }

        //We need to copy the first plane to the last.
        int idx = numPlanes*ptsPerPlane;
        xbuff.resize((numPlanes+1)*ptsPerPlane);
        ybuff.resize((numPlanes+1)*ptsPerPlane);
        zbuff.resize((numPlanes+1)*ptsPerPlane);
        for (int j = 0; j < ptsPerPlane; j++)
        {
            xbuff[idx+j] = xbuff[j];
            ybuff[idx+j] = ybuff[j];
            zbuff[idx+j] = zbuff[j];
        }
    }

    //create mesh elements between planes
    int numE, numV;
    int *vertexList = NULL;
    create_potential_mesh_vertex_list(&xbuff[0],
                                      &ybuff[0],
                                      &zbuff[0],
                                      //numPlanes,
                                     (xyzMesh ? numPlanes : numPlanes-1),
                                      ptsPerPlane,
                                      &vertexList,
                                      numE, numV);

    int numPts = xbuff.size();
    vtkPoints *pts = vtkPoints::New();
    pts->SetNumberOfPoints(numPts);

    mesh->SetPoints(pts);
    pts->Delete();

    //set points
    for (int i = 0; i < numPts; i++)
        pts->SetPoint(i, xbuff[i], ybuff[i], zbuff[i]);

    // add prism elements.
    vtkIdType wedge[16];
    for (int i = 0; i < numE; i++)
    {
        for (int j = 0; j < numV; j++)
            wedge[j] = vertexList[i*numV+j];
        mesh->InsertNextCell(VTK_WEDGE, 6, wedge);
    }
    if (!xyzMesh)
        return mesh;


    //Connect first and last plane.
    //This requires the igrid to map nodes from last plane to first plane.

    vector<int> pn(ptsPerPlane);
    //igrid is fortran, to convert to 0-based.
    //also, igrid needs the last point, so add it.
    for (int i = 0; i < igrid.size(); i++)
        igrid[i]--;
    igrid.push_back(ptsPerPlane);

    createMapping(igrid, indexShift, pn);

    int offset = ptsPerPlane*(numPlanes-1);
    int nnn = numE/(numPlanes-1);

    for (int i = 0; i < numE; i++)
    {
        int ids[3] = {vertexList[i*numV+0],
                      vertexList[i*numV+1],
                      vertexList[i*numV+2]};

        //If the ids are on plane 0, then add the connecting element.
        if (ids[0] < ptsPerPlane && ids[1] < ptsPerPlane && ids[2] < ptsPerPlane)
        {
            int idsN[3] = {pn[ids[0]], pn[ids[1]], pn[ids[2]]};

            wedge[0] = ids[0];
            wedge[1] = ids[1];
            wedge[2] = ids[2];
            wedge[3] = idsN[0] + offset;
            wedge[4] = idsN[1] + offset;
            wedge[5] = idsN[2] + offset;
            mesh->InsertNextCell(VTK_WEDGE, 6, wedge);
        }
    }

    return mesh;
}

vtkDataSet *
avtGTCFileFormat::GetMesh(int timestate, const char *meshname)
{
    if (!strcmp(meshname, "mesh_pt"))
        return GetPtMesh(timestate, meshname);

    bool xyzMesh = (string(meshname) == "mesh");

    if (xyzMesh && grid != NULL)
    {
        grid->Register(NULL);
        return grid;
    }
    else if (!xyzMesh && cylGrid != NULL)
    {
        cylGrid->Register(NULL);
        return cylGrid;
    }

    //Create the grid.
    vtkUnstructuredGrid *mesh = CreateMesh(xyzMesh);

    if (mesh == NULL)
        EXCEPTION1(ImproperUseException, "Invalid file");

    //Need to hold onto this point, so add-ref it.
    if (xyzMesh)
    {
        grid = mesh;
        grid->Register(NULL);
        return grid;
    }
    else
    {
        cylGrid = mesh;
        cylGrid->Register(NULL);
        return cylGrid;
    }
}

// ****************************************************************************
//  Method: avtGTCFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      varname    The name of the variable requested.
//
//  Programmer: pugmire -- generated by xml2avt
//  Creation:   Thu Apr 12 08:39:47 PDT 2018
//
// ****************************************************************************

vtkDataArray *
avtGTCFileFormat::GetVar(int timestate, const char *varname)
{
    string vname(varname);
    if (string(varname) == "potential_pt" || string(varname) == "potential_cyl")
        vname = "potential";

    adios2::Variable<float> var = io.InquireVariable<float>(vname);
    if (!var)
        return NULL;

    auto dims = var.Shape();

    vtkFloatArray *arr = vtkFloatArray::New();
    arr->SetNumberOfTuples(dims[0]*dims[1]);

    var.SetStepSelection({timestate, 1});
    var.SetSelection(adios2::Box<adios2::Dims>({0,0}, dims));
    reader.Get(var, (float*)arr->GetVoidPointer(0), adios2::Mode::Sync);

    return arr;
}

// ****************************************************************************
//  Method: avtGTCFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      varname    The name of the variable requested.
//
//  Programmer: pugmire -- generated by xml2avt
//  Creation:   Thu Apr 12 08:39:47 PDT 2018
//
// ****************************************************************************

vtkDataArray *
avtGTCFileFormat::GetVectorVar(int timestate, const char *varname)
{
    return NULL;
}
