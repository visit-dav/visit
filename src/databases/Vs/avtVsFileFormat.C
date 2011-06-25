#include <hdf5.h>
#include <visit-hdf5.h>
#if HDF5_VERSION_GE(1,8,1)
/**
 *
 * @file        avtVsFileFormat.cpp
 *
 * @brief       Implementation of base class for VSH5 visit plugins
 *
 * @version $Id: avtVsFileFormat.cpp 42 2008-03-31 23:09:33Z paulh $
 *
 * Copyright &copy; 2007, Tech-X Corporation
 * See LICENSE file for conditions of use.
 *
 */

//Along with PARALLEL (defined by build system) enables
// For testing outside of VisIt, be sure to do #define PARALLEL
// domain decomposition code added by Gunther
//#define PARALLEL
//#define VIZSCHEMA_DECOMPOSE_DOMAINS

// std includes
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <stdlib.h>
#include <string>

// VTK includes
#include <vtkPointData.h>
#include <vtkVisItUtility.h>
#include <vtkIntArray.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkStructuredGrid.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkCellType.h>

// VisIt includes
#include <avtDatabaseMetaData.h>
#include <avtMeshMetaData.h>
#include <DebugStream.h>
#include <InvalidVariableException.h>
#include <InvalidDBTypeException.h>
#include <VsPluginInfo.h>

#include <avtLogicalSelection.h>
#include <avtSpatialBoxSelection.h>

// definition of VISIT_VERSION
#include <visit-config.h>

#include <avtVsFileFormat.h>

// #define PARALLEL 1
// #define VIZSCHEMA_DECOMPOSE_DOMAINS 1

#if (defined PARALLEL && defined VIZSCHEMA_DECOMPOSE_DOMAINS)
#include <avtParallel.h>
#endif

// VizSchema includes
#include <VsUtils.h>
#include "VsVariable.h"
#include "VsMDVariable.h"
#include "VsVariableWithMesh.h"
#include "VsMesh.h"
#include "VsMDMesh.h"
#include "VsH5Dataset.h"
#include "VsRectilinearMesh.h"
#include "VsUniformMesh.h"
#include "VsUnstructuredMesh.h"
#include "VsStructuredMesh.h"
#include "VsH5Attribute.h"
#include "VsLog.h"
#include "VsRegistry.h"
#include "VsH5Reader.h"

using namespace std;

// *****************************************************************************
//  Method: avtImageVileFormat::avtVsFileFormat
//
//  Purpose:
//      Reads in the image.
//
//  Programmer: Marc Durant
//  Creation:   June, 2010
//
//  Modifications:
//
avtVsFileFormat::avtVsFileFormat(const char* dfnm) :
  avtSTMDFileFormat(&dfnm, 1), dataFileName(dfnm)
{
    VsLog::initialize(DebugStream::Stream3(),
                      DebugStream::Stream4(),
                      DebugStream::Stream5());

    string methodSig("avtVsFileFormat::constructor() - ");
    
    VsLog::debugLog() << methodSig << "entering" << endl;
    VsLog::debugLog() << methodSig << "VizSchema Revision #742" << endl;

    //reader starts off empty
    reader = NULL;
    
    //Initialize the registry for objects
    registry = new VsRegistry();
    
//  LoadData();

    //check types
    if (isFloatType(H5T_NATIVE_FLOAT)) {
      VsLog::debugLog() << methodSig << "float type checks out ok." << endl;
    } else {
      string msg = methodSig +
        "ERROR - H5T_NATIVE_FLOAT not recognized as a float.";
      VsLog::debugLog() << msg << endl;
      EXCEPTION1(InvalidDBTypeException, msg.c_str());
    }
    
    //check types
    if (isDoubleType(H5T_NATIVE_DOUBLE)) {
      VsLog::debugLog() << methodSig << "double type checks out ok." << endl;
    } else {
      string msg = methodSig +
        "ERROR - H5T_NATIVE_DOUBLE not recognized as a double.";
      VsLog::debugLog() << msg << endl;
      EXCEPTION1(InvalidDBTypeException, msg.c_str());
    }

    herr_t err = H5check();

    if (err < 0) {
      string msg = methodSig +
        "HDF5 version mismatch.  Vs reader built with " + H5_VERS_INFO + ".";
      VsLog::debugLog() << msg << endl;

      EXCEPTION1(InvalidDBTypeException, msg.c_str());
    }

    //NOTE: We used to initialize the VsH5Reader object here
    //But now do it on demand in 'populateDatabaseMetaData'
    //To minimize I/O

    VsLog::debugLog() << methodSig << "exiting" << endl;
}


// *****************************************************************************
//  Method: avtImageVileFormat::~avtVsFileFormat
//
//  Purpose:
//      How do you do the voododo that you do
//
//  Programmer: Marc Durant
//  Creation:   June, 2010
//
//  Modifications:
//

avtVsFileFormat::~avtVsFileFormat()
{
    VsLog::debugLog() << "avtVsFileFormat::destructor() - entering" << endl;

    if (reader != NULL) {
      delete reader;
      reader = NULL;
    }

    if (registry) {
      delete registry;
      registry = NULL;
    }
    
    VsLog::debugLog() << "avtVsFileFormat::destructor() - exiting" << endl;
}


// ****************************************************************************
//  Method: avtVsFileFormat::CanCacheVariable
//
//  Purpose:
//      To truly exercise the VS file format, we can't have VisIt caching
//      chunks of mesh and variables above the plugin.
//      
//  Programmer: Mark C. Miller 
//  Creation:   September 20, 2004 
//
// ****************************************************************************

bool
avtVsFileFormat::CanCacheVariable(const char *var)
{
    return false;
}


// ****************************************************************************
//  Method: avtVsFileFormat::RegisterDataSelections
//
//  Purpose:
//      The Vs format can exploit some data selections so get them. 
//      
//  Programmer: Allen Sanderson
//  Creation:   March 4, 2011
//
// ****************************************************************************

void
avtVsFileFormat::RegisterDataSelections(const vector<avtDataSelection_p> &sels,
                                        vector<bool> *selectionsApplied)
{
  selList     = sels;
  selsApplied = selectionsApplied;
}


// ****************************************************************************
//  Method: avtVsFileFormat::ProcessDataSelections
//
//  Purpose:
//      The Vs format can exploit some data selections so process them. 
//      
//  Programmer: Allen Sanderson
//  Creation:   March 4, 2011
//
// ****************************************************************************

bool
avtVsFileFormat::ProcessDataSelections(int *mins, int *maxs, int *strides)
{
    bool retval = false;

    avtLogicalSelection composedSel;

    for (int i = 0; i < selList.size(); i++)
    {
        if (string(selList[i]->GetType()) == "Logical Data Selection")
        {
            avtLogicalSelection *sel = (avtLogicalSelection *) *(selList[i]);

            // overrwrite method-scope arrays with the new indexing
            composedSel.Compose(*sel);
            (*selsApplied)[i] = true;
            retval = true;
        }
        else if (string(selList[i]->GetType()) == "Spatial Box Data Selection")
        {
            avtSpatialBoxSelection *sel =
              (avtSpatialBoxSelection *) *(selList[i]);

            double dmins[3], dmaxs[3];
            sel->GetMins(dmins);
            sel->GetMaxs(dmaxs);
            avtSpatialBoxSelection::InclusionMode imode =
                sel->GetInclusionMode();

            // we won't handle clipping of zones here
            if ((imode != avtSpatialBoxSelection::Whole) &&
                (imode != avtSpatialBoxSelection::Partial))
            {
                (*selsApplied)[i] = false;
                continue;
            }

            int imins[3], imaxs[3];
            for (int j = 0; j < 3; j++)
            {
                int imin = (int) dmins[j];
                if (((double) imin < dmins[j]) &&
                    (imode == avtSpatialBoxSelection::Whole))
                    imin++;
                
                int imax = (int) dmaxs[j];
                if (((double) imax < dmaxs[j]) &&
                    (imode == avtSpatialBoxSelection::Partial))
                    imax++;

                imins[j] = imin;
                imaxs[j] = imax;
            }

            avtLogicalSelection newSel;
            newSel.SetStarts(imins);
            newSel.SetStops(imaxs);

            composedSel.Compose(newSel);
            (*selsApplied)[i] = true;
            retval = true;
        }
        else
        {
            // indicate we won't handle this selection
            (*selsApplied)[i] = false;
        }
    }

    composedSel.GetStarts(mins);
    composedSel.GetStops(maxs);
    composedSel.GetStrides(strides);

    return retval;
}


// *****************************************************************************
//  Method: avtImageVileFormat::GetMesh
//
//  Purpose:
//      How do you do the voododo that you do
//
//  Programmer: Marc Durant
//  Creation:   June, 2010
//
//  Modifications:
//

vtkDataSet* avtVsFileFormat::GetMesh(int domain, const char* name)
{
    bool haveDataSelections;

    int mins[3], maxs[3], strides[3];

    // Adjust for the data selections which are ZONAL.
    if( haveDataSelections = ProcessDataSelections(mins, maxs, strides) )
    {
      VsLog::debugLog()
        << "Have a logical zonal selection for mesh "<< endl
        << "(" << mins[0] << "," << maxs[0] << " stride " << strides[0] << ") "
        << "(" << mins[1] << "," << maxs[1] << " stride " << strides[1] << ") "
        << "(" << mins[2] << "," << maxs[2] << " stride " << strides[2] << ") "
        << endl;
    }

    stringstream sstr;
    sstr << "avtVsFileFormat::GetMesh(" << domain << ", " << name << ") - ";
    string methodSig = sstr.str();
    VsLog::debugLog() << methodSig << "Entering function." << endl;
    string meshName = name;
    LoadData();

    // The MD system works by filtering the requests directed to it
    // into the name of the appropriate subordinate mesh.  For
    // example, in facets_core-edge-explicit we have three meshes
    // joined into one: MdMesh = {coreMesh, solMesh, privMesh} So if
    // we get a request for (MdMesh, 0), we change the name to
    // coreMesh and proceed normally

    // Check for MD mesh
    VsLog::debugLog() << methodSig << "Looking for MD mesh with name "
                      << meshName << endl;

    VsMDMesh* mdMeshMeta = registry->getMDMesh(meshName);
    VsMesh* meta = NULL;

    // Found an MD mesh with this name, try to load the mesh data from it
    if (mdMeshMeta != NULL) {
      VsLog::debugLog() << methodSig << "Found MD mesh with that name."
                        << endl;
      meshName = mdMeshMeta->getNameForBlock(domain);

      VsLog::debugLog() << methodSig
        << "Request for md mesh was filtered to regular mesh: "
        << meshName << endl;
      meta = mdMeshMeta->getBlock(domain);
    } else {
      VsLog::debugLog() << methodSig << "No MD mesh with that name."
                        << endl;
    }

    // Did we succeed in loading mesh data from MD mesh?
    if (meta == NULL) {
      VsLog::debugLog() << methodSig << "Trying to find regular mesh named: "
                        << meshName << endl;
      meta = registry->getMesh(meshName);
    }

    if (meta != NULL) {
      VsLog::debugLog() << methodSig
                        << "Found mesh named: " << meshName << endl;

      // Uniform Mesh
      if (meta->isUniformMesh()) {
        VsLog::debugLog() << methodSig
          << "Trying to load & return uniform mesh" << endl;

        return getUniformMesh(static_cast<VsUniformMesh*>(meta),
                              haveDataSelections, mins, maxs, strides);
      }

      // Rectilinear Mesh
      if (meta->isRectilinearMesh()) {
        VsLog::debugLog() << methodSig
          << "Trying to load & return rectilinear mesh." << endl;
        return getRectilinearMesh(static_cast<VsRectilinearMesh*>(meta),
                                  haveDataSelections, mins, maxs, strides);
      }
      
      // Structured Mesh
      if (meta->isStructuredMesh()) {
        VsLog::debugLog() << methodSig
          << "Trying to load & return structured mesh" << endl;
        return getStructuredMesh(static_cast<VsStructuredMesh*>(meta),
                                 haveDataSelections, mins, maxs, strides);
      }

#if (defined PARALLEL && defined VIZSCHEMA_DECOMPOSE_DOMAINS)
      // Don't know how to decompose any other type of mesh -> load it
      // on proc 0 only
      if (PAR_Rank() > 0) {
        VsLog::debugLog() << methodSig
          << "In parallel mode on processor " << PAR_Rank
          << " and mesh is not uniform.  "
          << "Returning NULL, mesh will be loaded on processor 0 only."
          << endl;
        return NULL;
      }
#endif

      // Unstructured Mesh
      if (meta->isUnstructuredMesh()) {
        VsLog::debugLog() << methodSig
          << "Trying to load & return unstructured mesh" << endl;
        return getUnstructuredMesh(static_cast<VsUnstructuredMesh*>(meta),
                                   haveDataSelections, mins, maxs, strides);
      }

      // At this point we don't know what kind of mesh it is.
      VsLog::debugLog() << methodSig
                        << "Mesh has unknown type: " << meta->getKind()
                        << ".  Returning NULL." << endl;
      return NULL;
    }

    // Variable with mesh
    VsLog::debugLog() << methodSig
      << "Looking for Variable With Mesh with this name." << endl;
    VsVariableWithMesh* vmMeta = registry->getVariableWithMesh(name);

    if (vmMeta != NULL)
    {
      VsLog::debugLog() << methodSig
        << "Found Variable With Mesh. Loading data and returning." << endl;
      return getPointMesh(vmMeta,
                          haveDataSelections, mins, maxs, strides);
    }

    // Curve
    VsLog::debugLog() << methodSig
                      << "Looking for Curve with this name." << endl;
    vtkDataArray* foundCurve = this->GetVar(domain, name);

    if (foundCurve != NULL)
    {
      VsLog::debugLog() << methodSig
                        << "Trying to load & return curve." << endl;
      return getCurve(domain, name);
    }

    VsLog::debugLog() << methodSig
                      << "Failed to load data for name and domain number.  "
                      << "Returning NULL." << endl;
    return NULL;
}


// *****************************************************************************
//  Method: avtImageVileFormat::getUniformMesh
//
//  Purpose:
//      How do you do the voododo that you do
//
//  Programmer: Marc Durant
//  Creation:   June, 2010
//
//  Modifications:
//

vtkDataSet* avtVsFileFormat::getUniformMesh(VsUniformMesh* uniformMesh,
                                            bool haveDataSelections,
                                            int* mins, int* maxs, int* strides)
{
    string methodSig("avtVsFileFormat::getUniformMesh() - ");
    VsLog::debugLog() << methodSig << "Entering function." << endl;
    LoadData();

    // Read int data
    vector<int> numCells;
    uniformMesh->getMeshDataDims(numCells); // Number of cells NOT nodes

    if (numCells.size() < 0) {
      string msg = methodSig + "Could not get dimensions of the uniform mesh.";
      VsLog::debugLog() << msg << endl;
      throw out_of_range(msg.c_str());
    }

    size_t numTopologicalDims = numCells.size();

    if (numTopologicalDims > 3) {
      VsLog::debugLog() << methodSig
        << "Error: numTopologicalDims of the mesh is larger than 3.  "
        << "Returning NULL." << endl;
      return NULL;
    }

    int numSpatialDims = uniformMesh->getNumSpatialDims();

    if (numSpatialDims > 3) {
      VsLog::debugLog() << methodSig
                        << "Error: numSpatialDims data is larger than 3.  "
                        << "Returning NULL." << endl;
      return NULL;
    }

    // The numTopologicalDims and numSpatialDims can be different for
    // structured grids but the numSpatialDims must be great than or
    // equal to the numTopologicalDims.
    if( numTopologicalDims > numSpatialDims )
    {
      VsLog::debugLog() << methodSig
                        << "Error: The numTopologicalDims ("
                        << numTopologicalDims
                        << ") is greater than numSpatialDims ("
                        << numSpatialDims << ").  "
                        << "Returning NULL." << endl;
      return NULL;
    }

    // VisIt downcasts to float, so go to float by default now.
    hid_t meshDataType = uniformMesh->getDataType();

    if (!isDoubleType(meshDataType) && !isFloatType(meshDataType))
    {
      VsLog::debugLog() << methodSig
                        << "Error: hdf5 data type not handled: " << meshDataType
                        << "Returning NULL." << endl;
      return NULL;
    }

    // startCell
    VsLog::debugLog() << methodSig
                      << "Loading optional startCells attribute." << endl;
    vector<int> startCell;
    herr_t err = uniformMesh->getStartCell(&startCell);
    if (err < 0) {
      VsLog::warningLog() << methodSig
        << "Uniform mesh does not have starting cell position." << endl;
    } else {
      // Adjust the box by startCell
      VsLog::debugLog() << methodSig
                        << "Adjusting numCells by startCells." << endl;
      for (size_t i = 0; i < numTopologicalDims; ++i)
        numCells[i] -= startCell[i];
    }

    // Storage for meshes in VisIt, which is a vtkRectilinearGrid which
    // is topologically 3D so the points must also be 3D.
    size_t vsdim = 3;

    VsLog::debugLog() << methodSig
                      << "Determining size of point arrays." << endl;

    vector<int> idims(vsdim);

    // Number of nodes is equal to number of cells plus one
    for (size_t i = 0; i < numTopologicalDims; ++i) 
      idims[i] = numCells[i]+1;

    // Set unused dims to 1
    for (size_t i = numTopologicalDims; i < vsdim; ++i)
      idims[i] = 1;

    // Adjust for the data selections which are ZONAL.
    if( haveDataSelections )
    {
      for (size_t i=0; i<numTopologicalDims; ++i)
      {
        if( maxs[i] < 0 || numCells[i] - 1 < maxs[i] )
          maxs[i] = numCells[i] - 1; // numCells - 1 = last cell
                                     // index, not number of cells
        if( maxs[i] < mins[i] )
          mins[i] = 0;
      }
    }
    else
    {
      for (size_t i=0; i<numTopologicalDims; ++i)
      {
        mins[i] = 0;
        maxs[i] = numCells[i] - 1; // numCells - 1 = last cell
                                   // index, not number of cells
        strides[i] = 1;
      }
    }

    for (size_t i=numTopologicalDims; i<vsdim; ++i)
    {
      mins[i] = 0;
      maxs[i] = 1;
      strides[i] = 1;
    }

    VsLog::debugLog() << methodSig
      << "Have a zonal inclusive selection for uniform mesh "<< endl
      << "(" << mins[0] << "," << maxs[0] << " stride " << strides[0] << ") "
      << "(" << mins[1] << "," << maxs[1] << " stride " << strides[1] << ") "
      << "(" << mins[2] << "," << maxs[2] << " stride " << strides[2] << ") "
      << endl;
 
    // Get the new number of node points.
    vector<int> gdims(vsdim);

    for (size_t i=0; i<numTopologicalDims; ++i)
    {
      gdims[i] = ((maxs[i]-mins[i]+1)/strides[i] +            // Number of cells
//  COMMENTED OUT FOR NOW AS HDF5 DOES NOT HANDLE PARTIAL CELLS
//                 ((maxs[i]-mins[i]+1)%strides[i] ? 1 : 0) + // Partial cell
                   1);                                        // Last Node
    }

    for (size_t i=numTopologicalDims; i<vsdim; ++i)
    {
      gdims[i] = 1;
    }

    VsLog::debugLog() <<  "Grid dims predicted "
         << gdims[0] << "  " << gdims[1] << "  " << gdims[2] << "  " << endl;

#if (defined PARALLEL && defined VIZSCHEMA_DECOMPOSE_DOMAINS)
    VsLog::debugLog() << methodSig
                      << "Parallel & Decompose_Domains are defined, "
                      << "entering parallel code." << endl;
    
    for (size_t i=0; i<numTopologicalDims; ++i)
    {
      int numNodes = gdims[i];
      
      // Integer number of nodes per processor
      size_t numNodesPerProc = numNodes / PAR_Size();
      size_t numProcsWithExtraNode = numNodes % PAR_Size();
      
      // To get all of the nodes adjust the count by one for those
      // processors that need an extra node.
      if (PAR_Rank() < numProcsWithExtraNode)
      {
        gdims[i] = numNodesPerProc + 1;
        mins[i] = PAR_Rank() * (numNodesPerProc + 1) * strides[i];
        maxs[i] = mins[i] + (numNodesPerProc) * strides[i];
      }
      else
      {
        gdims[i] = numNodesPerProc;
        // Processors w/extra node plus processors without extra node.
        mins[i] = (numProcsWithExtraNode * (numNodesPerProc + 1) +
                   (PAR_Rank() - numProcsWithExtraNode) * numNodesPerProc) *
          strides[i];
        maxs[i] = mins[i] + (numNodesPerProc-1) * strides[i];
      }
      // Adjust bounds
      float delta = (upperBounds[splitAxis] - lowerBounds[splitAxis]) /
        numCellsAlongSplitAxis;
      lowerBounds[splitAxis] += startCell * delta;
      upperBounds[splitAxis] = lowerBounds[splitAxis] + nCells * delta;

      numCells[splitAxis] = nCells;
      idims[splitAxis] = nCells + 1; // FIXME: May depend on centering
    }

    VsLog::debugLog() << methodSig
                      << "Parallel & Decompose_Domains are defined, "
                      << "exiting parallel code." << endl;
#endif

    VsLog::debugLog() << methodSig << "Getting lower bounds for mesh." << endl;
    vector<float> lowerBounds;
    uniformMesh->getLowerBounds(&lowerBounds);

    VsLog::debugLog() << methodSig << "Getting upper bounds for mesh." << endl;
    vector<float> upperBounds;
    uniformMesh->getUpperBounds(&upperBounds);

    // Storage for mesh points in VisIt are spatially 3D. So create 3
    // coordinate arrays and fill in zero for the others.
    VsLog::debugLog() << methodSig << "Creating coordinate arrays." << endl;

    vector<vtkDataArray*> coords(vsdim);

    for (size_t i=0; i<numSpatialDims; ++i)
    {
       if( isDoubleType( meshDataType ) )
         coords[i] = vtkDoubleArray::New();
       else if( isFloatType( meshDataType ) )
         coords[i] = vtkFloatArray::New();

      // Delta
      double delta = 0;
      if (idims[i] > 1)
        delta = (upperBounds[i] - lowerBounds[i]) / (idims[i]-1);

      int cc = 0;
      int j = mins[i];

      while( j <= maxs[i] )
      {
        double temp = lowerBounds[i] + j*delta;
        coords[i]->InsertTuple(cc, &temp);
        ++cc;
        j += strides[i];
      }

      // Last node ... may form a partial cell.
      j = maxs[i] + 1;
      // NO PARTIAL CELLS FOR NOW
      if( j % strides[i] == 0 )
      {
        double temp = lowerBounds[i] + j*delta;
        coords[i]->InsertTuple(cc, &temp);
        ++cc;
      }
    }

    // Unused axii
    for (size_t i=numSpatialDims; i<vsdim; ++i)
    {
      coords[i] = vtkFloatArray::New();
      coords[i]->SetNumberOfTuples(1);
      coords[i]->SetComponent(0, 0, 0);
    }
          
    // Create vtkRectilinearGrid
    VsLog::debugLog() << methodSig << "Creating rectilinear grid." << endl;
    vtkRectilinearGrid* rgrid = vtkRectilinearGrid::New();
    rgrid->SetDimensions(&(gdims[0]));

    // Set grid data
    VsLog::debugLog() << methodSig << "Adding coordinates to grid." << endl;
    rgrid->SetXCoordinates(coords[0]);
    rgrid->SetYCoordinates(coords[1]);
    rgrid->SetZCoordinates(coords[2]);

    // Cleanup local data
    VsLog::debugLog() << methodSig << "Cleaning up." << endl;
    for (size_t i = 0; i<vsdim; ++i)
      coords[i]->Delete();

    VsLog::debugLog() << methodSig << "Returning data." << endl;
    return rgrid;
}


// *****************************************************************************
//  Method: avtImageVileFormat::getRectilinearMesh
//
//  Purpose:
//      How do you do the voododo that you do
//
//  Programmer: Marc Durant
//  Creation:   June, 2010
//
//  Modifications:
//

vtkDataSet*
avtVsFileFormat::getRectilinearMesh(VsRectilinearMesh* rectilinearMesh,
                                    bool haveDataSelections,
                                    int* mins, int* maxs, int* strides)
{
    // TODO - make "cleanupAndReturnNull" label, and do a "go to"
    // instead of just returning NULL all the time.
    
    string methodSig("avtVsFileFormat::getRectilinearMesh() - ");
    VsLog::debugLog() << methodSig << "Entering function." << endl;
    LoadData();

    // Get dimensions
    vector<int> numNodes;
    rectilinearMesh->getMeshDataDims(numNodes); // Number of nodes NOT cells

    if (numNodes.size() < 0) {
      string msg = methodSig +
        "Could not get dimensions of the rectilinear mesh.";
      VsLog::debugLog() << msg << endl;
      throw out_of_range(msg.c_str());
    }

    // The numTopologicalDims and numSpatialDims are the same for
    // rectilinear grids.
    size_t numTopologicalDims = numNodes.size();

    if (numTopologicalDims > 3) {
      VsLog::debugLog() << methodSig
        << "Error: numTopologicalDims of data is larger than 3.  "
        << "Returning NULL." << endl;
      return NULL;
    }

    int numSpatialDims = rectilinearMesh->getNumSpatialDims();

    if (numSpatialDims > 3) {
      VsLog::debugLog() << methodSig
                        << "Error: numSpatialDims data is larger than 3.  "
                        << "Returning NULL." << endl;
      return NULL;
    }

    // VisIt downcasts to float, so go to float by default now.
    hid_t meshDataType = rectilinearMesh->getDataType();

    if (!isDoubleType(meshDataType) && !isFloatType(meshDataType))
    {
      VsLog::debugLog() << methodSig
                        << "Error: hdf5 data type not handled: " << meshDataType
                        << "Returning NULL." << endl;
      return NULL;
    }

    // Storage for meshes in VisIt, which is a vtkRectilinearGrid which
    // is topologically 3D so the points must also be 3D.
    size_t vsdim = 3;

    VsLog::debugLog() << methodSig
                      << "Determining size of point arrays." << endl;

    // Adjust for the data selections which are ZONAL.
    if( haveDataSelections )
    {
      for (size_t i=0; i<numTopologicalDims; ++i)
      {
        if( maxs[i] < 0 || numNodes[i] - 1 < maxs[i] )
          maxs[i] = numNodes[i] - 2; // dims - 2 = last cell index, not
                                     // number of cells
        if( maxs[i] < mins[i] )
          mins[i] = 0;
      }
    }
    else
    {
      for (size_t i=0; i<numTopologicalDims; ++i)
      {
        mins[i] = 0;
        maxs[i] = numNodes[i] - 2; // numNodes - 2 = last cell index, not
                                   // number of cells
        strides[i] = 1;
      }
    }

    for (size_t i=numTopologicalDims; i<vsdim; ++i)
    {
      mins[i] = 0;
      maxs[i] = 1;
      strides[i] = 1;
    }

    VsLog::debugLog() << methodSig
      << "Have a zonal inclusive selection for rectilinear mesh "<< endl
      << "(" << mins[0] << "," << maxs[0] << " stride " << strides[0] << ") "
      << "(" << mins[1] << "," << maxs[1] << " stride " << strides[1] << ") "
      << "(" << mins[2] << "," << maxs[2] << " stride " << strides[2] << ") "
      << endl;

    // Get the new number of node points.
    vector<int> gdims(vsdim);

    for (size_t i=0; i<numTopologicalDims; ++i)
    {
      gdims[i] = ((maxs[i]-mins[i]+1)/strides[i] +            // Number of cells
//  COMMENTED OUT FOR NOW AS HDF5 DOES NOT HANDLE PARTIAL CELLS
//                 ((maxs[i]-mins[i]+1)%strides[i] ? 1 : 0) + // Partial cell
                   1);                                        // Last Node
    }

    for (size_t i=numTopologicalDims; i<vsdim; ++i)
    {
      gdims[i] = 1;
    }

    VsLog::debugLog() <<  "Grid dims predicted "
         << gdims[0] << "  " << gdims[1] << "  " << gdims[2] << "  " << endl;

#if (defined PARALLEL && defined VIZSCHEMA_DECOMPOSE_DOMAINS)
    VsLog::debugLog() << methodSig
                      << "Parallel & Decompose_Domains are defined, "
                      << "entering parallel code." << endl;
    
    for (size_t i=0; i<numTopologicalDims; ++i)
    {
      int numNodes = gdims[i];
      
      // Integer number of nodes per processor
      size_t numNodesPerProc = numNodes / PAR_Size();
      size_t numProcsWithExtraNode = numNodes % PAR_Size();
      
      // To get all of the nodes adjust the count by one for those
      // processors that need an extra node.
      if (PAR_Rank() < numProcsWithExtraNode)
      {
        gdims[i] = numNodesPerProc + 1;
        mins[i] = PAR_Rank() * (numNodesPerProc + 1) * strides[i];
        maxs[i] = mins[i] + (numNodesPerProc) * strides[i];
      }
      else
      {
        gdims[i] = numNodesPerProc;
        // Processors w/extra node plus processors without extra node.
        mins[i] = (numProcsWithExtraNode * (numNodesPerProc + 1) +
                   (PAR_Rank() - numProcsWithExtraNode) * numNodesPerProc) *
            strides[i];
        maxs[i] = mins[i] + (numNodesPerProc-1) * strides[i];
      }
    }
    
    VsLog::debugLog() << methodSig
                      << "Parallel & Decompose_Domains are defined, "
                      << "exiting parallel code." << endl;
#endif

    vector<vtkDataArray*> coords(vsdim);

    for( int i=0; i<numTopologicalDims; ++i )
    {
      void* dataPtr;
      double* dblDataPtr;
      float* fltDataPtr;

      VsLog::debugLog() << methodSig << "Loading data for axis " << i << endl;
      VsH5Dataset* axisData = rectilinearMesh->getAxisDataset(i);

      if (axisData == NULL) {
        VsLog::debugLog() << methodSig << "Axis " << i
                          << " data not found. Returning NULL." << endl;
        return NULL;
      }

      hid_t type = axisData->getType();

      // Read points and add in zero for any lacking dimension
      if (isDoubleType(type)) {
        dblDataPtr = new double[numNodes[i]];
        dataPtr = dblDataPtr;
      }
      else if (isFloatType(type)) {
        fltDataPtr = new float[numNodes[i]];
        dataPtr = fltDataPtr;
      } else {
        VsLog::debugLog() << methodSig << "Unknown data type: "
                          << type << endl;
        return NULL;
      }

      if (!dataPtr) {
        VsLog::debugLog() << methodSig
                          << "Allocation failed, pointer is NULL."
                          << "Returning NULL." << endl;
        return NULL;
      }

      // Read in the point data
      herr_t err = reader->getDataSet(axisData, dataPtr);

      if (err < 0) {
        VsLog::debugLog() << methodSig
                          << "GetDataSet returned error: " << err << endl;
        VsLog::debugLog() << methodSig << "Returning NULL." << endl;

        if (isDoubleType(type))
          delete [] dblDataPtr;
        else if (isFloatType(type))
          delete [] fltDataPtr;

        return NULL;
      }

      // Storage for mesh points in VisIt are spatially 3D. So create 3
      // coordinate arrays and fill in zero for the others.
      VsLog::debugLog() << methodSig << "Creating coordinate arrays." << endl;

      // The mesh data type will the highest precision of all of the
      // axis data types. Declaring here allows the the mesh data type
      // to be either float or double.
      if( isDoubleType( meshDataType ) )
        coords[i] = vtkDoubleArray::New();
      else if( isFloatType( meshDataType ) )
        coords[i] = vtkFloatArray::New();

      int cc = 0;

      if (isDoubleType(type)) {

        int j = mins[i];

        while( j <= maxs[i] )
        {
          double temp = dblDataPtr[j];
          coords[i]->InsertTuple(cc, &temp);
          ++cc;
          j += strides[i];
        }

        // Last node ... may form a partial cell.
        j = maxs[i] + 1;
        // NO PARTIAL CELLS FOR NOW
        if( j % strides[i] == 0 )
        {
          double temp = dblDataPtr[j];
          coords[i]->InsertTuple(cc, &temp);
          ++cc;
        }

        delete [] dblDataPtr;

      } else if (isFloatType(type)) {

        int j = mins[i];

        while( j <= maxs[i] )
        {
          float temp = fltDataPtr[j];
          coords[i]->InsertTuple(cc, &temp);
          ++cc;
          j += strides[i];
        }

        // Last node ... may form a partial cell.
        j = maxs[i] + 1;
        // NO PARTIAL CELLS FOR NOW
        if( j % strides[i] == 0 )
        {
          float temp = fltDataPtr[j];
          coords[i]->InsertTuple(cc, &temp);
          ++cc;
        }

        delete [] fltDataPtr;
      }
    }

    for (int i=numTopologicalDims; i<vsdim; ++i) {
      if( isDoubleType(meshDataType) )
        coords[i] = vtkDoubleArray::New();
      else if( isFloatType(meshDataType) )
        coords[i] = vtkFloatArray::New();

      coords[i]->SetNumberOfTuples(1);
      coords[i]->SetComponent(0, 0, 0);    
    }

    // Create vtkRectilinearGrid
    VsLog::debugLog() << methodSig << "Creating rectilinear grid." << endl;
    vtkRectilinearGrid* rgrid = vtkRectilinearGrid::New();
    rgrid->SetDimensions(&(gdims[0]));
  
    // Set grid data
    VsLog::debugLog() << methodSig << "Adding coordinates to grid." << endl;
    rgrid->SetXCoordinates(coords[0]);
    rgrid->SetYCoordinates(coords[1]);
    rgrid->SetZCoordinates(coords[2]);

    // Cleanup local data
    VsLog::debugLog() << methodSig << "Cleaning up." << endl;
    for (size_t i = 0; i < vsdim; ++i) {
      coords[i]->Delete();
    }

    VsLog::debugLog() << methodSig << "Returning data." << endl;
    return rgrid;
}

  
// *****************************************************************************
//  Method: avtImageVileFormat::getStructuredMesh
//
//  Purpose:
//      How do you do the voododo that you do
//
//  Programmer: Marc Durant
//  Creation:   June, 2010
//
//  Modifications:
//

vtkDataSet* avtVsFileFormat::getStructuredMesh(VsStructuredMesh* structuredMesh,
                                               bool haveDataSelections,
                                               int* mins, int* maxs,
                                               int* strides)
{
    string methodSig("avtVsFileFormat::getStructuredMesh() - ");
    VsLog::debugLog() << methodSig << "Entering function." << endl;
    LoadData();

    // Find points: Structured meshes are datasets, and the data is
    // the points So, look for the dataset with the same name as the
    // mesh

    VsH5Dataset* pointsDataset =
      registry->getDataset(structuredMesh->getFullName());

    if (pointsDataset == NULL) {
      VsLog::debugLog() << methodSig << "Component '"
                        << structuredMesh->getFullName()
                        << "' not found.  Returning NULL." << endl;
      return NULL;
    }

    // Get dims of points array
    VsLog::debugLog() << methodSig
                      << "Determining dimension of points array." << endl;

    vector<int> numNodes;
    structuredMesh->getNumMeshDims(numNodes);  // Number of nodes NOT cells.

    if (numNodes.size() < 0) {
      string msg = methodSig +
        "Could not get dimensions of the structured mesh.";
      VsLog::debugLog() << msg << endl;
      throw out_of_range(msg.c_str());
    }

    // The numTopologicalDims and numSpatialDims can be different for
    // structured grids.
    size_t numTopologicalDims = numNodes.size();

    if (numTopologicalDims > 3) {
      VsLog::debugLog() << methodSig
                        << "Error: numTopologicalDims of data is larger than 3.  "
                        << "Returning NULL." << endl;
      return NULL;
    }

    int numSpatialDims = structuredMesh->getNumSpatialDims();

    if (numSpatialDims > 3) {
      VsLog::debugLog() << methodSig
                        << "Error: numSpatialDims data is larger than 3.  "
                        << "Returning NULL." << endl;
      return NULL;
    }

    // The numTopologicalDims and numSpatialDims can be different for
    // structured grids but the numSpatialDims must be great than or
    // equal to the numTopologicalDims.
    if( numTopologicalDims > numSpatialDims )
    {
      VsLog::debugLog() << methodSig
                        << "Error: The numTopologicalDims ("
                        << numTopologicalDims
                        << ") is greater than numSpatialDims ("
                        << numSpatialDims << ").  "
                        << "Returning NULL." << endl;
      return NULL;
    }

    // VisIt downcasts to float, so go to float by default now.
    hid_t meshDataType = pointsDataset->getType();

    if (!isDoubleType(meshDataType) && !isFloatType(meshDataType))
    {
      VsLog::debugLog() << methodSig
                        << "Error: hdf5 data type not handled: " << meshDataType
                        << "Returning NULL." << endl;
      return NULL;
    }

    // Cache the data type because it's faster 
    bool isDouble = isDoubleType(meshDataType);
    bool isFloat = isFloatType(meshDataType);

    // Storage for meshes in VisIt, which is a vtkRectilinearGrid which
    // is topologically 3D so the points must also be 3D.
    size_t vsdim = 3;

    // Adjust for the data selections which are ZONAL.
    if( haveDataSelections )
    {
      for (size_t i=0; i<numTopologicalDims; ++i)
      {
        if( maxs[i] < 0 || numNodes[i] - 1 < maxs[i] )
          maxs[i] = numNodes[i] - 2; // numNodes - 2 = last cell index, not
                                     // number of cells
        if( maxs[i] < mins[i] )
          mins[i] = 0;
      }
    }
    else
    {
      for (size_t i=0; i<numTopologicalDims; ++i)
      {
        mins[i] = 0;
        maxs[i] = numNodes[i] - 2; // numNodes - 2 = last cell index, not
                                   // number of cells
        strides[i] = 1;
      }
    }

    for (size_t i=numTopologicalDims; i<vsdim; ++i)
    {
      mins[i] = 0;
      maxs[i] = 1;
      strides[i] = 1;
    }

    VsLog::debugLog() << methodSig
      << "Have a zonal inclusive selection for structured mesh "<< endl
      << "(" << mins[0] << "," << maxs[0] << " stride " << strides[0] << ") "
      << "(" << mins[1] << "," << maxs[1] << " stride " << strides[1] << ") "
      << "(" << mins[2] << "," << maxs[2] << " stride " << strides[2] << ") "
      << endl;

    // Get the new number of node points.
    vector<int> gdims(vsdim);
    int numPoints = 1;

    for (size_t i=0; i<numTopologicalDims; ++i)
    {
      gdims[i] = ((maxs[i]-mins[i]+1)/strides[i] +            // Number of cells
//  COMMENTED OUT FOR NOW AS HDF5 DOES NOT HANDLE PARTIAL CELLS
//                 ((maxs[i]-mins[i]+1)%strides[i] ? 1 : 0) + // Partial cell
                   1);                                        // Last Node

      numPoints *= gdims[i];
    }

    for (size_t i=numTopologicalDims; i<vsdim; ++i)
    {
      gdims[i] = 1;

      numPoints *= gdims[i];
    }

    VsLog::debugLog() <<  "Grid dims predicted "
         << gdims[0] << "  " << gdims[1] << "  " << gdims[2] << "  " << endl;

    VsLog::debugLog() << methodSig
                      << "Total number of points is " << numPoints << endl;

    // Read points and add in zero for any lacking dimension
    VsLog::debugLog() << methodSig << "Reading in point data." << endl;

    size_t dsize = 0;
    void* dataPtr = 0;
    double* dblDataPtr = 0;
    float* fltDataPtr = 0;

    if (isDouble) {
      dsize = sizeof(double);
      dblDataPtr = new double[numPoints*3];
      dataPtr = dblDataPtr;
    }
    else if (isFloat) {
      dsize = sizeof(float);
      fltDataPtr = new float[numPoints*3];
      dataPtr = fltDataPtr;
    }

    if (!dataPtr) {
      VsLog::debugLog() << methodSig
                        << "Allocation failed, pointer is NULL.  "
                        << "Returning NULL." << endl;
      return NULL;
    }

    // Read in the data
    herr_t err;

    if( haveDataSelections )
      err = reader->getDataSet( pointsDataset, dataPtr,
                                // -1 read all coordinate components
                                structuredMesh->getIndexOrder(), -1,
                                mins, &(gdims[0]), strides );
    else
      err = reader->getDataSet( pointsDataset, dataPtr );

    if (err < 0)
    {
      VsLog::debugLog() << methodSig << "Error " << err
                        << " while reading data.  Returning NULL." << endl;
      
      if (isDouble)
        delete [] dblDataPtr;
      else if (isFloat)
        delete [] fltDataPtr;

      return NULL;
    }

    // If spatial dimension is less than 3, move coordinates to the
    // correct position and set the extra dimensions to zero.
    if (numSpatialDims < 3)
    {
      VsLog::debugLog() << methodSig << "Point spatial dimensionality ("
                        << numSpatialDims << " is less than 3.  "
                        << "Moving data into correct location." << endl;

      for (int i=numPoints-1; i>=0; --i)
      {
        unsigned char* destPtr
          = (unsigned char*) dataPtr + i*3*dsize;
        unsigned char* srcPtr
          = (unsigned char*) dataPtr + i*numSpatialDims*dsize;
        
        memmove(destPtr, srcPtr, numSpatialDims*dsize);
        destPtr += numSpatialDims*dsize;
        memset(destPtr, 0, (3-numSpatialDims)*dsize);
      }

      VsLog::debugLog() << methodSig << "Data move succeeded." << endl;
    }

    // Create the mesh and set its dimensions, including unused to zero
    VsLog::debugLog() << methodSig << "Creating the mesh." << endl;

    // Add the points, changing to C ordering
    VsLog::debugLog() << methodSig << "Adding points to mesh." << endl;

    vtkPoints* vpoints = vtkPoints::New();
    if (isDouble) {
      vpoints->SetDataTypeToDouble();
    }
    else if (isFloat) {
      vpoints->SetDataTypeToFloat();
    }

    vpoints->SetNumberOfPoints(numPoints);
    //void* ptsPtr = vpoints->GetVoidPointer(0);

    // Step through by global C index to reverse
    VsLog::debugLog() << methodSig << "Adding " << numPoints
                      << " points with index order '"
                      << structuredMesh->getIndexOrder() << "'." << endl;

    // Using FORTRAN data ordering.
    if (structuredMesh->isFortranOrder()) {
      VsLog::debugLog() << methodSig << "Using FORTRAN data ordering." << endl;

      if (isDouble) {
        for (size_t k = 0; k<numPoints; ++k) {
          vpoints->SetPoint(k, &dblDataPtr[k*3]);
        }
      } else if (isFloat) {
        for (size_t k = 0; k<numPoints; ++k) {
          vpoints->SetPoint(k, &fltDataPtr[k*3]);
        }
      }
    }

    // Using C data ordering.
    else {
      VsLog::debugLog() << methodSig << "Using C data ordering." << endl;
      size_t indices[3] = {0, 0, 0};

      // Step through by global C index
      for (size_t k = 0; k<numPoints; ++k) {
        
        // Accumulate the Fortran index
        size_t indx = indices[2];
        for (size_t j = 2; j<=3; ++j) {
          indx = indx*gdims[3-j] + indices[3-j];
        }

        // Set the value in the VTK array at the Fortran index to the
        // value in the C array at the C index
        if (isDouble) {
          vpoints->SetPoint(indx, &dblDataPtr[k*3]);
        }
        else if (isFloat) {
          vpoints->SetPoint(indx, &fltDataPtr[k*3]);
        }

        // Update the index tuple
        size_t j = 3;
        do {
          --j;
          ++indices[j];

          if (indices[j] == gdims[j])
            indices[j] = 0;
          else
            break;
        } while (j != 0);
      }
    }

    if (isDouble) {
      delete [] dblDataPtr;
    } else if (isFloat) {
      delete [] fltDataPtr;
    }

    /*
     // debug: output points
     fltDataPtr = (float*)ptsPtr;
     dblDataPtr = (double*)ptsPtr;
     for (size_t i = 0; i < numPoints; ++i) {
     VsLog::debugLog() << i << ":";
     for (size_t j = 0; j < 3; ++j) {
     if (H5Tequal(type, H5T_NATIVE_DOUBLE)) {
     VsLog::debugLog() << " " << dblDataPtr[(3*i)+j];
     }
     else VsLog::debugLog() << " " << fltDataPtr[(3*i)+j];
     }
     VsLog::debugLog() << endl;
     }
     // end debug
     */
    vtkStructuredGrid* sgrid = vtkStructuredGrid::New();
    sgrid->SetDimensions(&(gdims[0]));
    sgrid->SetPoints(vpoints);

    // Cleanup local data
    VsLog::debugLog() << methodSig << "Cleaning up." << endl;
    vpoints->Delete();

    VsLog::debugLog() << methodSig << "Returning data." << endl;
    return sgrid;
}


// *****************************************************************************
//  Method: avtImageVileFormat::getUnstructuredMesh
//
//  Purpose:
//      How do you do the voododo that you do
//
//  Programmer: Marc Durant
//  Creation:   June, 2010
//
//  Modifications:
//

vtkDataSet*
avtVsFileFormat::getUnstructuredMesh(VsUnstructuredMesh* unstructuredMesh,
                                     bool haveDataSelections,
                                     int* mins, int* maxs, int* strides)
{
    herr_t err;
      
    stringstream sstr;
    sstr << "avtVsFileFormat::getUnstructuredMesh() - ";
    string methodSig = sstr.str();
    VsLog::debugLog() << methodSig << "Entering function." << endl;
    LoadData();

    // Check for points type
    hid_t type = unstructuredMesh->getDataType();

    if (!isDoubleType(type) && !isFloatType(type)) {
      VsLog::debugLog() << methodSig
                        << "Points data type not recognized:" << type << endl;
      VsLog::debugLog() << methodSig << "Returning NULL" << endl;
      return NULL;
    }

    VsLog::debugLog() << methodSig
                      << "Checking dimensionality of mesh." << endl;
    size_t numNodes = unstructuredMesh->getNumPoints();
    size_t numSpatialDims = unstructuredMesh->getNumSpatialDims();

    // Get ready to read in points
    VsLog::debugLog() << methodSig << "Setting up data structures." << endl;
    vtkUnstructuredGrid* ugridPtr = vtkUnstructuredGrid::New();

    // Create and set points while small so minimal memory usage
    vtkPoints* vpts = vtkPoints::New();
    ugridPtr->SetPoints(vpts);
    vpts->Delete();
    vtkPoints* vpoints = ugridPtr->GetPoints();

    // If there is no polygon data, then this mesh was registered as a
    // point mesh so only read in the points requested

    // Adjust for the data selections which are ZONAL.
    if (unstructuredMesh->isPointMesh() && haveDataSelections)
    {
      if( maxs[0] < 0 || numNodes - 1 < maxs[0] )
        maxs[0] = numNodes - 1; // numNodes - 1 = last cell index,
                                // not number of cells

      if( maxs[0] < mins[0] )
        mins[0] = 0;

      VsLog::debugLog() << methodSig
        << "Have a zonal inclusive selection for unstructured point mesh "<< endl
        << "(" << mins[0] << "," << maxs[0] << " stride " << strides[0] << ") "   << endl;
      
      // New number of nodes based on the above.
      numNodes = (maxs[0]-mins[0]+1) / strides[0];
    }

    // Allocate
    size_t dsize = 0;
    if (isDoubleType(type)) {
      vpoints->SetDataTypeToDouble();
      dsize = sizeof(double);
    }
    else if (isFloatType(type)) {
      vpoints->SetDataTypeToFloat();
      dsize = sizeof(float);
    } else {
      VsLog::debugLog() << methodSig << "Unknown data type: " << type << endl;
      return NULL;
    }

    VsLog::debugLog() << methodSig
                      << "VPoints array will have " << numNodes << " points."
                      << endl;
    vpoints->SetNumberOfPoints(numNodes);
    void* dataPtr = vpoints->GetVoidPointer(0);
    if (!dataPtr) {
      VsLog::debugLog() << methodSig
                        << "Unable to allocate the points.  Cleaning up."
                        << endl;
      ugridPtr->Delete();
      VsLog::debugLog() << methodSig << "Returning NULL." << endl;
      return NULL;
    }

    // Read in the data

    // Mesh points are in separate data sets.
    if (unstructuredMesh->usesSplitPoints())
    {
      VsLog::debugLog() << methodSig << "Using split-points method" << endl;

      // Kind of grim but read each of the coordinate components in
      // using the generic reader.

      // To do so set the location in memory where each cordinate
      // componenet will be stored.
      int srcMins[1] = {mins[0]};
      int srcMaxs[1] = {numNodes};
      int srcStrides[1] = {strides[0]};

      int destSize[1] = {numNodes*3};
      int destMins[1] = {0};
      int destMaxs[1] = {numNodes};
      int destStrides[1] = {3};
      
      for( int i=0; i<numSpatialDims; ++i )
      {
        VsH5Dataset* pointDataset = unstructuredMesh->getPointsDataset(i);

        destMins[0] = i;

        herr_t err = reader->getDataSet(pointDataset, dataPtr,
                                        unstructuredMesh->getIndexOrder(),
                                        -2,
                                        &srcMins[0], &srcMaxs[0], &srcStrides[0],
                                        1, &destSize[0],
                                        &destMins[0], &destMaxs[0], &destStrides[0] );

        if (err < 0) {
          VsLog::debugLog() << methodSig
                            << "Call to getDataSet returned error: "
                            << err << "Returning NULL." << endl;
          return NULL;
        }
      }

      // The above stores the value in the correct location but make
      // sure the remaing values are all zero.
      for (int i=numSpatialDims; i<3; ++i)
      {
        VsLog::debugLog() << methodSig
                          << "Zeroing data at unused positions." << endl;
        
        if (isDoubleType(type)) {
        
          double* dblDataPtr = &(((double*) dataPtr)[i]);
        
          for (int j=0; j<numNodes; ++j)
          {
            *dblDataPtr = 0;
            dblDataPtr += 3;
          }
        }
        else if (isFloatType(type)) {
        
          float* fltDataPtr = &(((float*) dataPtr)[i]);
        
          for (int j=0; j<numNodes; ++j)
          {
            *fltDataPtr = 0;
            fltDataPtr += 3;
          }
        }
      }
    }

    // Mesh points are in one data set.
    else
    {
      VsLog::debugLog() << methodSig << "Using all-in-one method" << endl;

      VsH5Dataset* pointsDataset = unstructuredMesh->getPointsDataset();
      
      if (unstructuredMesh->isPointMesh() && haveDataSelections)
      {
        int srcMins[1] = {mins[0]};
        int srcMaxs[1] = {numNodes};
        int srcStrides[1] = {strides[0]};

        err = reader->getDataSet( pointsDataset, dataPtr,
                                  // -1 read all coordinate components
                                  unstructuredMesh->getIndexOrder(), -1,
                                  &(srcMins[0]), &(srcMaxs[0]), &(srcStrides[0]) );
      }
      else
        err = reader->getDataSet(pointsDataset, dataPtr);

      if (err < 0) {
        VsLog::debugLog() << methodSig
                          << "Call to getDataSet returned error: "
                          << err << "Returning NULL." << endl;
        return NULL;
      }

      // If spatial dimension is less than 3, move coordinates to the
      // correct position and set the extra dimensions to zero.
      if (numSpatialDims < 3)
      {
        VsLog::debugLog() << methodSig << "Point spatial dimensionality ("
                          << numSpatialDims << " is less than 3.  "
                          << "Moving data into correct location." << endl;
        
        for (int i=numNodes-1; i>=0; --i)
        {
          unsigned char* destPtr
            = (unsigned char*) dataPtr + i*3*dsize;
          unsigned char* srcPtr
            = (unsigned char*) dataPtr + i*numSpatialDims*dsize;

          memmove(destPtr, srcPtr, numSpatialDims*dsize);
          destPtr += numSpatialDims*dsize;
          memset(destPtr, 0, (3-numSpatialDims)*dsize);
        }

        VsLog::debugLog() << methodSig << "Data move succeeded." << endl;
      }
    }

    // If there is no polygon data, then this mesh was registered as a
    // point mesh and we don't need to go any further
    if (unstructuredMesh->isPointMesh())
    {
      VsLog::debugLog() << methodSig
                        << "Mesh was registered as a point mesh." << endl;
      VsLog::debugLog() << methodSig
                        << "Because no connectivity data was found." << endl;
      VsLog::debugLog() << methodSig
                        << "Adding vertices as single points and returning."
                        << endl;

      vtkIdType vertex;
      for (int i=0; i<numNodes; ++i) {
        vertex = i;
        ugridPtr->InsertNextCell(VTK_VERTEX, 1, &vertex);
      }
      return ugridPtr;
    }

    // Next, look for connectivity data
    VsH5Dataset* connectivityMeta = 0;

    unsigned int numTopologicalDims;
    unsigned int haveConnectivityCount = 0;
    string connectivityDatasetName;

    // For now users can have only one connectivity dataset.
    if( (connectivityMeta = unstructuredMesh->getLinesDataset())) {
      haveConnectivityCount = 2;
      numTopologicalDims = 2;
      connectivityDatasetName = unstructuredMesh->getLinesDatasetName();
    } else if( (connectivityMeta = unstructuredMesh->getPolygonsDataset()) ) {
      haveConnectivityCount = 0;
      numTopologicalDims = 2;
      connectivityDatasetName = unstructuredMesh->getPolygonsDatasetName();
    } else if( (connectivityMeta = unstructuredMesh->getTrianglesDataset()) ) {
      haveConnectivityCount = 3;
      numTopologicalDims = 2;
      connectivityDatasetName = unstructuredMesh->getTrianglesDatasetName();
    } else if( (connectivityMeta = unstructuredMesh->getQuadrilateralsDataset()) ) {
      haveConnectivityCount = 4;
      numTopologicalDims = 2;
      connectivityDatasetName = unstructuredMesh->getQuadrilateralsDatasetName();
    } else if( (connectivityMeta = unstructuredMesh->getPolyhedraDataset()) ) {
      haveConnectivityCount = 0;
      numTopologicalDims = 3;
      connectivityDatasetName = unstructuredMesh->getPolyhedraDatasetName();
    } else if( (connectivityMeta = unstructuredMesh->getTetrahedralsDataset()) ) {
      haveConnectivityCount = 4;
      numTopologicalDims = 3;
      connectivityDatasetName = unstructuredMesh->getTetrahedralsDatasetName();
    } else if( (connectivityMeta = unstructuredMesh->getPyramidsDataset()) ) {
      haveConnectivityCount = 5;
      numTopologicalDims = 3;
      connectivityDatasetName = unstructuredMesh->getPyramidsDatasetName();
    } else if( (connectivityMeta = unstructuredMesh->getPrismsDataset()) ) {
      haveConnectivityCount = 6;
      numTopologicalDims = 3;
      connectivityDatasetName = unstructuredMesh->getPrismsDatasetName();
    } else if( (connectivityMeta = unstructuredMesh->getHexahedralsDataset()) ) {
      haveConnectivityCount = 8;
      numTopologicalDims = 3;
      connectivityDatasetName = unstructuredMesh->getHexahedralsDatasetName();
    }

    else {
      VsLog::debugLog() << methodSig
                        << "ERROR - unable to find connectivity dataset." << endl;
      VsLog::debugLog() << methodSig
                        << "ERROR - Returning points data without connectivity." << endl;
      //we used to try to convert this into a pointMesh
      // but it never worked
      // So basically we're giving up here
      return ugridPtr;
    }

    VsLog::debugLog() << methodSig << "Found connectivity data." << endl;
    VsLog::debugLog() << methodSig
                      << "haveConnectivityCount = " << haveConnectivityCount << endl;
    VsLog::debugLog() << methodSig
      << "Connectivity dataset name is " << connectivityDatasetName << endl;
    
    VsH5Dataset* connectivityDataset =
      registry->getDataset(connectivityDatasetName);
    
    vector<int> connectivityDims = connectivityMeta->getDims();
    
    size_t numCells, numVerts;
    if( unstructuredMesh->isCompMajor() )
    {
      numCells = connectivityDims[1];
      numVerts = connectivityDims[0];
    }
    else
    {
      numCells = connectivityDims[0];
      numVerts = connectivityDims[1];
    }

    // Adjust for the data selections which are ZONAL.
    if( haveDataSelections )
    {
      if( maxs[0] < 0 || numCells - 1 < maxs[0] )
        maxs[0] = numCells - 1; // numCells - 1 = last cell index,
                                // not number of cells

      if( maxs[0] < mins[0] )
        mins[0] = 0;

      VsLog::debugLog() << methodSig
        << "Have a zonal inclusive selection for unstructured mesh cells " << endl
        << "(" << mins[0] << "," << maxs[0] << " stride " << strides[0] << ") "   << endl;
      
      // New number of cells based on the above.
      numCells = (maxs[0]-mins[0]+1) / strides[0];
    }

    VsLog::debugLog() << methodSig
                      << "numCells = " << numCells
                      << ", numVerts = " << numVerts << "." << endl;
    size_t datasetLength = 1;
    for (size_t i =0; i< connectivityDims.size(); ++i)
      datasetLength *= connectivityDims[i];

    // Check for connectivity list type
    if (!isIntegerType( connectivityMeta->getType() )) {
      VsLog::debugLog() << methodSig
                        << "Indices are not integers. Cleaning up" << endl;
      ugridPtr->Delete();
      VsLog::debugLog() << methodSig << "Returning NULL" << endl;
      return NULL;
    }

    // Vertices
    VsLog::debugLog() << methodSig << "Allocating space for " << datasetLength
                      << " integers of connectivity data." << endl;
    int* vertices = new int[datasetLength];
    if (!vertices) {
      VsLog::debugLog() << methodSig
                        << "Unable to allocate vertices.  Cleaning up." << endl;
      ugridPtr->Delete();
      VsLog::debugLog() << methodSig << "Returning NULL." << endl;
      return NULL;
    }

    VsLog::debugLog() << methodSig << "Reading connectivity list data." << endl;

    if( haveDataSelections )
    {
      int srcMins[1] = {mins[0]};
      int srcMaxs[1] = {numCells};
      int srcStrides[1] = {strides[0]};
      
      err = reader->getDataSet( connectivityDataset, vertices,
                                // -1 read all coordinate components
                                unstructuredMesh->getIndexOrder(), -1,
                                &(srcMins[0]), &(srcMaxs[0]), &(srcStrides[0]) );
    }
    else
      err = reader->getDataSet(connectivityDataset, vertices);
    
    if (err < 0) {
      VsLog::debugLog() << methodSig
                        << "Call to getDataSet returned error: "
                        << err << "Returning NULL." << endl;
      return NULL;
    }


    try {
      VsLog::debugLog() << methodSig << "Allocating " << numCells << " cells.  "
        << "If old VTK and this fails, it will just abort." << endl;
      ugridPtr->Allocate(numCells);
      VsLog::debugLog() << methodSig << "Allocation succeeded." << endl;
    }
    // JRC: what goes here to detect failure to allocate?
    catch (vtkstd::bad_alloc) {
      VsLog::debugLog() << methodSig
                        << "Caught vtkstd::bad_alloc exception." << endl;
      VsLog::debugLog() << methodSig
                        << "Unable to allocate space for cells.  Cleaning up."
      << endl;
      delete [] vertices;
      ugridPtr->Delete();
      VsLog::debugLog() << methodSig << "Returning NULL.";
      return NULL;
    } catch (...) {
      VsLog::debugLog() << methodSig
                        << "Unknown exception allocating cells.  Cleaning up."
      << endl;
      delete [] vertices;
      ugridPtr->Delete();
      VsLog::debugLog() << methodSig << "Returning NULL.";
      return NULL;
    }

    VsLog::debugLog() << methodSig << "Inserting cells into grid." << endl;
    size_t k = 0;
    int warningCount = 0;
    int cellCount = 0;
    unsigned int cellVerts = 0; // cell's connected node indices
    int cellType;

    // Dealing with fixed length connectivity lists.
    if( haveConnectivityCount ) {
      cellVerts = haveConnectivityCount;
      VsLog::debugLog() << methodSig << "Inserting " << cellVerts
                        << " vertices into each cell." << endl;
    }

    for (size_t i = 0; i < numCells; ++i) {
      if (k >= datasetLength) {
        VsLog::debugLog() << methodSig
          << "While iterating over the vertices, the index variable 'k' went beyond the end of the array." << endl;
        VsLog::debugLog() << methodSig
          << "Existing cells will be returned but the loop is terminating now." << endl;
        break;
      }

      ++cellCount;

      // Dealing with cells with variable number of vertices.
      if( haveConnectivityCount == 0 )
        cellVerts = vertices[k++];

      if (cellVerts > numVerts) {
        //funny, the number of cells exceeds the length of the line
        // this must be an error
        // we will drop back to adding individual vertices
        cellVerts = 0;
      }

      switch (cellVerts) {
        case 1:
          cellType = VTK_VERTEX;
        break;
        case 2:
          cellType = VTK_LINE;
        break;
        case 3:
          cellType = VTK_TRIANGLE;
        break;
        case 4:
          if( numTopologicalDims == 2 ) cellType = VTK_QUAD;
          else if( numTopologicalDims == 3 ) cellType = VTK_TETRA;
        break;
        case 5:
          if( numTopologicalDims == 2 ) cellType = VTK_POLYGON;
          else if( numTopologicalDims == 3 ) cellType = VTK_PYRAMID;
        break;
        case 6:
          if( numTopologicalDims == 2 ) cellType = VTK_POLYGON;
          else if( numTopologicalDims == 3 ) cellType = VTK_WEDGE;
        break;
        case 8:
          if( numTopologicalDims == 2 ) cellType = VTK_POLYGON;
          else if( numTopologicalDims == 3 ) cellType = VTK_HEXAHEDRON;
        break;
        default:
        if (warningCount < 30) {
          VsLog::debugLog() << methodSig
                            << "Error: invalid number of vertices for cell #"
                            << cellCount << ": " << cellVerts << endl;
        } else if (warningCount == 30) {
          VsLog::debugLog() << methodSig
                            << "Exceeded maximum number of errors.  Error messages disabled for remaining cells." << endl;
        }
        ++warningCount;
        cellType = VTK_EMPTY_CELL;
        break;
      }

      //create cell and insert into mesh
      if (cellType != VTK_EMPTY_CELL) {
        vector<vtkIdType> verts(cellVerts);
        for (size_t j = 0; j < cellVerts; ++j) {
          verts[j] = (vtkIdType) vertices[k++];
          if ((verts[j] < 0) || (verts[j] >= numNodes)) {
            VsLog::errorLog() << methodSig
              << "ERROR in connectivity dataset - requested vertex number "
              << verts[j] << " exceeds number of vertices" << endl;
            verts[j] = 0;
          }
        }
        // insert cell into mesh
        ugridPtr->InsertNextCell(cellType, cellVerts, &verts[0]);

        if( haveConnectivityCount == 0 )
          k += numVerts - 1 - cellVerts;

      } else {
        // There was some error so add each vertex as a single point.
        // NO!  Unless we've registered as a pointmesh, adding single
        //points won't work so we treat the entire row of the dataset
        //as a single cell Maybe something will work!
        vector<vtkIdType> verts(numVerts);
        k--;
        for (size_t j = 0; j < numVerts; ++j) {
          if (warningCount < 30) {
            VsLog::debugLog()<< methodSig << "WARNING: ADDING cell #"
                             << cellCount << " as cell: "
                             << vertices[k] << endl;
          }
          verts[j] = (vtkIdType) vertices[k++];
        }
        ugridPtr->InsertNextCell (VTK_POLYGON, numVerts, &verts[0]);
      }
    }

    VsLog::debugLog() << methodSig << "Finished.  Cleaning up." << endl;
    // Done, so clean up memory and return
    delete [] vertices;

    VsLog::debugLog() << methodSig << "Returning data." << endl;
    return ugridPtr;
}
  

// *****************************************************************************
//  Method: avtImageVileFormat::getPointMesh
//
//  Purpose:
//      How do you do the voododo that you do
//
//  Programmer: Marc Durant
//  Creation:   June, 2010
//
//  Modifications:
//

vtkDataSet* avtVsFileFormat::getPointMesh(VsVariableWithMesh* variableWithMesh,
                                          bool haveDataSelections,
                                          int* mins, int* maxs, int* strides)
{
    string methodSig("avtVsFileFormat::getPointMesh() - ");
    VsLog::debugLog() << methodSig << "Entering function." << endl;
    LoadData();

    hid_t type = variableWithMesh->getType();

    if (!isDoubleType(type) && !isFloatType(type)) {
      VsLog::debugLog() << methodSig << "Unsupported data type: "
                        << type << endl;
      VsLog::debugLog() << methodSig << "Returning NULL." << endl;
      return NULL;
    }

    // Get the number of values
    VsLog::debugLog() << methodSig << "Getting the number of points." << endl;

    size_t numNodes = variableWithMesh->getNumPoints();
    size_t numSpatialDims = variableWithMesh->getNumSpatialDims();

    // Adjust for the data selections which are ZONAL.
    if (haveDataSelections)
    {
      if( maxs[0] <= 0 || numNodes - 1 < maxs[0] )
        maxs[0] = numNodes - 1; // numNodes - 1 = last cell index,
                                // not number of cells
      if( maxs[0] < mins[0] )
        mins[0] = 0;

      VsLog::debugLog() << methodSig
        << "Have a zonal inclusive selection for variable with mesh "<< endl
        << "(" << mins[0] << "," << maxs[0] << " stride " << strides[0] << ") "   << endl;
    }
    else
    {
      mins[0] = 0;
      maxs[0] = numNodes - 1; // numNodes - 1 = last cell index, not
                              // number of cells
      strides[0] = 1;
    }

    // New number of nodes based on the above.
    numNodes = (maxs[0]-mins[0]+1) / strides[0];

    VsLog::debugLog() << methodSig << "There are " << numNodes
                      << " points." << endl;

#if (defined PARALLEL && defined VIZSCHEMA_DECOMPOSE_DOMAINS)
    VsLog::debugLog() << methodSig
                      << "Parallel & Decompose_Domains are defined, "
                      << "entering parallel code." << endl;

    // Integer number of nodes per processor
    size_t numNodesPerProc = numNodes / PAR_Size();
    size_t numProcsWithExtraNode = numNodes % PAR_Size();

    // To get all of the nodes adjust the count by one for those
    // processors that need an extra node.
    if (PAR_Rank() < numProcsWithExtraNode)
    {
      numNodes = numNodesPerProc + 1;
      mins[0] = PAR_Rank() * (numNodesPerProc + 1)  * strides[0];
    }
    else
    {
      numNodes = numNodesPerProc;
      // Processors w/extra node plus processors without extra node.
      mins[0] = (numProcsWithExtraNode * (numNodesPerProc + 1) +
               (PAR_Rank() - numProcsWithExtraNode) * numNodesPerProc) *
        strides[0];
    }

    VsLog::debugLog() << methodSig
                      << "Parallel & Decompose_Domains are defined, "
                      << "exiting parallel code." << endl;
#endif

    // Read in points
    //
    // Create the unstructured meshPtr
    VsLog::debugLog() << methodSig
                      << "Creating the vtkUnstructuredGrid." << endl;
    vtkUnstructuredGrid* meshPtr = vtkUnstructuredGrid::New();

    // Create and set points while small so minimal memory usage
    vtkPoints* vpts = vtkPoints::New();
    meshPtr->SetPoints(vpts);
    vpts->Delete();
    vtkPoints* vpoints = meshPtr->GetPoints();

    // Allocate
    size_t dsize = 0;
    if (isDoubleType(type)) {
      vpoints->SetDataTypeToDouble();
      dsize = sizeof(double);
      VsLog::debugLog() << "Double data" << endl;
    }
    else if (isFloatType(type)) {
      vpoints->SetDataTypeToFloat();
      dsize = sizeof(float);
      VsLog::debugLog() << "Float data" << endl;
    }
    
    VsLog::debugLog() << methodSig
                      << "Allocating for " << numNodes << " values." << endl;
    vpoints->SetNumberOfPoints(numNodes);

    void* dataPtr = vpoints->GetVoidPointer(0);
    if (!dataPtr) {
      VsLog::debugLog() << methodSig
                        << "Unable to allocate the points.  Cleaning up."
      << endl;
      meshPtr->Delete();
      VsLog::debugLog() << methodSig << "Returning NULL." << endl;
      return NULL;
    }

    herr_t err;

    // Read in the data
    VsLog::debugLog() << methodSig << "Reading data." << endl;

    // Kind of grim but read each of the coordinate components in
    // using the generic reader.

    // To do so set the location in memory where each cordinate
    // componenet will be stored.
    int srcMins[1] = {mins[0]};
    int srcMaxs[1] = {numNodes};
    int srcStrides[1] = {strides[0]};

    int destSize[1] = {numNodes*3};
    int destMins[1] = {0};
    int destMaxs[1] = {numNodes};
    int destStrides[1] = {3};
      
    for (unsigned int i= 0; i< numSpatialDims; i++) {

      int componentIndex = variableWithMesh->getSpatialDim(i);

      destMins[0] = i;

      VsH5Dataset* variableDataset =
        registry->getDataset(variableWithMesh->getFullName());

      err = reader->getDataSet(variableDataset, dataPtr,
                               variableWithMesh->getIndexOrder(),
                               componentIndex,
                               &srcMins[0], &srcMaxs[0], &srcStrides[0],
                               1, &destSize[0],
                               &destMins[0], &destMaxs[0], &destStrides[0] );

      if (err < 0) {
        VsLog::debugLog() << methodSig
                          << "Call to getVarWithMeshMeta returned error: "
                          << err << "Returning NULL." << endl;
        return NULL;
      }
    }

    // The above stores the value in the correct location but make
    // sure the remaing value are all zero.
    for (int i=numSpatialDims; i<3; ++i)
    {
      VsLog::debugLog() << methodSig
                        << "Zeroing data at unused positions." << endl;

      if (isDoubleType(type)) {
        
        double* dblDataPtr = &(((double*) dataPtr)[i]);
        
        for (int j=0; j<numNodes; ++j)
        {
          *dblDataPtr = 0;
          dblDataPtr += 3;
        }
      }
      else if (isFloatType(type)) {
        
        float* fltDataPtr = &(((float*) dataPtr)[i]);
        
        for (int j=0; j<numNodes; ++j)
        {
          *fltDataPtr = 0;
          fltDataPtr += 3;
        }
      }
    }

    // create point mesh
    try {
      VsLog::debugLog() << methodSig << "Allocating " << numNodes
        << " vertices.  If old VTK and this fails, it will just abort." << endl;
      meshPtr->Allocate(numNodes);
    } catch (vtkstd::bad_alloc) {
      VsLog::debugLog() << methodSig
                        << "Caught vtkstd::bad_alloc. Unable to allocate cells."
      << "Cleaning up." << endl;
      meshPtr->Delete();
      VsLog::debugLog() << methodSig << "Returning NULL." << endl;
      return NULL;
    } catch (...) {
      VsLog::debugLog() << methodSig
                        << "Unknown exception. Unable to allocate cells."
      << "Cleaning up." << endl;
      meshPtr->Delete();
      VsLog::debugLog() << methodSig << "Returning NULL." << endl;
      return NULL;
    }

    VsLog::debugLog() << methodSig
      << "Allocation succeeded.  Setting mesh to connectivity 'VERTEX'." << endl;
    vtkIdType vertex;
    for (int i=0; i<numNodes; ++i) {
      vertex = i;
      meshPtr->InsertNextCell(VTK_VERTEX, 1, &vertex);
    }

    // Clean up memory
    VsLog::debugLog() << methodSig << "Returning data." << endl;

    return meshPtr;
}


// *****************************************************************************
//  Method: avtImageVileFormat::getCurve
//
//  Purpose:
//      How do you do the voododo that you do
//
//  Programmer: Marc Durant
//  Creation:   June, 2010
//
//  Modifications:
//
vtkDataSet* avtVsFileFormat::getCurve(int domain, const string& requestedName) {

    string methodSig("avtVsFileFormat::getCurve() - ");
    VsLog::debugLog() << methodSig << "Entering function." << endl;
    LoadData();

    // Save the name in a temporary variable as it gets mucked with if
    // searching for a component.
    string name = requestedName;

    ///TODO: This method piggybacks on getVar and getMesh -
    ///       Could be made much more efficient
    // 1. get var
    // 2. get matching mesh
    // 3. create 2 coord arrays - x from mesh and y from var
    // 4. Combine coord arrays to form 1-d Rectilinear mesh

    // Attempts to follow visit plugin Curve2d in
    // ./visit/src/databases/Curve2d/avtCurve2DFileFormat.C

    // Retrieve var metadata and extract the mesh name
    VsLog::debugLog() << methodSig << "Looking for variable metadata." << endl;
    VsVariable* varMeta = registry->getVariable(name);

    if (varMeta == NULL) {
      VsLog::debugLog() << methodSig
        << "No variable metadata found under name " << name << endl;
      VsLog::debugLog() << methodSig
        << "Looking for information in component registry." << endl;

      // Is this variable a component?  If so, swap the component name
      // with the "real" variable name and remember that it is a
      // component.
      bool isAComponent = false;
      int componentIndex = -2; // No components
      NamePair foundName;
      registry->getComponentInfo(name, &foundName);

      if (!foundName.first.empty()) {
        name = foundName.first.c_str();
        varMeta = registry->getVariable(foundName.first);
        componentIndex = foundName.second;
        VsLog::debugLog() << methodSig
                          << "Found a component, which refers to variable "
                          << name << " and index " << componentIndex << endl;
        isAComponent = true;
      }
    }

    if (varMeta == NULL) {
      VsLog::debugLog() << methodSig
                        << "Failed to find variable in component list.  "
                        << "Returning NULL." << endl;
      return NULL;
    }

    hid_t varDataType = varMeta->getType();

    if (isDoubleType(varDataType)) {
      VsLog::debugLog() << methodSig << "Var is 64-bit real" << endl;
    } else if (isFloatType(varDataType)) {
      VsLog::debugLog() << methodSig << "Var is 32-bit real" << endl;
    } else if (isIntegerType(varDataType)) {
      VsLog::debugLog() << methodSig << "Var is int" << endl;
    } else {
      VsLog::debugLog() << methodSig
        << "Var is unknown type (known are double, float, int).  "
        << "Returning NULL" << endl;
      return NULL;
    }

    vtkDataArray* varData = NULL;
    try {
      varData = GetVar(domain, requestedName.c_str());
    } catch (...) {
      VsLog::debugLog() << methodSig
        << "Caught exception from GetVar().  Returning NULL." << endl;
      return NULL;
    }

    if (varData == NULL) {
      VsLog::debugLog() << methodSig
                        << "Failed to load var data.  Returning NULL." << endl;
      return NULL;
    }

    // Have the variable now get the mesh.
    string meshName = varMeta->getMeshName();
    VsMesh* meshMeta = varMeta->getMesh();

    if (meshMeta == NULL) {
      VsLog::debugLog() << methodSig
                        << "No mesh metadata found.  Returning NULL." << endl;
      return NULL;
    }

    vtkRectilinearGrid* meshData = NULL;
    try {
      meshData = (vtkRectilinearGrid*)GetMesh(domain, meshName.c_str());
    } catch (...) {
      VsLog::debugLog() << methodSig
                        << "Caught exception from GetMesh().  Returning NULL."
                        << endl;
      varData->Delete();
      return NULL;
    }

    if (meshData == NULL) {
      VsLog::debugLog() << methodSig
                        << "Failed to load mesh data.  Returning NULL." << endl;
      varData->Delete();
      return NULL;
    }

    // Get the number of points on the curve.
    int nPts = 0;
    if (varMeta->isCompMinor()) {
      nPts = varMeta->getDims()[0];
    } else {
      nPts = varMeta->getDims()[1];
    }

    VsLog::debugLog() << methodSig
                      << "Variable has " << nPts << " points." << endl;

    // Create 1-D RectilinearGrid
    VsLog::debugLog() << methodSig << "Building Rectilinear grid." << endl;
    vtkFloatArray* vals = vtkFloatArray::New();
    vals->SetNumberOfComponents(1);
    vals->SetNumberOfTuples(nPts);
    vals->SetName(name.c_str());

    vtkRectilinearGrid* rg = vtkVisItUtility::Create1DRGrid(nPts, VTK_FLOAT);
    rg->GetPointData()->SetScalars(vals);

    vtkFloatArray* xc = vtkFloatArray::SafeDownCast(rg->GetXCoordinates());
    vtkDataArray* meshXCoord = meshData->GetXCoordinates();

    for (int i = 0; i < nPts; i++) {
      double* var_i = varData->GetTuple(i);
      double* mesh_i = meshXCoord->GetTuple(i);
      xc->SetValue(i, mesh_i[0]);
      vals->SetValue(i, var_i[0]);
    }

    VsLog::debugLog() << methodSig << "Deleting temporary variables." << endl;
    vals->Delete();

    VsLog::debugLog() << methodSig << "Returning data." << endl;
    return rg;
}


// *****************************************************************************
//  Method: avtVsFileFormat::GetVar
//
//  Purpose:
//      How do you do the voododo that you do
//
//  Programmer: Marc Durant
//  Creation:   June, 2010
//
//  Modifications:
//

vtkDataArray* avtVsFileFormat::GetVar(int domain, const char* requestedName)
{
    bool haveDataSelections;
    int mins[3], maxs[3], strides[3];

    if( haveDataSelections = ProcessDataSelections(mins, maxs, strides) )
    {
      VsLog::debugLog() << "have a data selection for a variable "<< endl
           << "(" << mins[0] << "," << maxs[0] << " stride " << strides[0] << ") "
           << "(" << mins[1] << "," << maxs[1] << " stride " << strides[1] << ") "
           << "(" << mins[2] << "," << maxs[2] << " stride " << strides[2] << ") "
           << endl;
    }

    // Save the name in a temporary variable as it gets mucked with if
    // searching for a component.
    string name = requestedName;

    stringstream sstr;
    sstr << "avtVsFileFormat::getVar(" << domain << ", " << name << ") - ";
    string methodSig = sstr.str();

    LoadData();

    // Is this variable a component?  If so, swap the component name
    // with the "real" variable name and remember that it is a
    // component.
    bool isAComponent = false;
    int componentIndex = -2; // No components
    NamePair foundName;
    registry->getComponentInfo(name, &foundName);

    if (!foundName.first.empty()) {
      name = foundName.first.c_str();
      componentIndex = foundName.second;
      VsLog::debugLog() << methodSig
                        << "Found a component, which refers to variable "
                        << name << " and index " << componentIndex << endl;
      isAComponent = true;
    }

    // The goal in all of the metadata loading is to fill one of
    // these two variables:
    VsVariable* meta = NULL;
    VsVariableWithMesh* vmMeta = NULL;
    VsH5Dataset* variableDataset = NULL;

    // It could be an MD variable if so, retrieve the md metadata,
    // look up the "real" variable name using the domain number, and
    // replace "name" with the name of that variable.
    VsLog::debugLog() << methodSig << "Checking for possible MD var." << endl;
    VsMDVariable* mdMeta = registry->getMDVariable(name);
    if (mdMeta == NULL) {
      VsLog::debugLog() << methodSig
                        << "No MD Var or component found under the name: "
                        << name << endl;
    } else {
      VsLog::debugLog() << methodSig
                        << "Found MD metadata for this name: "
                        << name << endl;

      if ((domain < 0) || (domain > mdMeta->blocks.size())) {
        VsLog::warningLog()
          << methodSig
          << "Requested domain number is out of bounds for this variable."
          << endl;
      } else {
        meta = mdMeta->blocks[domain];
        name = meta->getFullName();
      }
    }

    // No meta data so, look for a "regular" variable with this name.
    if (meta == NULL) {
      VsLog::debugLog() << methodSig
                        << "Looking for regular (non-md) variable." << endl;
      meta = registry->getVariable(name);

      if( meta )
        variableDataset = registry->getDataset(meta->getFullName());
    }

    // No meta data so, look for a VarWithMesh with this name.
    if (meta == NULL) {
      VsLog::debugLog() << methodSig
                        << "Looking for VarWithMesh variable." << endl;
      vmMeta = registry->getVariableWithMesh(name);

      if( vmMeta )
        variableDataset = registry->getDataset(vmMeta->getFullName());
    }

    // Haven't found metadata yet, so give up.
    if ((meta == NULL) && (vmMeta == NULL)) {
      VsLog::debugLog() << methodSig
                        << "ERROR: Could not find metadata for name: "
                        << name << endl;
// DEBUG CODE
//       vector<string> varNames;
//       registry->getAllVariableNames(varNames);
//       VsLog::debugLog() << methodSig << "All available names = ";
//       for (unsigned int i = 0; i < varNames.size(); i++) {
//         VsLog::debugLog() << varNames[i] << ", ";
//       }
//       VsLog::debugLog() << endl;
//       VsLog::debugLog() << "Returning NULL" << endl;
// END DEBUG
      return NULL;
    }

    // Have metadata from the variable, so load some info and look for
    // the mesh.
    hid_t varType = 0;
    vector<int> varDims;
    bool isFortranOrder = false;
    bool isCompMajor = false;
    bool isZonal = false;

    std::string indexOrder;

    size_t numTopologicalDims;

    // VarWithMesh variable - Note that there is no mesh metadata for
    // VarWithMesh.
    if (vmMeta) {
      varDims = vmMeta->getDims();
      varType = vmMeta->getType();

      isZonal = true;

      indexOrder = vmMeta->getIndexOrder();
      isCompMajor  = vmMeta->isCompMajor();

      numTopologicalDims = 1;
    }

    else {
      varDims = meta->getDims();
      varType = meta->getType();

      if (meta->isFortranOrder())
        isFortranOrder = true;

      if (meta->isZonal())
        isZonal = true;

      indexOrder = meta->getIndexOrder();
      isCompMajor  = meta->isCompMajor();

      string meshName = meta->getMeshName();
      VsMesh* meshMetaPtr = registry->getMesh(meshName);

      VsLog::debugLog() << methodSig
                        << "Mesh for variable is '" << meshName << "'." << endl;
      VsLog::debugLog() << methodSig << "Getting metadata for mesh." << endl;

      if (!meshMetaPtr) {
        VsLog::debugLog() << methodSig
                          << "Metadata not found for mesh '" << meshName
                          << "'." << endl;
        VsLog::debugLog() << methodSig << "Returning NULL" << endl;
        return NULL;
      }

      vector<int> meshDims;
      meshMetaPtr->getMeshDataDims(meshDims);
      numTopologicalDims = meshDims.size();

      // Structured and unstructured mesh dimension contain the
      // spatial dimension so subtract it off.
      if( meshMetaPtr->isStructuredMesh() || meshMetaPtr->isUnstructuredMesh() )
        numTopologicalDims -= 1;

      if( varDims.size() - (int) isAComponent < numTopologicalDims )
      {
        VsLog::debugLog() << methodSig
                          << "Var and mesh do not have the spatial dimension"
                          << "Returning NULL." << endl;
        return NULL;
      }

      // For unstructured data only subset the variable is the data is
      // zonal.
      if( meshMetaPtr->isUnstructuredMesh() && !isZonal )
        haveDataSelections = false;
    }

    bool isDouble = isDoubleType(varType);
    bool isFloat = isFloatType(varType);
    bool isInteger = isIntegerType(varType);

    if (!isDoubleType && !isFloatType  && !isIntegerType) {
      VsLog::debugLog() << methodSig << "Unknown data type:" << varType << endl;
      VsLog::debugLog() << methodSig << "Returning NULL." << endl;
      return NULL;
    }

    // The variable dims should reflect the topological dims plus one
    // more dimension for the spatial dimension of the variable.
    size_t vsdim = numTopologicalDims + 1;

    // Adjust for the data selections which are ZONAL.
    if( haveDataSelections )
    {
      for (size_t i=0; i<numTopologicalDims; ++i)
      {
        if( maxs[i] <= 0 ||
            ( isZonal && varDims[i+isCompMajor] - 1 < maxs[i]) ||
            (!isZonal && varDims[i+isCompMajor] - 2 < maxs[i]) )
        {
          if( isZonal )
            maxs[i] = varDims[i+isCompMajor] - 1;  // varDims - 2 = last cell index, not
                                       // number of cells
          else // is nodal
            maxs[i] = varDims[i+isCompMajor] - 2;  // varDims - 1 = last cell index,
                                       // not number of cells
        }

        if( maxs[i] < mins[i] )
          mins[i] = 0;
      }
    }
    else
    {
      for (size_t i=0; i<numTopologicalDims; ++i)
      {
        mins[i] = 0;
        
        if( isZonal ) // varDims - 2 = last cell index, not number of cells
          maxs[i] = varDims[i+isCompMajor] - 1;  
                                     
        else // is nodal // varDims - 1 = last cell index, not number of cells
          maxs[i] = varDims[i+isCompMajor] - 2;

        strides[i] = 1;
      }
    }

    for (size_t i=numTopologicalDims; i<vsdim; ++i)
    {
      mins[i] = 0;
      maxs[i] = 1;
      strides[i] = 1;
    }

    if( haveDataSelections )
      VsLog::debugLog() << methodSig
        << "Have a zonal inclusive selection for getting a variable "<< endl
        << "(" << mins[0] << "," << maxs[0] << " stride " << strides[0] << ") "
        << "(" << mins[1] << "," << maxs[1] << " stride " << strides[1] << ") "
        << "(" << mins[2] << "," << maxs[2] << " stride " << strides[2] << ") "
        << endl;

    // Get the new number of variables.
    vector<int> vdims(vsdim);
    int numVariables = 1;

    for (size_t i=0; i<numTopologicalDims; ++i)
    {
      vdims[i] = ((maxs[i]-mins[i]+1)/strides[i] +    // Number of cells
//  COMMENTED OUT FOR NOW AS HDF5 DOES NOT HANDLE PARTIAL CELLS
//                 ((maxs[i]-mins[i]+1)%strides[i] ? 1 : 0) + // Partial cell
                  (isZonal ? 0 : 1) );                // Last variable

      numVariables *= vdims[i];
    }

    for (size_t i=numTopologicalDims; i<vsdim; ++i)
    {
      vdims[i] = 1;
    }

#if (defined PARALLEL && defined VIZSCHEMA_DECOMPOSE_DOMAINS)
    if (vmMeta || isAComponent) {
      VsLog::debugLog() << methodSig
                        << "Parallel & Decompose_Domains are defined, "
                        << "entering parallel code." << endl;

      numVariables = 1;

      for (size_t i=0; i<numTopologicalDims; ++i)
      {
        int numNodes = vdims[i];

        // Integer number of nodes per processor
        size_t numNodesPerProc = numNodes / PAR_Size();
        size_t numProcsWithExtraNode = numNodes % PAR_Size();
      
        // To get all of the nodes adjust the count by one for those
        // processors that need an extra node.
        if (PAR_Rank() < numProcsWithExtraNode)
        {
          vdims[i] = numNodesPerProc + 1;
          mins[i] = PAR_Rank() * (numNodesPerProc + 1) * strides[i];
        }
        else
        {
          vdims[i] = numNodesPerProc;
          // Processors w/extra node plus processors without extra node.
          mins[i] = (numProcsWithExtraNode * (numNodesPerProc + 1) +
                      (PAR_Rank() - numProcsWithExtraNode) * numNodesPerProc) *
            strides[i];
        }

        numVariables *= vdims[i];
      }

      VsLog::debugLog() << methodSig
                        << "Parallel & Decompose_Domains are defined, "
                        << "exiting parallel code." << endl;
    }
#endif

    VsLog::debugLog() << methodSig
                      << "Total number of variable is " << numVariables << endl;

    VsLog::debugLog() << methodSig
                      << "Variable " << name << " has dimensions = ";

    VsLog::debugLog() << methodSig
                      << ", numTopologicalDims = " << numTopologicalDims
                      << ", isComponent = " << isAComponent << "." << endl;


    size_t varSize = H5Tget_size(varType);

    // Read in the data
    VsLog::debugLog() << methodSig << "Reading in the data." << endl;
    void* dataPtr = 0;
    double* dblDataPtr = 0;
    float* fltDataPtr = 0;
    int* intDataPtr = 0;

    // Do the allocation here no matter the variable being read -
    // except for the case above.
    if (isDouble) {
      dblDataPtr = new double[numVariables];
      dataPtr = dblDataPtr;
    }
    else if (isFloat) {
      fltDataPtr = new float[numVariables];
      dataPtr = fltDataPtr;
    }
    else if (isInteger) {
      intDataPtr = new int[numVariables];
      dataPtr = intDataPtr;
    }
    
    if (!dataPtr) {
      VsLog::debugLog() << methodSig
                        << "Allocation failed, pointer is NULL.  "
                        << "Returning NULL." << endl;
      return NULL;
    }

    // Variable and mesh data or a variable component.
    if (vmMeta || isAComponent) {
      VsLog::debugLog() << methodSig << "Entering VarWithMesh section." << endl;

      herr_t err = reader->getDataSet(variableDataset, dataPtr,
                                      indexOrder, componentIndex,
                                      mins, &(vdims[0]), strides);

      if (err < 0) {
        VsLog::debugLog() << methodSig
          << "GetVarWithMeshComponent returned error: " << err << endl;
        VsLog::debugLog() << methodSig << "Returning NULL." << endl;

        if (isDouble)
          delete [] dblDataPtr;
        else if (isFloat)
          delete [] fltDataPtr;
        else if (isInteger)
          delete [] intDataPtr;

        return NULL;
      }
    }

    // Read a regular variable
    else {
      VsLog::debugLog() << methodSig << "Loading variable data." << endl;

      herr_t err;
    
#if (defined PARALLEL && defined VIZSCHEMA_DECOMPOSE_DOMAINS)
      err = reader->getDataSet( variableDataset, dataPtr,
                                meta->getIndexOrder(), -2, // -2 no components
                                mins, &(vdims[0]), strides );
#else
      if( haveDataSelections )
        err = reader->getDataSet( variableDataset, dataPtr,
                                  meta->getIndexOrder(), -2, // -2 no components
                                  mins, &(vdims[0]), strides );
      else
        err = reader->getDataSet( variableDataset, dataPtr );
#endif
      if (err < 0) {
        VsLog::debugLog() << methodSig
                          << "GetDataSet returned error: " << err << endl;
        VsLog::debugLog() << methodSig << "Returning NULL." << endl;

        if (isDouble)
          delete [] dblDataPtr;
        else if (isFloat)
          delete [] fltDataPtr;
        else if (isInteger)
          delete [] intDataPtr;

        return NULL;
      }
    }

    VsLog::debugLog() << methodSig
      << "Finished reading the data, building VTK structures." << endl;

    //DEBUG
    /*
     VsLog::debugLog() << methodSig << "Dumping data: " << endl;
     for (int i = 0; i < numVariables; i++) {
     if (H5Tequal(type, H5T_NATIVE_DOUBLE)) {
     VsLog::debugLog() << "data[" << i << "] = " << ((double*)data)[i] << endl;}
     else if (H5Tequal(type, H5T_NATIVE_FLOAT)) {
     VsLog::debugLog() << "data[" << i << "] = " << ((float*)data)[i] << endl;}
     else if (H5Tequal(type, H5T_NATIVE_INT)) {
     VsLog::debugLog() << "data[" << i << "] = " << ((int*)data)[i] << endl;}
     }
     VsLog::debugLog() << methodSig << "Finished dumping data. " << endl;
     */
    //END DEBUG

    vtkDataArray* rv = 0;

    if (isDoubleType) {
      VsLog::debugLog() << methodSig << "Declaring vtkDoubleArray." << endl;
      rv = vtkDoubleArray::New();
    }
    else if (isFloatType) {
      VsLog::debugLog() << methodSig << "Declaring vtkFloatArray." << endl;
      rv = vtkFloatArray::New();
    }
    else if (isIntegerType) {
      VsLog::debugLog() << methodSig << "Declaring vtkIntArray." << endl;
      rv = vtkIntArray::New();
    }

    rv->SetNumberOfTuples(numVariables);

    // Perform if needed permutation of index as VTK expect Fortran order

    // The index tuple is initially all zeros
    size_t* indices = new size_t[numTopologicalDims];
    for (size_t k=0; k<numTopologicalDims; ++k)
      indices[k] = 0;

    // Store data
    VsLog::debugLog() << methodSig
                      << "Storing " << numVariables << " data elements" << endl;

    // Attempt to reverse data in place
    //#define IN_PLACE
#ifdef IN_PLACE
    VsLog::debugLog() << methodSig
                      << "Attempting to swap data in place." << endl;
    double* dblDataPtr = (double*) data;
    float* fltDataPtr = (float*) data;
    int* intDataPtr = (int*) data;

    // Step through by global C index and reverse
    if (numTopologicalDims > 1) {
      for (size_t k = 0; k<numVariables; ++k) {
        // Accumulate the Fortran index
        size_t indx = indices[numTopologicalDims-1];
        for (size_t j = 2; j<=numTopologicalDims; ++j)
        indx = indx*dims[numTopologicalDims-j] + indices[numTopologicalDims-j];
        // Set the value in the VTK array at the Fortran index to the
        // value in the C array at the C index
        if (isDouble) {
          double tmp = dblDataPtr[indx];
          dblDataPtr[indx] = dblDataPtr[k];
          dblDataPtr[k] = tmp;
        }
        else if (isFloat) {
          float tmp = fltDataPtr[indx];
          fltDataPtr[indx] = fltDataPtr[k];
          fltDataPtr[k] = tmp;
        }
        else if (isInteger) {
          int tmp = intDataPtr[indx];
          intDataPtr[indx = intDataPtr[k];
          intDataPtr[k] = tmp;
        } else {
          VsLog::debugLog() << methodSig
                            << "Unknown data type: " << type << endl;
          return NULL;
        }
        // Update the index tuple
        size_t j = numTopologicalDims;
        do {
          --j;
          ++indices[j];
          if (indices[j] == dims[j]) indices[j] = 0;
          else break;
        }while (j != 0);
      }
    }

    // Reversed in place so now can copy
    if (isDoubleType)  {
      for (size_t k = 0; k<numVariables; ++k) {
        rv->SetTuple(k, &dblDataPtr[k]);
      }
    } else if (isFloatType) {
      for (size_t k = 0; k<numVariables; ++k) {
        rv->SetTuple(k, &fltDataPtr[k]);
      }
    } else if (isIntegerType) {
      for (size_t k = 0; k<numVariables; ++k) {
        rv->SetTuple(k, &intDataPtr[k]);
      }
    } else {
        VsLog::debugLog() << methodSig << "Unknown data type: " << type << endl;
        return NULL;
    }

    VsLog::debugLog() << methodSig << "Done swapping data in place." << endl;
#else

    VsLog::debugLog() << methodSig
       << "Swapping data into correct places, NOT using 'in place' code." << endl;

    // Using FORTRAN data ordering.
    if (isFortranOrder) {
      VsLog::debugLog() << methodSig << "Using FORTRAN data ordering." << endl;

      if (isDoubleType) {
        for (size_t k = 0; k<numVariables; ++k) {
          rv->SetTuple(k, &dblDataPtr[k]);
        }
      } else if (isFloatType) {
        for (size_t k = 0; k<numVariables; ++k) {
          rv->SetTuple(k, &fltDataPtr[k]);
        }
      } else if (isIntegerType) {
        for (size_t k = 0; k<numVariables; ++k) {
          // Convert to float because SetTuple doesn't take ints
          float temp = intDataPtr[k];
          rv->SetTuple(k, &temp);
        }
      }
    } else {

      VsLog::debugLog() << methodSig << "Using C data ordering." << endl;

      // Step through by global C index
      for (size_t k = 0; k<numVariables; ++k) {

        // Accumulate the Fortran index
        size_t indx = indices[numTopologicalDims-1];
        for (size_t j = 2; j<=numTopologicalDims; ++j) {
          indx = indx*vdims[numTopologicalDims-j] +
            indices[numTopologicalDims-j];
        }

        // Set the value in the VTK array at the Fortran index to the
        // value in the C array at the C index
        if (isDouble) {
          rv->SetTuple(indx, &dblDataPtr[k]);
        }
        else if (isFloat) {
          rv->SetTuple(indx, &fltDataPtr[k]);
        }
        else if (isInteger) {
          // Convert to float because SetTuple doesn't take ints
          float temp = intDataPtr[k];
          rv->SetTuple(indx, &temp);
        }

        // Update the index tuple
        size_t j = numTopologicalDims;
        do {
          --j;
          ++indices[j];
          if (indices[j] == vdims[j])
            indices[j] = 0;
          else
            break;
        } while (j != 0);
      }
    }

    VsLog::debugLog() << methodSig
      << "Done swapping data into correct places, NOT using 'in place' code."
      << endl;
#endif

    // Done with data
    VsLog::debugLog() << methodSig << "Cleaning up." << endl;
    delete [] indices;

    if (isDouble) {
      delete [] dblDataPtr;
    } else if (isFloat) {
      delete [] fltDataPtr;
    } else if (isInteger) {
      delete [] intDataPtr;
    }

    VsLog::debugLog() << methodSig << "Returning data." << endl;
    return rv;
}


// *****************************************************************************
//  Method: avtImageVileFormat::FreeUpResources
//
//  Purpose:
//      How do you do the voododo that you do
//
//  Programmer: Marc Durant
//  Creation:   June, 2010
//
//  Modifications:
//

void avtVsFileFormat::FreeUpResources(void)
{
    VsLog::debugLog() << "avtVsFileFormat::FreeUpResources() enter/exit."
                      << endl;
}

 
// *****************************************************************************
//  Method: avtImageVileFormat::RegisterExpressions
//
//  Purpose:
//      How do you do the voododo that you do
//
//  Programmer: Marc Durant
//  Creation:   June, 2010
//
//  Modifications:
//

void avtVsFileFormat::RegisterExpressions(avtDatabaseMetaData* md)
{
    stringstream sstr;
    sstr << "avtVsFileFormat::RegisterExpressions() - ";
    string methodSig = sstr.str();
    VsLog::debugLog() << methodSig << "Entering function." << endl;
    LoadData();

    //get list of expressions from reader
    map<string, string>* expressions = registry->getAllExpressions();
    
    if (expressions->empty()) {
      VsLog::debugLog() << methodSig
        << "WARNING: No expressions found in file. Returning." << endl;
      return;
    } else {
      VsLog::debugLog() << methodSig
                        << "Found " << expressions->size()
                        << " expressions in file." << endl;
    }

    //iterate over list of expressions, insert each one into database
    map<string, string>::const_iterator iv;
    for (iv = expressions->begin(); iv != expressions->end(); ++iv) {
      VsLog::debugLog() << methodSig
                        << "Adding expression " << iv->first << " = "
      << iv->second << endl;
      Expression e;
      e.SetName (iv->first);

      //TODO: if the user has supplied a label for a component but the
      // vsVar expression still refers to the old component name we
      // need to either 1. register the old component name as an extra
      // component or 2. edit the vsVar expression to replace old
      // component names with the user-specified labels.
      e.SetDefinition(iv->second);

      // See if the expression is a vector
      if ((iv->second.size() > 0) && (iv->second[0] == '{')) {
        VsLog::debugLog() << methodSig << "It is a vector expression." << endl;
        e.SetType(Expression::VectorMeshVar);
      }
      else {
        VsLog::debugLog() << methodSig << "It is a scalar expression." << endl;
        e.SetType(Expression::ScalarMeshVar);
      }
      md->AddExpression(&e);
    }

    VsLog::debugLog() << methodSig << "Exiting normally." << endl;
}


// *****************************************************************************
//  Method: avtImageVileFormat::RegisterVars
//
//  Purpose:
//      How do you do the voododo that you do
//
//  Programmer: Marc Durant
//  Creation:   June, 2010
//
//  Modifications:
//

void avtVsFileFormat::RegisterVars(avtDatabaseMetaData* md)
{
    stringstream sstr;
    sstr << "avtVsFileFormat::RegisterVars() - ";
    string methodSig = sstr.str();
    VsLog::debugLog() << methodSig << "Entering function." << endl;
    LoadData();

    // Get var names
    vector<string> names;
    registry->getAllVariableNames(names);
    
    if (names.empty()) {
      VsLog::debugLog() << methodSig
        << "WARNING: No variables were found in this file.  Returning." << endl;
      return;
    } else {
      VsLog::debugLog() << methodSig
                        << "Found " << names.size()
                        << " variables in this file." << endl;
    }

    vector<string>::const_iterator it;
    for (it = names.begin(); it != names.end(); ++it) {
      VsLog::debugLog() << methodSig << "Processing var: "<< *it << endl;

      //get metadata for var
      VsVariable* vMeta = registry->getVariable(*it);
      
      //If this var is part of an MD var, we don't register it separately
      VsVariable* testForMD = registry->findSubordinateMDVar(*it);
      if (testForMD) {
        VsLog::debugLog() << methodSig
                          << "Var is part of an MD variable, "
                          << "will not be registered separately." << endl;
        continue;
      }

      // Name of the mesh of the var
      string mesh = vMeta->getMeshName();
      VsMesh* meshMeta = vMeta->getMesh();
      VsLog::debugLog() << methodSig
                        << "Var lives on mesh " << mesh << "." << endl;

      // Centering of the variable
      // Default is node centered data
      avtCentering centering = AVT_NODECENT;
      if (vMeta->isZonal()) {
        VsLog::debugLog() << methodSig << "Var is zonal." << endl;
        centering = AVT_ZONECENT;
      } else {
        VsLog::debugLog() << methodSig << "Var is nodal." << endl;
      }

      // 1-D variable?
      VsLog::debugLog() << methodSig << "Determining if var is 1-D." << endl;
      int numSpatialDims = 3;
      if (meshMeta) {
        numSpatialDims = meshMeta->getNumSpatialDims();
      } else {
        numSpatialDims = 3;
        VsLog::errorLog() << methodSig
                          << "Unable to load mesh for variable?" << endl;
        VsLog::errorLog() << methodSig
                          << "Using numSpatialDims = 3, hope it's right!"
                          << endl;     
      }

      //if this mesh is 1-D, we leave it for later (curves)
      bool isOneDVar = false;
      if (numSpatialDims == 1) {
        VsLog::debugLog() << methodSig << "Var is 1-D." << endl;
        isOneDVar = true;
      } else {
        VsLog::debugLog() << methodSig << "Var is not 1-D." << endl;
        isOneDVar = false;
      }

      // Number of component of the var
      size_t numComps = vMeta->getNumComps();

      if (isOneDVar) {
        if (numComps > 1) {
          //go through list of components
          //generate a name for each one
          //then add to VisIt registry
          for (size_t i = 0; i < numComps; ++i) {
            //First we look for a match in the component registry
            //using var name and component index to search
            string componentName = registry->getComponentName(*it, i);
  
            if (!componentName.empty()) {
              VsLog::debugLog() << methodSig
                                << "Adding curve component "
                                << componentName << "." << endl;
              avtCurveMetaData* cmd =
                new avtCurveMetaData(componentName.c_str());
              cmd->hasDataExtents = false;
              md->Add(cmd);
            } else {
              VsLog::debugLog() << methodSig
                << "Unable to find match for curve variable in component registry." << endl;
            }
          }
        } else {
          //When there is only one component, we don't create a component name
          //Instead, we just use the straight-up name
          avtCurveMetaData* cmd = new avtCurveMetaData(*it);
          cmd->hasDataExtents = false;
          md->Add(cmd);
        }
      }
      else if (numComps > 1) {
        //go through list of components
        //generate a name for each one
        //then add to VisIt registry
        for (size_t i = 0; i < numComps; ++i) {
          //First we look for a match in the component registry
          //using var name and component index to search
          string componentName = registry->getComponentName(*it, i);

          if (!componentName.empty()) {
            VsLog::debugLog() << methodSig
                              << "Adding variable component "
                              << componentName << "." << endl;
            avtScalarMetaData* smd =
              new avtScalarMetaData(componentName, mesh.c_str(), centering);
            smd->hasUnits = false;
            md->Add(smd);
          } else {
            VsLog::debugLog() << methodSig << "Unable to find match for variable "
                    << *it << "in component (" << i << ") registry." << endl;
          }
        }
      }
      else if (numComps == 1) {
        VsLog::debugLog() << methodSig
                          << "Adding single-component variable." << endl;
        avtScalarMetaData* smd =
          new avtScalarMetaData(*it, mesh.c_str(), centering);
        smd->hasUnits = false;
        md->Add(smd);
      }
      else {
        VsLog::debugLog() << methodSig
                          << "Variable '" << *it
                          << "' has no components. Not being added." << endl;
      }
    }

    VsLog::debugLog() << methodSig << "Exiting normally." << endl;
}


// *****************************************************************************
//  Method: avtImageVileFormat::RegisterMeshes
//
//  Purpose:
//      How do you do the voododo that you do
//
//  Programmer: Marc Durant
//  Creation:   June, 2010
//
//  Modifications:
//

void avtVsFileFormat::RegisterMeshes(avtDatabaseMetaData* md)
{
    stringstream sstr;
    sstr << "avtVsFileFormat::RegisterMeshes() - ";
    string methodSig = sstr.str();
    VsLog::debugLog() << methodSig << "Entering function." << endl;
    LoadData();

    // Number of mesh dims
    int mdims;

    // All meshes names
    vector<string> names;
    registry->getAllMeshNames(names);
    if (names.empty()) {
      VsLog::debugLog() << methodSig
        << "WARNING: no meshes were found in this file. Returning." << endl;
      return;
    } else {
      VsLog::debugLog() << methodSig
                        << "Found " << names.size() << " meshes." << endl;
    }

    vector<string>::const_iterator it;
    for (it = names.begin(); it != names.end(); ++it) {
      VsMesh* meta = registry->getMesh(*it);
      mdims = meta->getNumSpatialDims();

      //if this mesh is 1-D, we leave it for later (curves)
      if (mdims == 1) {
        VsLog::debugLog() << methodSig
          << "Found 1-d mesh.  Skipping for now, will be added as a curve."
          << endl;
        continue;
      } else if ((mdims <= 0) || (mdims > 3)) {
        VsLog::errorLog() << methodSig
                          << "NumSpatialDims is out of range: " << mdims
                          << endl;
        VsLog::errorLog() << methodSig << "Skipping mesh." << endl;
        continue;
      }
      
      //If this mesh is part of an MD mesh, we don't register it separately
      VsMesh* testForMD = registry->findSubordinateMDMesh(*it);
      if (testForMD) {
        VsLog::debugLog() << methodSig
          << "Mesh is part of an MD mesh, will not be registered separately."
          << endl;
        continue;
      }
      
      VsLog::debugLog() << methodSig << "Found mesh '"
      << *it << "' of kind '" << meta->getKind() << "'." << endl;

      // Uniform Mesh
      if (meta->isUniformMesh()) {
        // 09.06.01 Marc Durant We used to report uniform cartesian
        // meshes as being 3d no matter what I have changed this to
        // use the correct mesh dimensions The changed plugin passes
        // vstests, and is motivated because the VisIt Lineout
        // operator requires 2-d data.  EXCEPT! then we can't plot 3-d
        // vectors on the 2-d data, so for now we continue to report 3
        // mdims = dims.size();
        VsLog::debugLog() << methodSig
                          << "Mesh's dimension = " << mdims << endl;
        if (mdims != 3) {
          VsLog::debugLog() << methodSig
            << "But reporting as dimension 3 to side-step VisIt bug." << endl;
          mdims = 3;
        }
        
        VsLog::debugLog() << methodSig
                          << "Adding uniform mesh " << *it << "." << endl;
        // Add in the logical bounds of the mesh.
        vector<int> dims;
        static_cast<VsUniformMesh*>(meta)->getNumMeshDims(dims);
        int bounds[3] = {1,1,1};
         for( int i=0; i<dims.size(); ++i )
           bounds[i] = dims[i] - 1; // Subtract 1 because the logical
                                    // bounds are zone centric.

        avtMeshMetaData* vmd = new avtMeshMetaData(bounds, 0, it->c_str(),
            1, 1, 1, 0, mdims, mdims, AVT_RECTILINEAR_MESH);
        setAxisLabels(vmd);
        md->Add(vmd);
        VsLog::debugLog() << methodSig
                          << "Succeeded in adding mesh " << *it << "." << endl;
      }

      // Rectilinear Mesh
      else if (meta->isRectilinearMesh()) {
        VsLog::debugLog() << methodSig
                          << "Adding rectilinear mesh" << *it << "." << endl;
        VsLog::debugLog() << methodSig << "MDims = " << mdims << "." << endl;

        // Add in the logical bounds of the mesh.
        vector<int> dims;
        meta->getMeshDataDims(dims);
        static_cast<VsRectilinearMesh*>(meta)->getNumMeshDims(dims);
        int bounds[3] = {1,1,1};
        for( int i=0; i<dims.size(); ++i )
          bounds[i] = dims[i] - 1; // Subtract 1 because the logical
                                   // bounds are zone centric.

        avtMeshMetaData* vmd = new avtMeshMetaData(bounds, 0, it->c_str(),
            1, 1, 1, 0, mdims, mdims, AVT_RECTILINEAR_MESH);
        setAxisLabels(vmd);
        md->Add(vmd);
      }

      // Structured Mesh
      else if (meta->isStructuredMesh()) {
        VsLog::debugLog() << methodSig
                          << "Adding structured mesh " << *it << "." << endl;

        // Add in the logical bounds of the mesh.
        vector<int> dims;
        meta->getMeshDataDims(dims);
        static_cast<VsStructuredMesh*>(meta)->getNumMeshDims(dims);
        int bounds[3] = {1,1,1};
        for( int i=0; i<dims.size(); ++i )
          bounds[i] = dims[i] - 1; // Subtract 1 because the logical
                                   // bounds are zone centric.

        avtMeshMetaData* vmd = new avtMeshMetaData(bounds, 0, it->c_str(),
            1, 1, 1, 0, mdims, mdims, AVT_CURVILINEAR_MESH);
        setAxisLabels(vmd);
        md->Add(vmd);
      }

      // Unstructured Mesh
      else if (meta->isUnstructuredMesh()) {
        //Unstructured meshes without connectivity data are registered
        //as point meshes
        VsUnstructuredMesh* unstructuredMesh = (VsUnstructuredMesh*)meta;
        if (unstructuredMesh->isPointMesh()) {
          VsLog::debugLog() << methodSig
                            << "Registering mesh " << it->c_str()
                            << " as AVT_POINT_MESH" << endl;

          // Add in the logical bounds of the mesh which is the number
          // of nodes.
          int numNodes =  unstructuredMesh->getNumPoints();
          int bounds[3] = {numNodes,0,0};

          avtMeshMetaData* vmd = new avtMeshMetaData(bounds, 0, it->c_str(),
              1, 1, 1, 0, mdims, 0, AVT_POINT_MESH);
          setAxisLabels(vmd);
          md->Add(vmd);
        }
        else {
          VsLog::debugLog() << methodSig
                            << "Registering mesh " << it->c_str()
                            << " as AVT_UNSTRUCTURED_MESH" << endl;
          mdims = meta->getNumSpatialDims();
          VsLog::debugLog() << methodSig
                            << "Adding unstructured mesh " << *it
                            << " with " << mdims
                            << " spatial dimensions." << endl;

          // Add in the logical bounds of the mesh which is the number
          // of cells.
          int numCells =  unstructuredMesh->getNumCells();
          int bounds[3] = {numCells,0,0};

          avtMeshMetaData* vmd = new avtMeshMetaData(bounds, 0, it->c_str(),
              1, 1, 1, 0, mdims, mdims, AVT_UNSTRUCTURED_MESH);
          setAxisLabels(vmd);
          md->Add(vmd);
        }
      }
      else {
        VsLog::debugLog() << methodSig
                          << "Unrecognized mesh kind: " << meta->getKind()
                          << "." << endl;
      }
    }
    VsLog::debugLog() << methodSig << "Exiting normally." << endl;
}


// *****************************************************************************
//  Method: avtImageVileFormat::RegisterMdVars
//
//  Purpose:
//      How do you do the voododo that you do
//
//  Programmer: Marc Durant
//  Creation:   June, 2010
//
//  Modifications:
//

void avtVsFileFormat::RegisterMdVars(avtDatabaseMetaData* md)
{
    stringstream sstr;
    sstr << "avtVsFileFormat::RegisterMdVars() - ";
    string methodSig = sstr.str();
    VsLog::debugLog() << methodSig << "Entering function." << endl;
    LoadData();

    // Get vars names
    vector<string> names;
    registry->getAllMDVariableNames(names);
    
    if (names.empty()) {
      VsLog::debugLog() << methodSig
        << "WARNING: No MD variables were found in this file.  Returning."
        << endl;
      return;
    } else {
      VsLog::debugLog() << methodSig
                        << "Found " << names.size()
                        << " MD variables in this file." << endl;
    }

    vector<string>::const_iterator it;
    for (it = names.begin(); it != names.end(); ++it) {
      VsLog::debugLog() << methodSig << "Processing md var '"
      << *it << "'." << endl;
      VsMDVariable* vMeta = registry->getMDVariable(*it);
      
      // Name of the mesh of the var
      string mesh = vMeta->getMesh();
      string vscentering = vMeta->getCentering();
      VsLog::debugLog() << methodSig
                        << "MD var lives on mesh " << mesh << "." << endl;

      //TODO: Mesh should either exist in an mdMesh, or separately in
      //the list of meshes

      // Centering of the variable
      // Default is node centered data
      avtCentering centering = AVT_NODECENT;
      if (vMeta->isZonal()) {
        VsLog::debugLog() << methodSig << "Var is zonal" << endl;
        centering = AVT_ZONECENT;
      } else {
        VsLog::debugLog() << methodSig << "Var is nodal" << endl;
      }

      // Number of component of the var
      size_t numComps = vMeta->getNumComps();

      VsLog::debugLog() << methodSig << "Variable has " << numComps
              << " components." << endl;

      if (numComps > 1) {
        for (size_t i = 0; i<numComps; ++i) {
          //first, get a unique name for this component
          string compName = registry->getComponentName(*it, i);
          
          if (!compName.empty()) {
            VsLog::debugLog() << methodSig
                              << "Adding variable component " << compName
                              << "." << endl;
            avtScalarMetaData* smd = new avtScalarMetaData(compName.c_str(),
              mesh.c_str(), centering);
            smd->hasUnits = false;
            md->Add(smd);
          } else {
            VsLog::debugLog() << methodSig
                              << "Unable to find component name for var "
                              << *it << " and index " << i << endl;
          }
        }
      }
      else if (numComps == 1) {
        VsLog::debugLog() << methodSig
                          << "Adding single variable component " << *it
                          << "." << endl;
        avtScalarMetaData* smd =
          new avtScalarMetaData(*it, mesh.c_str(), centering);
        smd->hasUnits = false;
        md->Add(smd);
      } else {
        VsLog::debugLog() << methodSig
                          << "Variable '" << *it
                          << "' has no components. Not being added." << endl;
      }
    }
    VsLog::debugLog() << methodSig << "Exiting normally." << endl;
}


// *****************************************************************************
//  Method: avtImageVileFormat::RegisterMdMeshes
//
//  Purpose:
//      How do you do the voododo that you do
//
//  Programmer: Marc Durant
//  Creation:   June, 2010
//
//  Modifications:
//

void avtVsFileFormat::RegisterMdMeshes(avtDatabaseMetaData* md)
{
    stringstream sstr;
    sstr << "avtVsFileFormat::RegisterMdMeshes() - ";
    string methodSig = sstr.str();
    VsLog::debugLog() << methodSig << "Entering function." << endl;
    LoadData();

    vector<string> names;
    registry->getAllMDMeshNames(names);
    if (names.empty()) {
      VsLog::debugLog() << methodSig
        << "WARNING: no md meshes were found in this file. Returning" << endl;
      return;
    } else {
      VsLog::debugLog() << methodSig
                        << "Found " << names.size()
                        << " MD meshes in this file." << endl;
    }

    vector<string>::const_iterator it;
    for (it = names.begin(); it != names.end(); ++it) {
      VsLog::debugLog() << methodSig << " Adding md mesh '"
      << *it << "'." << endl;

      VsMDMesh* meta = registry->getMDMesh(*it);
      if (!meta) {
        VsLog::debugLog() << methodSig << "Unable to find mesh " << *it << endl;
        continue;
      }

      avtMeshType meshType;
      string kind = meta->getMeshKind();
      if (meta->isUniformMesh()) {
        VsLog::debugLog() << methodSig << "Mesh is rectilinear" << endl;
        meshType = AVT_RECTILINEAR_MESH;
      } else if (meta->isUnstructuredMesh()) {
        VsLog::debugLog() << methodSig << "Mesh is unstructured" << endl;
        meshType = AVT_UNSTRUCTURED_MESH;
      } else if (meta->isStructuredMesh()) {
        VsLog::debugLog() << methodSig << "Mesh is structured" << endl;
        meshType = AVT_CURVILINEAR_MESH;
      }

      VsLog::debugLog() << methodSig
                        << "Mesh has dimension " << meta->getNumSpatialDims()
                        << "." << endl;

      avtMeshMetaData* vmd =
        new avtMeshMetaData(it->c_str(), meta->getNumBlocks(), 1, 1, 0,
                            meta->getNumSpatialDims(),
                            meta->getNumSpatialDims(), meshType);
      setAxisLabels(vmd);
      md->Add(vmd);
    }

    VsLog::debugLog() << methodSig << "Exiting normally." << endl;
}


// *****************************************************************************
//  Method: avtImageVileFormat::RegisterVarsWithMesh
//
//  Purpose:
//      Register both the mesh and the associated variables
//
//  Programmer: Marc Durant
//  Creation:   June, 2010
//
//  Modifications:
//

void avtVsFileFormat::RegisterVarsWithMesh(avtDatabaseMetaData* md)
{
    stringstream sstr;
    sstr << "avtVsFileFormat::RegisterVarsWithMesh() - ";
    string methodSig = sstr.str();
    VsLog::debugLog() << methodSig << "Entering function." << endl;
    LoadData();

    vector<string> names;
    registry->getAllVariableWithMeshNames(names);
    
    if (names.empty()) {
      VsLog::debugLog()
        << methodSig
        << "WARNING: no variables with mesh were found in this file. Returning."
        << endl;
      return;
    } else {
      VsLog::debugLog() << methodSig
                        << "Found " << names.size()
                        << " variables with mesh in this file." << endl;
    }

    vector<string>::const_iterator it;
    for (it = names.begin(); it != names.end(); ++it) {
      VsLog::debugLog() << methodSig << "Processing varWithMesh '"
                        << *it << "'." << endl;
      VsVariableWithMesh* vMeta = registry->getVariableWithMesh(*it);

      // add var components
      vector<int> dims;
      vMeta->getMeshDataDims(dims);
      if (dims.size() <= 0) {
        string msg = methodSig +
          "could not get dimensions of variable with mesh.";
        VsLog::debugLog() << msg << endl;
        throw out_of_range(msg.c_str());
      }

      size_t lastDim = 0;
      if (vMeta->isCompMinor())
        lastDim = dims[dims.size()-1];
      else
        lastDim = dims[0];

      if (lastDim < vMeta->getNumSpatialDims()) {
        VsLog::debugLog() << methodSig
                          << "Error: for variable with mesh '" << *it
                          << "', numSpatialDims = "
                          << vMeta->getNumSpatialDims()
                          << " must be larger then the last dimension, "
                          << "lastDim = " << lastDim << "." << endl;
        //JRC: Remove the associated mesh from the system!  Actually,
        //the associated mesh doesn't get added until the end of this
        //method So there is no associated mesh to remove We just drop
        //through the loop and start on the next varWithMesh
        continue;
      }

      // SS: we need all components so going to lastDim here.
      //    for (size_t i = 0; i < lastDim-vm->numSpatialDims; ++i) {
      for (size_t i = 0; i < lastDim; ++i) {
        //first, get a unique name for this component
        string compName = registry->getComponentName(*it, i);
        
        if (!compName.empty()) {
          //register with VisIt
          VsLog::debugLog() << methodSig
                            << "Adding variable component " << compName
                            << "." << endl;
          avtScalarMetaData* smd = new avtScalarMetaData(compName.c_str(),
            it->c_str(), AVT_NODECENT);
          smd->hasUnits = false;
          md->Add(smd);
        } else {
          VsLog::debugLog() << methodSig
                            << "Unable to get component name for variable "
                            << *it << " and index " << i << endl;
        }
      }

      // add var mesh
      // Add in the logical bounds of the mesh.
      int numNodes =  vMeta->getNumPoints();
      int bounds[3] = {numNodes,0,0};
      VsLog::debugLog() << methodSig
                        << "Adding point mesh for this variable." << endl;
      avtMeshMetaData* vmd = new avtMeshMetaData(bounds, 0, it->c_str(),
          1, 1, 1, 0, vMeta->getNumSpatialDims(), 0, AVT_POINT_MESH);
      setAxisLabels(vmd);
      md->Add(vmd);
    }

    VsLog::debugLog() << methodSig << "Exiting normally." << endl;
  }

  /**
   * Called to alert the database reader that VisIt is about to ask for data
   * at this timestep.  Since this reader is single-time, this is 
   * only provided for reference and does nothing.
   */
// *****************************************************************************
//  Method: avtImageVileFormat::RegisterVarsWithMesh
//
//  Purpose:
//      How do you do the voododo that you do
//
//  Programmer: Marc Durant
//  Creation:   June, 2010
//
//  Modifications:
//

void avtVsFileFormat::ActivateTimestep()
{
  LoadData();
}
  

// *****************************************************************************
//  Method: avtImageVileFormat::RegisterVarsWithMesh
//
//  Purpose:
//      How do you do the voododo that you do
//
//  Programmer: Marc Durant
//  Creation:   June, 2010
//
//  Modifications:
//

void avtVsFileFormat::UpdateCyclesAndTimes(avtDatabaseMetaData* md)
{
    if (!md) {
      VsLog::debugLog() << "avtVsFileFormat::UpdateCyclesAndTimes() - "
                        << "md was NULL, returning." <<endl;
      return;
    }

    LoadData();
    
    // If we have time data, tell VisIt
    if (registry->hasTime()) {
      VsLog::debugLog()
        << "avtVsFileFormat::UpdateCyclesAndTimes() - This file supplies time: "
        << registry->getTime() << endl;
      doubleVector times;
      times.push_back(registry->getTime());
      md->SetTimes(times);
      md->SetTimeIsAccurate(true, registry->getTime());
    }
    
    //If we have step data, tell VisIt
    if (registry->hasStep()) {
      VsLog::debugLog()
        << "avtVsFileFormat::UpdateCyclesAndTimes() - This file supplies step: "
        << registry->getStep() << endl;
      intVector cycles;
      cycles.push_back(registry->getStep());
      md->SetCycles(cycles);
      md->SetCycleIsAccurate(true, registry->getStep());
    }
}
  
  
// *****************************************************************************
//  Method: avtImageVileFormat::LoadData
//
//  Purpose:
//      How do you do the voododo that you do
//
//  Programmer: Marc Durant
//  Creation:   June, 2010
//
//  Modifications:
//

void avtVsFileFormat::LoadData()
{
    if (reader)
      return;

    string methodSig("avtVsFileFormat::LoadData() - ");

    VsLog::debugLog() << methodSig << "loading data for file "
                      << dataFileName << endl;

    //Actually open the file & read metadata for the first time
    VsLog::debugLog() << methodSig << "Initializing VsH5Reader()" << endl;

    try {
      reader = new VsH5Reader(dataFileName, registry);
    }
    catch (invalid_argument& ex) {
      string msg = methodSig + " Error initializing VsH5Reader: " + ex.what();
      VsLog::debugLog() << msg << endl;
      EXCEPTION1(InvalidDBTypeException, msg.c_str());
    }

    VsLog::debugLog() << "avtVsFileFormat::LoadData() - returning." << endl;
}


// *****************************************************************************
//  Method: avtImageVileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      How do you do the voododo that you do
//
//  Programmer: Marc Durant
//  Creation:   June, 2010
//
//  Modifications:
//

void avtVsFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData* md)
{
    string methodSig("avtVsFileFormat::PopulateDatabaseMetaData() - ");
    VsLog::debugLog() << methodSig << "Entering function." << endl;
    LoadData();

    // Tell visit that we can split meshes into subparts when running
    // in parallel

    // NOTE that we can't decompose domains if we have MD meshes
    // So it's one or the other
    vector<string> names;
#ifdef VIZSCHEMA_DECOMPOSE_DOMAINS
    VsLog::debugLog() << methodSig
                      << "Decompose_domains is defined.  Entering code block."
                      << endl;
    if (registry->numMDMeshes() > 0) {
      VsLog::debugLog() << methodSig
                        << "MD meshes are present in the data file.  "
                        << "Domain Decomposition is turned off." << endl;
      md->SetFormatCanDoDomainDecomposition(false);
    } else {
      VsLog::debugLog() << methodSig
                        << "NO MD meshes are present in the data file.  "
                        << "Domain Decomposition is turned on." << endl;
      md->SetFormatCanDoDomainDecomposition(true);
    }
    VsLog::debugLog() << methodSig
                      << "Decompose_domains is defined.  Exiting code block."
                      << endl;
#endif

    RegisterMeshes(md);
    RegisterMdMeshes(md);

    RegisterVarsWithMesh(md);

    RegisterVars(md);
    RegisterMdVars(md);

    RegisterExpressions(md);

    //add desperation last-ditch mesh if none exist in metadata
    if (md->GetNumMeshes() == 0 && md->GetNumCurves() == 0) {
      VsLog::debugLog()
        << methodSig
        << "Warning: " << dataFileName
        << " contains no mesh information. Creating default mesh." << endl;
      avtMeshMetaData* mmd =
        new avtMeshMetaData("ERROR_READING_FILE", 1, 1, 1, 0, 3, 3,
          AVT_RECTILINEAR_MESH);
      setAxisLabels(mmd);
      md->Add(mmd);
    }

    UpdateCyclesAndTimes(md);
    
    VsLog::debugLog() << methodSig << "Exiting normally." << endl;
}


// *****************************************************************************
//  Method: avtImageVileFormat::setAxisLabels
//
//  Purpose:
//      How do you do the voododo that you do
//
//  Programmer: Marc Durant
//  Creation:   June, 2010
//
//  Modifications:
//

void avtVsFileFormat::setAxisLabels(avtMeshMetaData* mmd)
{
    VsLog::debugLog()
      << "avtVsFileFormat::setAxisLabels() - entering." << endl;

    if (mmd == NULL) {
      VsLog::debugLog()
        << "avtVsFileFormat::setAxisLabels() - Input pointer was NULL?" << endl;
      return;
    } else {
      VsMesh* mesh = registry->getMesh(mmd->name);
      if (!mesh) {
        //default to x, y, z
        mmd->xLabel = "x";
        mmd->yLabel = "y";
        mmd->zLabel = "z";
      } else {
        mmd->xLabel = mesh->getAxisLabel(0);
        mmd->yLabel = mesh->getAxisLabel(1);
        mmd->zLabel = mesh->getAxisLabel(2);
      }
    }
    VsLog::debugLog() << "avtVsFileFormat::setAxisLabels() - exiting." << endl;
}

  bool avtVsFileFormat::ReturnsValidCycle() {
    VsLog::debugLog() <<"avtVsFileFormat::ReturnsValidCycle()  - entering" <<endl;
    LoadData();

    if (registry->hasStep()) {
      VsLog::debugLog() <<"avtVsFileFormat::ReturnsValidCycle()  - returning TRUE." <<endl;
      return true;
    }
    
    VsLog::debugLog() <<"avtVsFileFormat::ReturnsValidCycle()  - returning FALSE." <<endl;
    return false;
  }

  int avtVsFileFormat::GetCycle() {
    VsLog::debugLog() <<"avtVsFileFormat::GetCycle()  - entering" <<endl;
    LoadData();

    if (registry->hasStep()) {
      VsLog::debugLog() <<"avtVsFileFormat::GetCycle() - This file supplies cycle: " <<registry->getStep() <<std::endl;
      return registry->getStep();
    }

    VsLog::debugLog() <<"avtVsFileFormat::GetCycle() - This file does not supply cycle.  Returning INVALID_CYCLE." <<std::endl;
    return INVALID_CYCLE;
  }

  bool avtVsFileFormat::ReturnsValidTime() {
    VsLog::debugLog() <<"avtVsFileFormat::ReturnsValidTime()  - entering" <<endl;
    LoadData();

    if (registry->hasTime()) {
      VsLog::debugLog() <<"avtVsFileFormat::ReturnsValidTime()  - returning TRUE." <<endl;
      return true;
    }

    VsLog::debugLog() <<"avtVsFileFormat::ReturnsValidTime()  - returning FALSE." <<endl;
    return false;
  }

  double avtVsFileFormat::GetTime() {
    VsLog::debugLog() <<"avtVsFileFormat::GetTime()  - entering" <<endl;
    LoadData();

    if (registry->hasTime()) {
      VsLog::debugLog() <<"avtVsFileFormat::GetTime() - This file supplies time: " <<registry->getTime() <<std::endl;
      return registry->getTime();
    }

    VsLog::debugLog() <<"avtVsFileFormat::GetTime() - This file does not supply time.  Returning INVALID_TIME." <<std::endl;
    return INVALID_TIME;
  }
#endif
