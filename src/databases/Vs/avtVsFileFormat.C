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

#include <DBOptionsAttributes.h>
#include <avtLogicalSelection.h>
//#include <avtSpatialBoxSelection.h>

// definition of VISIT_VERSION
#include <visit-config.h>

#include <avtVsFileFormat.h>
#include <avtVsOptions.h>

//#define PARALLEL 1
//#define VIZSCHEMA_DECOMPOSE_DOMAINS 1

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

#define __CLASS__ "avtVsFileFormat::"


int avtVsFileFormat::instanceCounter = 0;

// *****************************************************************************
//  Method: avtVsFileFormat::avtVsFileFormat
//
//  Purpose:
//      Reads in the image.
//
//  Programmer: Marc Durant
//  Creation:   June, 2010
//
//  Modifications:
//
avtVsFileFormat::avtVsFileFormat(const char* filename,
                                 DBOptionsAttributes *readOpts) :
  avtSTMDFileFormat(&filename, 1), dataFileName(filename),
  processDataSelections(false), haveReadWholeData(true)
{
    instanceCounter++;
  
    VsLog::initialize(DebugStream::Stream3(),
                      DebugStream::Stream4(),
                      DebugStream::Stream5());

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "entering" << std::endl;
    

    if (readOpts != NULL) {
      for (int i=0; i<readOpts->GetNumberOfOptions(); ++i) {
        if (readOpts->GetName(i) == "Process Data Selections in the Reader")
          processDataSelections =
            readOpts->GetBool("Process Data Selections in the Reader");
      }
    }
    
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "VizSchema reader will "
                      << (processDataSelections? "" : "not ")
                      << "process data selections" << std::endl;

    //reader starts off empty
    reader = NULL;
    
    //Initialize the registry for objects
    registry = new VsRegistry();
    curveNames.clear();

//  LoadData();

    //check types
    if (isFloatType(H5T_NATIVE_FLOAT)) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "float type checks out ok." << std::endl;
    } else {
      std::ostringstream msg;
      msg << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
          << "ERROR - H5T_NATIVE_FLOAT not recognized as a float.";
      VsLog::debugLog() << msg.str() << std::endl;
      EXCEPTION1(InvalidDBTypeException, msg.str().c_str());
    }
    
    //check types
    if (isDoubleType(H5T_NATIVE_DOUBLE)) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "double type checks out ok." << std::endl;
    } else {
      std::ostringstream msg;
      msg << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
          << "ERROR - H5T_NATIVE_DOUBLE not recognized as a double.";
      VsLog::debugLog() << msg.str() << std::endl;
      EXCEPTION1(InvalidDBTypeException, msg.str().c_str());
    }

    herr_t err = H5check();

    if (err < 0) {
      std::ostringstream msg;
      msg << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
          << "HDF5 version mismatch.  Vs reader built with "
          << H5_VERS_INFO << ".";
      VsLog::debugLog() << msg.str() << std::endl;

      EXCEPTION1(InvalidDBTypeException, msg.str().c_str());
    }

    //NOTE: We used to initialize the VsH5Reader object here
    //But now do it on demand in 'populateDatabaseMetaData'
    //To minimize I/O

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "exiting" << std::endl;
}


// *****************************************************************************
//  Method: avtVsFileFormat::~avtVsFileFormat
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
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "entering" << std::endl;

    if (reader != NULL) {
      delete reader;
      reader = NULL;
    }

    if (registry) {
      delete registry;
      registry = NULL;
    }
    
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "exiting" << std::endl;
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
    // If processing the selections turn caching off.
    return !processDataSelections;

//    return haveReadWholeData;
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
avtVsFileFormat::RegisterDataSelections(const std::vector<avtDataSelection_p> &sels,
                                        std::vector<bool> *selectionsApplied)
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

    if( !processDataSelections )
      return retval;

    avtLogicalSelection composedSel;

    for (int i = 0; i < selList.size(); i++)
    {
        if (std::string(selList[i]->GetType()) == "Logical Data Selection")
        {
            avtLogicalSelection *sel = (avtLogicalSelection *) *(selList[i]);

            // overrwrite method-scope arrays with the new indexing
            composedSel.Compose(*sel);
            (*selsApplied)[i] = true;
            retval = true;
        }

        // Cannot handle avtSpatialBoxSelection without knowing the mesh.

//         else if (std::string(selList[i]->GetType()) == "Spatial Box Data Selection")
//         {
//             avtSpatialBoxSelection *sel =
//               (avtSpatialBoxSelection *) *(Sellist[i]);

//             double dmins[3], dmaxs[3];
//             sel->GetMins(dmins);
//             sel->GetMaxs(dmaxs);
//             avtSpatialBoxSelection::InclusionMode imode =
//                 sel->GetInclusionMode();

//             // we won't handle clipping of zones here
//             if ((imode != avtSpatialBoxSelection::Whole) &&
//                 (imode != avtSpatialBoxSelection::Partial))
//             {
//                 (*selsApplied)[i] = false;
//                 continue;
//             }

//             int imins[3], imaxs[3];
//             for (int j = 0; j < 3; j++)
//             {
//                 int imin = (int) dmins[j];
//                 if (((double) imin < dmins[j]) &&
//                     (imode == avtSpatialBoxSelection::Whole))
//                     imin++;
                
//                 int imax = (int) dmaxs[j];
//                 if (((double) imax < dmaxs[j]) &&
//                     (imode == avtSpatialBoxSelection::Partial))
//                     imax++;

//                 imins[j] = imin;
//                 imaxs[j] = imax;
//             }

//             avtLogicalSelection newSel;
//             newSel.SetStarts(imins);
//             newSel.SetStops(imaxs);

//             composedSel.Compose(newSel);
//             (*selsApplied)[i] = true;
//             retval = true;
//         }
        else
        {
            // indicate we won't handle this selection
            (*selsApplied)[i] = false;
        }
    }

    composedSel.GetStarts(mins);
    composedSel.GetStops(maxs);
    composedSel.GetStrides(strides);

    // If the user is a dumb ass and selects a dimention lower than
    // the actual dimension the min, max, and stride will be zero. So
    // fix it to be the full bounds and a stride of 1.
    for (int i = 0; i < 3; i++)
    {
      if( strides[i] == 0 )
      {
        mins[i] = 0;
        maxs[i] = -1;
        strides[i] = 1;
      }
    }

    return retval;
}


// *****************************************************************************
//  Method: avtVsFileFormat::GetMesh
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
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Entering function." << std::endl;
    LoadData();

    // Save the name in a temporary variable as it gets mucked with if
    // searching for a MD mesh.
    std::string meshName = name;

    bool haveDataSelections;
    int mins[3], maxs[3], strides[3];

    // Adjust for the data selections which are NODAL.
    if( haveDataSelections = ProcessDataSelections(mins, maxs, strides) )
    {
      VsLog::debugLog()
        << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
        << "Have a logical nodal selection for mesh  "
        << "(" << mins[0] << "," << maxs[0] << " stride " << strides[0] << ") "
        << "(" << mins[1] << "," << maxs[1] << " stride " << strides[1] << ") "
        << "(" << mins[2] << "," << maxs[2] << " stride " << strides[2] << ") "
        << std::endl;

      haveReadWholeData = false;
    }
    else
      haveReadWholeData = true;

    // The MD system works by filtering the requests directed to it
    // into the name of the appropriate subordinate mesh.  For
    // example, in facets_core-edge-explicit we have three meshes
    // joined into one: MdMesh = {coreMesh, solMesh, privMesh} So if
    // we get a request for (MdMesh, 0), we change the name to
    // coreMesh and proceed normally

    // Check for MD mesh
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Looking for MD mesh with name "
                      << meshName << std::endl;

    VsMDMesh* mdMeshMeta = registry->getMDMesh(meshName);
    VsMesh* meta = NULL;

    // Found an MD mesh with this name, try to load the mesh data from it
    if (mdMeshMeta != NULL) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Found MD mesh with that name."
                        << std::endl;
      meshName = mdMeshMeta->getNameForBlock(domain);

      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
        << "Request for md mesh was filtered to regular mesh: "
        << meshName << std::endl;
      meta = mdMeshMeta->getBlock(domain);
    } else {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "No MD mesh with that name."
                        << std::endl;
    }

    // Did we succeed in loading mesh data from MD mesh?
    if (meta == NULL) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Trying to find regular mesh named: "
                        << meshName << std::endl;
      meta = registry->getMesh(meshName);
    }

    if (meta != NULL) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Found mesh named: " << meshName << std::endl;

      // Uniform Mesh
      if (meta->isUniformMesh()) {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
          << "Trying to load & return uniform mesh" << std::endl;

        return getUniformMesh(static_cast<VsUniformMesh*>(meta),
                              haveDataSelections, mins, maxs, strides);
      }

      // Rectilinear Mesh
      if (meta->isRectilinearMesh()) {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Trying to load & return rectilinear mesh."
                          << std::endl;
        return getRectilinearMesh(static_cast<VsRectilinearMesh*>(meta),
                                  haveDataSelections, mins, maxs, strides);
      }
      
      // Structured Mesh
      if (meta->isStructuredMesh()) {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Trying to load & return structured mesh" << std::endl;
        return getStructuredMesh(static_cast<VsStructuredMesh*>(meta),
                                 haveDataSelections, mins, maxs, strides);
      }

#if (defined PARALLEL && defined VIZSCHEMA_DECOMPOSE_DOMAINS)
      // Don't know how to decompose any other type of mesh -> load it
      // on proc 0 only
      if (PAR_Rank() > 0) {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
          << "In parallel mode on processor " << PAR_Rank
          << " and mesh is not uniform.  "
          << "Returning NULL, mesh will be loaded on processor 0 only."
          << std::endl;
        return NULL;
      }
#endif

      // Unstructured Mesh
      if (meta->isUnstructuredMesh()) {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
          << "Trying to load & return unstructured mesh" << std::endl;

        return getUnstructuredMesh(static_cast<VsUnstructuredMesh*>(meta),
                                   haveDataSelections, mins, maxs, strides);
      }

      // At this point we don't know what kind of mesh it is.
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Mesh has unknown type: " << meta->getKind()
                        << ".  Returning NULL." << std::endl;
      return NULL;
    }

    // Variable with mesh
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "

      << "Looking for Variable With Mesh with this name." << std::endl;
    VsVariableWithMesh* vmMeta = registry->getVariableWithMesh(name);

    if (vmMeta != NULL)
    {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "

        << "Found Variable With Mesh. Loading data and returning." << std::endl;
      return getPointMesh(vmMeta,
                          haveDataSelections, mins, maxs, strides);
    }

    // Curve
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Looking for Curve with this name." << std::endl;

    vtkDataArray* foundCurve = this->GetVar(domain, name);

    if (foundCurve != NULL)
    {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Trying to load & return curve." << std::endl;

      return getCurve(domain, name);
    }

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Failed to load data for name and domain number.  "
                      << "Returning NULL." << std::endl;
    return NULL;
}


// *****************************************************************************
//  Method: avtVsFileFormat::getUniformMesh
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
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Entering function." << std::endl;
    LoadData();

    // Read int data
    std::vector<int> numCells;
    uniformMesh->getMeshDataDims(numCells); // Number of cells NOT nodes

    if (numCells.size() < 0) {
      std::ostringstream msg;
      msg << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
          << "Could not get dimensions of the uniform mesh.";
      VsLog::debugLog() << msg.str() << std::endl;
      throw std::out_of_range(msg.str().c_str());
    }

    size_t numTopologicalDims = uniformMesh->getNumTopologicalDims();

    if (numTopologicalDims > 3) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Error: numTopologicalDims of the mesh is larger than 3.  "
                        << "Returning NULL." << std::endl;
      return NULL;
    }

    int numSpatialDims = uniformMesh->getNumSpatialDims();

    if (numSpatialDims > 3) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Error: numSpatialDims data is larger than 3.  "
                        << "Returning NULL." << std::endl;
      return NULL;
    }

    // The numTopologicalDims and numSpatialDims can be different for
    // structured grids but the numSpatialDims must be great than or
    // equal to the numTopologicalDims.
    if( numTopologicalDims > numSpatialDims )
    {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Error: The numTopologicalDims ("
                        << numTopologicalDims
                        << ") is greater than numSpatialDims ("
                        << numSpatialDims << ").  "
                        << "Returning NULL." << std::endl;
      return NULL;
    }

    // VisIt downcasts to float, so go to float by default now.
    hid_t meshDataType = uniformMesh->getDataType();

    // Cache the data type because it's faster 
    bool isDouble = isDoubleType(meshDataType);
    bool isFloat = isFloatType(meshDataType);

    if (!isDouble && !isFloat)
    {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Error: hdf5 mesh data type not handled: "
                        << meshDataType << "Returning NULL." << std::endl;
      return NULL;
    }

    // startCell
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Loading optional startCells attribute." << std::endl;
    std::vector<int> startCell;
    herr_t err = uniformMesh->getStartCell(&startCell);
    if (err < 0) {
      VsLog::warningLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Uniform mesh does not have starting cell position."
                          << std::endl;
    } else {
      // Adjust the box by startCell
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Adjusting numCells by startCells." << std::endl;
      for (size_t i = 0; i < numTopologicalDims; ++i)
        numCells[i] -= startCell[i];
    }

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Determining size of point arrays." << std::endl;


    // Storage for meshes in VisIt, which is a vtkRectilinearGrid which
    // is topologically 3D so the points must also be 3D.
    size_t vsdim = 3;
    std::vector<int> gdims(vsdim);

    // Number of nodes is equal to number of cells plus one
    std::vector<int> numNodes(vsdim);
    for (size_t i = 0; i < numTopologicalDims; ++i) 
      numNodes[i] = numCells[i]+1;

    // Set unused dims to 1
    for (size_t i=numTopologicalDims; i<vsdim; ++i)
      numNodes[i] = 1;

    // Adjust for the data selections which are NODAL. If no selection
    // the bounds are set to 0 and max with a stride of 1.
    GetSelectionBounds( numTopologicalDims, numNodes, gdims,
                        mins, maxs, strides, haveDataSelections );

#if (defined PARALLEL && defined VIZSCHEMA_DECOMPOSE_DOMAINS)
    if( !GetParallelDecomp( numTopologicalDims, gdims, mins, maxs, strides ) )
      return NULL; // No work for this processor.

    haveDataSelections = 1;
#endif

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Getting bounds for mesh." << std::endl;

    std::vector<float> lowerBounds;
    uniformMesh->getLowerBounds(&lowerBounds);

    std::vector<float> upperBounds;
    uniformMesh->getUpperBounds(&upperBounds);

    // Storage for mesh points in VisIt are spatially 3D. So create 3
    // coordinate arrays and fill in zero for the others.
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Creating coordinate arrays." << std::endl;

    std::vector<vtkDataArray*> coords(vsdim);

    for (size_t i=0; i<numSpatialDims; ++i)
    {
       if( isDouble )
         coords[i] = vtkDoubleArray::New();
       else if( isFloat )
         coords[i] = vtkFloatArray::New();

      // Delta
      double delta = 0;
      if (numNodes[i] > 1)
        delta = (upperBounds[i] - lowerBounds[i]) / (numNodes[i]-1);

      int cc = 0;
      int j = mins[i];

      while( j <= maxs[i] )
      {
        double temp = lowerBounds[i] + j*delta;
        coords[i]->InsertTuple(cc, &temp);
        ++cc;
        j += strides[i];
      }

      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Dumped " << cc << " nodes to the rectilinear grid."
                        << std::endl;
    }

    // Unused axii
    for (size_t i=numSpatialDims; i<vsdim; ++i)
    {
      coords[i] = vtkFloatArray::New();
      coords[i]->SetNumberOfTuples(1);
      coords[i]->SetComponent(0, 0, 0);
    }
          
    // Create vtkRectilinearGrid
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Creating rectilinear grid." << std::endl;
    vtkRectilinearGrid* rgrid = vtkRectilinearGrid::New();
    rgrid->SetDimensions(&(gdims[0]));

    // Set grid data
    rgrid->SetXCoordinates(coords[0]);
    rgrid->SetYCoordinates(coords[1]);
    rgrid->SetZCoordinates(coords[2]);

    // Cleanup local data
    for (size_t i = 0; i<vsdim; ++i)
      coords[i]->Delete();

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Returning data." << std::endl;
    return rgrid;
}


// *****************************************************************************
//  Method: avtVsFileFormat::getRectilinearMesh
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

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Entering function." << std::endl;
    LoadData();

    // Get dimensions
    std::vector<int> numNodes;
    rectilinearMesh->getMeshDataDims(numNodes); // Number of nodes NOT cells

    if (numNodes.size() < 0) {
      std::ostringstream msg;
      msg << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
          << "Could not get dimensions of the rectilinear mesh.";
      VsLog::debugLog() << msg.str() << std::endl;
      throw std::out_of_range(msg.str().c_str());
    }

    size_t numTopologicalDims = rectilinearMesh->getNumTopologicalDims();

    if (numTopologicalDims > 3) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Error: numTopologicalDims of data is larger than 3.  "
                        << "Returning NULL." << std::endl;
      return NULL;
    }

    int numSpatialDims = rectilinearMesh->getNumSpatialDims();

    if (numSpatialDims > 3) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Error: numSpatialDims data is larger than 3.  "
                        << "Returning NULL." << std::endl;
      return NULL;
    }

    // VisIt downcasts to float, so go to float by default now.
    hid_t meshDataType = rectilinearMesh->getDataType();

    // Cache the data type because it's faster 
    bool isDouble = isDoubleType(meshDataType);
    bool isFloat = isFloatType(meshDataType);

    if (!isDouble && !isFloat)
    {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Error: hdf5 mesh data type not handled: "
                        << meshDataType << "Returning NULL." << std::endl;
      return NULL;
    }

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Determining size of point arrays." << std::endl;

    // Storage for meshes in VisIt, which is a vtkRectilinearGrid which
    // is topologically 3D so the points must also be 3D.
    size_t vsdim = 3;
    std::vector<int> gdims(vsdim);

    // Adjust for the data selections which are NODAL. If no selection
    // the bounds are set to 0 and max with a stride of 1.
    GetSelectionBounds( numTopologicalDims, numNodes, gdims,
                        mins, maxs, strides, haveDataSelections );

#if (defined PARALLEL && defined VIZSCHEMA_DECOMPOSE_DOMAINS)
    if( !GetParallelDecomp( numTopologicalDims, gdims, mins, maxs, strides ) )
      return NULL; // No work for this processor.

    haveDataSelections = 1;
#endif

    std::vector<vtkDataArray*> coords(vsdim);

    for( int i=0; i<numTopologicalDims; ++i )
    {
      void* dataPtr;
      double* dblDataPtr;
      float* fltDataPtr;

      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
        << "Loading data for axis " << i << std::endl;
      VsH5Dataset* axisData = rectilinearMesh->getAxisDataset(i);

      if (axisData == NULL) {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Axis " << i
                          << " data not found. Returning NULL." << std::endl;
        return NULL;
      }

      hid_t axisDataType = axisData->getType();

      // Read points and add in zero for any lacking dimension
      if( isDoubleType( axisDataType ) ) {
        dblDataPtr = new double[numNodes[i]];
        dataPtr = dblDataPtr;
      }
      else if( isFloatType( axisDataType ) ) {
        fltDataPtr = new float[numNodes[i]];
        dataPtr = fltDataPtr;
      } else {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Unknown axis data type: "
                          << axisDataType << std::endl;
        return NULL;
      }

      if (!dataPtr) {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Allocation failed, pointer is NULL."
                          << "Returning NULL." << std::endl;
        return NULL;
      }

      // Read in the point data. NOTE: Even when doing index selecting
      // or in parallel mode read in all of te data. As for the grid
      // there is not that much data. When building the mesh the
      // correct nodes are used.
      herr_t err = reader->getDataSet(axisData, dataPtr);

      if (err < 0) {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "GetDataSet returned error: " << err << "  "
                          << "Returning NULL." << std::endl;

        if( isDoubleType( axisDataType ) )
          delete [] dblDataPtr;
        else if( isFloatType( axisDataType ) )
          delete [] fltDataPtr;

        return NULL;
      }

      // Storage for mesh points in VisIt are spatially 3D. So create 3
      // coordinate arrays and fill in zero for the others.
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Creating coordinate arrays." << std::endl;

      // The mesh data type will the highest precision of all of the
      // axis data types. Declaring here allows the the mesh data type
      // to be either float or double.
      if( isDouble )
        coords[i] = vtkDoubleArray::New();
      else if( isFloat )
        coords[i] = vtkFloatArray::New();

      int cc = 0;

      if (isDouble) {

        int j = mins[i];

        while( j <= maxs[i] )
        {
          double temp;

          if( isDoubleType( axisDataType ) )
            temp = dblDataPtr[j];
          else if( isFloatType( axisDataType ) ) 
            temp = fltDataPtr[j];

          coords[i]->InsertTuple(cc, &temp);
          ++cc;
          j += strides[i];
        }

        if( isDoubleType( axisDataType ) )
          delete [] dblDataPtr;
        else if( isFloatType( axisDataType ) ) 
          delete [] fltDataPtr;

      } else if (isFloat) {

        int j = mins[i];

        while( j <= maxs[i] )
        {
          float temp;

          if( isDoubleType( axisDataType ) )
            temp = dblDataPtr[j];
          else if( isFloatType( axisDataType ) ) 
            temp = fltDataPtr[j];

          coords[i]->InsertTuple(cc, &temp);
          ++cc;
          j += strides[i];
        }

        if( isDoubleType( axisDataType ) )
          delete [] dblDataPtr;
        else if( isFloatType( axisDataType ) ) 
          delete [] fltDataPtr;
      }

      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Dumped " << cc << " nodes to the rectilinear grid."
                        << std::endl;
    }

    for (int i=numTopologicalDims; i<vsdim; ++i) {
      if( isDouble )
        coords[i] = vtkDoubleArray::New();
      else if( isFloat )
        coords[i] = vtkFloatArray::New();

      coords[i]->SetNumberOfTuples(1);
      coords[i]->SetComponent(0, 0, 0);    
    }

    // Create vtkRectilinearGrid
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Creating rectilinear grid." << std::endl;
    vtkRectilinearGrid* rgrid = vtkRectilinearGrid::New();
    rgrid->SetDimensions(&(gdims[0]));
  
    // Set grid data
    rgrid->SetXCoordinates(coords[0]);
    rgrid->SetYCoordinates(coords[1]);
    rgrid->SetZCoordinates(coords[2]);

    // Cleanup local data
    for (size_t i = 0; i < vsdim; ++i) {
      coords[i]->Delete();
    }

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Returning data." << std::endl;
    return rgrid;
}

  
// *****************************************************************************
//  Method: avtVsFileFormat::getStructuredMesh
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
    // TODO - make "cleanupAndReturnNull" label, and do a "go to"
    // instead of just returning NULL all the time.

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Entering function." << std::endl;
    LoadData();

    // Find points: Structured meshes are datasets, and the data is
    // the points So, look for the dataset with the same name as the
    // mesh

    VsH5Dataset* pointsDataset =
      registry->getDataset(structuredMesh->getFullName());

    if (pointsDataset == NULL) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Component '"
                        << structuredMesh->getFullName()
                        << "' not found.  Returning NULL." << std::endl;
      return NULL;
    }

    // Get dims of points array
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Determining dimension of points array." << std::endl;

    std::vector<int> numNodes;
    structuredMesh->getNumMeshDims(numNodes);  // Number of nodes NOT cells.

    if (numNodes.size() < 0) {
      std::ostringstream msg;
      msg << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
          << "Could not get dimensions of the structured mesh.";
      VsLog::debugLog() << msg.str() << std::endl;
      throw std::out_of_range(msg.str().c_str());
    }

    // The numTopologicalDims and numSpatialDims can be different for
    // structured grids.
    size_t numTopologicalDims = structuredMesh->getNumTopologicalDims();

    if (numTopologicalDims > 3) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Error: numTopologicalDims of data is larger than 3.  "
                        << "Returning NULL." << std::endl;
      return NULL;
    }

    int numSpatialDims = structuredMesh->getNumSpatialDims();

    if (numSpatialDims > 3) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Error: numSpatialDims data is larger than 3.  "
                        << "Returning NULL." << std::endl;
      return NULL;
    }

    // The numTopologicalDims and numSpatialDims can be different for
    // structured grids but the numSpatialDims must be great than or
    // equal to the numTopologicalDims.
    if( numTopologicalDims > numSpatialDims )
    {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Error: The numTopologicalDims ("
                        << numTopologicalDims
                        << ") is greater than numSpatialDims ("
                        << numSpatialDims << ").  "
                        << "Returning NULL." << std::endl;
      return NULL;
    }

    // VisIt downcasts to float, so go to float by default now.
    hid_t meshDataType = pointsDataset->getType();

    // Cache the data type because it's faster 
    bool isDouble = isDoubleType(meshDataType);
    bool isFloat = isFloatType(meshDataType);

    if (!isDouble && !isFloat) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Error: hdf5 mesh data type not handled: "
                        << meshDataType << "Returning NULL." << std::endl;
      return NULL;
    }


    // Storage for meshes in VisIt, which is a vtkRectilinearGrid which
    // is topologically 3D so the points must also be 3D.
    size_t vsdim = 3;
    std::vector<int> gdims(vsdim);

    // Adjust for the data selections which are NODAL. If no selection
    // the bounds are set to 0 and max with a stride of 1.
    GetSelectionBounds( numTopologicalDims, numNodes, gdims,
                        mins, maxs, strides, haveDataSelections );

#if (defined PARALLEL && defined VIZSCHEMA_DECOMPOSE_DOMAINS)
    if( !GetParallelDecomp( numTopologicalDims, gdims, mins, maxs, strides ) )
      return NULL; // No work for this processor.

    haveDataSelections = 1;
#endif

    int numPoints = 1;
    for (size_t i=0; i<numTopologicalDims; ++i)
      numPoints *= gdims[i];

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Total number of points is " << numPoints
                      << std::endl;

    // Read points and add in zero for any lacking dimension
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
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Allocation failed, pointer is NULL.  "
                        << "Returning NULL." << std::endl;
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
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Error " << err
                        << " while reading data.  Returning NULL." << std::endl;
      
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
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Point spatial dimensionality ("
                        << numSpatialDims << " is less than 3.  "
                        << "Moving data into correct location." << std::endl;

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

      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Data move succeeded." << std::endl;
    }

    // Create the mesh and set its dimensions, including unused to zero
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Creating the mesh." << std::endl;

    // Add the points, changing to C ordering
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Adding points to mesh." << std::endl;

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
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Adding " << numPoints
                      << " points with index order '"
                      << structuredMesh->getIndexOrder() << "'." << std::endl;

    // Using FORTRAN data ordering.
    if (structuredMesh->isFortranOrder()) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Using FORTRAN data ordering." << std::endl;

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
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Using C data ordering." << std::endl;
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
     VsLog::debugLog() << std::endl;
     }
     // end debug
     */
    vtkStructuredGrid* sgrid = vtkStructuredGrid::New();
    sgrid->SetDimensions(&(gdims[0]));
    sgrid->SetPoints(vpoints);

    // Cleanup local data
    vpoints->Delete();

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Returning data." << std::endl;
    return sgrid;
}


// *****************************************************************************
//  Method: avtVsFileFormat::getUnstructuredMesh
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
    // TODO - make "cleanupAndReturnNull" label, and do a "go to"
    // instead of just returning NULL all the time.

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Entering function." << std::endl;
    LoadData();

    // Check for points type
    hid_t meshDataType = unstructuredMesh->getDataType();

    // Cache the data type because it's faster 
    bool isDouble = isDoubleType(meshDataType);
    bool isFloat = isFloatType(meshDataType);

    if (!isDouble && !isFloat) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Error: hdf5 mesh data type not handled: "
                        << meshDataType << "Returning NULL." << std::endl;
      return NULL;
    }

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Checking dimensionality of mesh." << std::endl;

    size_t numNodes = unstructuredMesh->getNumPoints();
    size_t numSpatialDims = unstructuredMesh->getNumSpatialDims();

    // Get ready to read in points
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Setting up data structures." << std::endl;

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
        maxs[0] = numNodes - 1;          // last cell index,
                                         // not number of cells

      if( maxs[0] < mins[0] )
        mins[0] = 0;

      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
        << "Have a zonal inclusive selection for unstructured point mesh "
        << "(" << mins[0] << "," << maxs[0] << " stride " << strides[0] << ") "
        << std::endl;
      
      // New number of nodes based on the above.
      numNodes = (maxs[0]-mins[0]) / strides[0] + 1;
    }

    // Allocate
    size_t dsize = 0;
    if (isDouble) {
      vpoints->SetDataTypeToDouble();
      dsize = sizeof(double);
    }
    else if (isFloat) {
      vpoints->SetDataTypeToFloat();
      dsize = sizeof(float);
    }

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "VPoints array will have " << numNodes << " points."
                      << std::endl;
    vpoints->SetNumberOfPoints(numNodes);
    void* dataPtr = vpoints->GetVoidPointer(0);
    if (!dataPtr) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Unable to allocate the points.  Cleaning up."
                        << std::endl;
      ugridPtr->Delete();
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Returning NULL." << std::endl;
      return NULL;
    }

    // Read in the data

    // Mesh points are in separate data sets.
    if (unstructuredMesh->usesSplitPoints())
    {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Using split-points method" << std::endl;

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
          VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                            << "Call to getDataSet returned error: "
                            << err << "Returning NULL." << std::endl;
          return NULL;
        }
      }

      // The above stores the value in the correct location but make
      // sure the remaing values are all zero.
      for (int i=numSpatialDims; i<3; ++i)
      {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Zeroing data at unused positions." << std::endl;
        
        if (isDouble) {
        
          double* dblDataPtr = &(((double*) dataPtr)[i]);
        
          for (int j=0; j<numNodes; ++j)
          {
            *dblDataPtr = 0;
            dblDataPtr += 3;
          }
        }
        else if (isFloat) {
        
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
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Using all-in-one method" << std::endl;

      VsH5Dataset* pointsDataset = unstructuredMesh->getPointsDataset();

      herr_t err;
      
      // If a point mesh and with data selections read in only the
      // points needed. If any other structured mesh read in all of
      // the points because it is not know which will be used to build
      // the cells.
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
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Call to getDataSet returned error: "
                          << err << "Returning NULL." << std::endl;
        return NULL;
      }

      // If spatial dimension is less than 3, move coordinates to the
      // correct position and set the extra dimensions to zero.
      if (numSpatialDims < 3)
      {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Point spatial dimensionality ("
                          << numSpatialDims << " is less than 3.  "
                          << "Moving data into correct location." << std::endl;
        
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

        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Data move succeeded." << std::endl;
      }
    }

    // If there is no polygon data, then this mesh was registered as a
    // point mesh and we don't need to go any further
    if (unstructuredMesh->isPointMesh())
    {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Mesh was registered as a point mesh."      
                        << "Because no connectivity data was found. "
                        << "Adding vertices as single points and returning."
                        << std::endl;

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
    std::string connectivityDatasetName;

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
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "ERROR - unable to find connectivity dataset. "
                        << "Returning points data without connectivity."
                        << std::endl;

      // We used to try to convert this into a pointMesh but it never
      // worked (Perhaps because the mesh was registered
      // differently. So instead give up.

//       vtkIdType vertex;
//       for (int i=0; i<numNodes; ++i) {
//         vertex = i;
//         ugridPtr->InsertNextCell(VTK_VERTEX, 1, &vertex);
//       }

      return ugridPtr;
    }

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Found connectivity data. "
                      << "Connectivity dataset name is "
                      << connectivityDatasetName << "  "
                      << "haveConnectivityCount = "
                      << haveConnectivityCount
                      << std::endl;
    
    VsH5Dataset* connectivityDataset =
      registry->getDataset(connectivityDatasetName);
    
    std::vector<int> connectivityDims = connectivityMeta->getDims();
    
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

    // Adjust for the data selections which are ZONAL which will limit
    // which connections are read in.
    if( haveDataSelections )
    {
      if( maxs[0] < 0 || numCells - 1 < maxs[0] )
        maxs[0] = numCells - 1; // numCells - 1 = last cell index,
                                // not number of cells

      if( maxs[0] < mins[0] )
        mins[0] = 0;

      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
        << "Have a zonal inclusive selection for unstructured mesh cells "
        << "(" << mins[0] << "," << maxs[0] << " stride " << strides[0] << ") "
        << std::endl;
      
      // New number of cells based on the above.
      numCells = (maxs[0]-mins[0]) / strides[0] + 1;
    }

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "numCells = " << numCells
                      << ", numVerts = " << numVerts << "." << std::endl;

    size_t datasetLength = 1;
    for (size_t i =0; i< connectivityDims.size(); ++i)
      datasetLength *= connectivityDims[i];

    // Check for connectivity list type
    if (!isIntegerType( connectivityMeta->getType() )) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Indices are not integers. Cleaning up" << std::endl;

      ugridPtr->Delete();

      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
        << "Returning NULL" << std::endl;

      return NULL;
    }

    // Vertices
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Allocating space for " << datasetLength
                      << " integers of connectivity data." << std::endl;

    int* vertices = new int[datasetLength];
    if (!vertices) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Unable to allocate vertices.  Cleaning up." << std::endl;

      ugridPtr->Delete();

      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Returning NULL." << std::endl;
      return NULL;
    }

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Reading connectivity list data." << std::endl;

    herr_t err;

    // Here the connections are being read. So sub select if needed.
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
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Call to getDataSet returned error: " << err
                        << "Returning NULL." << std::endl;
      return NULL;
    }


    try {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Allocating " << numCells << " cells.  "
                        << "If old VTK and this fails, it will just abort."
                        << std::endl;

      ugridPtr->Allocate(numCells);
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Allocation succeeded." << std::endl;
    }
    // JRC: what goes here to detect failure to allocate?
    catch (vtkstd::bad_alloc) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Caught vtkstd::bad_alloc exception. "
                        << "Unable to allocate space for cells.  Cleaning up."
                        << "Returning NULL." << std::endl;
      delete [] vertices;
      ugridPtr->Delete();
      return NULL;
    } catch (...) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Unknown exception allocating cells.  Cleaning up."
                        << "Returning NULL." << std::endl;
      delete [] vertices;
      ugridPtr->Delete();
      return NULL;
    }

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Inserting cells into grid." << std::endl;
    size_t k = 0;
    int warningCount = 0;
    int cellCount = 0;
    unsigned int cellVerts = 0; // cell's connected node indices
    int cellType;

    // Dealing with fixed length connectivity lists.
    if( haveConnectivityCount ) {
      cellVerts = haveConnectivityCount;
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Inserting " << cellVerts
                        << " vertices into each cell." << std::endl;
    }

    for (size_t i = 0; i < numCells; ++i) {
      if (k >= datasetLength) {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "While iterating over the vertices, "
                          << "the index variable 'k' "
                          << "went beyond the end of the array. "
                          << "Existing cells will be returned "
                          << "but the loop is terminating now." << std::endl;
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
          if (numTopologicalDims == 2) cellType = VTK_POLYGON;
          else {
            if (warningCount < 30) {
              VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                                << "Error: invalid number of vertices for cell #"
                                << cellCount << ": " << cellVerts << std::endl;
            } else if (warningCount == 30) {
              VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                                << "Exceeded maximum number of errors.  "
                                << "Error messages disabled for remaining cells."
                                << std::endl;
            }
            ++warningCount;
            cellType = VTK_EMPTY_CELL;
          }
        break;
      }

      //create cell and insert into mesh
      if (cellType != VTK_EMPTY_CELL) {
        std::vector<vtkIdType> verts(cellVerts);
        for (size_t j = 0; j < cellVerts; ++j) {
          verts[j] = (vtkIdType) vertices[k++];
          if ((verts[j] < 0) || (verts[j] >= numNodes)) {
            VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
              << "ERROR in connectivity dataset - requested vertex number "
              << verts[j] << " exceeds number of vertices" << std::endl;

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
        // points won't work so we treat the entire row of the dataset
        // as a single cell Maybe something will work!
        std::vector<vtkIdType> verts(numVerts);
        k--;
        for (size_t j = 0; j < numVerts; ++j) {
          if (warningCount < 30) {
            VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                             << "WARNING: ADDING cell #"
                             << cellCount << " as cell: "
                             << vertices[k] << std::endl;
          }
          verts[j] = (vtkIdType) vertices[k++];
        }
        ugridPtr->InsertNextCell (VTK_POLYGON, numVerts, &verts[0]);
      }
    }
    
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Finished.  Cleaning up." << std::endl;
    // Done, so clean up memory and return
    delete [] vertices;

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Returning data." << std::endl;
    return ugridPtr;
}
  

// *****************************************************************************
//  Method: avtVsFileFormat::getPointMesh
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
    // TODO - make "cleanupAndReturnNull" label, and do a "go to"
    // instead of just returning NULL all the time.

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Entering function." << std::endl;
    LoadData();

    hid_t meshDataType = variableWithMesh->getType();

    // Cache the data type because it's faster 
    bool isDouble = isDoubleType(meshDataType);
    bool isFloat = isFloatType(meshDataType);

    if (!isDouble && !isFloat) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Error: hdf5 mesh data type not handled: "
                        << meshDataType << "Returning NULL." << std::endl;
      return NULL;
    }

    // Get the number of values
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Getting the number of points." << std::endl;

    size_t numSpatialDims = variableWithMesh->getNumSpatialDims();
    size_t numTopologicalDims = 1;

    std::vector<int> numNodes(numTopologicalDims);
    numNodes[0] = variableWithMesh->getNumPoints();

    // Storage for meshes in VisIt, which is a vtkRectilinearGrid which
    // is topologically 3D so the points must also be 3D.
    size_t vsdim = 1;
    std::vector<int> gdims(vsdim);

    // Adjust for the data selections which are NODAL. If no selection
    // the bounds are set to 0 and max with a stride of 1.
    GetSelectionBounds( numTopologicalDims, numNodes, gdims,
                        mins, maxs, strides, haveDataSelections, false );

#if (defined PARALLEL && defined VIZSCHEMA_DECOMPOSE_DOMAINS)
    if( !GetParallelDecomp( 1, gdims, mins, maxs, strides, 0 ) )
      return NULL; // No work for this processor.

    haveDataSelections = 1;
#endif

    // Read in points
    int numPoints = gdims[0];

    // Create the unstructured meshPtr
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Creating the vtkUnstructuredGrid." << std::endl;
    vtkUnstructuredGrid* meshPtr = vtkUnstructuredGrid::New();

    // Create and set points while small so minimal memory usage
    vtkPoints* vpts = vtkPoints::New();
    meshPtr->SetPoints(vpts);
    vpts->Delete();
    vtkPoints* vpoints = meshPtr->GetPoints();

    // Allocate
    size_t dsize = 0;
    if (isDouble) {
      vpoints->SetDataTypeToDouble();
      dsize = sizeof(double);
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Double data" << std::endl;
    }
    else if (isFloat) {
      vpoints->SetDataTypeToFloat();
      dsize = sizeof(float);
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Float data" << std::endl;
    }
    
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Allocating for " << numPoints << " values." << std::endl;
    vpoints->SetNumberOfPoints(numPoints);

    void* dataPtr = vpoints->GetVoidPointer(0);
    if (!dataPtr) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Unable to allocate the points.  Cleaning up."
                        << "Returning NULL." << std::endl;
      meshPtr->Delete();
      return NULL;
    }

    herr_t err;

    // Read in the data
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Reading data." << std::endl;

    // Kind of grim but read each of the coordinate components in
    // using the generic reader.

    // To do so set the location in memory where each cordinate
    // componenet will be stored.
    int srcMins[1] = {mins[0]};
    int srcMaxs[1] = {numPoints};
    int srcStrides[1] = {strides[0]};

    int destSize[1] = {gdims[0]*3};
    int destMins[1] = {0};
    int destMaxs[1] = {gdims[0]};
    int destStrides[1] = {3};
      
    for (unsigned int i= 0; i< numSpatialDims; i++)
    {
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
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Call to getVarWithMeshMeta returned error: "
                          << err << "Returning NULL." << std::endl;
        return NULL;
      }
    }

    // The above stores the value in the correct location but make
    // sure the remaing value are all zero.
    for (int i=numSpatialDims; i<3; ++i)
    {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Zeroing data at unused positions." << std::endl;

      if (isDouble) {
        double* dblDataPtr = &(((double*) dataPtr)[i]);
        
        for (int j=0; j<numNodes[0]; ++j)
        {
          *dblDataPtr = 0;
          dblDataPtr += 3;
        }
      }
      else if (isFloat) {        
        float* fltDataPtr = &(((float*) dataPtr)[i]);
        
        for (int j=0; j<numNodes[0]; ++j)
        {
          *fltDataPtr = 0;
          fltDataPtr += 3;
        }
      }
    }

    // create point mesh
    try {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
        << "Allocating " << numNodes[0]
        << " vertices.  If old VTK and this fails, it will just abort." << std::endl;
      meshPtr->Allocate(numNodes[0]);
    } catch (vtkstd::bad_alloc) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Caught vtkstd::bad_alloc. Unable to allocate cells."
                        << "Returning NULL." << std::endl;
      meshPtr->Delete();
      return NULL;
    } catch (...) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Unknown exception. Unable to allocate cells."
                        << "Returning NULL." << std::endl;
      meshPtr->Delete();
      return NULL;
    }

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
      << "Allocation succeeded.  Setting mesh to connectivity 'VERTEX'." << std::endl;
    vtkIdType vertex;
    for (int i=0; i<numNodes[0]; ++i) {
      vertex = i;
      meshPtr->InsertNextCell(VTK_VERTEX, 1, &vertex);
    }

    // Clean up memory
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Returning data." << std::endl;

    return meshPtr;
}


// *****************************************************************************
//  Method: avtVsFileFormat::getCurve
//
//  Purpose:
//      How do you do the voododo that you do
//
//  Programmer: Marc Durant
//  Creation:   June, 2010
//
//  Modifications:
//
vtkDataSet* avtVsFileFormat::getCurve(int domain, const std::string& requestedName)
{
    // TODO - make "cleanupAndReturnNull" label, and do a "go to"
    // instead of just returning NULL all the time.

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Entering function." << std::endl;
    LoadData();

    // Save the name in a temporary variable as it gets mucked with if
    // searching for a component.
    std::string name = requestedName;

    ///TODO: This method piggybacks on getVar and getMesh -
    ///       Could be made much more efficient
    // 1. get var
    // 2. get matching mesh
    // 3. create 2 coord arrays - x from mesh and y from var
    // 4. Combine coord arrays to form 1-d Rectilinear mesh

    // Attempts to follow visit plugin Curve2d in
    // ./visit/src/databases/Curve2d/avtCurve2DFileFormat.C

    // Retrieve var metadata and extract the mesh name
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Looking for variable metadata." << std::endl;
    VsVariable* varMeta = registry->getVariable(name);

    if (varMeta == NULL) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "No variable metadata found under name "
                        << name << "  "
                        << "Looking for information in component registry."
                        << std::endl;

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
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Found a component, which refers to variable "
                          << name << " and index " << componentIndex << std::endl;
        isAComponent = true;
      }
    }

    if (varMeta == NULL) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Failed to find variable in component list.  "
                        << "Returning NULL." << std::endl;
      return NULL;
    }

    hid_t varDataType = varMeta->getType();

    if (isDoubleType(varDataType)) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Var is 64-bit real" << std::endl;
    } else if (isFloatType(varDataType)) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Var is 32-bit real" << std::endl;
    } else if (isIntegerType(varDataType)) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Var is int" << std::endl;
    } else {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Var is unknown type (known are double, float, int).  "
                        << "Returning NULL" << std::endl;
      return NULL;
    }

    vtkDataArray* varData = NULL;
    try {
      varData = GetVar(domain, requestedName.c_str());
    } catch (...) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
        << "Caught exception from GetVar().  Returning NULL." << std::endl;
      return NULL;
    }

    if (varData == NULL) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Failed to load var data.  Returning NULL." << std::endl;
      return NULL;
    }

    // Have the variable now get the mesh.
    std::string meshName = varMeta->getMeshName();
    VsMesh* meshMeta = varMeta->getMesh();

    if (meshMeta == NULL) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "No mesh metadata found.  Returning NULL." << std::endl;
      return NULL;
    }

    vtkRectilinearGrid* meshData = NULL;
    try {
      meshData = (vtkRectilinearGrid*) GetMesh(domain, meshName.c_str());
    } catch (...) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Caught exception from GetMesh().  Returning NULL."
                        << std::endl;
      varData->Delete();
      return NULL;
    }

    if (meshData == NULL) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Failed to load mesh data.  Returning NULL." << std::endl;
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

    // Cross-reference against the number of points in the mesh
    int nPtsInOutput = nPts;
    std::vector<int> meshDims;
    meshMeta->getMeshDataDims(meshDims);
    int nPtsInMesh = meshDims[0];
    if (varMeta->isZonal()) {
      if (nPts != (nPtsInMesh - 1)) {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "ERROR - mesh and var dimensionalities don't match." <<std::endl;
        //Use the lower of the two values
        if (nPts > (nPtsInMesh - 1)) {
          nPtsInOutput = nPtsInMesh - 1;
        }
      }
    } else if (nPts != nPtsInMesh) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "ERROR - mesh and var dimensionalities don't match." <<std::endl;
      //Use the lower of the two values                                                                                           
      if (nPts > nPtsInMesh) {
        nPtsInOutput = nPtsInMesh;
      }
    }

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Variable has " << nPts << " points." << std::endl;
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Mesh has " << nPtsInMesh << " points." << std::endl;
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Output will have " << nPtsInOutput << " points." << std::endl;

    // Create 1-D RectilinearGrid
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Building Rectilinear grid." << std::endl;
    vtkFloatArray* vals = vtkFloatArray::New();
    vals->SetNumberOfComponents(1);
    vals->SetNumberOfTuples(nPtsInOutput);
    vals->SetName(name.c_str());

    vtkRectilinearGrid* rg = vtkVisItUtility::Create1DRGrid(nPtsInOutput, VTK_FLOAT);
    rg->GetPointData()->SetScalars(vals);

    vtkFloatArray* xc = vtkFloatArray::SafeDownCast(rg->GetXCoordinates());
    vtkDataArray* meshXCoord = meshData->GetXCoordinates();

    for (int i = 0; i < nPtsInOutput; i++) {
      double* var_i = varData->GetTuple(i);
      double* mesh_i = meshXCoord->GetTuple(i);
      xc->SetValue(i, mesh_i[0]);
      vals->SetValue(i, var_i[0]);
    }

    // Done, so clean up memory and return
    vals->Delete();

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Returning data." << std::endl;
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
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Entering function." << std::endl;
    LoadData();

    // Save the name in a temporary variable as it gets mucked with if
    // searching for a component.
    std::string name = requestedName;

    bool haveDataSelections;
    int mins[3], maxs[3], strides[3];

    // Adjust for the data selections which are NODAL (typical) or ZONAL.
    if( haveDataSelections = ProcessDataSelections(mins, maxs, strides) )
    {
      VsLog::debugLog()
        << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
        << "have a data selection for a variable "
        << "(" << mins[0] << "," << maxs[0] << " stride " << strides[0] << ") "
        << "(" << mins[1] << "," << maxs[1] << " stride " << strides[1] << ") "
        << "(" << mins[2] << "," << maxs[2] << " stride " << strides[2] << ") "
        << std::endl;

      haveReadWholeData = false;
    }
    else
      haveReadWholeData = true;

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
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Found a component, which refers to variable "
                        << name << " and index " << componentIndex << std::endl;
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
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Checking for possible MD var." << std::endl;
    VsMDVariable* mdMeta = registry->getMDVariable(name);
    if (mdMeta) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Found MD metadata for this name: "
                        << name << std::endl;

      if (0 <= domain && domain < mdMeta->blocks.size()) {
        meta = mdMeta->blocks[domain];
        name = meta->getFullName();

        if( meta )
          variableDataset = registry->getDataset(meta->getFullName());

      } else {
        VsLog::warningLog()
          << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
          << "Requested domain number is out of bounds for this variable."
          << std::endl;
      }
    }

    // No meta data so, look for a "regular" variable with this name.
    if (meta == NULL) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Looking for regular (non-md) variable." << std::endl;
      meta = registry->getVariable(name);

      if( meta )
        variableDataset = registry->getDataset(meta->getFullName());
    }

    // No meta data so, look for a VarWithMesh with this name.
    if (meta == NULL) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Looking for VarWithMesh variable." << std::endl;
      vmMeta = registry->getVariableWithMesh(name);

      if( vmMeta )
        variableDataset = registry->getDataset(vmMeta->getFullName());
    }

    // Haven't found metadata yet, so give up.
    if ((meta == NULL) && (vmMeta == NULL)) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "ERROR: Could not find metadata for name: "
                        << name << std::endl;
// DEBUG CODE
//       std::vector<std::string> varNames;
//       registry->getAllVariableNames(varNames);
//       VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
//                         << "All available names = ";
//       for (unsigned int i = 0; i < varNames.size(); i++) {
//         VsLog::debugLog() << varNames[i] << ", ";
//       }
//       VsLog::debugLog() << std::endl;
//       VsLog::debugLog() << "Returning NULL" << std::endl;
// END DEBUG
      return NULL;
    }

    // Have metadata from the variable, so load some info and look for
    // the mesh.
    hid_t varType = 0;
    std::vector<int> varDims;
    bool isFortranOrder = false;
    bool isCompMajor = false;
    bool isZonal = false;

    bool parallelRead = true;

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

      std::string meshName = meta->getMeshName();
      VsMesh* meshMetaPtr = registry->getMesh(meshName);

      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Mesh for variable is '" << meshName << "'." << std::endl;
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Getting metadata for mesh." << std::endl;

      if (!meshMetaPtr) {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Metadata not found for mesh '" << meshName
                          << "'." << std::endl;
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Returning NULL" << std::endl;
        return NULL;
      }

      std::vector<int> meshDims;
      meshMetaPtr->getMeshDataDims(meshDims);
      numTopologicalDims = meshDims.size();

      // Structured and unstructured mesh dimension contain the
      // spatial dimension so subtract it off.
      if( meshMetaPtr->isStructuredMesh() || meshMetaPtr->isUnstructuredMesh() )
        numTopologicalDims -= 1;

      if( varDims.size() - (int) isAComponent != numTopologicalDims )
      {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Var and mesh do not have the topological dimension"
                          << "Returning NULL." << std::endl;
        return NULL;
      }

      // For unstructured data, the variable can be subselected only when
      // the data is zonal. Also no parallel reads for unstructured data.
      if( meshMetaPtr->isUnstructuredMesh() )
      {
        haveDataSelections &= isZonal;
        parallelRead = false;
      }
    }

    bool isDouble = isDoubleType(varType);
    bool isFloat = isFloatType(varType);
    bool isInteger = isIntegerType(varType);

    if (!isDouble && !isFloat && !isInteger) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Error: hdf5 variable data type not handled: "
                        << varType << "Returning NULL." << std::endl;
      return NULL;
    }

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Variable " << name << " has dimensions =";
    for (size_t i=0; i<numTopologicalDims; ++i)
      VsLog::debugLog() << " " << varDims[i+isCompMajor];
    VsLog::debugLog() << ", numTopologicalDims = " << numTopologicalDims
                      << ", isComponent = " << isAComponent << "." << std::endl;

    // Temporary array for variable counts as the selection is NODAL.
    std::vector<int> numVars(numTopologicalDims);

    for (size_t i=0; i<numTopologicalDims; ++i)
      numVars[i] = varDims[i+isCompMajor];

    // The variable dims should reflect the topological dims plus one
    // more dimension for the spatial dimension of the variable.
    size_t vsdim = numTopologicalDims;
    std::vector<int> vdims(vsdim+1);

    // Adjust for the data selections which are NODAL. If no selection
    // the bounds are set to 0 and max with a stride of 1.
    GetSelectionBounds( numTopologicalDims, numVars, vdims,
                        mins, maxs, strides, haveDataSelections, !isZonal );

#if (defined PARALLEL && defined VIZSCHEMA_DECOMPOSE_DOMAINS)
    if (parallelRead)
    {
      if( !GetParallelDecomp( numTopologicalDims, vdims, mins, maxs, strides,
                              !isZonal ) )
        return NULL; // No work for this processor.

      haveDataSelections = 1;
    }
#endif

    int numVariables = 1;
    for (size_t i=0; i<numTopologicalDims; ++i)
      numVariables *= vdims[i];

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Total number of variables is " << numVariables
                      << std::endl;

    size_t varSize = H5Tget_size(varType);

    // Read in the data
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
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Allocation failed, pointer is NULL.  "
                        << "Returning NULL." << std::endl;
      return NULL;
    }

    // Variable and mesh data or a variable component.
    if (vmMeta || isAComponent)
    {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Entering VarWithMesh section." << std::endl;

      herr_t err = reader->getDataSet(variableDataset, dataPtr,
                                      indexOrder, componentIndex,
                                      mins, &(vdims[0]), strides);

      if (err < 0) {
        VsLog::debugLog()
          << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
          << "GetVarWithMeshComponent returned error: "
          << err << "  " << "Returning NULL." << std::endl;

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
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Loading variable data." << std::endl;

      herr_t err;
    
      if( haveDataSelections )
        err = reader->getDataSet( variableDataset, dataPtr,
                                  meta->getIndexOrder(), -2, // -2 no components
                                  mins, &(vdims[0]), strides );
      else
        err = reader->getDataSet( variableDataset, dataPtr );

      if (err < 0) {
        VsLog::debugLog()
          << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
          << "GetDataSet returned error: " << err << "  "
          << "Returning NULL." << std::endl;

        if (isDouble)
          delete [] dblDataPtr;
        else if (isFloat)
          delete [] fltDataPtr;
        else if (isInteger)
          delete [] intDataPtr;

        return NULL;
      }
    }

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
      << "Finished reading the data, building VTK structures." << std::endl;

    //DEBUG
    /*
     VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
        << "Dumping data: " << std::endl;
     for (int i = 0; i < numVariables; i++) {
     if (H5Tequal(type, H5T_NATIVE_DOUBLE)) {
     VsLog::debugLog() << "data[" << i << "] = " << ((double*)data)[i] << std::endl;}
     else if (H5Tequal(type, H5T_NATIVE_FLOAT)) {
     VsLog::debugLog() << "data[" << i << "] = " << ((float*)data)[i] << std::endl;}
     else if (H5Tequal(type, H5T_NATIVE_INT)) {
     VsLog::debugLog() << "data[" << i << "] = " << ((int*)data)[i] << std::endl;}
     }
     VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
        << "Finished dumping data. " << std::endl;
     */
    //END DEBUG

    vtkDataArray* rv = 0;

    if (isDouble) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Declaring vtkDoubleArray." << std::endl;
      rv = vtkDoubleArray::New();
    }
    else if (isFloat) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Declaring vtkFloatArray." << std::endl;
      rv = vtkFloatArray::New();
    }
    else if (isInteger) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Declaring vtkIntArray." << std::endl;
      rv = vtkIntArray::New();
    }

    rv->SetNumberOfTuples(numVariables);

    // Perform if needed permutation of index as VTK expect Fortran order

    // The index tuple is initially all zeros
    size_t* indices = new size_t[numTopologicalDims];
    for (size_t k=0; k<numTopologicalDims; ++k)
      indices[k] = 0;

    // Store data
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Storing " << numVariables << " data elements" << std::endl;

    // Attempt to reverse data in place
    //#define IN_PLACE
#ifdef IN_PLACE
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Attempting to swap data in place." << std::endl;
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
          VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                            << "Unknown data type: " << type << std::endl;
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
    if (isDouble)  {
      for (size_t k = 0; k<numVariables; ++k) {
        rv->SetTuple(k, &dblDataPtr[k]);
      }
    } else if (isFloat) {
      for (size_t k = 0; k<numVariables; ++k) {
        rv->SetTuple(k, &fltDataPtr[k]);
      }
    } else if (isInteger) {
      for (size_t k = 0; k<numVariables; ++k) {
        rv->SetTuple(k, &intDataPtr[k]);
      }
    } else {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Unknown data type: " << type << std::endl;
        return NULL;
    }

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Done swapping data in place." << std::endl;
#else

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Swapping data into correct places, NOT using 'in place' code."
                      << std::endl;

    // Using FORTRAN data ordering.
    if (isFortranOrder) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
        << "Using FORTRAN data ordering." << std::endl;

      if (isDouble) {
        for (size_t k = 0; k<numVariables; ++k) {
          rv->SetTuple(k, &dblDataPtr[k]);
        }
      } else if (isFloat) {
        for (size_t k = 0; k<numVariables; ++k) {
          rv->SetTuple(k, &fltDataPtr[k]);
        }
      } else if (isInteger) {
        for (size_t k = 0; k<numVariables; ++k) {
          // Convert to float because SetTuple doesn't take ints
          float temp = intDataPtr[k];
          rv->SetTuple(k, &temp);
        }
      }
    } else {

      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
        << "Using C data ordering." << std::endl;

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

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Done swapping data into correct places, NOT using 'in place' code."
                      << std::endl;
#endif

    // Done with data
    delete [] indices;

    if (isDouble) {
      delete [] dblDataPtr;
    } else if (isFloat) {
      delete [] fltDataPtr;
    } else if (isInteger) {
      delete [] intDataPtr;
    }

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Returning data." << std::endl;
    return rv;
}


// *****************************************************************************
//  Method: avtVsFileFormat::FreeUpResources
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
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Entering/Exiting function." << std::endl;
}

 
// *****************************************************************************
//  Method: avtVsFileFormat::RegisterExpressions
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
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Entering function." << std::endl;
    LoadData();

    //get list of expressions from reader
    std::map<std::string, std::string>* expressions =
      registry->getAllExpressions();
    
    if (expressions->empty()) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "WARNING: No expressions found in file. Returning."
                        << std::endl;
      return;
    } else {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Found " << expressions->size()
                        << " expressions in file." << std::endl;
    }

    //iterate over list of expressions, insert each one into database
    std::map<std::string, std::string>::const_iterator iv;
    for (iv = expressions->begin(); iv != expressions->end(); ++iv) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Adding expression " << iv->first << " = "
                        << iv->second << std::endl;
      Expression e;
      e.SetName (iv->first);

      //TODO: if the user has supplied a label for a component but the
      // vsVar expression still refers to the old component name we
      // need to either 1. register the old component name as an extra
      // component or 2. edit the vsVar expression to replace old
      // component names with the user-specified labels.
      e.SetDefinition(iv->second);

      // What kind of variable should we register?
      // By default we use scalar
      e.SetType(Expression::ScalarMeshVar);

      if ((iv->second.size() > 0) && (iv->second[0] == '{')) {
        VsLog::debugLog()
          << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
          << "It is a std::vector expression." << std::endl;

        e.SetType(Expression::VectorMeshVar);
      }
      else { //CURVE variable
        //Although this is not universally true, we make the assumption that
        //an expression defined on curve variables will be a curve expression
        for (std::vector<std::string>::const_iterator it = curveNames.begin();
             it != curveNames.end();
             it++)
        {
          size_t foundIndex = std::string::npos;
          foundIndex = iv->second.find(*it);
          if (foundIndex != std::string::npos) {
            VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                              <<"It is a curve expression." << std::endl;
            VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                              <<"Because we found an embedded curve name: " <<*it <<std::endl;
            e.SetType(Expression::CurveMeshVar);
            //No need to stay in the loop
            break;
          }
        }
      }

      //If we've still got scalar, tell the user
      if (e.GetType() == Expression::ScalarMeshVar) {
        VsLog::debugLog()
          << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
          << "It is a scalar expression." << std::endl;
      }

      //Add the new expression to the metadata object
      md->AddExpression(&e);
    }

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Exiting normally." << std::endl;
}


// *****************************************************************************
//  Method: avtVsFileFormat::RegisterVars
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
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Entering function." << std::endl;
    LoadData();

    // Get var names
    std::vector<std::string> names;
    registry->getAllVariableNames(names);
    
    if (names.empty()) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
        << "WARNING: No variables were found in this file.  Returning." << std::endl;
      return;
    } else {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Found " << names.size()
                        << " variables in this file." << std::endl;
    }

    std::vector<std::string>::const_iterator it;
    for (it = names.begin(); it != names.end(); ++it) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
        << "Processing var: "<< *it << std::endl;

      //get metadata for var
      VsVariable* vMeta = registry->getVariable(*it);
      
      //If this var is part of an MD var, we don't register it separately
      VsVariable* testForMD = registry->findSubordinateMDVar(*it);
      if (testForMD) {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Var is part of an MD variable, "
                          << "will not be registered separately." << std::endl;
        continue;
      }

      // Name of the mesh of the var
      std::string mesh = vMeta->getMeshName();
      VsMesh* meshMeta = vMeta->getMesh();
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Var lives on mesh " << mesh << "." << std::endl;

      // Centering of the variable
      // Default is node centered data
      avtCentering centering = AVT_NODECENT;
      if (vMeta->isZonal()) {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Var is zonal." << std::endl;
        centering = AVT_ZONECENT;
      } else {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Var is nodal." << std::endl;
      }

      // 1-D variable?
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Determining if var is 1-D." << std::endl;
      int numSpatialDims = 3;
      if (meshMeta) {
        numSpatialDims = meshMeta->getNumSpatialDims();
      } else {
        numSpatialDims = 3;
        VsLog::errorLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Unable to load mesh for variable?" << std::endl;
        VsLog::errorLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Using numSpatialDims = 3, hope it's right!"
                          << std::endl;     
      }

      //if this mesh is 1-D, we leave it for later (curves)
      bool isOneDVar = false;
      if (numSpatialDims == 1) {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Var is 1-D." << std::endl;
        isOneDVar = true;
      } else {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Var is not 1-D." << std::endl;
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
            std::string componentName = registry->getComponentName(*it, i);
  
            if (!componentName.empty()) {
              VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                                << "Adding curve component "
                                << componentName << "." << std::endl;
              avtCurveMetaData* cmd =
                new avtCurveMetaData(componentName.c_str());
              curveNames.push_back(componentName);
              cmd->hasDataExtents = false;
              md->Add(cmd);
            } else {
              VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                << "Unable to find match for curve variable in component registry."
                                << std::endl;
            }
          }
        } else {
          //When there is only one component, we don't create a component name
          //Instead, we just use the straight-up name
          avtCurveMetaData* cmd = new avtCurveMetaData(*it);
          curveNames.push_back(*it);
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
          std::string componentName = registry->getComponentName(*it, i);

          if (!componentName.empty()) {
            VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                              << "Adding variable component "
                              << componentName << "." << std::endl;
            avtScalarMetaData* smd =
              new avtScalarMetaData(componentName, mesh.c_str(), centering);
            smd->hasUnits = false;
            md->Add(smd);
          } else {
            VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                              << "Unable to find match for variable "
                    << *it << "in component (" << i << ") registry." << std::endl;
          }
        }
      }
      else if (numComps == 1) {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Adding single-component variable." << std::endl;
        avtScalarMetaData* smd =
          new avtScalarMetaData(*it, mesh.c_str(), centering);
        smd->hasUnits = false;
        md->Add(smd);
      }
      else {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Variable '" << *it
                          << "' has no components. Not being added." << std::endl;
      }
    }

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Exiting normally." << std::endl;
}


// *****************************************************************************
//  Method: avtVsFileFormat::RegisterMeshes
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
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Entering function." << std::endl;
    LoadData();

    // Number of mesh dims
    int spatialDims;
    int topologicalDims;

    // All meshes names
    std::vector<std::string> names;
    registry->getAllMeshNames(names);
    if (names.empty()) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
        << "WARNING: no meshes were found in this file. Returning." << std::endl;
      return;
    } else {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Found " << names.size() << " meshes." << std::endl;
    }

    std::vector<std::string>::const_iterator it;
    for (it = names.begin(); it != names.end(); ++it) {
      VsMesh* meta = registry->getMesh(*it);

      spatialDims = meta->getNumSpatialDims();

      //if this mesh is 1-D, we leave it for later (curves)
      if (spatialDims == 1) {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
          << "Found 1-d mesh.  Skipping for now, will be added as a curve."
          << std::endl;
        continue;
      } else if ((spatialDims <= 0) || (3 < spatialDims)) {
        VsLog::errorLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "NumSpatialDims is out of range: "
                          << spatialDims << "  "
                          << "Skipping mesh." << std::endl;
        continue;
      }
      
      //If this mesh is part of an MD mesh, we don't register it separately
      VsMesh* testForMD = registry->findSubordinateMDMesh(*it);
      if (testForMD) {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Mesh is part of an MD mesh, will not be registered separately."
                          << std::endl;
        continue;
      }
      
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Found mesh '" << *it << "' of kind '"
                        << meta->getKind() << "'." << std::endl;

      avtMeshType meshType = AVT_UNKNOWN_MESH;
      std::vector<int> dims;
      int bounds[3] = {1,1,1};
      int numCells = 1;

      // Uniform Mesh
      if (meta->isUniformMesh()) {
        // 09.06.01 Marc Durant We used to report uniform cartesian
        // meshes as being 3d no matter what I have changed this to
        // use the correct mesh dimensions The changed plugin passes
        // vstests, and is motivated because the VisIt Lineout
        // operator requires 2-d data.  EXCEPT! then we can't plot 3-d
        // std::vectors on the 2-d data, so for now we continue to report 3
        // spatialDimss = dims.size();
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Mesh's dimension = " << spatialDims << std::endl;
        // 11.08.01 MDurant We are no longer going to do this.  2d is 2d.
        //if (spatialDims != 3) {
        //  VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
        //                    << "But reporting as dimension 3 to side-step VisIt bug."
        //                    << std::endl;
        //  spatialDims = 3;
        //}
        
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Adding uniform mesh " << *it << "." << std::endl;

        meshType = AVT_RECTILINEAR_MESH;

        // Add in the logical bounds of the mesh.
        static_cast<VsUniformMesh*>(meta)->getNumMeshDims(dims);
        for( int i=0; i<dims.size(); ++i )
        {
          bounds[i] = dims[i]; // Logical bounds are node centric.
          numCells *= (dims[i]-1);
        }
      }

      // Rectilinear Mesh
      else if (meta->isRectilinearMesh()) {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Adding rectilinear mesh" << *it << ".  " 
                          << "spatialDims = " << spatialDims << "." << std::endl;

        meshType = AVT_RECTILINEAR_MESH;

        // Add in the logical bounds of the mesh.
        static_cast<VsRectilinearMesh*>(meta)->getNumMeshDims(dims);
        for( int i=0; i<dims.size(); ++i )
        {
          bounds[i] = dims[i]; // Logical bounds are node centric.
          numCells *= (dims[i]-1);
        }
      }

      // Structured Mesh
      else if (meta->isStructuredMesh()) {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Adding structured mesh " << *it << "." << std::endl;

        meshType = AVT_CURVILINEAR_MESH;

        // Add in the logical bounds of the mesh.
        static_cast<VsRectilinearMesh*>(meta)->getNumMeshDims(dims);
        for( int i=0; i<dims.size(); ++i )
        {
          bounds[i] = dims[i]; // Logical bounds are node centric.
          numCells *= (dims[i]-1);
        }
      }

      // Unstructured Mesh
      else if (meta->isUnstructuredMesh()) {
        VsUnstructuredMesh* unstructuredMesh = (VsUnstructuredMesh*)meta;

        // Unstructured meshes without connectivity data are registered
        // as point meshes
        if (unstructuredMesh->isPointMesh()) {
          VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                            << "Registering mesh " << it->c_str()
                            << " as AVT_POINT_MESH" << std::endl;

          meshType = AVT_POINT_MESH;
          topologicalDims = 1;

          // Add in the logical bounds of the mesh.
          numCells = unstructuredMesh->getNumPoints();
        }
        else {
          VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                            << "Registering mesh " << it->c_str()
                            << " as AVT_UNSTRUCTURED_MESH" << std::endl;


          spatialDims = meta->getNumSpatialDims();
          VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                            << "Adding unstructured mesh " << *it
                            << " with " << spatialDims
                            << " spatial dimensions." << std::endl;

          meshType = AVT_UNSTRUCTURED_MESH;

          // For now users can have only one connectivity dataset.
          if( unstructuredMesh->getLinesDataset() ||
              unstructuredMesh->getPolygonsDataset() ||
              unstructuredMesh->getTrianglesDataset() ||
              unstructuredMesh->getQuadrilateralsDataset() ) {
            topologicalDims = 2;
          } else if( unstructuredMesh->getPolyhedraDataset() ||
                     unstructuredMesh->getTetrahedralsDataset() ||
                     unstructuredMesh->getPyramidsDataset() ||
                     unstructuredMesh->getPrismsDataset() ||
                     unstructuredMesh->getHexahedralsDataset() ) {
            topologicalDims = 3;
          }
          else {
            VsLog::debugLog()
              << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
              << "ERROR - unable to find connectivity dataset. "
              << "Returning points data without connectivity."
              << std::endl;

            continue;
          }

          // Add in the logical bounds of the mesh.
          numCells = unstructuredMesh->getNumCells();
        }

        bounds[0] = numCells;
        bounds[1] = bounds[2] = 0;
      }
      else {

        meshType = AVT_UNKNOWN_MESH;

        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Unrecognized mesh kind: " << meta->getKind()
                          << "." << std::endl;

        continue;
      }

      int topologicalDims = meta->getNumTopologicalDims();

      if( meshType != AVT_UNKNOWN_MESH )
      {
        //Add a note for this interesting case.  It is legal, but since it's a new feature
        //we want to keep an eye on it
        if (topologicalDims > spatialDims) {
          VsLog::errorLog() <<__CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                            <<"ERROR - num topological dims (" << topologicalDims
                            <<") > num spatial dims (" << spatialDims <<")" <<std::endl;
          topologicalDims = spatialDims;
        } else if (spatialDims != topologicalDims) {
          VsLog::debugLog() <<__CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                            <<"Interesting - num topological dims (" << topologicalDims
                            <<") != num spatial dims (" << spatialDims <<")" <<std::endl;
        }

        avtMeshMetaData* vmd =
          new avtMeshMetaData(it->c_str(), 1, 1, 1, 0,
                              spatialDims, topologicalDims, meshType);
        vmd->SetBounds( bounds );
//        vmd->SetNumberCells( numCells );
        setAxisLabels(vmd);
        md->Add(vmd);
      }
    }

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Exiting normally." << std::endl;
}


// *****************************************************************************
//  Method: avtVsFileFormat::RegisterMdVars
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
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Entering function." << std::endl;
    LoadData();

    // Get vars names
    std::vector<std::string> names;
    registry->getAllMDVariableNames(names);
    
    if (names.empty()) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
        << "WARNING: No MD variables were found in this file.  Returning."
        << std::endl;
      return;
    } else {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Found " << names.size()
                        << " MD variables in this file." << std::endl;
    }

    std::vector<std::string>::const_iterator it;
    for (it = names.begin(); it != names.end(); ++it) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Processing md var '"
                        << *it << "'." << std::endl;
      VsMDVariable* vMeta = registry->getMDVariable(*it);
      
      // Name of the mesh of the var
      std::string mesh = vMeta->getMesh();
      std::string vscentering = vMeta->getCentering();
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "MD var lives on mesh " << mesh << "." << std::endl;

      //TODO: Mesh should either exist in an mdMesh, or separately in
      //the list of meshes

      // Centering of the variable
      // Default is node centered data
      avtCentering centering = AVT_NODECENT;
      if (vMeta->isZonal()) {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Var is zonal" << std::endl;
        centering = AVT_ZONECENT;
      } else {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Var is nodal" << std::endl;
      }

      // Number of component of the var
      size_t numComps = vMeta->getNumComps();

      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Variable has " << numComps
                        << " components." << std::endl;

      if (numComps > 1) {
        for (size_t i = 0; i<numComps; ++i) {
          //first, get a unique name for this component
          std::string compName = registry->getComponentName(*it, i);
          
          if (!compName.empty()) {
            VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                              << "Adding variable component " << compName
                              << "." << std::endl;
            avtScalarMetaData* smd = new avtScalarMetaData(compName.c_str(),
              mesh.c_str(), centering);
            smd->hasUnits = false;
            md->Add(smd);
          } else {
            VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                              << "Unable to find component name for var "
                              << *it << " and index " << i << std::endl;
          }
        }
      }
      else if (numComps == 1) {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Adding single variable component " << *it
                          << "." << std::endl;
        avtScalarMetaData* smd =
          new avtScalarMetaData(*it, mesh.c_str(), centering);
        smd->hasUnits = false;
        md->Add(smd);
      } else {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Variable '" << *it
                          << "' has no components. Not being added." << std::endl;
      }
    }
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Exiting normally." << std::endl;
}


// *****************************************************************************
//  Method: avtVsFileFormat::RegisterMdMeshes
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
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Entering function." << std::endl;
    LoadData();

    std::vector<std::string> names;
    registry->getAllMDMeshNames(names);
    if (names.empty()) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
        << "WARNING: no md meshes were found in this file. Returning" << std::endl;
      return;
    } else {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Found " << names.size()
                        << " MD meshes in this file." << std::endl;
    }

    std::vector<std::string>::const_iterator it;
    for (it = names.begin(); it != names.end(); ++it) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << " Adding md mesh '"
                        << *it << "'." << std::endl;

      VsMDMesh* meta = registry->getMDMesh(*it);
      if (!meta) {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Unable to find mesh " << *it << std::endl;
        continue;
      }

      avtMeshType meshType;
      std::string kind = meta->getMeshKind();
      if (meta->isUniformMesh()) {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Mesh is rectilinear" << std::endl;
        meshType = AVT_RECTILINEAR_MESH;
      } else if (meta->isUnstructuredMesh()) {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Mesh is unstructured" << std::endl;
        meshType = AVT_UNSTRUCTURED_MESH;
      } else if (meta->isStructuredMesh()) {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Mesh is structured" << std::endl;
        meshType = AVT_CURVILINEAR_MESH;
      }

      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Mesh has dimension " << meta->getNumSpatialDims()
                        << "." << std::endl;

      avtMeshMetaData* vmd =
        new avtMeshMetaData(it->c_str(), meta->getNumBlocks(), 1, 1, 0,
                            meta->getNumSpatialDims(),
                            meta->getNumSpatialDims(), meshType);
      setAxisLabels(vmd);
      md->Add(vmd);
    }

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Exiting normally." << std::endl;
}


// *****************************************************************************
//  Method: avtVsFileFormat::RegisterVarsWithMesh
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
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Entering function." << std::endl;
    LoadData();

    std::vector<std::string> names;
    registry->getAllVariableWithMeshNames(names);
    
    if (names.empty()) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
        << "WARNING: no variables with mesh were found in this file. Returning."
        << std::endl;
      return;
    } else {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Found " << names.size()
                        << " variables with mesh in this file." << std::endl;
    }

    std::vector<std::string>::const_iterator it;
    for (it = names.begin(); it != names.end(); ++it) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Processing varWithMesh '"
                        << *it << "'." << std::endl;
      VsVariableWithMesh* vMeta = registry->getVariableWithMesh(*it);

      // add var components
      std::vector<int> dims;
      vMeta->getMeshDataDims(dims);
      if (dims.size() <= 0) {
        std::ostringstream msg;
        msg << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
            << "could not get dimensions of variable with mesh.";
        VsLog::debugLog() << msg.str() << std::endl;
        throw std::out_of_range(msg.str().c_str());
      }

      size_t lastDim = 0;
      if (vMeta->isCompMinor())
        lastDim = dims[dims.size()-1];
      else
        lastDim = dims[0];

      if (lastDim < vMeta->getNumSpatialDims()) {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Error: for variable with mesh '" << *it
                          << "', numSpatialDims = "
                          << vMeta->getNumSpatialDims()
                          << " must be larger then the last dimension, "
                          << "lastDim = " << lastDim << "." << std::endl;
        //JRC: Remove the associated mesh from the system!  Actually,
        //the associated mesh doesn't get added until the end of this
        //method So there is no associated mesh to remove We just drop
        //through the loop and start on the next varWithMesh
        continue;
      }

      // This loop registers all VARIABLES
      // SS: we need all components so going to lastDim here.
      //    for (size_t i = 0; i < lastDim-vm->numSpatialDims; ++i) {
      for (size_t i = 0; i < lastDim; ++i) {
        //first, get a unique name for this component
        std::string compName = registry->getComponentName(*it, i);
        
        if (!compName.empty()) {
          //register with VisIt
          VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                            << "Adding variable component " << compName
                            << "." << std::endl;
          avtScalarMetaData* smd = new avtScalarMetaData(compName.c_str(),
            it->c_str(), AVT_NODECENT);
          smd->hasUnits = false;
          md->Add(smd);
        } else {
          VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                            << "Unable to get component name for variable "
                            << *it << " and index " << i << std::endl;
        }
      }

      // This loop registers all of the MESHES
      // Add in the logical bounds of the mesh.
      int numCells =  vMeta->getNumPoints();
      int bounds[3] = {numCells,0,0};
      int spatialDims = vMeta->getNumSpatialDims();
      if (spatialDims == 1) {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          <<"Found 1-d var with mesh, artificially elevating it to 2-d." <<std::endl;
        spatialDims = 2;
      } 
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Adding point mesh for this variable." << std::endl;
      avtMeshMetaData* vmd = new avtMeshMetaData(it->c_str(),
          1, 1, 1, 0, spatialDims, 0, AVT_POINT_MESH);
      vmd->SetBounds( bounds );
//      vmd->SetNumberCells( numCells );
      setAxisLabels(vmd);
      md->Add(vmd);
    }

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Exiting normally." << std::endl;
  }

// *****************************************************************************
//  Method: avtVsFileFormat::ActivateTimestep
//
//  Purpose:
//   Called to alert the database reader that VisIt is about to ask for data
//   at this timestep.  Since this reader is single-time, this is
//   only provided for reference and does nothing.
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
//  Method: avtVsFileFormat::UpdateCyclesAndTimes
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
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Entering function." << std::endl;
    LoadData();

    if (!md) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "md was NULL, returning." <<std::endl;
      return;
    }

    int timeStep = 0;

    // Extract timestep from filename
    // First, get file name from full path
    std::string fileName(dataFileName);
    int lastSlash = fileName.find_last_of('/');
    if (lastSlash == -1) {
      lastSlash = fileName.find_last_of('\\');
    }
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      <<"Last slash is " <<lastSlash << std::endl;
    if (lastSlash != -1) {
      int nameLength = fileName.length() - (lastSlash + 1);
      fileName = fileName.substr(lastSlash + 1, nameLength);
    }
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      <<"Extracted filename is \"" <<fileName <<"\"" << std::endl;
  
    // Timestep = the number between the last underscore and the first dot
    int lastUnderscore = fileName.find_last_of('_');
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      <<"lastUnderscore is " <<lastUnderscore << std::endl;

    int firstDot = fileName.find_first_of('.');
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      <<"firstDot is " <<firstDot << std::endl;

    if ((lastUnderscore != -1) &&
        (firstDot != -1) &&
        (firstDot > lastUnderscore + 1)) {
      std::string step =
        fileName.substr(lastUnderscore + 1, firstDot - (lastUnderscore + 1));

      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Step is \"" << step << "\""
                        << std::endl;

      timeStep = atoi(step.c_str());

      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        <<"Converted to integer is \"" << timeStep << "\""
                        << std::endl;
    }

    // If time data is present, tell VisIt
    if (registry->hasTime()) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "This file supplies time: "
                        << registry->getTime() << std::endl;

      md->SetTime(timeStep, registry->getTime());
      md->SetTimeIsAccurate(true, registry->getTime());
    }
    
    // If time cycle is present, tell VisIt
    if (registry->hasCycle()) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "This file supplies cycle: "
        << registry->getCycle() << std::endl;

      md->SetCycle(timeStep, registry->getCycle());
      md->SetCycleIsAccurate(true, registry->getCycle());
    }
}
  
  
// *****************************************************************************
//  Method: avtVsFileFormat::LoadData
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

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Initializing VsH5Reader()" << std::endl;

    //Actually open the file & read metadata for the first time

    try {
      reader = new VsH5Reader(dataFileName, registry);
    }
    catch (std::invalid_argument& ex) {
        std::ostringstream msg;
        msg << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
            << " Error initializing VsH5Reader: " << ex.what();
      VsLog::debugLog() << msg.str() << std::endl;
      EXCEPTION1(InvalidDBTypeException, msg.str().c_str());
    }

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Exiting" << std::endl;
}


// *****************************************************************************
//  Method: avtVsFileFormat::PopulateDatabaseMetaData
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
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Entering function." << std::endl;
    LoadData();

    // Tell visit that we can split meshes into subparts when running
    // in parallel

    // NOTE that we can't decompose domains if we have MD meshes
    // So it's one or the other

#ifdef VIZSCHEMA_DECOMPOSE_DOMAINS
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Decompose_domains is defined.  Entering code block."
                      << std::endl;
    if (registry->numMDMeshes() > 0) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "MD meshes are present in the data file.  "
                        << "Domain Decomposition is turned off." << std::endl;
      md->SetFormatCanDoDomainDecomposition(false);
    } else {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "NO MD meshes are present in the data file.  "
                        << "Domain Decomposition is turned on." << std::endl;
      md->SetFormatCanDoDomainDecomposition(true);
    }
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Decompose_domains is defined.  Exiting code block."
                      << std::endl;
#endif

    RegisterMeshes(md);
    RegisterMdMeshes(md);

    RegisterVarsWithMesh(md);

    RegisterVars(md);
    RegisterMdVars(md);

    RegisterExpressions(md);

    //add desperation last-ditch mesh if none exist in metadata
    if (md->GetNumMeshes() == 0 && md->GetNumCurves() == 0) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Warning: " << dataFileName
                        << " contains no mesh information. Creating default mesh."
                        << std::endl;
      avtMeshMetaData* mmd =
        new avtMeshMetaData("ERROR_READING_FILE", 1, 1, 1, 0, 3, 3,
          AVT_RECTILINEAR_MESH);
      setAxisLabels(mmd);
      md->Add(mmd);
    }

    //VsLog::debugLog() <<"Calling UpdateCyclesAndTimes for file: " <<dataFileName <<std::endl;
    //UpdateCyclesAndTimes(md);
    
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Exiting normally." << std::endl;
}


// *****************************************************************************
//  Method: avtVsFileFormat::setAxisLabels
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
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Entering function." << std::endl;
    
    if (mmd == NULL) {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Input pointer was NULL?" << std::endl;
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

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Exiting normally." << std::endl;
}


// *****************************************************************************
//  Method: avtVsFileFormat::ReturnsValidCycle
//
//  Purpose:
//      How do you do the voododo that you do
//
//  Programmer: Marc Durant
//  Creation:   June, 2010
//
//  Modifications:
//

bool avtVsFileFormat::ReturnsValidCycle()
{
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "entering" << std::endl;
    LoadData();

    if (registry->hasCycle())
    {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "returning TRUE." << std::endl;
      return true;
    }
    else
    {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "returning FALSE." << std::endl;
      return false;
    }
}


// *****************************************************************************
//  Method: avtVsFileFormat::GetCycle
//
//  Purpose:
//      How do you do the voododo that you do
//
//  Programmer: Marc Durant
//  Creation:   June, 2010
//
//  Modifications:
//

int avtVsFileFormat::GetCycle()
{
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "entering for file " <<dataFileName << std::endl;
    LoadData();

    if (registry->hasCycle())
    {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "The file " <<dataFileName <<" supplies cycle: "
                        << registry->getCycle() << std::endl;
      return registry->getCycle();
    }
    else
    {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "The file " <<dataFileName <<" does not supply cycle.  "
                        << "Returning INVALID_CYCLE." << std::endl;

      return INVALID_CYCLE;
    }
}


// *****************************************************************************
//  Method: avtVsFileFormat::ReturnsValidTime
//
//  Purpose:
//      How do you do the voododo that you do
//
//  Programmer: Marc Durant
//  Creation:   June, 2010
//
//  Modifications:
//

bool avtVsFileFormat::ReturnsValidTime()
{
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "entering" << std::endl;
    LoadData();

    if (registry->hasTime())
    {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "returning TRUE." <<std::endl;
      return true;
    }
    else
    {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "returning FALSE." <<std::endl;
      return false;
    }
}


// *****************************************************************************
//  Method: avtVsFileFormat::GetTime
//
//  Purpose:
//      How do you do the voododo that you do
//
//  Programmer: Marc Durant
//  Creation:   June, 2010
//
//  Modifications:
//

double avtVsFileFormat::GetTime()
{
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "entering" << std::endl;
    LoadData();

    if (registry->hasTime())
    {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "The file " <<dataFileName <<" supplies time: "
                        <<registry->getTime() << std::endl;
      return registry->getTime();
    }
    else
    {
      VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "The file " <<dataFileName <<" does not supply time.  "
                        << "Returning INVALID_TIME." << std::endl;
      return INVALID_TIME;
    }
}


// *****************************************************************************
//  Method: avtVsFileFormat::GetSelectionBounds
//
//  Purpose:
//      How do you do the voododo that you do
//
//  Programmer: Allen Sanderson
//  Creation:   July, 2011
//
//  Modifications:
//

void avtVsFileFormat::GetSelectionBounds( int numTopologicalDims,
                                          std::vector<int> &numNodes,
                                          std::vector<int> &gdims,
                                          int *mins,
                                          int *maxs,
                                          int *strides,
                                          bool haveDataSelections,
                                          bool isNodal )
{
    // Adjust for the data selections which are NODAL (typically) or ZONAL.
    if( haveDataSelections )
    {
      for (size_t i=0; i<numTopologicalDims; ++i)
      {
        if( maxs[i] < 0 || numNodes[i] - 1 < maxs[i] )
          maxs[i] = numNodes[i] - 1;  // last node index, not number of nodes

        if( maxs[i] < mins[i] )
          mins[i] = 0;

        if( strides[i] > 1 )
        {
          int lastNode = mins[i] + ((maxs[i] - mins[i]) / strides[i]) * strides[i];
            
          if( isNodal )
          {
            // Make sure a complete stride can be taken
            if( lastNode + strides[i] > maxs[i] )
              maxs[i] = lastNode;
          }
          else
          {
            // Make sure a complete stride can be taken
            if( lastNode + strides[i] - 1 > maxs[i] - 1 )
              maxs[i] = lastNode - 1;
          }
        }
      }
    }
    else
    {
      for (size_t i=0; i<numTopologicalDims; ++i)
      {
        mins[i] = 0;
        maxs[i] = numNodes[i] - 1;  // last node index, not number of nodes
        strides[i] = 1;
      }
    }

    for (size_t i=numTopologicalDims; i<3; ++i)
    {
      mins[i] = 0;
      maxs[i] = 1;
      strides[i] = 1;
    }

    // Storage for meshes in VisIt, which is is topologically 3D so
    // the points must also be 3D.
    for (size_t i=0; i<numTopologicalDims; ++i)
      gdims[i] = (maxs[i]-mins[i]) / strides[i] + 1;        // Number of nodes

    for (size_t i=numTopologicalDims; i<gdims.size(); ++i)
      gdims[i] = 1;

    if( haveDataSelections )
    {
      VsLog::debugLog()
        << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
        << "Have a " << (isNodal ? "nodal" : "zonal") << " inclusive selection  ";

      for (size_t i=0; i<numTopologicalDims; ++i)
      {
        VsLog::debugLog()
          << "(" << mins[i] << "," << maxs[i] << " stride " << strides[i] << ") ";
      }

      VsLog::debugLog() << std::endl;
    }

    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      <<  "Dimensions ";

    for (size_t i=0; i<numTopologicalDims; ++i)
      VsLog::debugLog() << gdims[i] << "  ";
    
    VsLog::debugLog() << std::endl;
}
 

// *****************************************************************************
//  Method: avtVsFileFormat::GetParallelDecomp
//
//  Purpose:
//      How do you do the voododo that you do
//
//  Programmer: Allen Sanderson
//  Creation:   July, 2011
//
//  Modifications:
//

bool avtVsFileFormat::GetParallelDecomp( int numTopologicalDims,
                                         std::vector<int> &dims,
                                         int *mins,
                                         int *maxs,
                                         int *strides,
                                         bool isNodal )
{
#if (defined PARALLEL && defined VIZSCHEMA_DECOMPOSE_DOMAINS)

  VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Parallel & Decompose_Domains are defined, "
                    << "entering parallel code." << std::endl;
  
  // Get the axis with greatest number of nodes.
  int splitAxis = 0;

  for (size_t i=0; i<numTopologicalDims; ++i)
  {
    if( dims[splitAxis] < dims[i] )
      splitAxis = i;
  }

  splitAxis = 0;

  // Integer number of nodes processor - note subtract off one node
  // because to join sections the "next" node is always added in. Thus
  // by default the last node will be added in.
  size_t numNodes = dims[splitAxis];
  size_t numNodesPerProc = (numNodes-1) / PAR_Size();
  size_t numProcsWithExtraNode = (numNodes-1) % PAR_Size();

  if( PAR_Rank() == 0 )
  {
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "splitAxis = " << splitAxis << "  "
                      << "min = " << mins[splitAxis] << "  "
                      << "max = " << maxs[splitAxis] << "  "
                      << "strides = " << strides[splitAxis] << "  "
                      << "numNodes = " << numNodes << "  "
                      << "numNodesPerProc = " << numNodesPerProc << "  "
                      << "numProcsWithExtraNode = " << numProcsWithExtraNode
                      << std::endl;
    
    for( int i=0; i<PAR_Size(); ++i )
    {
      int min, max;

      // To get all of the nodes adjust the count by one for those
      // processors that need an extra node.
      if (i < numProcsWithExtraNode)
      {
        min = i * (numNodesPerProc + 1) * strides[splitAxis];
        max = min + (numNodesPerProc + 1) * strides[splitAxis] +
          // To get the complete mesh (i.e. the overlay between one
          // section to the next) add one more node. But for zonal and
          // point based meshes it is not necessary.
          (isNodal ? 0 : -1);
      }
      else
      {
        // Processors w/extra node plus processors without extra node.
        min = (numProcsWithExtraNode * (numNodesPerProc + 1) +
               (i - numProcsWithExtraNode) * numNodesPerProc) *
          strides[splitAxis];
        
        max = min + (numNodesPerProc) * strides[splitAxis] +
          // To get the complete mesh (i.e. the overlay between one
          // section to the next) add one more node. But for zonal and
          // point based meshes it is not necessary.
          (isNodal ? 0 : -1);
      }

      // Number of nodes plus one if the node topology is greater than one.
      numNodes = (max-min) / strides[splitAxis] + 1;
      if( i == 0 || (i && numNodes > 1) )
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Predicted bounds for processor " << i << "  "
                          << "min = " << min << "  max = " << max << "  "
                          << "strides = " << strides[splitAxis] << "  "
                          << "nodes = " << numNodes << std::endl;
      else
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "No work for processor " << i << std::endl;
    }
  }

  // To get all of the nodes adjust the count by one for those
  // processors that need an extra node.
  if (PAR_Rank() < numProcsWithExtraNode)
  {
    mins[splitAxis] = PAR_Rank() * (numNodesPerProc + 1) * strides[splitAxis];
    maxs[splitAxis] = mins[splitAxis] + (numNodesPerProc +1) * strides[splitAxis] +
      // To get the complete mesh (i.e. the overlay between one
      // section to the next) add one more node. But for zonal and
      // point based meshes it is not necessary.
      (isNodal ? 0 : -1);
  }
  else
  {
    // Processors w/extra node plus processors without extra node.
    mins[splitAxis] = (numProcsWithExtraNode * (numNodesPerProc + 1) +
                       (PAR_Rank() - numProcsWithExtraNode) * numNodesPerProc) *
      strides[splitAxis];
    
    maxs[splitAxis] = mins[splitAxis] + (numNodesPerProc) * strides[splitAxis] +
      
      (isNodal ? 0 : -1);
  }

  // Number of nodes plus one if the node topology is greater than
  // one.  Point data meshes or for zonal variable data are the
  // cases where one does need to adjust for nodes.
  numNodes = (maxs[splitAxis]-mins[splitAxis]) / strides[splitAxis] + 1;
  
  dims[splitAxis] = numNodes;

  bool work;

  if( (PAR_Rank() == 0) || (PAR_Rank() && numNodes > 1) )
  {
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Actual bounds for processor " << PAR_Rank() << "  "
                      << "min = " << mins[splitAxis] << "  "
                      << "max = " << maxs[splitAxis] << "  "
                      << "strides = " << strides[splitAxis] << "  "
                      << "nodes = " << numNodes << std::endl;
  }
  else
  {
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "No work for processor " << PAR_Rank() << std::endl;
  }

  VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Parallel & Decompose_Domains are defined, "
                    << "exiting parallel code." << std::endl;

  // If not on processor 0 and if the porcessor has only one node
  // skip it as it is slice which will have been drawn by the previous
  // processor.
  return( (PAR_Rank() == 0) || (PAR_Rank() && numNodes > 1) );

#endif
  return false;
}
#endif
