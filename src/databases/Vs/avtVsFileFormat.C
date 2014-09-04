#include <hdf5.h>
#include <visit-hdf5.h>
#include <avtGhostData.h>
#if HDF5_VERSION_GE(1,8,1)
/**
 *
 * @file        avtVsFileFormat.cpp
 *
 * @brief       Implementation of base class for VSH5 visit plugins
 *
 * @version $Id: $
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
#include <vtkUnsignedCharArray.h>
#include <vtkIntArray.h>
#include <vtkInformation.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkStructuredGrid.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkCellType.h>

// VisIt includes
#include <avtDatabaseMetaData.h>
#include <avtMeshMetaData.h>
#include <avtVariableCache.h>
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

#include "HighOrderUnstructuredData.h"

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
#include "VsDataset.h"
#include "VsRectilinearMesh.h"
#include "VsUniformMesh.h"
#include "VsUnstructuredMesh.h"
#include "VsStructuredMesh.h"
#include "VsAttribute.h"
#include "VsLog.h"
#include "VsRegistry.h"
#include "VsReader.h"
#include "VsSchema.h"

#define __CLASS__ "avtVsFileFormat::"

#define CLASSFUNCLINE __CLASS__ <<"(" <<instanceCounter <<")" << __FUNCTION__ << "  " << __LINE__

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
processDataSelections(false)
{
    instanceCounter++;

    VsLog::initialize(DebugStream::Stream3(),
            DebugStream::Stream4(),
            DebugStream::Stream5());

    //VsLog::debugLog() << CLASSFUNCLINE << "  "
    //<< "entering" << std::endl;

    VsLog::debugLog() << CLASSFUNCLINE << "  " << "entering" << std::endl;

    if (readOpts != NULL) {
        for (int i=0; i<readOpts->GetNumberOfOptions(); ++i) {
            if (readOpts->GetName(i) == "Process Data Selections in the Reader")
            processDataSelections =
            readOpts->GetBool("Process Data Selections in the Reader");
        }
    }

    VsLog::debugLog() << CLASSFUNCLINE << "  "
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
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "float type checks out ok." << std::endl;
    } else {
        std::ostringstream msg;
        msg << CLASSFUNCLINE << "  "
        << "ERROR - H5T_NATIVE_FLOAT not recognized as a float.";
        VsLog::debugLog() << msg.str() << std::endl;
        EXCEPTION1(InvalidDBTypeException, msg.str().c_str());
    }

    //check types
    if (isDoubleType(H5T_NATIVE_DOUBLE)) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "double type checks out ok." << std::endl;
    } else {
        std::ostringstream msg;
        msg << CLASSFUNCLINE << "  "
        << "ERROR - H5T_NATIVE_DOUBLE not recognized as a double.";
        VsLog::debugLog() << msg.str() << std::endl;
        EXCEPTION1(InvalidDBTypeException, msg.str().c_str());
    }

    herr_t err = H5check();

    if (err < 0) {
        std::ostringstream msg;
        msg << CLASSFUNCLINE << "  "
        << "HDF5 version mismatch.  Vs reader built with "
        << H5_VERS_INFO << ".";
        VsLog::debugLog() << msg.str() << std::endl;

        EXCEPTION1(InvalidDBTypeException, msg.str().c_str());
    }

    //NOTE: We used to initialize the VsReader object here
    //But now do it on demand in 'populateDatabaseMetaData'
    //To minimize I/O

    VsLog::debugLog() << CLASSFUNCLINE << "  "
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
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "entering" << std::endl;

    FreeUpResources();
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "exiting" << std::endl;
}

// ***************************************************************************
//  Method: avtVsFileFormat::CreateCacheNameIncludingSelections
//
//  Purpose:
//      The reader will return different data sets based on data
//      selections.  This method gives a description of what is returned
//      so that the resulting data can be reliably cached.
//
//  Programmer: Hank Childs
//  Creation:   December 20, 2011
//
// **************************************************************************

std::string
avtVsFileFormat::CreateCacheNameIncludingSelections(std::string s)
{
    int mins[3], maxs[3], strides[3];
    bool haveDataSelections = ProcessDataSelections(mins, maxs, strides);
    if (!haveDataSelections)
    return s;

    char str[1024];
    strcpy(str, s.c_str());
    int amt = strlen(str);
    for (size_t i = 0; i < selList.size(); i++)
    {
        if ((*selsApplied)[i])
        {
            std::string s = selList[i]->DescriptionString();
            SNPRINTF(str+amt, 1024-amt, "_%s", s.c_str());
            amt += strlen(str);
        }
    }

    return std::string(str);
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
    selList = sels;
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

    for (size_t i = 0; i < selList.size(); i++)
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
//    Gunther H. Weber, Tue May 13 18:22:27 PDT 2014
//    Added missing () to PAR_Rank() function call for diagnostic output
//

vtkDataSet* avtVsFileFormat::GetMesh(int domain, const char* name)
{
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Entering function." << std::endl;
    LoadData();

    // Save the original name in a temporary variable
    // so that we can do some string manipulations on it
    std::string meshName = name;

    // Roopa: Check if this mesh name is a transformed mesh name. If
    // so, use the original mesh name to get data associated
    bool transform = false;
    std::string origMeshName = registry->getOriginalMeshName(meshName);
    if (!origMeshName.empty()) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Original mesh for this transformed mesh " << meshName << " is "
        << origMeshName << std::endl;
        transform = true;
        meshName = origMeshName;
    }

    bool haveDataSelections;
    int mins[3], maxs[3], strides[3];

    // Adjust for the data selections which are NODAL.
    if( (haveDataSelections = ProcessDataSelections(mins, maxs, strides)) ) ///TODO: check fix for assignment
    {
        VsLog::debugLog()
        << CLASSFUNCLINE << "  "
        << "Have a logical nodal selection for mesh  "
        << "(" << mins[0] << "," << maxs[0] << " stride " << strides[0] << ") "
        << "(" << mins[1] << "," << maxs[1] << " stride " << strides[1] << ") "
        << "(" << mins[2] << "," << maxs[2] << " stride " << strides[2] << ") "
        << std::endl;
    }

    // The MD system works by filtering the requests directed to it
    // into the name of the appropriate subordinate mesh.  For
    // example, in facets_core-edge-explicit we have three meshes
    // joined into one: MdMesh = {coreMesh, solMesh, privMesh} So if
    // we get a request for (MdMesh, 0), we change the name to
    // coreMesh and proceed normally

    // Check for MD mesh
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Looking for MD mesh with name "
    << meshName << std::endl;

    VsMDMesh* mdMeshMeta = registry->getMDMesh(meshName);
    VsMesh* meta = NULL;

    // Found an MD mesh with this name, try to load the mesh data from it
    if (mdMeshMeta != NULL) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Found MD mesh with that name."
        << std::endl;
        meshName = mdMeshMeta->getNameForBlock(domain);

        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Request for md mesh was filtered to regular mesh: "
        << meshName << std::endl;
        meta = mdMeshMeta->getBlock(domain);
    } else {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "No MD mesh with that name."
        << std::endl;
    }

    // Did we succeed in loading mesh data from MD mesh?
    if (meta == NULL) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Trying to find regular mesh named: "
        << meshName << std::endl;
        meta = registry->getMesh(meshName);
    }

    if (meta != NULL) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Found mesh named: " << meshName << std::endl;

        // Uniform Mesh
        if (meta->isUniformMesh()) {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Trying to load & return uniform mesh" << std::endl;

            return getUniformMesh(static_cast<VsUniformMesh*>(meta),
                    haveDataSelections, mins, maxs, strides);
        }

        // Rectilinear Mesh
        if (meta->isRectilinearMesh()) {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Trying to load & return rectilinear mesh."
            << std::endl;
            return getRectilinearMesh(static_cast<VsRectilinearMesh*>(meta),
                    haveDataSelections, mins, maxs, strides, transform);
        }

        // Structured Mesh
        if (meta->isStructuredMesh()) {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Trying to load & return structured mesh" << std::endl;
            return getStructuredMesh(static_cast<VsStructuredMesh*>(meta),
                    haveDataSelections, mins, maxs, strides);
        }

#if (defined PARALLEL && defined VIZSCHEMA_DECOMPOSE_DOMAINS)
        // Don't know how to decompose any other type of mesh -> load it
        // on proc 0 only
        if (PAR_Rank() > 0) {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "In parallel mode on processor " << PAR_Rank()
            << " and mesh is not uniform.  "
            << "Returning NULL, mesh will be loaded on processor 0 only."
            << std::endl;
            return NULL;
        }
#endif

        // Unstructured Mesh
        if (meta->isUnstructuredMesh()) {

            if(meta->isHighOrder()) {
                VsLog::debugLog() << CLASSFUNCLINE << "  " << "Trying to load & return high order unstructured mesh" << std::endl;
                return getHighOrderUnstructuredMesh(static_cast<VsUnstructuredMesh*>(meta),
                        haveDataSelections, mins, maxs, strides);
            }

            VsLog::debugLog() << CLASSFUNCLINE << "  " << "Trying to load & return unstructured mesh" << std::endl;
            return getUnstructuredMesh(static_cast<VsUnstructuredMesh*>(meta),
                    haveDataSelections, mins, maxs, strides);
        }

        // At this point we don't know what kind of mesh it is.
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Mesh has unknown type: " << meta->getKind()
        << ".  Returning NULL." << std::endl;
        return NULL;
    }

    // Variable with mesh
    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")"
    << __FUNCTION__ << "  " << __LINE__ << "  "
    << "Looking for Variable-With-Mesh named " << meshName
    << ".\n";
    VsVariableWithMesh* vmMeta = registry->getVariableWithMesh(meshName);

    if (vmMeta != NULL)
    {
        VsLog::debugLog() << CLASSFUNCLINE << "  "

        << "Found Variable With Mesh. Loading data and returning." << std::endl;
        return getPointMesh(vmMeta, haveDataSelections, mins, maxs, strides, transform);
    }

    // Curve
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Looking for Curve with this name." << std::endl;

    vtkDataArray* foundCurve = this->GetVar(domain, name);

    if (foundCurve != NULL)
    {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Trying to load & return curve." << std::endl;

        foundCurve->Delete();

        return getCurve(domain, name);
    }

    VsLog::debugLog() << CLASSFUNCLINE << "  "
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
//    Gunther H. Weber, Tue May 13 18:23:13 PDT 2014
//    Fixed computation of grid spacing
//

vtkDataSet* avtVsFileFormat::getUniformMesh(VsUniformMesh* uniformMesh,
        bool haveDataSelections,
        int* mins, int* maxs, int* strides)
{
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Entering function." << std::endl;
    LoadData();

    // Read int data
    std::vector<int> numCells;
    //OLD: uniformMesh->getMeshDataDims(numCells); // Number of cells NOT nodes
    uniformMesh->getCellDims(numCells);

    if (numCells.size() <= 0) { ///TODO: are number of cells allow to be 0?
        std::ostringstream msg;
        msg << CLASSFUNCLINE << "  "
        << "Could not get dimensions of the uniform mesh.";
        VsLog::debugLog() << msg.str() << std::endl;
        throw std::out_of_range(msg.str().c_str());
    }

    size_t numTopologicalDims = uniformMesh->getNumTopologicalDims();

    if (numTopologicalDims > 3) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Error: numTopologicalDims of the mesh is larger than 3.  "
        << "Returning NULL." << std::endl;
        return NULL;
    }

    size_t numSpatialDims = uniformMesh->getNumSpatialDims();

    if (numSpatialDims > 3) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Error: numSpatialDims data is larger than 3.  "
        << "Returning NULL." << std::endl;
        return NULL;
    }

    // The numTopologicalDims and numSpatialDims can be different for
    // structured grids but the numSpatialDims must be great than or
    // equal to the numTopologicalDims.
    if( numTopologicalDims > numSpatialDims )
    {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
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
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Error: hdf5 mesh data type not handled: "
        << meshDataType << "Returning NULL." << std::endl;
        return NULL;
    }

    // startCell
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Loading optional startCells attribute." << std::endl;
    std::vector<int> startCell;
    herr_t err = uniformMesh->getStartCell(&startCell);
    if (err < 0) {
        VsLog::warningLog() << CLASSFUNCLINE << "  "
        << "Uniform mesh does not have starting cell position."
        << std::endl;
    } else {
        // Adjust the box by startCell
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Adjusting numCells by startCells." << std::endl;
        for (size_t i = 0; i < numTopologicalDims; ++i)
        numCells[i] -= startCell[i];
    }

    VsLog::debugLog() << CLASSFUNCLINE << "  "
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

    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Getting bounds for mesh." << std::endl;

    std::vector<float> lowerBounds;
    uniformMesh->getLowerBounds(&lowerBounds);

    std::vector<float> upperBounds;
    uniformMesh->getUpperBounds(&upperBounds);

    // Storage for mesh points in VisIt are spatially 3D. So create 3
    // coordinate arrays and fill in zero for the others.
    VsLog::debugLog() << CLASSFUNCLINE << "  "
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
        if (gdims[i] > 1)
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

        VsLog::debugLog() << CLASSFUNCLINE << "  "
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
    VsLog::debugLog() << CLASSFUNCLINE << "  "
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

    VsLog::debugLog() << CLASSFUNCLINE << "  "
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
        int* mins, int* maxs, int* strides,
        bool transform)
{
    // TODO - make "cleanupAndReturnNull" label, and do a "go to"
    // instead of just returning NULL all the time.

    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Entering function." << std::endl;
    LoadData();

    // Get dimensions
    std::vector<int> numNodes;
    //rectilinearMesh->getMeshDataDims(numNodes); // Number of nodes NOT cells
    rectilinearMesh->getCellDims(numNodes);

    if (numNodes.size() <= 0) { ///TODO: are number of cells allow to be zero?
        std::ostringstream msg;
        msg << CLASSFUNCLINE << "  "
        << "Could not get dimensions of the rectilinear mesh.";
        VsLog::debugLog() << msg.str() << std::endl;
        throw std::out_of_range(msg.str().c_str());
    }

    size_t numTopologicalDims = rectilinearMesh->getNumTopologicalDims();

    if (numTopologicalDims > 3) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Error: numTopologicalDims of data is larger than 3.  "
        << "Returning NULL." << std::endl;
        return NULL;
    }

    int numSpatialDims = rectilinearMesh->getNumSpatialDims();

    if (numSpatialDims > 3) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
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
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Error: hdf5 mesh data type not handled: "
        << meshDataType << "Returning NULL." << std::endl;
        return NULL;
    }

    VsLog::debugLog() << CLASSFUNCLINE << "  "
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

    for( size_t i=0; i<numTopologicalDims; ++i )
    {
        void* dataPtr;
        double* dblDataPtr = NULL; //TODO: check fix for uninitialized values
        float* fltDataPtr = NULL; ///TODO: check fix for uninitialized values

        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Loading data for axis " << i << std::endl;
        VsDataset* axisData = rectilinearMesh->getAxisDataset(i);

        if (axisData == NULL) {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Axis " << i
            << " data not found. Returning NULL." << std::endl;
            return NULL;
        }

        hid_t axisDataType = axisData->getType();

        // Read points and add in zero for any lacking dimension
        if( isDoubleType( axisDataType ) ) {
            dblDataPtr = new double[gdims[i]];
            dataPtr = dblDataPtr;
        }
        else if( isFloatType( axisDataType ) ) {
            fltDataPtr = new float[gdims[i]];
            dataPtr = fltDataPtr;
        } else {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Unknown axis data type: "
            << axisDataType << std::endl;
            return NULL;
        }

        if (!dataPtr) {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Allocation failed, pointer is NULL."
            << "Returning NULL." << std::endl;
            return NULL;
        }

        // Read in the point data. NOTE: Even when doing index selecting
        // or in parallel mode read in all of te data. As for the grid
        // there is not that much data. When building the mesh the
        // correct nodes are used.
        herr_t err = reader->getData(axisData, dataPtr);

        if (err < 0) {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
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
        VsLog::debugLog() << CLASSFUNCLINE << "  "
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

        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Dumped " << cc << " nodes to the rectilinear grid."
        << std::endl;
    }

    for (size_t i=numTopologicalDims; i<vsdim; ++i) {
        if( isDouble )
        coords[i] = vtkDoubleArray::New();
        else if( isFloat )
        coords[i] = vtkFloatArray::New();

        coords[i]->SetNumberOfTuples(1);
        coords[i]->SetComponent(0, 0, 0);
    }

    // Create a Rectilinear mesh and send it if there is no
    // transformation requested
    if (!transform) {
        // Create vtkRectilinearGrid
        VsLog::debugLog() << CLASSFUNCLINE << "  "
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

        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Returning data." << std::endl;
        return rgrid;
    } else {
        // Roopa: Transformation to cylindrical co-ords if transform is
        // true. Calculate the points and return a Structured mesh instead
        // of a Rectilinear mesh
        //
        float /*temp,*/ tempk, tempj, tempi;
        vtkPoints* vpoints = vtkPoints::New();
        if (isDouble) {
            vpoints->SetDataTypeToDouble();
        }
        else if (isFloat) {
            vpoints->SetDataTypeToFloat();
        }
        vpoints->SetNumberOfPoints((maxs[0] + 1) * (maxs[1] + 1) * (maxs[2] + 1));

        int ind = 0;
        if (isDouble) {
            for (int j = mins[2]; j <= maxs[2]; j++) {
                for (int i = mins[1]; i <= maxs[1]; i++) {
                    for (int k = mins[0]; k <= maxs[0]; k++) {
                        tempi = static_cast<vtkDoubleArray*>(coords[1])->GetValue(i)
                        * cos(static_cast<vtkDoubleArray*>(coords[2])->GetValue(j));
                        tempj = static_cast<vtkDoubleArray*>(coords[1])->GetValue(i)
                        * sin(static_cast<vtkDoubleArray*>(coords[2])->GetValue(j));
                        tempk = static_cast<vtkDoubleArray*>(coords[0])->GetValue(k);
                        vpoints->SetPoint(ind, tempi, tempj, tempk);
                        ind++;
                    }
                }
            }
        } else if (isFloat) {
            for (int j = mins[2]; j <= maxs[2]; j++) {
                for (int i = mins[1]; i <= maxs[1]; i++) {
                    for (int k = mins[0]; k <= maxs[0]; k++) {
                        tempi = static_cast<vtkFloatArray*>(coords[1])->GetValue(i)
                        * cos(static_cast<vtkFloatArray*>(coords[2])->GetValue(j));
                        tempj = static_cast<vtkFloatArray*>(coords[1])->GetValue(i)
                        * sin(static_cast<vtkFloatArray*>(coords[2])->GetValue(j));
                        tempk = static_cast<vtkFloatArray*>(coords[0])->GetValue(k);
                        vpoints->SetPoint(ind, tempi, tempj, tempk);
                        ind++;
                    }
                }
            }
        }

        // Create a Structured grid
        vtkStructuredGrid* sgrid = vtkStructuredGrid::New();
        sgrid->SetDimensions(&(gdims[0]));
        sgrid->SetPoints(vpoints);

        // Cleanup local data
        vpoints->Delete();

        // Cleanup local data
        for (size_t i = 0; i < vsdim; ++i) {
            coords[i]->Delete();
        }

        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Returning transformed data." << std::endl;
        return sgrid;
    }
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

    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Entering function." << std::endl;
    LoadData();

    // Find points: Structured meshes are datasets, and the data is
    // the points So, look for the dataset with the same name as the
    // mesh

    VsDataset* pointsDataset =
    registry->getDataset(structuredMesh->getFullName());

    if (pointsDataset == NULL) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Component '"
        << structuredMesh->getFullName()
        << "' not found.  Returning NULL." << std::endl;
        return NULL;
    }

    // Get dims of points array
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Determining dimension of points array." << std::endl;

    std::vector<int> numNodes;
    structuredMesh->getCellDims(numNodes);// Number of nodes NOT cells.

    if (numNodes.size() <= 0) { ///TODO: < 0 can never happen, but can size be equal to zero?
        std::ostringstream msg;
        msg << CLASSFUNCLINE << "  "
        << "Could not get dimensions of the structured mesh.";
        VsLog::debugLog() << msg.str() << std::endl;
        throw std::out_of_range(msg.str().c_str());
    }

    // The numTopologicalDims and numSpatialDims can be different for
    // structured grids.
    size_t numTopologicalDims = structuredMesh->getNumTopologicalDims();

    if (numTopologicalDims > 3) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Error: numTopologicalDims of data is larger than 3.  "
        << "Returning NULL." << std::endl;
        return NULL;
    }

    size_t numSpatialDims = structuredMesh->getNumSpatialDims();

    if (numSpatialDims > 3) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Error: numSpatialDims data is larger than 3.  "
        << "Returning NULL." << std::endl;
        return NULL;
    }

    // The numTopologicalDims and numSpatialDims can be different for
    // structured grids but the numSpatialDims must be great than or
    // equal to the numTopologicalDims.
    if( numTopologicalDims > numSpatialDims )
    {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
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
        VsLog::debugLog() << CLASSFUNCLINE << "  "
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

    VsLog::debugLog() << CLASSFUNCLINE << "  "
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
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Allocation failed, pointer is NULL.  "
        << "Returning NULL." << std::endl;
        return NULL;
    }

    // Read in the data
    herr_t err;

    if( haveDataSelections )
    err = reader->getData( pointsDataset, dataPtr,
    // -1 read all coordinate components
            structuredMesh->getIndexOrder(), -1,
            mins, &(gdims[0]), strides );
    else
    err = reader->getData( pointsDataset, dataPtr );

    if (err < 0)
    {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
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
        VsLog::debugLog() << CLASSFUNCLINE << "  "
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

        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Data move succeeded." << std::endl;
    }

    // Create the mesh and set its dimensions, including unused to zero
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Creating the mesh." << std::endl;

    // Add the points, changing to C ordering
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Adding points to mesh." << std::endl;

    vtkPoints* vpoints = vtkPoints::New();
    bool isFortranOrder = structuredMesh->isFortranOrder();

    if (isDouble) {
        // Note: must precede SetNumberOfPoints for alloc to succeed
        vpoints->SetDataTypeToDouble();
        vpoints->SetNumberOfPoints(numPoints);// will allocate

        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")"
        << __FUNCTION__ << "  " << __LINE__
        << " Adding " << numPoints << " with isFortranOrder = "
        << isFortranOrder << ", type is double.\n";

        this->setStructuredMeshCoords(gdims, dblDataPtr, isFortranOrder, vpoints);
        delete [] dblDataPtr;
    }
    else if (isFloat) {
        // Note: must precede SetNumberOfPoints for alloc to succeed
        vpoints->SetDataTypeToFloat();
        vpoints->SetNumberOfPoints(numPoints);// will allocate

        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")"
        << __FUNCTION__ << "  " << __LINE__
        << " Adding " << numPoints << " with isFortranOrder = "
        << isFortranOrder << ", type is float.\n";

        this->setStructuredMeshCoords(gdims, fltDataPtr, isFortranOrder, vpoints);
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

    std::string maskName = structuredMesh->getMaskName();
    if (maskName != "") {
        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")"
        << __FUNCTION__ << "  " << __LINE__ << "  "
        << "Mask detected, name is " << maskName << std::endl;
        // To access the data
        VsDataset* mask = registry->getDataset(maskName);
        // To access the attributes
        VsVariable* maskVar = registry->getVariable(maskName);
        if (!mask || !maskVar) {
            VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")"
            << __FUNCTION__ << "  " << __LINE__ << "  "
            << "Cannot access mask dataset "
            << maskName << std::endl;
        }
        else {
            hid_t maskType = mask->getType();
            bool maskIsFortranOrder = maskVar->isFortranOrder();
            std::string maskCentering = maskVar->getCentering();

            // Masking array, we use the same technique used elsewhere to
            // turn off ghost nodes
            vtkUnsignedCharArray *maskedNodes = vtkUnsignedCharArray::New();
            // Must have the name "avtGhostNodes"
            maskedNodes->SetName("avtGhostNodes");
            maskedNodes->SetNumberOfTuples(numPoints);
            for (size_t k = 0; k < (size_t)numPoints; ++k) {
                // Zero means no masking, node is visible
                maskedNodes->SetValue(k, 0);
            }

            if (isDoubleType(maskType)) {
                VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")"
                << __FUNCTION__ << "  " << __LINE__ << "  "
                << "Mask is of type double." << std::endl;
                this->fillInMaskNodeArray<double>(gdims, mask,
                        maskIsFortranOrder,
                        maskedNodes);
            }
            else if (isFloatType(maskType)) {
                VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")"
                << __FUNCTION__ << "  " << __LINE__ << "  "
                << "Mask is of type float." << std::endl;
                this->fillInMaskNodeArray<float>(gdims, mask,
                        maskIsFortranOrder,
                        maskedNodes);
            }
            else if (isIntType(maskType)) {
                VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")"
                << __FUNCTION__ << "  " << __LINE__ << "  "
                << "Mask is of type int." << std::endl;
                this->fillInMaskNodeArray<int>(gdims, mask,
                        maskIsFortranOrder,
                        maskedNodes);
            }
            else if (isShortType(maskType)) {
                VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")"
                << __FUNCTION__ << "  " << __LINE__ << "  "
                << "Mask is of type short." << std::endl;
                this->fillInMaskNodeArray<short>(gdims, mask,
                        maskIsFortranOrder,
                        maskedNodes);
            }
            else if (isCharType(maskType)) {
                VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")"
                << __FUNCTION__ << "  " << __LINE__ << "  "
                << "Mask is of type char." << std::endl;
                this->fillInMaskNodeArray<char>(gdims, mask,
                        maskIsFortranOrder,
                        maskedNodes);
            }
            else if (isUnsignedCharType(maskType)) {
                VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")"
                << __FUNCTION__ << "  " << __LINE__ << "  "
                << "Mask is of type unsigned char." << std::endl;
                this->fillInMaskNodeArray<unsigned char>(gdims, mask,
                        maskIsFortranOrder,
                        maskedNodes);
            }
            else {
                VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")"
                << __FUNCTION__ << "  " << __LINE__ << "  "
                << "Mask is of unsupported type." << std::endl;
            }
            sgrid->GetPointData()->AddArray(maskedNodes);
            maskedNodes->Delete();
        }
    }

    // Cleanup local data
    vpoints->Delete();

    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Returning data." << std::endl;

    //This is the "alternate" way to attach information to a dataset in the VisIt pipeline.
    //It works for meshes, where the vtkInformation approach works for data
    //Kept for historical purposes
    //Attach offset information to dataset
    //debug5 <<"avtVsFileFormat::getStructuredMesh() - attaching node offset as field data." <<std::endl;
    //vtkDoubleArray* offsetArray = vtkDoubleArray::New();
    //offsetArray->SetNumberOfTuples(3);
    //offsetArray->SetValue(0, 0.1);
    //offsetArray->SetValue(1, 0.2);
    //offsetArray->SetValue(2, 0.3);
    //offsetArray->SetName("nodeOffset");
    //sgrid->GetFieldData()->AddArray(offsetArray);
    //offsetArray->Delete();

    return sgrid;
}

//Most of this really should be moved into the HighOrderUnstrucured class.
vtkDataSet*
avtVsFileFormat::getHighOrderUnstructuredMesh(VsUnstructuredMesh* unstructuredMesh,
        bool haveDataSelections,
        int* mins, int* maxs, int* strides) {
    VsLog::debugLog() << CLASSFUNCLINE << "  " << "Entering" << std::endl;
    LoadData();

    thisData.setReader(reader);
    thisData.setRegistry(registry);

    return thisData.getMesh(unstructuredMesh);
}

// *****************************************************************************
//  Method: avtVsFileFormat::getUnstructuredMesh
//
//  Purpose:
//      Request an unstructured mesh
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

    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Entering function." << std::endl;
    LoadData();

    // Check for points type
    hid_t meshDataType = unstructuredMesh->getDataType();

    // Cache the data type because it's faster
    bool isDouble = isDoubleType(meshDataType);
    bool isFloat = isFloatType(meshDataType);

    if (!isDouble && !isFloat) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Error: hdf5 mesh data type not handled: "
        << meshDataType << "Returning NULL." << std::endl;
        return NULL;
    }

    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Checking dimensionality of mesh." << std::endl;

    size_t numNodes = unstructuredMesh->getNumPoints();
    size_t numSpatialDims = unstructuredMesh->getNumSpatialDims();

    // Get ready to read in points
    VsLog::debugLog() << CLASSFUNCLINE << "  "
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
        if( maxs[0] < 0 || numNodes - 1 < (size_t)maxs[0] )
        maxs[0] = numNodes - 1;          // last cell index,
                                         // not number of cells

        if( maxs[0] < mins[0] )
        mins[0] = 0;

        VsLog::debugLog() << CLASSFUNCLINE << "  "
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

    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "VPoints array will have " << numNodes << " points."
    << std::endl;
    vpoints->SetNumberOfPoints(numNodes);
    void* dataPtr = vpoints->GetVoidPointer(0);
    if (!dataPtr) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Unable to allocate the points.  Cleaning up."
        << std::endl;
        ugridPtr->Delete();
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Returning NULL." << std::endl;
        return NULL;
    }

    // Mesh points are in separate data sets.
    if (unstructuredMesh->usesSplitPoints())
    {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Using split-points method" << std::endl;

        // Kind of grim but read each of the coordinate components in
        // using the generic reader.

        // To do so set the location in memory where each cordinate
        // component will be stored.
        int srcMins[1] = {0};
        int srcMaxs[1] = {0};
        int srcStrides[1] = {0};

        if( haveDataSelections )
        {
            srcMins[0] = mins[0];
            srcMaxs[0] = numNodes;
            srcStrides[0] = strides[0];
        }
        else
        {
            srcMins[0] = 0;
            srcMaxs[0] = numNodes;
            srcStrides[0] = 1;
        }

        int destSize[1] = {numNodes*3};
        int destMins[1] = {0};
        int destMaxs[1] = {numNodes};
        int destStrides[1] = {3};

        for( size_t i=0; i<numSpatialDims; ++i )
        {
            VsDataset* pointDataset = unstructuredMesh->getPointsDataset(i);

            destMins[0] = i;

            herr_t err = reader->getData(pointDataset, dataPtr,
                    unstructuredMesh->getIndexOrder(),
                    -2,
                    &srcMins[0], &srcMaxs[0], &srcStrides[0],
                    1, &destSize[0],
                    &destMins[0], &destMaxs[0], &destStrides[0] );

            if (err < 0) {
                VsLog::debugLog() << CLASSFUNCLINE << "  "
                << "Call to getDataSet returned error: "
                << err << "Returning NULL." << std::endl;
                return NULL;
            }
        }

        // The above stores the value in the correct location but make
        // sure the remaing values are all zero.
        for (int i=numSpatialDims; i<3; ++i)
        {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Zeroing data at unused positions." << std::endl;

            if (isDouble) {

                double* dblDataPtr = &(((double*) dataPtr)[i]);

                for (size_t j=0; j<numNodes; ++j)
                {
                    *dblDataPtr = 0;
                    dblDataPtr += 3;
                }
            }
            else if (isFloat) {

                float* fltDataPtr = &(((float*) dataPtr)[i]);

                for (size_t j=0; j<numNodes; ++j)
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
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Using all-in-one method" << std::endl;

        VsDataset* pointsDataset = unstructuredMesh->getPointsDataset();

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

            err = reader->getData( pointsDataset, dataPtr,
                    // -1 read all coordinate components
                    unstructuredMesh->getIndexOrder(), -1,
                    &(srcMins[0]), &(srcMaxs[0]), &(srcStrides[0]) );
        }
        else
        err = reader->getData(pointsDataset, dataPtr);

        if (err < 0) {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Call to getDataSet returned error: "
            << err << "Returning NULL." << std::endl;
            return NULL;
        }

        // If spatial dimension is less than 3, move coordinates to the
        // correct position and set the extra dimensions to zero.
        if (numSpatialDims < 3)
        {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
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

            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Data move succeeded." << std::endl;
        }
    }

    // If there is no polygon data, then this mesh was registered as a
    // point mesh and we don't need to go any further
    if (unstructuredMesh->isPointMesh())
    {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Mesh was registered as a point mesh."
        << "Because no connectivity data was found. "
        << "Adding vertices as single points and returning."
        << std::endl;

        vtkIdType vertex;
        for (size_t i=0; i<numNodes; ++i) {
            vertex = i;
            ugridPtr->InsertNextCell(VTK_VERTEX, 1, &vertex);
        }

        return ugridPtr;
    }

    // Next, look for connectivity data
    VsDataset* connectivityMeta = 0;

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
        VsLog::debugLog() << CLASSFUNCLINE << "  "
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

    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Found connectivity data. "
    << "Connectivity dataset name is "
    << connectivityDatasetName << "  "
    << "haveConnectivityCount = "
    << haveConnectivityCount
    << std::endl;

    VsDataset* connectivityDataset =
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
        if( maxs[0] < 0 || numCells - 1 < (size_t)maxs[0] )
        maxs[0] = numCells - 1; // numCells - 1 = last cell index,
                                // not number of cells

        if( maxs[0] < mins[0] )
        mins[0] = 0;

        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Have a zonal inclusive selection for unstructured mesh cells "
        << "(" << mins[0] << "," << maxs[0] << " stride " << strides[0] << ") "
        << std::endl;

                                // New number of cells based on the above.
        numCells = (maxs[0]-mins[0]) / strides[0] + 1;
    }

    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "numCells = " << numCells
    << ", numVerts = " << numVerts << "." << std::endl;

    size_t datasetLength = 1;
    for (size_t i =0; i< connectivityDims.size(); ++i)
    datasetLength *= connectivityDims[i];

    // Check for connectivity list type
    if (!isIntType( connectivityMeta->getType() )) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Indices are not integers. Cleaning up" << std::endl;

        ugridPtr->Delete();

        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Returning NULL" << std::endl;

        return NULL;
    }

    // Vertices
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Allocating space for " << datasetLength
    << " integers of connectivity data." << std::endl;

    int* vertices = new int[datasetLength];
    if (!vertices) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Unable to allocate vertices.  Cleaning up." << std::endl;

        ugridPtr->Delete();

        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Returning NULL." << std::endl;
        return NULL;
    }

    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Reading connectivity list data." << std::endl;

    herr_t err;

    // Here the connections are being read. So sub select if needed.
    if( haveDataSelections )
    {
        int srcMins[1] = {mins[0]};
        int srcMaxs[1] = {numCells};
        int srcStrides[1] = {strides[0]};

        err = reader->getData( connectivityDataset, vertices,
                // -1 read all coordinate components
                unstructuredMesh->getIndexOrder(), -1,
                &(srcMins[0]), &(srcMaxs[0]), &(srcStrides[0]) );
    }
    else
    err = reader->getData(connectivityDataset, vertices);

    if (err < 0) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Call to getDataSet returned error: " << err
        << "Returning NULL." << std::endl;
        return NULL;
    }

    try {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Allocating " << numCells << " cells.  "
        << "If old VTK and this fails, it will just abort."
        << std::endl;

        ugridPtr->Allocate(numCells);
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Allocation succeeded." << std::endl;
    }
    // JRC: what goes here to detect failure to allocate?
    catch (std::bad_alloc) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Caught std::bad_alloc exception. "
        << "Unable to allocate space for cells.  Cleaning up."
        << "Returning NULL." << std::endl;
        delete [] vertices;
        ugridPtr->Delete();
        return NULL;
    } catch (...) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Unknown exception allocating cells.  Cleaning up."
        << "Returning NULL." << std::endl;
        delete [] vertices;
        ugridPtr->Delete();
        return NULL;
    }

    //Tweak for Nautilus
    //Prepare to fix up the connectivity list if node correction data is available
    int* correctionList = NULL;
    int correctionListSize = 0;
    if (unstructuredMesh->hasNodeCorrectionData()) {
        VsDataset* correctionDataset = registry->getDataset(unstructuredMesh->getNodeCorrectionDatasetName());
        if (correctionDataset) {
            correctionListSize = correctionDataset->getLength();
            int* localToGlobalNodeMapping = new int[correctionListSize];
            reader->getData(correctionDataset, localToGlobalNodeMapping);

            //Build up a list of which global ids are "taken" by more than one node
            //And when that happens, remember the local ids that need to be mapped to the original local id
            correctionList = new int[correctionListSize];
            int* globalToLocalNodeMapping = new int[correctionListSize];
            for (int i = 0; i < correctionListSize; i++) {
                correctionList[i] = -1;
                globalToLocalNodeMapping[i] = -1;
            }

            //Build up the correction list
            for (int i = 0; i < correctionListSize; i++) {
                int localId = i;
                int globalId = localToGlobalNodeMapping[i];
                if (globalToLocalNodeMapping[globalId] != -1) {
                    //VsLog::debugLog() <<"Global id " <<globalId <<" is already taken by local node " <<globalToLocalNodeMapping[globalId] <<" so we know that node " <<i <<" should be replaced by node " <<globalToLocalNodeMapping[globalId] <<std::endl;
                    correctionList[i] = globalToLocalNodeMapping[globalId];
                } else {
                    //VsLog::debugLog() <<"Local id " <<localId <<" is claiming global id " <<globalId <<std::endl;
                    globalToLocalNodeMapping[globalId] = localId;
                }
            }
        }
    }
    //end tweak

    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Inserting cells into grid." << std::endl;
    size_t k = 0;
    int warningCount = 0;
    int cellCount = 0;
    unsigned int cellVerts = 0;// cell's connected node indices
    int cellType;

    // Dealing with fixed length connectivity lists.
    if( haveConnectivityCount ) {
        cellVerts = haveConnectivityCount;
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Inserting " << cellVerts
        << " vertices into each cell." << std::endl;
    }

    for (size_t i = 0; i < numCells; ++i) {
        if (k >= datasetLength) {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
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
                    VsLog::debugLog() << CLASSFUNCLINE << "  "
                    << "Error: invalid number of vertices for cell #"
                    << cellCount << ": " << cellVerts << std::endl;
                } else if (warningCount == 30) {
                    VsLog::debugLog() << CLASSFUNCLINE << "  "
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
                if ((verts[j] < 0) || ((size_t)verts[j] >= numNodes)) {
                    VsLog::debugLog() << CLASSFUNCLINE << "  "
                    << "ERROR in connectivity dataset - requested vertex number "
                    << verts[j] << " exceeds number of vertices" << std::endl;

                    verts[j] = 0;
                }
            }

            //Tweak for Nautilus
            //Apply node corrections
            if (correctionList) {
                for (size_t j = 0; j < cellVerts; j++) {
                    if ((verts[j] >= 0) && (verts[j] < correctionListSize)) {
                        vtkIdType localVertexId = verts[j];
                        vtkIdType newId = correctionList[localVertexId];
                        if (newId != -1) {
                            //VsLog::debugLog() <<"Node #" <<localVertexId <<" should be switched to local id " <<newId <<std::endl;
                            verts[j] = newId;
                        }
                    }
                }
            }
            //end tweak

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
                    VsLog::debugLog() << CLASSFUNCLINE << "  "
                    << "WARNING: ADDING cell #"
                    << cellCount << " as cell: "
                    << vertices[k] << std::endl;
                }
                verts[j] = (vtkIdType) vertices[k++];
            }
            ugridPtr->InsertNextCell (VTK_POLYGON, numVerts, &verts[0]);
        }
    }

    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Finished.  Cleaning up." << std::endl;
    // Done, so clean up memory and return
    delete [] vertices;

    VsLog::debugLog() << CLASSFUNCLINE << "  "
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
        int* mins, int* maxs, int* strides,
        bool transform)
{
    // TODO - make "cleanupAndReturnNull" label, and do a "go to"
    // instead of just returning NULL all the time.

    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Entering function." << std::endl;
    LoadData();

    hid_t meshDataType = variableWithMesh->getType();

    // Cache the data type because it's faster
    bool isDouble = isDoubleType(meshDataType);
    bool isFloat = isFloatType(meshDataType);

    if (!isDouble && !isFloat) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Error: hdf5 mesh data type not handled: "
        << meshDataType << "Returning NULL." << std::endl;
        return NULL;
    }

    // Get the number of values
    VsLog::debugLog() << CLASSFUNCLINE << "  "
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
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Creating the vtkUnstructuredGrid." << std::endl;
    vtkUnstructuredGrid* meshPtr = vtkUnstructuredGrid::New();

    // Create and set points while small so minimal memory usage
    vtkPoints* vpts = vtkPoints::New();
    meshPtr->SetPoints(vpts);
    vpts->Delete();
    vtkPoints* vpoints = meshPtr->GetPoints();

    // Allocate
    size_t dsize = 0; (void) dsize;
    if (isDouble) {
        vpoints->SetDataTypeToDouble();
        dsize = sizeof(double);
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Double data" << std::endl;
    }
    else if (isFloat) {
        vpoints->SetDataTypeToFloat();
        dsize = sizeof(float);
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Float data" << std::endl;
    }

    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Allocating for " << numPoints << " values." << std::endl;
    vpoints->SetNumberOfPoints(numPoints);

    void* dataPtr = vpoints->GetVoidPointer(0);
    if (!dataPtr) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Unable to allocate the points.  Cleaning up."
        << "Returning NULL." << std::endl;
        meshPtr->Delete();
        return NULL;
    }

    herr_t err;

    // Read in the data
    VsLog::debugLog() << CLASSFUNCLINE << "  "
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

        VsDataset* variableDataset =
        registry->getDataset(variableWithMesh->getFullName());

        err = reader->getData(variableDataset, dataPtr,
                variableWithMesh->getIndexOrder(),
                componentIndex,
                &srcMins[0], &srcMaxs[0], &srcStrides[0],
                1, &destSize[0],
                &destMins[0], &destMaxs[0], &destStrides[0] );

        if (err < 0) {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Call to getVarWithMeshMeta returned error: "
            << err << "Returning NULL." << std::endl;
            return NULL;
        }
    }

    // The above stores the value in the correct location but make
    // sure the remaining values are all zero.
    for (int i=numSpatialDims; i<3; ++i)
    {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Zeroing data at unused positions." << std::endl;

        if (isDouble) {
            double* dblDataPtr = &(((double*) dataPtr)[i]);

            for (int j=0; j<gdims[0]; ++j)
            {
                *dblDataPtr = 0;
                dblDataPtr += 3;
            }
        }
        else if (isFloat) {
            float* fltDataPtr = &(((float*) dataPtr)[i]);

            for (int j=0; j<gdims[0]; ++j)
            {
                *fltDataPtr = 0;
                fltDataPtr += 3;
            }
        }
    }

    //If a transform is requested, do it
    //Note that we only have a 3d transform available right now
    if (transform && (numSpatialDims == 3)) {
        VsLog::debugLog() <<"Creating transformed point mesh." <<std::endl;
        if (isDouble) {
            double* dblDataPtr = (double*)dataPtr;

            for (int i = 0; i < gdims[0]; i++) {
                int baseIndex = i * 3;
                //Get original values
                double zValue = dblDataPtr[baseIndex];
                double rValue = dblDataPtr[baseIndex + 1];
                double phiValue = dblDataPtr[baseIndex + 2];

                //Calculate transform (remember, Z = Z)
                double xValue = rValue * cos(phiValue);
                double yValue = rValue * sin(phiValue);

                //Debugging output
                //VsLog::debugLog() <<"Transforming point " <<i <<" z = " <<zValue <<" r = " <<rValue <<" phi = " <<phiValue <<std::endl;
                //VsLog::debugLog() <<"Transforming point " <<i <<" x = " <<xValue <<" y = " <<yValue <<" z = "   <<zValue   <<std::endl;

                //Replace old values with new values
                dblDataPtr[baseIndex] = xValue;
                dblDataPtr[baseIndex+1] = yValue;
                dblDataPtr[baseIndex+2] = zValue;
            }
        } else if (isFloat) {
            float* fltDataPtr = (float*)dataPtr;

            for (int i = 0; i < gdims[0]; i++) {
                int baseIndex = i * 3;
                //Get original values
                float zValue = fltDataPtr[baseIndex];
                float rValue = fltDataPtr[baseIndex + 1];
                float phiValue = fltDataPtr[baseIndex + 2];

                //Calculate transform (remember, Z = Z)
                float xValue = rValue * cos(phiValue);
                float yValue = rValue * sin(phiValue);

                //Debugging output
                //VsLog::debugLog() <<"Transforming point " <<i <<" z = " <<zValue <<" r = " <<rValue <<" phi = " <<phiValue <<std::endl;
                //VsLog::debugLog() <<"Transforming point " <<i <<" x = " <<xValue <<" y = " <<yValue <<" z = "   <<zValue   <<std::endl;

                //Replace old values with new values
                fltDataPtr[baseIndex] = xValue;
                fltDataPtr[baseIndex+1] = yValue;
                fltDataPtr[baseIndex+2] = zValue;
            }
        }
    }

    // create point mesh
    try {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Allocating " << gdims[0]
        << " vertices.  If old VTK and this fails, it will just abort." << std::endl;
        meshPtr->Allocate(gdims[0]);
    } catch (std::bad_alloc) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Caught std::bad_alloc. Unable to allocate cells."
        << "Returning NULL." << std::endl;
        meshPtr->Delete();
        return NULL;
    } catch (...) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Unknown exception. Unable to allocate cells."
        << "Returning NULL." << std::endl;
        meshPtr->Delete();
        return NULL;
    }

    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Allocation succeeded.  Setting mesh to connectivity 'VERTEX'." << std::endl;
    vtkIdType vertex;
    for (int i=0; i<gdims[0]; ++i) {
        vertex = i;
        meshPtr->InsertNextCell(VTK_VERTEX, 1, &vertex);
    }

    // Clean up memory
    VsLog::debugLog() << CLASSFUNCLINE << "  "
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

    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Entering function." << std::endl;
    LoadData();

    // Save the original name in a temporary variable
    // so that we can do some string manipulation on it
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
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Looking for variable metadata." << std::endl;
    VsVariable* varMeta = registry->getVariable(name);

    if (varMeta == NULL) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "No variable metadata found under name "
        << name << "  "
        << "Looking for information in component registry."
        << std::endl;

        // Is this variable a component?  If so, swap the component name
        // with the "real" variable name and remember that it is a
        // component.
        bool isAComponent = false; (void) isAComponent;
        int componentIndex = -2;// No components
        NamePair foundName;
        registry->getComponentInfo(name, &foundName);

        if (!foundName.first.empty()) {
            name = foundName.first.c_str();
            varMeta = registry->getVariable(foundName.first);
            componentIndex = foundName.second;
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Found a component, which refers to variable "
            << name << " and index " << componentIndex << std::endl;
            isAComponent = true;
        }
    }

    if (varMeta == NULL) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Failed to find variable in component list.  "
        << "Returning NULL." << std::endl;
        return NULL;
    }

    hid_t varDataType = varMeta->getType();

    if (isDoubleType(varDataType)) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Var is 64-bit real" << std::endl;
    } else if (isFloatType(varDataType)) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Var is 32-bit real" << std::endl;
    } else if (isIntType(varDataType)) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Var is int" << std::endl;
    } else {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Var is unknown type (known are double, float, int).  "
        << "Returning NULL" << std::endl;
        return NULL;
    }

    vtkDataArray* varData = NULL;
    try {
        varData = GetVar(domain, requestedName.c_str());
    } catch (...) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Caught exception from GetVar().  Returning NULL." << std::endl;
        return NULL;
    }

    if (varData == NULL) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Failed to load var data.  Returning NULL." << std::endl;
        return NULL;
    }

    // Have the variable now get the mesh.
    std::string meshName = varMeta->getMeshName();
    VsMesh* meshMeta = varMeta->getMesh();

    if (meshMeta == NULL) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "No mesh metadata found.  Returning NULL." << std::endl;
        return NULL;
    }

    vtkDataSet* meshData = NULL;
    try {
        meshData = GetMesh(domain, meshName.c_str());
    } catch (...) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Caught exception from GetMesh().  Returning NULL."
        << std::endl;
        varData->Delete();
        return NULL;
    }

    if (meshData == NULL) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
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
    meshMeta->getCellDims(meshDims);
    int nPtsInMesh = meshDims[0];
    if (varMeta->isZonal()) {
        if (nPts != (nPtsInMesh - 1)) {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "ERROR - mesh and var dimensionalities don't match." <<std::endl;
            //Use the lower of the two values
            if (nPts > (nPtsInMesh - 1)) {
                nPtsInOutput = nPtsInMesh - 1;
            }
        }
    } else if (nPts != nPtsInMesh) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "ERROR - mesh and var dimensionalities don't match." <<std::endl;
        //Use the lower of the two values
        if (nPts > nPtsInMesh) {
            nPtsInOutput = nPtsInMesh;
        }
    }

    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Variable has " << nPts << " points." << std::endl;
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Mesh has " << nPtsInMesh << " points." << std::endl;
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Output will have " << nPtsInOutput << " points." << std::endl;

    // Create 1-D RectilinearGrid
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Building Rectilinear grid." << std::endl;
    vtkFloatArray* vals = vtkFloatArray::New();
    vals->SetNumberOfComponents(1);
    vals->SetNumberOfTuples(nPtsInOutput);
    vals->SetName(requestedName.c_str());

    vtkRectilinearGrid* rg =
    vtkVisItUtility::Create1DRGrid(nPtsInOutput, VTK_FLOAT);
    rg->GetPointData()->SetScalars(vals);

    vtkFloatArray* xc = vtkFloatArray::SafeDownCast(rg->GetXCoordinates());

    vtkRectilinearGrid* meshDataRect =
    dynamic_cast<vtkRectilinearGrid*>(meshData);

    if (meshDataRect) {
        //Handles the incoming mesh as a rectilinear mesh
        vtkDataArray* meshXCoord = meshDataRect->GetXCoordinates();

        for (int i = 0; i < nPtsInOutput; i++) {
            double* var_i = varData->GetTuple(i);
            double* mesh_i = meshXCoord->GetTuple(i);
            xc->SetValue(i, mesh_i[0]);
            vals->SetValue(i, var_i[0]);
        }
    } else {
        //Handles the incoming mesh as a structured mesh
        vtkStructuredGrid* meshDataStructuredGrid =
        dynamic_cast<vtkStructuredGrid*>(meshData);

        if (!meshDataStructuredGrid) {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Unable to extract points from mesh to construct curve.  Giving up." << std::endl;
            vals->Delete();
            return NULL;
        }

        vtkPoints* points = meshDataStructuredGrid->GetPoints();

        for (int i = 0; i < nPtsInOutput; i++) {
            double* var_i = varData->GetTuple(i);
            double* mesh_i = points->GetPoint(i);
            xc->SetValue(i, mesh_i[0]);
            vals->SetValue(i, var_i[0]);
        }
    }

    // Done, so clean up memory and return
    meshData->Delete();
    varData->Delete();
    vals->Delete();

    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Returning data." << std::endl;

    return rg;
}

bool avtVsFileFormat::nameIsComponent(std::string& name, int& componentIndex) {
    // Is this variable a component?  If so, swap the component name
    // with the "real" variable name and remember that it is a
    // component.
    bool isAComponent = false;
//int componentIndex = -2; // No components
    NamePair foundName;
    registry->getComponentInfo(name, &foundName);

    if (!foundName.first.empty()) {
        name = foundName.first.c_str();
        componentIndex = foundName.second;
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Found a component, which refers to variable "
        << name << " and index " << componentIndex << std::endl;
        isAComponent = true;
    }
    return isAComponent;
}

bool avtVsFileFormat::isTransformedName(std::string& name) {
    // Check if this var name is a transformed var name. If
    // so, use the original var name to get data associated
    bool transform = false;
    std::string origVarName = registry->getOriginalVarName(name);
    if (!origVarName.empty()) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Original var for the transformed var " << name << " is "
        << origVarName << std::endl;
        transform = true;
        name = origVarName;
    } else {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Unable to find an original var for the possible transform name " <<name <<std::endl;
    }

    return transform;
}

vtkDataArray* avtVsFileFormat::StandardVar(int domain, const char* requestedName) {
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Entering function." << std::endl;
    LoadData();

    // Save the original name in a temporary variable
    // so that we can do some string manipulation on it
    std::string name = requestedName;

    bool haveDataSelections;
    int mins[3], maxs[3], strides[3];

    // Adjust for the data selections which are NODAL (typical) or ZONAL.
    if( (haveDataSelections = ProcessDataSelections(mins, maxs, strides)) ) ///TODO: check on fix for assignment
    {
        VsLog::debugLog()
        << CLASSFUNCLINE << "  "
        << "have a data selection for a variable "
        << "(" << mins[0] << "," << maxs[0] << " stride " << strides[0] << ") "
        << "(" << mins[1] << "," << maxs[1] << " stride " << strides[1] << ") "
        << "(" << mins[2] << "," << maxs[2] << " stride " << strides[2] << ") "
        << std::endl;
    }

    int componentIndex = -2;
    bool isAComponent = nameIsComponent(name, componentIndex);
    bool transform = isTransformedName(name); (void) transform;

    // The goal in all of the metadata loading is to fill one of
    // these two variables:
    VsVariable* meta = NULL;
    VsVariableWithMesh* vmMeta = NULL;
    VsDataset* variableDataset = NULL;

    // It could be an MD variable if so, retrieve the md metadata,
    // look up the "real" variable name using the domain number, and
    // replace "name" with the name of that variable.
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Checking for possible MD var." << std::endl;
    VsMDVariable* mdMeta = registry->getMDVariable(name);
    if (mdMeta) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Found MD metadata for this name: "
        << name << std::endl;

        if (0 <= domain && (size_t)domain < mdMeta->getNumBlocks()) {
            meta = mdMeta->getBlock(domain);
            name = meta->getFullName();

            if( meta )
            variableDataset = registry->getDataset(meta->getFullName());

        } else {
            VsLog::warningLog()
            << CLASSFUNCLINE << "  "
            << "Requested domain number is out of bounds for this variable."
            << std::endl;
        }
    }

    // No meta data so, look for a "regular" variable with this name.
    if (meta == NULL) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Looking for regular (non-md) variable." << std::endl;
        meta = registry->getVariable(name);

        if( meta )
        variableDataset = registry->getDataset(meta->getFullName());
    }

    // No meta data so, look for a VarWithMesh with this name.
    if (meta == NULL) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Looking for VarWithMesh variable." << std::endl;
        vmMeta = registry->getVariableWithMesh(name);

        if( vmMeta )
        variableDataset = registry->getDataset(vmMeta->getFullName());
    }

    // Haven't found metadata yet, so give up.
    if ((meta == NULL) && (vmMeta == NULL)) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "ERROR: Could not find metadata for name: "
        << name << std::endl;
        // DEBUG CODE
        //       std::vector<std::string> varNames;
        //       registry->getAllVariableNames(varNames);
        //       VsLog::debugLog() << CLASSFUNCLINE << "  "
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
    bool parallelRead = true; (void) parallelRead;
    bool hasOffset = false;
    std::vector<double> offset(3, 0);
    std::string indexOrder;

    size_t numTopologicalDims;

    // VarWithMesh variable - Note that there is no mesh metadata for
    // VarWithMesh.
    if (vmMeta) {
        varDims = vmMeta->getDims();
        varType = vmMeta->getType();

        isZonal = true;

        indexOrder = vmMeta->getIndexOrder();
        isCompMajor = vmMeta->isCompMajor();

        numTopologicalDims = 1;
    }

    else {
        varDims = meta->getDims();
        varType = meta->getType();

        if (meta->isFortranOrder())
        isFortranOrder = true;

        if (meta->isZonal())
        isZonal = true;

        if (meta->hasNodeOffset()) {
            hasOffset = true;
            offset = meta->getNodeOffset();
        }

        indexOrder = meta->getIndexOrder();
        isCompMajor = meta->isCompMajor();

        std::string meshName = meta->getMeshName();
        VsMesh* meshMetaPtr = registry->getMesh(meshName);

        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Mesh for variable is '" << meshName << "'." << std::endl;
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Getting metadata for mesh." << std::endl;

        if (!meshMetaPtr) {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Metadata not found for mesh '" << meshName
            << "'." << std::endl;
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Returning NULL" << std::endl;
            return NULL;
        }

        std::vector<int> meshDims;
        meshMetaPtr->getCellDims(meshDims);
        numTopologicalDims = meshDims.size();

        // Structured and unstructured mesh dimension contain the
        // spatial dimension so subtract it off.
        if( meshMetaPtr->isStructuredMesh() || meshMetaPtr->isUnstructuredMesh() )
        numTopologicalDims -= 1;

        if (varDims.size() - (int) isAComponent != numTopologicalDims )
        {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "WARNING: We don't think that var and mesh have the same topological dimension." <<std::endl;
            //                          << "Returning NULL." << std::endl;
            //return NULL;
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
    bool isInteger = isIntType(varType);

    if (!isDouble && !isFloat && !isInteger) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Error: hdf5 variable data type not handled: "
        << varType << "Returning NULL." << std::endl;
        return NULL;
    }

    VsLog::debugLog() << CLASSFUNCLINE << "  "
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

    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Total number of variables is " << numVariables
    << std::endl;

    size_t varSize = H5Tget_size(varType); (void) varSize;

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
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Allocation failed, pointer is NULL.  "
        << "Returning NULL." << std::endl;
        return NULL;
    }

    // Variable and mesh data or a variable component.
    if (vmMeta || isAComponent)
    {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Entering VarWithMesh section." << std::endl;

        herr_t err = reader->getData(variableDataset, dataPtr,
                indexOrder, componentIndex,
                mins, &(vdims[0]), strides);

        if (err < 0) {
            VsLog::debugLog()
            << CLASSFUNCLINE << "  "
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
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Loading variable data." << std::endl;

        herr_t err;

        if( haveDataSelections )
        err = reader->getData( variableDataset, dataPtr,
                meta->getIndexOrder(), -2, // -2 no components
                mins, &(vdims[0]), strides );
        else
        err = reader->getData( variableDataset, dataPtr );

        if (err < 0) {
            VsLog::debugLog()
            << CLASSFUNCLINE << "  "
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

    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Finished reading the data, building VTK structures." << std::endl;

    //DEBUG
    /*
     VsLog::debugLog() << CLASSFUNCLINE << "  "
     << "Dumping data: " << std::endl;
     for (int i = 0; i < numVariables; i++) {
     if (H5Tequal(type, H5T_NATIVE_DOUBLE)) {
     VsLog::debugLog() << "data[" << i << "] = " << ((double*)data)[i] << std::endl;}
     else if (H5Tequal(type, H5T_NATIVE_FLOAT)) {
     VsLog::debugLog() << "data[" << i << "] = " << ((float*)data)[i] << std::endl;}
     else if (H5Tequal(type, H5T_NATIVE_INT)) {
     VsLog::debugLog() << "data[" << i << "] = " << ((int*)data)[i] << std::endl;}
     }
     VsLog::debugLog() << CLASSFUNCLINE << "  "
     << "Finished dumping data. " << std::endl;
     */
    //END DEBUG
    vtkDataArray* rv = 0;

    if (isDouble) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Declaring vtkDoubleArray." << std::endl;
        rv = vtkDoubleArray::New();
    }
    else if (isFloat) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Declaring vtkFloatArray." << std::endl;
        rv = vtkFloatArray::New();
    }
    else if (isInteger) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Declaring vtkIntArray." << std::endl;
        rv = vtkIntArray::New();
    }

    //Attach offset information to array
    if (hasOffset) {
        vtkInformation* info = rv->GetInformation();
        info->Set(avtVariableCache::OFFSET_3(), offset[0], offset[1], offset[2]);
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        <<"Attaching nodeOffset information to dataset: " <<requestedName <<std::endl;
    }

    rv->SetNumberOfTuples(numVariables);

    // If in C order, perform permutation of index as VTK expect Fortran order

    // The index tuple is initially all zeros
    size_t* indices = new size_t[numTopologicalDims];
    for (size_t k=0; k<numTopologicalDims; ++k)
    indices[k] = 0;

    // Store data
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Storing " << numVariables << " data elements" << std::endl;

    // Attempt to reverse data in place
    //#define IN_PLACE
#ifdef IN_PLACE
    VsLog::debugLog() << CLASSFUNCLINE << "  "
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
                intDataPtr[indx] = intDataPtr[k];
                intDataPtr[k] = tmp;
            } else {
                VsLog::debugLog() << CLASSFUNCLINE << "  "
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
            rv->SetTuple(k, &intDataPtr[k]);
        }
    } else {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Unknown data type: " << type << std::endl;
        return NULL;
    }

    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Done swapping data in place." << std::endl;
#else

    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Swapping data into correct places, NOT using 'in place' code."
    << std::endl;

    // Using FORTRAN data ordering.
    if (isFortranOrder) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Using FORTRAN data ordering." << std::endl;

        if (isDouble) {
            for (size_t k = 0; k<(size_t)numVariables; ++k) {
                rv->SetTuple(k, &dblDataPtr[k]);
            }
        } else if (isFloat) {
            for (size_t k = 0; k<(size_t)numVariables; ++k) {
                rv->SetTuple(k, &fltDataPtr[k]);
            }
        } else if (isInteger) {
            for (size_t k = 0; k<(size_t)numVariables; ++k) {
                // Convert to float because SetTuple doesn't take ints
                float temp = intDataPtr[k];
                rv->SetTuple(k, &temp);
            }
        }
    } else {

        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Using C data ordering." << std::endl;

        // Step through by global C index
        for (size_t k = 0; k<(size_t)numVariables; ++k) {

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
                if (indices[j] == (size_t)vdims[j])
                indices[j] = 0;
                else
                break;
            }while (j != 0);
        }
    }

    VsLog::debugLog() << CLASSFUNCLINE << "  "
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

    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Returning data." << std::endl;
    return rv;
}

VsVariable* avtVsFileFormat::getVariableMeta(int domain, std::string requestedName, int &componentIndex) {

// Save the original name in a temporary variable
    // so that we can do some string manipulation on it
    std::string name = requestedName;

    componentIndex = -2;
    bool isAComponent = nameIsComponent(name, componentIndex); (void) isAComponent;
    bool transform = isTransformedName(name); (void) transform;

    // The goal in all of the metadata loading is to fill one of
    // these two variables:
    VsVariable* meta = NULL;
    VsVariableWithMesh* vmMeta = NULL;
    VsDataset* variableDataset = NULL; (void) variableDataset;

    // It could be an MD variable if so, retrieve the md metadata,
    // look up the "real" variable name using the domain number, and
    // replace "name" with the name of that variable.
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Checking for possible MD var." << std::endl;
    VsMDVariable* mdMeta = registry->getMDVariable(name);
    if (mdMeta) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Found MD metadata for this name: "
        << name << std::endl;

        if (0 <= domain && (size_t)domain < mdMeta->getNumBlocks()) {
            meta = mdMeta->getBlock(domain);
            name = meta->getFullName();

            if( meta )
            variableDataset = registry->getDataset(meta->getFullName());

        } else {
            VsLog::warningLog()
            << CLASSFUNCLINE << "  "
            << "Requested domain number is out of bounds for this variable."
            << std::endl;
        }
    }

    // No meta data so, look for a "regular" variable with this name.
    if (meta == NULL) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Looking for regular (non-md) variable." << std::endl;
        meta = registry->getVariable(name);

        if( meta )
        variableDataset = registry->getDataset(meta->getFullName());
    }

    // No meta data so, look for a VarWithMesh with this name.
    if (meta == NULL) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Looking for VarWithMesh variable." << std::endl;
        vmMeta = registry->getVariableWithMesh(name);

        if( vmMeta )
        variableDataset = registry->getDataset(vmMeta->getFullName());
    }

    // Haven't found metadata yet, so give up.
    if ((meta == NULL) && (vmMeta == NULL)) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "ERROR: Could not find metadata for name: "
        << name << std::endl;
        return NULL;
    }

    return meta;
}

vtkDataArray* avtVsFileFormat::NodalVar(VsVariable* meta, std::string name, int component) {
    VsLog::debugLog() << CLASSFUNCLINE << "  " << "entering." << std::endl;
    LoadData();
    thisData.setReader(reader);
    thisData.setRegistry(registry);
    return thisData.getData(meta, name, component);
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
    VsLog::debugLog() << CLASSFUNCLINE << "  " << "entering." << std::endl;
    int component;
    VsVariable* meta = getVariableMeta(domain, requestedName, component);
    if(meta!=NULL) {
        if(meta->getMesh()->isHighOrder()) {
            VsLog::debugLog() << CLASSFUNCLINE << "  " << "exiting Nodal case." << std::endl;
            return NodalVar(meta, requestedName, component);
        }
    }
    VsLog::debugLog() << CLASSFUNCLINE << "  " << "exiting Standard case." << std::endl;
    return StandardVar(domain, requestedName);
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
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Entering/Exiting function." << std::endl;
    if (reader != NULL) {
        delete reader;
        reader = NULL;
    }

    if (registry) {
        delete registry;
        registry = NULL;
    }

    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "exiting" << std::endl;

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
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Entering function." << std::endl;
    LoadData();

    //get list of expressions from reader
    std::map<std::string, std::string>* expressions =
    registry->getAllExpressions();

    if (expressions->empty()) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "WARNING: No expressions found in file. Returning."
        << std::endl;
        return;
    } else {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Found " << expressions->size()
        << " expressions in file." << std::endl;
    }

    //iterate over list of expressions, insert each one into database
    std::map<std::string, std::string>::const_iterator iv;
    for (iv = expressions->begin(); iv != expressions->end(); ++iv) {

        if (iv->first == "Rho") {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Ignoring expression " << iv->first << " = "
            << iv->second << " because it crashesVisIt." <<std::endl;
            continue;
        }

        VsLog::debugLog() << CLASSFUNCLINE << "  "
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
            << CLASSFUNCLINE << "  "
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
                    VsLog::debugLog() << CLASSFUNCLINE << "  "
                    <<"It is a curve expression." << std::endl;
                    VsLog::debugLog() << CLASSFUNCLINE << "  "
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
            << CLASSFUNCLINE << "  "
            << "It is a scalar expression." << std::endl;
        }

        //Add the new expression to the metadata object
        md->AddExpression(&e);
    }

    VsLog::debugLog() << CLASSFUNCLINE << "  "
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
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Entering function." << std::endl;
    LoadData();

    // Get var names
    std::vector<std::string> names;
    registry->getAllVariableNames(names);

    if (names.empty()) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "WARNING: No variables were found in this file.  Returning." << std::endl;
        return;
    } else {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Found " << names.size()
        << " variables in this file." << std::endl;
    }

    std::vector<std::string>::const_iterator it;
    for (it = names.begin(); it != names.end(); ++it) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Processing var: "<< *it << std::endl;

        //get metadata for var
        VsVariable* vMeta = registry->getVariable(*it);

        //If this var is part of an MD var, we don't register it separately
        VsVariable* testForMD = registry->findSubordinateMDVar(*it);
        if (testForMD) {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Var is part of an MD variable, "
            << "will not be registered separately." << std::endl;
            continue;
        }

        // Name of the mesh of the var
        std::string meshName = vMeta->getMeshName();
        VsMesh* meshMeta = vMeta->getMesh();

        // Name of the transformed mesh (if it exists)
        //bool hasTransform = false;
        std::string transformMeshName = "";
        //VsRectilinearMesh* rectMesh = static_cast<VsRectilinearMesh*> (meshMeta);
        if (meshMeta && meshMeta->hasTransform()) {
            transformMeshName = meshMeta->getTransformedMeshName();
            VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")"
            << __FUNCTION__ << "  " << __LINE__ << "  "
            << "Variable lives on a mesh with a transform: "
            <<transformMeshName
            << std::endl;
        }

        // Centering of the variable
        // Default is node centered data
        avtCentering centering = AVT_NODECENT;
        if (vMeta->isZonal()) {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Var is zonal." << std::endl;
            centering = AVT_ZONECENT;
        } else {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Var is nodal." << std::endl;
        }

        // 1-D variable?
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Determining if var is 1-D." << std::endl;
        int numSpatialDims = 3;
        if (meshMeta) {
            numSpatialDims = meshMeta->getNumSpatialDims();
        } else {
            numSpatialDims = 3;
            VsLog::errorLog() << CLASSFUNCLINE << "  "
            << "Unable to load mesh for variable?" << std::endl;
            VsLog::errorLog() << CLASSFUNCLINE << "  "
            << "Using numSpatialDims = 3, hope it's right!"
            << std::endl;
        }

        //if this mesh is 1-D, we leave it for later (curves)
        bool isOneDVar = false;
        if (numSpatialDims == 1) {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Var is 1-D." << std::endl;
            isOneDVar = true;
        } else {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Var is not 1-D." << std::endl;
            isOneDVar = false;
        }

        // Number of component of the var
        size_t numComps = vMeta->getNumComps();

        if (numComps == 0) continue;

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
                        VsLog::debugLog() << CLASSFUNCLINE << "  "
                        << "Adding curve component "
                        << componentName << "." << std::endl;
                        avtCurveMetaData* cmd =
                        new avtCurveMetaData(componentName.c_str());
                        curveNames.push_back(componentName);
                        cmd->hasDataExtents = false;
                        md->Add(cmd);
                    } else {
                        VsLog::debugLog() << CLASSFUNCLINE << "  "
                        << "Unable to find match for curve variable in component registry."
                        << std::endl;
                    }
                }
            } else {
                //When there is only one component, we don't create a component name
                //Instead, we just use the straight-up name
                VsLog::debugLog() << "Creating a curve." << std::endl;
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
                    VsLog::debugLog() << CLASSFUNCLINE << "  "
                    << "Adding variable component " << componentName << "." << std::endl;
                    avtScalarMetaData* smd = new avtScalarMetaData(componentName, meshName.c_str(), centering);
                    smd->hasUnits = false;
                    md->Add(smd);

                    if (vMeta->hasTransform()) {
                        std::string transformVarName = vMeta->getFullTransformedName();
                        std::string transformComponentName = registry->getComponentName(transformVarName, i);
                        if (!transformComponentName.empty()) {
                            VsLog::debugLog() << CLASSFUNCLINE << "  "
                            << "Registering transformed component: " <<transformVarName <<std::endl;
                            avtScalarMetaData* smd_transformed =
                            new avtScalarMetaData(transformComponentName.c_str(), transformMeshName.c_str(), centering);
                            smd_transformed->hasUnits = false;
                            md->Add(smd_transformed);
                        }
                    }

                } else {
                    VsLog::debugLog() << CLASSFUNCLINE << "  "
                    << "Unable to find match for variable "
                    << *it << "in component (" << i << ") registry." << std::endl;
                }
            }
        }
        else if (numComps == 1) {
            VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")"
            << __FUNCTION__ << "  " << __LINE__ << "  "
            << "Adding single-component variable "
            << *it << " attached to mesh "
            << meshName << " with centering "
            << centering << std::endl;
            avtScalarMetaData* smd =
            new avtScalarMetaData(*it, meshName.c_str(), centering);
            smd->hasUnits = false;
            md->Add(smd);

            if (vMeta->hasTransform()) {
                std::string transformVarName = vMeta->getFullTransformedName();
                avtScalarMetaData* smd_transformed =
                new avtScalarMetaData(transformVarName.c_str(), transformMeshName.c_str(), centering);
                VsLog::debugLog() << CLASSFUNCLINE << "  "
                << "Registering transformed variable: " <<transformVarName <<std::endl;
                smd_transformed->hasUnits = false;
                md->Add(smd_transformed);
            }

        }
        else {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Variable '" << *it
            << "' has no components. Not being added." << std::endl;
        }
    }

    VsLog::debugLog() << CLASSFUNCLINE << "  "
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
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Entering function." << std::endl;
    LoadData();

    // Number of mesh dims
    int spatialDims;
    int topologicalDims; (void) topologicalDims;

    // All meshes names
    std::vector<std::string> names;
    registry->getAllMeshNames(names);
    if (names.empty()) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "WARNING: no meshes were found in this file. Returning." << std::endl;
        return;
    } else {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Found " << names.size() << " meshes." << std::endl;
    }

    std::vector<std::string>::const_iterator it;
    for (it = names.begin(); it != names.end(); ++it) {

        VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")"
        << __FUNCTION__ << "  " << __LINE__ << "  "
        << "now registering mesh " << *it << std::endl;

        VsMesh* meta = registry->getMesh(*it);

        spatialDims = meta->getNumSpatialDims();

        //if this mesh is 1-D, we leave it for later (curves)
        if (spatialDims == 1) {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Found 1-d mesh.  Skipping for now, will be added as a curve."
            << std::endl;
            continue;
        } else if ((spatialDims <= 0) || (3 < spatialDims)) {
            VsLog::errorLog() << CLASSFUNCLINE << "  "
            << "NumSpatialDims is out of range: "
            << spatialDims << "  "
            << "Skipping mesh." << std::endl;
            continue;
        }

        //If this mesh is part of an MD mesh, we don't register it separately
        VsMesh* testForMD = registry->findSubordinateMDMesh(*it);
        if (testForMD) {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Mesh is part of an MD mesh, will not be registered separately."
            << std::endl;
            continue;
        }

        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Found mesh '" << *it << "' of kind '"
        << meta->getKind() << "'." << std::endl;

        avtMeshType meshType = AVT_UNKNOWN_MESH;
        std::vector<int> dims;
        int bounds[3] = {1,1,1};
        int numCells = 1;

        //std::cout << "isDgMesh is " << meta->isDgMesh() << "\n";

        // Uniform Mesh
        if (meta->isUniformMesh()) {
            // 09.06.01 Marc Durant We used to report uniform cartesian
            // meshes as being 3d no matter what I have changed this to
            // use the correct mesh dimensions The changed plugin passes
            // vstests, and is motivated because the VisIt Lineout
            // operator requires 2-d data.  EXCEPT! then we can't plot 3-d
            // std::vectors on the 2-d data, so for now we continue to report 3
            // spatialDimss = dims.size();
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Mesh's dimension = " << spatialDims << std::endl;
            // 11.08.01 MDurant We are no longer going to do this.  2d is 2d.
            //if (spatialDims != 3) {
            //  VsLog::debugLog() << CLASSFUNCLINE << "  "
            //                    << "But reporting as dimension 3 to side-step VisIt bug."
            //                    << std::endl;
            //  spatialDims = 3;
            //}

            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Adding uniform mesh " << *it << "." << std::endl;

            meshType = AVT_RECTILINEAR_MESH;

            // Add in the logical bounds of the mesh.
            static_cast<VsUniformMesh*>(meta)->getCellDims(dims);
            for( size_t i=0; i<dims.size(); ++i )
            {
                bounds[i] = dims[i]; // Logical bounds are node centric.
                numCells *= (dims[i]-1);
            }
        }

        // Rectilinear Mesh
        else if (meta->isRectilinearMesh()) {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Adding rectilinear mesh" << *it << ".  "
            << "spatialDims = " << spatialDims << "." << std::endl;

            VsRectilinearMesh* rectMesh = static_cast<VsRectilinearMesh*>(meta);
            meshType = AVT_RECTILINEAR_MESH;

            // Add in the logical bounds of the mesh.
            rectMesh->getCellDims(dims);
            for( size_t i=0; i<dims.size(); ++i )
            {
                bounds[i] = dims[i]; // Logical bounds are node centric.
                numCells *= (dims[i]-1);
            }

            // Roopa: Check if there is a transformation specified for this
            // mesh. If so, register the transformed mesh here. The
            // original mesh will also be registered later by default
            if (rectMesh->hasTransform()) {
                VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")"
                << __FUNCTION__ << "  " << __LINE__ << "  "
                << "Registering transform for rectilinear mesh" << *it << "." <<std::endl;
                std::string transformedMeshName = rectMesh->getTransformedMeshName();
                VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")"
                << __FUNCTION__ << "  " << __LINE__ << "  "
                << "Adding rectilinear mesh" << *it << ".  "
                << "transformedMeshName = " << transformedMeshName
                << "." << std::endl;

                int topologicalDims = meta->getNumTopologicalDims();
                // Add a note for this interesting case.  It is legal, but since it's a new feature
                // we want to keep an eye on it
                if (topologicalDims > spatialDims) {
                    VsLog::errorLog() <<CLASSFUNCLINE << "  "
                    <<"ERROR - num topological dims (" << topologicalDims
                    <<") > num spatial dims (" << spatialDims <<")" <<std::endl;
                    topologicalDims = spatialDims;
                } else if (spatialDims != topologicalDims) {
                    VsLog::debugLog() <<CLASSFUNCLINE << "  "
                    <<"Interesting - num topological dims (" << topologicalDims
                    <<") != num spatial dims (" << spatialDims <<")" <<std::endl;
                }

                avtMeshMetaData* vmd =
                new avtMeshMetaData(transformedMeshName, 1, 1, 1, 0,
                        spatialDims, topologicalDims, meshType);
                vmd->SetBounds( bounds );
                vmd->SetNumberCells( numCells );
                setAxisLabels(vmd);
                md->Add(vmd);
            }
        }
        // Structured Mesh
        else if (meta->isStructuredMesh()) {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Adding structured mesh " << *it << "." << std::endl;

            meshType = AVT_CURVILINEAR_MESH;

            // Add in the logical bounds of the mesh.
            static_cast<VsRectilinearMesh*>(meta)->getCellDims(dims);
            for( size_t i=0; i<dims.size(); ++i )
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
                VsLog::debugLog() << CLASSFUNCLINE << "  "
                << "Registering mesh " << it->c_str()
                << " as AVT_POINT_MESH" << std::endl;

                meshType = AVT_POINT_MESH;
                topologicalDims = 1;

                // Add in the logical bounds of the mesh.
                numCells = unstructuredMesh->getNumPoints();
            }
            else {
                VsLog::debugLog() << CLASSFUNCLINE << "  "
                << "Registering mesh " << it->c_str()
                << " as AVT_UNSTRUCTURED_MESH" << std::endl;

                spatialDims = meta->getNumSpatialDims();
                VsLog::debugLog() << CLASSFUNCLINE << "  "
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

        /*else if (meta->isHighOrderUnstructuredMesh()) {
         VsHighOrderUnstructuredMesh* dgMesh = (VsHighOrderUnstructuredMesh*)meta;

         // Unstructured meshes without connectivity data are registered
         // as point meshes

         VsLog::debugLog() << CLASSFUNCLINE << "  "
         << "Registering mesh " << it->c_str()
         << " as AVT_UNSTRUCTURED_MESH" << std::endl;

         spatialDims = meta->getNumSpatialDims();
         VsLog::debugLog() << CLASSFUNCLINE << "  "
         << "Adding high order unstructured mesh " << *it
         << " with " << spatialDims
         << " spatial dimensions." << std::endl;

         meshType = AVT_UNSTRUCTURED_MESH;

         // For now users can have only one connectivity dataset.
         if( dgMesh->getLinesDataset() ||
         dgMesh->getPolygonsDataset() ||
         dgMesh->getTrianglesDataset() ||
         dgMesh->getQuadrilateralsDataset() ) {
         topologicalDims = 2;
         } else if( dgMesh->getPolyhedraDataset() ||
         dgMesh->getTetrahedralsDataset() ||
         dgMesh->getPyramidsDataset() ||
         dgMesh->getPrismsDataset() ||
         dgMesh->getHexahedralsDataset() ) {
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

         // Add in the logical bounds of the mesh.  Number of cells
         // before each cell is triangulated
         numCells = dgMesh->getNumCells();

         bounds[0] = numCells;
         bounds[1] = bounds[2] = 0;
         }*/

        else {

            meshType = AVT_UNKNOWN_MESH;

            VsLog::debugLog() << CLASSFUNCLINE << "  "
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
                VsLog::errorLog() <<CLASSFUNCLINE << "  "
                <<"ERROR - num topological dims (" << topologicalDims
                <<") > num spatial dims (" << spatialDims <<")" <<std::endl;
                topologicalDims = spatialDims;
            } else if (spatialDims != topologicalDims) {
                VsLog::debugLog() <<CLASSFUNCLINE << "  "
                <<"Interesting - num topological dims (" << topologicalDims
                <<") != num spatial dims (" << spatialDims <<")" <<std::endl;
            }

            avtMeshMetaData* vmd =
            new avtMeshMetaData(it->c_str(), 1, 1, 1, 0,
                    spatialDims, topologicalDims, meshType);
            vmd->SetBounds( bounds );
            vmd->SetNumberCells( numCells );
            setAxisLabels(vmd);
            md->Add(vmd);

        }
    }

    VsLog::debugLog() << CLASSFUNCLINE << "  "
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
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Entering function." << std::endl;
    LoadData();

    // Get vars names
    std::vector<std::string> names;
    registry->getAllMDVariableNames(names);

    if (names.empty()) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "WARNING: No MD variables were found in this file.  Returning."
        << std::endl;
        return;
    } else {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Found " << names.size()
        << " MD variables in this file." << std::endl;
    }

    std::vector<std::string>::const_iterator it;
    for (it = names.begin(); it != names.end(); ++it) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Processing md var '"
        << *it << "'." << std::endl;
        VsMDVariable* vMeta = registry->getMDVariable(*it);

        // Name of the mesh of the var
        std::string mesh = vMeta->getMesh();
        std::string vscentering = vMeta->getCentering();
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "MD var lives on mesh " << mesh << "." << std::endl;

        //TODO: Mesh should either exist in an mdMesh, or separately in
        //the list of meshes

        // Centering of the variable
        // Default is node centered data
        avtCentering centering = AVT_NODECENT;
        if (vMeta->isZonal()) {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Var is zonal" << std::endl;
            centering = AVT_ZONECENT;
        } else {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Var is nodal" << std::endl;
        }

        // Number of component of the var
        size_t numComps = vMeta->getNumComps();

        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Variable has " << numComps
        << " components." << std::endl;

        if (numComps > 1) {
            for (size_t i = 0; i<numComps; ++i) {
                //first, get a unique name for this component
                std::string compName = registry->getComponentName(*it, i);

                if (!compName.empty()) {
                    VsLog::debugLog() << CLASSFUNCLINE << "  "
                    << "Adding variable component " << compName << "." << std::endl;
                    avtScalarMetaData* smd = new avtScalarMetaData(compName.c_str(),
                            mesh.c_str(), centering);
                    smd->hasUnits = false;
                    md->Add(smd);
                } else {
                    VsLog::debugLog() << CLASSFUNCLINE << "  "
                    << "Unable to find component name for var "
                    << *it << " and index " << i << std::endl;
                }
            }
        }
        else if (numComps == 1) {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Adding single variable component " << *it
            << "." << std::endl;
            avtScalarMetaData* smd =
            new avtScalarMetaData(*it, mesh.c_str(), centering);
            smd->hasUnits = false;
            md->Add(smd);
        } else {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Variable '" << *it
            << "' has no components. Not being added." << std::endl;
        }
    }
    VsLog::debugLog() << CLASSFUNCLINE << "  "
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
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Entering function." << std::endl;
    LoadData();

    std::vector<std::string> names;
    registry->getAllMDMeshNames(names);
    if (names.empty()) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "WARNING: no md meshes were found in this file. Returning" << std::endl;
        return;
    } else {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Found " << names.size()
        << " MD meshes in this file." << std::endl;
    }

    std::vector<std::string>::const_iterator it;
    for (it = names.begin(); it != names.end(); ++it) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << " Adding md mesh '"
        << *it << "'." << std::endl;

        VsMDMesh* meta = registry->getMDMesh(*it);
        if (!meta) {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Unable to find mesh " << *it << std::endl;
            continue;
        }

        avtMeshType meshType = AVT_RECTILINEAR_MESH; ///TODO: check fix for uninitialized value
        std::string kind = meta->getMeshKind();
        if (meta->isUniformMesh()) {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Mesh is rectilinear" << std::endl;
            meshType = AVT_RECTILINEAR_MESH;
        } else if (meta->isUnstructuredMesh()) {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Mesh is unstructured" << std::endl;
            meshType = AVT_UNSTRUCTURED_MESH;
        } else if (meta->isStructuredMesh()) {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Mesh is structured" << std::endl;
            meshType = AVT_CURVILINEAR_MESH;
        }

        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Mesh has dimension " << meta->getNumSpatialDims()
        << "." << std::endl;

        avtMeshMetaData* vmd =
        new avtMeshMetaData(it->c_str(), meta->getNumBlocks(), 1, 1, 0,
                meta->getNumSpatialDims(),
                meta->getNumSpatialDims(), meshType);
        setAxisLabels(vmd);
        md->Add(vmd);
    }

    VsLog::debugLog() << CLASSFUNCLINE << "  "
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
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Entering function." << std::endl;
    LoadData();

    std::vector<std::string> names;
    registry->getAllVariableWithMeshNames(names);

    if (names.empty()) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "WARNING: no variables with mesh were found in this file. Returning."
        << std::endl;
        return;
    } else {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Found " << names.size()
        << " variables with mesh in this file." << std::endl;
    }

    std::vector<std::string>::const_iterator it;
    for (it = names.begin(); it != names.end(); ++it) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Processing varWithMesh '"
        << *it << "'." << std::endl;
        VsVariableWithMesh* vMeta = registry->getVariableWithMesh(*it);

        // add var components
        std::vector<int> dims;
        vMeta->getCellDims(dims);
        if (dims.size() <= 0) {
            std::ostringstream msg;
            msg << CLASSFUNCLINE << "  "
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
            VsLog::debugLog() << CLASSFUNCLINE << "  "
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
                VsLog::debugLog() << CLASSFUNCLINE << "  "
                << "Adding variable component " << compName << "." << std::endl;
                avtScalarMetaData* smd = new avtScalarMetaData(compName.c_str(), it->c_str(), AVT_NODECENT);
                smd->hasUnits = false;
                md->Add(smd);

                // Register the transformed var (if this variable has a transform)
                if (vMeta->hasTransform()) {
                    std::string transformMeshName = vMeta->getTransformedMeshName();
                    std::string transformVarName = vMeta->getFullTransformedName();
                    std::string transformComponentName = registry->getComponentName(transformVarName, i);
                    VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")"
                    << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Registering a transformed component: "
                    <<transformComponentName << std::endl;
                    avtScalarMetaData* smd_transformed =
                    new avtScalarMetaData(transformComponentName.c_str(), transformMeshName.c_str(), AVT_NODECENT);
                    smd_transformed->hasUnits = false;
                    md->Add(smd_transformed);
                }
            } else {
                VsLog::debugLog() << CLASSFUNCLINE << "  "
                << "Unable to get component name for variable "
                << *it << " and index " << i << std::endl;
            }
        }

        // This loop registers all of the MESHES
        // Add in the logical bounds of the mesh.
        int numCells = vMeta->getNumPoints();
        int bounds[3] = {numCells,0,0};
        int spatialDims = vMeta->getNumSpatialDims();
        if (spatialDims == 1) {
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            <<"Found 1-d var with mesh, artificially elevating it to 2-d." <<std::endl;
            spatialDims = 2;
        }
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Adding point mesh for this variable." << std::endl;
        avtMeshMetaData* vmd = new avtMeshMetaData(it->c_str(),
                1, 1, 1, 0, spatialDims, 0, AVT_POINT_MESH);
        vmd->SetBounds( bounds );
        vmd->SetNumberCells( numCells );
        setAxisLabels(vmd);
        md->Add(vmd);

        // Register the transformed mesh (if this variable has a transform)
        if (vMeta->hasTransform()) {
            std::string transformMeshName = vMeta->getTransformedMeshName();
            VsLog::debugLog() << __CLASS__ <<"(" <<instanceCounter <<")"
            << __FUNCTION__ << "  " << __LINE__ << "  "
            << "Variable lives on a mesh with a transform: "
            <<transformMeshName
            << std::endl;
            avtMeshMetaData* vmd_transform = new avtMeshMetaData(transformMeshName.c_str(),
                    1, 1, 1, 0, spatialDims, 0, AVT_POINT_MESH);
            vmd_transform->SetBounds(bounds);
            setAxisLabels(vmd_transform);
            md->Add(vmd_transform);
        }
    }

    VsLog::debugLog() << CLASSFUNCLINE << "  "
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
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Entering function." << std::endl;
    LoadData();

    if (!md) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
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
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    <<"Last slash is " <<lastSlash << std::endl;
    if (lastSlash != -1) {
        int nameLength = fileName.length() - (lastSlash + 1);
        fileName = fileName.substr(lastSlash + 1, nameLength);
    }
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    <<"Extracted filename is \"" <<fileName <<"\"" << std::endl;

    // Timestep = the number between the last underscore and the first dot
    int lastUnderscore = fileName.find_last_of('_');
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    <<"lastUnderscore is " <<lastUnderscore << std::endl;

    int firstDot = fileName.find_first_of('.');
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    <<"firstDot is " <<firstDot << std::endl;

    if ((lastUnderscore != -1) &&
            (firstDot != -1) &&
            (firstDot > lastUnderscore + 1)) {
        std::string step =
        fileName.substr(lastUnderscore + 1, firstDot - (lastUnderscore + 1));

        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Step is \"" << step << "\""
        << std::endl;

        timeStep = atoi(step.c_str());

        VsLog::debugLog() << CLASSFUNCLINE << "  "
        <<"Converted to integer is \"" << timeStep << "\""
        << std::endl;
    }

    // If time data is present, tell VisIt
    if (registry->hasTime()) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "This file supplies time: "
        << registry->getTime() << std::endl;

        md->SetTime(timeStep, registry->getTime());
        md->SetTimeIsAccurate(true, registry->getTime());
    }

    // If time cycle is present, tell VisIt
    if (registry->hasCycle()) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
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

    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Initializing VsReader()" << std::endl;

    //Actually open the file & read metadata for the first time

    try {
        if (!registry) registry = new VsRegistry();
        reader = new VsReader(dataFileName, registry);
    }
    catch (std::invalid_argument& ex) {
        std::ostringstream msg;
        msg << CLASSFUNCLINE << "  "
        << " Error initializing VsReader: " << ex.what();
        VsLog::debugLog() << msg.str() << std::endl;
        EXCEPTION1(InvalidDBTypeException, msg.str().c_str());
    }

    VsLog::debugLog() << CLASSFUNCLINE << "  "
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
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Entering function." << std::endl;
    LoadData();

    // Tell visit that we can split meshes into subparts when running
    // in parallel

    // NOTE that we can't decompose domains if we have MD meshes
    // So it's one or the other

#ifdef VIZSCHEMA_DECOMPOSE_DOMAINS
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Decompose_domains is defined.  Entering code block."
    << std::endl;
    if (registry->numMDMeshes() > 0) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "MD meshes are present in the data file.  "
        << "Domain Decomposition is turned off." << std::endl;
        md->SetFormatCanDoDomainDecomposition(false);
    } else {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "NO MD meshes are present in the data file.  "
        << "Domain Decomposition is turned on." << std::endl;
        md->SetFormatCanDoDomainDecomposition(true);
    }
    VsLog::debugLog() << CLASSFUNCLINE << "  "
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
        VsLog::debugLog() << CLASSFUNCLINE << "  "
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

    VsLog::debugLog() << CLASSFUNCLINE << "  "
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
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Entering function." << std::endl;

    if (mmd == NULL) {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
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

    VsLog::debugLog() << CLASSFUNCLINE << "  "
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
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "entering" << std::endl;
    LoadData();

    if (registry->hasCycle())
    {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "returning TRUE." << std::endl;
        return true;
    }
    else
    {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
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
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "entering for file " <<dataFileName << std::endl;
    LoadData();

    if (registry->hasCycle())
    {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "The file " <<dataFileName <<" supplies cycle: "
        << registry->getCycle() << std::endl;
        return registry->getCycle();
    }
    else
    {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
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
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "entering" << std::endl;
    LoadData();

    if (registry->hasTime())
    {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "returning TRUE." <<std::endl;
        return true;
    }
    else
    {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
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
    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "entering" << std::endl;
    LoadData();

    if (registry->hasTime())
    {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "The file " <<dataFileName <<" supplies time: "
        <<registry->getTime() << std::endl;
        return registry->getTime();
    }
    else
    {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
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
        for (size_t i=0; i<(size_t)numTopologicalDims; ++i)
        {
            if( maxs[i] < 0 || numNodes[i] - 1 < maxs[i] )
            maxs[i] = numNodes[i] - 1; // last node index, not number of nodes

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
        for (size_t i=0; i<(size_t)numTopologicalDims; ++i)
        {
            mins[i] = 0;
            maxs[i] = numNodes[i] - 1; // last node index, not number of nodes
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
    for (size_t i=0; i<(size_t)numTopologicalDims; ++i)
    gdims[i] = (maxs[i]-mins[i]) / strides[i] + 1;// Number of nodes

    for (size_t i=numTopologicalDims; i<gdims.size(); ++i)
    gdims[i] = 1;

    if( haveDataSelections )
    {
        VsLog::debugLog()
        << CLASSFUNCLINE << "  "
        << "Have a " << (isNodal ? "nodal" : "zonal") << " inclusive selection  ";

        for (size_t i=0; i<(size_t)numTopologicalDims; ++i)
        {
            VsLog::debugLog()
            << "(" << mins[i] << "," << maxs[i] << " stride " << strides[i] << ") ";
        }

        VsLog::debugLog() << std::endl;
    }

    VsLog::debugLog() << CLASSFUNCLINE << "  "
    << "Dimensions ";

    for (size_t i=0; i<(size_t)numTopologicalDims; ++i)
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

    VsLog::debugLog() << CLASSFUNCLINE << "  "
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
        VsLog::debugLog() << CLASSFUNCLINE << "  "
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
            VsLog::debugLog() << CLASSFUNCLINE << "  "
            << "Predicted bounds for processor " << i << "  "
            << "min = " << min << "  max = " << max << "  "
            << "strides = " << strides[splitAxis] << "  "
            << "nodes = " << numNodes << std::endl;
            else
            VsLog::debugLog() << CLASSFUNCLINE << "  "
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
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "Actual bounds for processor " << PAR_Rank() << "  "
        << "min = " << mins[splitAxis] << "  "
        << "max = " << maxs[splitAxis] << "  "
        << "strides = " << strides[splitAxis] << "  "
        << "nodes = " << numNodes << std::endl;
    }
    else
    {
        VsLog::debugLog() << CLASSFUNCLINE << "  "
        << "No work for processor " << PAR_Rank() << std::endl;
    }

    VsLog::debugLog() << CLASSFUNCLINE << "  "
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

// *****************************************************************************
//  Method: avtVsFileFormat::fillInMaskNodeArray
//
//  Purpose:
//      Fill in mask array (maskedNodes) from mask dataset
//
//  Programmer: Alex Pletzer
//  Creation:   October, 2011
//
//  Modifications:
//
template<typename TYPE>
void avtVsFileFormat::fillInMaskNodeArray(const std::vector<int>& gdims,
        VsDataset *mask, bool maskIsFortranOrder,
        vtkUnsignedCharArray *maskedNodes) {

    int numPoints = gdims[0] * gdims[1] * gdims[2];
    std::vector<TYPE> maskArray(numPoints);
    herr_t err = reader->getData(mask, &maskArray[0]);
    unsigned char* mp = maskedNodes->GetPointer(0);
    if (err < 0) {
        VsLog::debugLog() << __CLASS__ << "(" << instanceCounter << ")"
                << __FUNCTION__ << "  " << __LINE__ << "  "
                << "Cannot read mask dataset " << std::endl;
    } else {
        if (maskIsFortranOrder) {
            for (size_t k = 0; k < (size_t)numPoints; ++k) {
                if (maskArray[k] != 0) {
                    avtGhostData::AddGhostNodeType(mp[k],
                            NODE_NOT_APPLICABLE_TO_PROBLEM);
                }
            }
        } else {
            // maskArray follows C index order, but
            // mp follows Fortran index order!
            for (int k = 0; k < numPoints; ++k) {
                int i0 = k % gdims[0];
                int i1 = (k / gdims[0]) % gdims[1];
                int i2 = (k / gdims[0] / gdims[1]) % gdims[2];
                int kC = i2 + gdims[2] * (i1 + gdims[1] * i0);
                if (maskArray[kC] != 0) {
                    avtGhostData::AddGhostNodeType(mp[k],
                            NODE_NOT_APPLICABLE_TO_PROBLEM);
                }
            }
        }
    }
}

// *****************************************************************************
//  Method: avtVsFileFormat::setStructuredMeshCoords
//
//  Purpose:
//     Set the node coordinates
//
//  Programmer: Alex Pletzer
//  Creation:   October, 2011
//
//  Modifications:
//
template<typename TYPE>
void avtVsFileFormat::setStructuredMeshCoords(const std::vector<int>& gdims,
        const TYPE* dataPtr, bool isFortranOrder, vtkPoints* vpoints) {

    int numPoints = gdims[0] * gdims[1] * gdims[2];
    int index[3];
    TYPE xyz[3];

    // mulSizes will be used to convert from a flat index to an index set.
    int mulSizes[3];
    if (isFortranOrder) {
        // Fortran order
        mulSizes[0] = 1;
        mulSizes[1] = gdims[0];
        mulSizes[2] = gdims[0] * gdims[1];
    } else {
        // C order
        mulSizes[0] = gdims[2] * gdims[1];
        mulSizes[1] = gdims[2];
        mulSizes[2] = 1;
    }

    for (int k = 0; k < numPoints; ++k) {

        // Set the base coordinates
        for (size_t j = 0; j < 3; ++j) {
            index[j] = (k / mulSizes[j]) % gdims[j];
            xyz[j] = dataPtr[3 * k + j];
        }

        // Set the node coordinates. Note: vpoints wants Fortran ordering, regardless
        // of whether isFortranOrder == true or not.
        int kF = index[0] + gdims[0] * (index[1] + gdims[1] * index[2]);

        vpoints->SetPoint(kF, xyz);

    }
}
