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
#include <iostream>
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

avtVsFileFormat::avtVsFileFormat(const char* dfnm, std::vector<int> settings) :
  avtSTMDFileFormat(&dfnm, 1), dataFileName(dfnm) {

    VsLog::initialize(DebugStream::Stream3(), DebugStream::Stream4(), DebugStream::Stream5());
    
    VsLog::debugLog() <<"avtVsFileFormat::constructor() - entering" <<std::endl;
    VsLog::debugLog() <<"avtVsFileFormat::constructor() - VizSchema Revision #742" <<std::endl;

    //reader starts off empty
    reader = NULL;
    
    //Initialize the registry for objects
    registry = new VsRegistry();
    
    //    LoadData();

    //check types
    if (isFloatType(H5T_NATIVE_FLOAT)) {
      VsLog::debugLog() <<"avtVsFileFormat::constructor() - float type checks out ok." <<std::endl;
    } else {
      std::string msg("avtVsFileFormat::constructor() - ERROR - H5T_NATIVE_FLOAT not recognized as a float.");
      VsLog::debugLog() <<msg <<std::endl;
      EXCEPTION1(InvalidDBTypeException, msg.c_str());
    }
    
    //check types
    if (isDoubleType(H5T_NATIVE_DOUBLE)) {
      VsLog::debugLog() <<"avtVsFileFormat::constructor() - double type checks out ok." <<std::endl;
    } else {
      std::string msg("avtVsFileFormat::constructor() - ERROR - H5T_NATIVE_DOUBLE not recognized as a double.");
      VsLog::debugLog() <<msg <<std::endl;
      EXCEPTION1(InvalidDBTypeException, msg.c_str());
    }

    //initialize settings
    stride.resize(3);
    if (settings.size() != 3) {
      VsLog::debugLog() <<"avtVsFileFormat::constructor() - Expected settings array of length 3." <<std::endl;
      VsLog::debugLog() <<"avtVsFileFormat::constructor() - Got length " <<settings.size() <<"." <<std::endl;
      stride[0] = VsCommonPluginInfo::defaultStride;
      stride[1] = VsCommonPluginInfo::defaultStride;
      stride[2] = VsCommonPluginInfo::defaultStride;
    } else {
      stride[0] = settings[0];
      stride[1] = settings[1];
      stride[2] = settings[2];
    }

    VsLog::debugLog() <<"avtVsFileFormat::constructor() - strides are: " <<stride[0] <<", " <<stride[1] <<", " <<stride[2] <<"." <<std::endl;

    herr_t err = H5check();

    if (err < 0) {
      std::string msg("avtVsFileFormat::constructor(): HDF5 version mismatch."
          "  Vs reader built with ");
      msg += H5_VERS_INFO;
      msg += ".";
      VsLog::debugLog() << msg << endl;
      EXCEPTION1(InvalidDBTypeException, msg.c_str());
    }

    //NOTE: We used to initialize the VsH5Reader object here
    //But now do it on demand in 'populateDatabaseMetaData'
    //To minimize I/O

    VsLog::debugLog() <<"avtVsFileFormat::constructor() - exiting" <<std::endl;
  }

  avtVsFileFormat::~avtVsFileFormat() {
    VsLog::debugLog() <<"avtVsFileFormat::destructor() - entering" <<std::endl;
    if (reader != NULL) {
      delete reader;
      reader = NULL;
    }

    if (registry) {
      delete registry;
      registry = NULL;
    }
    
    VsLog::debugLog() <<"avtVsFileFormat::destructor() - exiting" <<std::endl;
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
            avtSpatialBoxSelection *sel = (avtSpatialBoxSelection *) *(selList[i]);

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


  vtkDataSet* avtVsFileFormat::GetMesh(int domain, const char* name) {

    int mins[3], maxs[3], strides[3];
    if( 0 && ProcessDataSelections(mins, maxs, strides) )
    {
      cerr << "have a data selection for mesh "<< endl
           << "(" << mins[0] << "," << maxs[0] << " stride " << strides[0] << ") "
           << "(" << mins[1] << "," << maxs[1] << " stride " << strides[1] << ") "
           << "(" << mins[2] << "," << maxs[2] << " stride " << strides[2] << ") "
           << endl;
    }

    std::stringstream sstr;
    sstr <<"avtVsFileFormat::GetMesh(" <<domain <<", " <<name <<") - ";
    std::string methodSig = sstr.str();
    VsLog::debugLog() <<methodSig <<"Entering function." <<std::endl;
    std::string meshName = name;
    LoadData();

    //The MD system works by filtering the requests directed to it
    // into the name of the appropriate subordinate mesh.
    // For example, in facets_core-edge-explicit we have three meshes joined into one:
    // MdMesh = {coreMesh, solMesh, privMesh}
    // So if we get a request for (MdMesh, 0), we change the name to coreMesh and proceed normally

    //Check for MD mesh
    VsLog::debugLog() <<methodSig <<"Looking for MD mesh with name " <<meshName <<std::endl;
    VsMDMesh* mdMeshMeta = registry->getMDMesh(meshName);

    //If we found an MD mesh with this name, try to load the mesh data from it
    VsMesh* meta = NULL;
    if (mdMeshMeta != NULL) {
      VsLog::debugLog() <<methodSig <<"Found MD mesh with that name." <<std::endl;
      meshName = mdMeshMeta->getNameForBlock(domain);
      VsLog::debugLog() <<methodSig <<"Request for md mesh was filtered to regular mesh: " <<meshName <<std::endl;
      meta = mdMeshMeta->getBlock(domain);
    } else {
      VsLog::debugLog() <<methodSig <<"No MD mesh with that name." <<std::endl;
    }

    //Did we succeed in loading mesh data from MD mesh?
    if (meta == NULL) {
      VsLog::debugLog() <<methodSig <<"Trying to find regular mesh named: " <<meshName <<std::endl;
      meta = registry->getMesh(meshName);
    }

    if (meta != NULL) {
      VsLog::debugLog() <<methodSig <<"Found mesh named: " <<meshName <<std::endl;
      if (meta->isUniformMesh()) {
        VsLog::debugLog() <<methodSig <<"Trying to load & return uniform mesh" <<std::endl;
        return getUniformMesh(static_cast<VsUniformMesh*>(meta));
      }
#if (defined PARALLEL && defined VIZSCHEMA_DECOMPOSE_DOMAINS)
      // Don't know how to decompose any other type of mesh -> load it on proc 0 only
      if (PAR_Rank() > 0) {
        VsLog::debugLog() <<methodSig <<"In parallel mode on processor " <<PAR_Rank <<" and mesh is not uniform" <<std::endl;
        VsLog::debugLog() <<methodSig <<"Returning NULL, mesh will be loaded on processor 0 only." <<std::endl;
        return NULL;
      }
#endif
      if (meta->isUnstructuredMesh()) {
        VsLog::debugLog() <<methodSig <<"Trying to load & return unstructured mesh" <<std::endl;
        //VsUnstructuredMesh* unstructuredMesh = (VsUnstructuredMesh*)meta;
        // getUnstructuredMesh handles split points so getSplitPointMesh is not needed.
        //  if (unstructuredMesh->usesSplitPoints())
        //    return getSplitPointMesh(meshName, *unstructuredMesh);
        //  else
        return getUnstructuredMesh(static_cast<VsUnstructuredMesh*>(meta));
      }

      if (meta->isStructuredMesh()) {
        VsLog::debugLog() <<methodSig <<"Trying to load & return structured mesh" <<std::endl;
        return getStructuredMesh(static_cast<VsStructuredMesh*>(meta));
      }

      if (meta->isRectilinearMesh()) {
        VsLog::debugLog() <<methodSig <<"Trying to load & return rectilinear mesh." <<std::endl;
        return getRectilinearMesh(static_cast<VsRectilinearMesh*>(meta));
      }
      
      //if we get here, we don't know what kind of mesh it is.
      VsLog::debugLog() <<methodSig <<"Mesh has unknown type: " <<meta->getKind() <<"Returning." << endl;
      return NULL;
    }

    //Variable with mesh
    VsLog::debugLog() <<methodSig <<"Looking for Variable With Mesh with this name." << endl;
    VsVariableWithMesh* vmeta = registry->getVariableWithMesh(name);
    if (vmeta != NULL) {
      VsLog::debugLog() <<methodSig <<"Found Variable With Mesh. Loading data and returning." << endl;
      return getPointMesh(vmeta);
    } else {
      VsLog::debugLog() <<methodSig <<"Did not find Variable With Mesh." << endl;
    }

    //Curve
    VsLog::debugLog() <<methodSig <<"Looking for Curve with this name." << endl;
    vtkDataArray* foundCurve = this->GetVar(domain, name);
    if (foundCurve != NULL) {
      VsLog::debugLog() <<methodSig <<"Found curve.  Loading data and returning. <<std::endl";
      return getCurve(domain, name);
    }

    VsLog::debugLog() <<methodSig <<"Failed to load data for given name and domain number.  Returning NULL." <<std::endl;
    return NULL;
  }

  vtkDataSet* avtVsFileFormat::getCurve(int domain, const std::string& name) {
    std::stringstream sstr;
    sstr <<"avtVsFileFormat::getCurve(" <<domain <<", " <<name <<") - ";
    std::string methodSig = sstr.str();
    VsLog::debugLog() <<methodSig <<"Entering function." <<std::endl;
    LoadData();

    ///TODO: This method piggybacks on getVar and getMesh -
    ///       Could be made much more efficient
    //1. get var
    //2. get matching mesh
    //3. create 2 coord arrays - x from mesh and y from var
    //4. Combine coord arrays to form 1-d Rectilinear mesh
    // (attempts to follow visit plugin Curve2d in /visit/src/databases/Curve2d/avtCurve2DFileFormat.C)

    //retrieve var metadata and extract the mesh name
    VsLog::debugLog() <<methodSig <<"Looking for variable metadata." <<endl;
    VsVariable* varMeta = registry->getVariable(name);
    if (varMeta == NULL) {
      VsLog::debugLog() <<methodSig <<"No variable metadata found under name " <<name <<endl;
      VsLog::debugLog() <<methodSig <<"Looking for information in component registry." <<std::endl;

      //Is this a component?
      //If so, we swap the component name with the "real" variable name
      //And remember that we're a component
      int componentIndex = 0;
      NamePair foundName;
      registry->getComponentInfo(name, &foundName);
      if (!foundName.first.empty()) {
        VsLog::debugLog() <<methodSig <<"This is a component, and actually refers to variable " <<foundName.first <<" and index " <<foundName.second <<std::endl;
        varMeta = registry->getVariable(foundName.first);
      }
      
      if (varMeta == NULL) {
        VsLog::debugLog() <<methodSig <<"Failed to find variable in component list, giving up and returning NULL." <<std::endl;
        return NULL;
      }
    }

    VsLog::debugLog() <<methodSig <<"Found variable metadata." <<name <<endl;

    std::string meshName = varMeta->getMeshName();
    VsMesh* meshMeta = varMeta->getMesh();
    if (meshMeta == NULL) {
      VsLog::debugLog() <<methodSig <<"No mesh metadata found. Returning NULL." <<endl;
      return NULL;
    }

    VsLog::debugLog() <<methodSig <<"Found all metadata, loading variable data." <<endl;
    vtkDataArray* varData = NULL;
    try {
      varData = GetVar(domain, name.c_str());
    } catch (...) {
      VsLog::debugLog() <<methodSig <<"Caught exception from GetVar() - returning NULL." <<endl;
      return NULL;
    }

    if (varData == NULL) {
      VsLog::debugLog() <<methodSig <<"Failed to load var data - returning NULL." <<endl;
      return NULL;
    }

    VsLog::debugLog() <<methodSig <<"Loaded variable data, trying to load mesh data." <<endl;
    vtkRectilinearGrid* meshData = NULL;
    try {
      meshData = (vtkRectilinearGrid*)GetMesh(domain, meshName.c_str());
    } catch (...) {
      VsLog::debugLog() <<methodSig <<"Caught exception from GetMesh() - returning NULL." <<endl;
      ///TODO: delete varData?
      varData->Delete();
      return NULL;
    }

    if (meshData == NULL) {
      VsLog::debugLog() <<methodSig <<"Failed to load mesh data - returning NULL." <<endl;
      ///TODO: delete varData?
      varData->Delete();
      return NULL;
    }

    int numDims = meshMeta->getNumSpatialDims();
    VsLog::debugLog() <<methodSig <<"Mesh has " <<numDims <<" dimensions" <<endl;

    //This used to be "varMeta->getLength()"
    //Which is incorrect if the variable has multiple components
    //Instead, we have to check the proper dimensional index
    int nPts = 0;
    if (varMeta->isCompMinor()) {
      nPts = varMeta->getDims()[0];
    } else {
      nPts = varMeta->getDims()[1];
    }

    VsLog::debugLog() <<methodSig <<"Variable has " <<nPts <<" points." <<endl;

    // Create 1-D RectilinearGrid
    hid_t varDataType = varMeta->getType();
    if (isDoubleType(varDataType)) {
      VsLog::debugLog() <<methodSig <<"Var is 64-bit real" <<endl;
    } else if (isFloatType(varDataType)) {
      VsLog::debugLog() <<methodSig <<"Var is 32-bit real" <<endl;
    } else if (isIntegerType(varDataType)) {
      VsLog::debugLog() <<methodSig <<"Var is int" <<endl;
    } else {
      VsLog::debugLog() <<methodSig <<"Var is unknown type (known are double, float, int)." <<endl;
      VsLog::debugLog() <<methodSig <<"Returning NULL" <<endl;
      return NULL;
    }

    VsLog::debugLog() <<methodSig <<"Building Rectilinear grid." <<std::endl;
    vtkFloatArray* vals = vtkFloatArray::New();
    vals->SetNumberOfComponents(1);
    vals->SetNumberOfTuples(nPts);
    vals->SetName(name.c_str());

    vtkRectilinearGrid* rg = vtkVisItUtility::Create1DRGrid(nPts, VTK_FLOAT);
    rg->GetPointData()->SetScalars(vals);
    vtkFloatArray* xc = vtkFloatArray::SafeDownCast(rg->GetXCoordinates());

    VsLog::debugLog() <<methodSig <<"Retrieving X coordinates from mesh." <<std::endl;
    vtkDataArray* meshXCoord = meshData->GetXCoordinates();

    VsLog::debugLog() <<methodSig <<"Adding all points to curve" <<std::endl;
    for (int i = 0; i < nPts; i++) {
      double* var_i = varData->GetTuple(i);
      double* mesh_i = meshXCoord->GetTuple(i);
      //VsLog::debugLog() <<"Adding tuple[" <<i <<"] = (" <<mesh_i[0] <<", " <<var_i[0] <<")" <<endl;

      xc->SetValue(i, mesh_i[0]);
      vals->SetValue(i, var_i[0]);
    }

    VsLog::debugLog() <<methodSig <<"Deleting temporary variables." <<std::endl;
    vals->Delete();

    VsLog::debugLog() <<methodSig <<"Returning data." <<endl;
    return rg;
  }

  vtkDataSet* avtVsFileFormat::getStructuredMesh(VsStructuredMesh* structuredMesh) {
    std::stringstream sstr;
    sstr <<"avtVsFileFormat::getStructuredMesh() - ";
    std::string methodSig = sstr.str();
    VsLog::debugLog() <<methodSig <<"Entering function." <<std::endl;
    LoadData();

    //Find points
    // Structured meshes are datasets, and the data is the points
    //So, we look for the dataset with the same name as the mesh

    VsH5Dataset* pointsDataset = registry->getDataset(structuredMesh->getFullName());
    if (pointsDataset == NULL) {
      VsLog::debugLog() <<methodSig <<"Component '" <<
      structuredMesh->getFullName() << "' not found. Returning NULL." << endl;
      return NULL;
    }

    // Get dims of points array
    VsLog::debugLog() <<methodSig <<"Determining dimension of points array." <<std::endl;
    std::vector<int> dims;
    structuredMesh->getMeshDims(&dims, reader->useStride, this->stride);
    if (dims.size() < 0) {
      std::string msg = "avtVsFileFormat::getStructuredMesh: could not get dimensions of structured mesh.";
      VsLog::debugLog() << msg << std::endl;
      throw std::out_of_range(msg.c_str());
    }
    size_t nDims = dims[dims.size()-1];
    size_t numPoints = 1;
    for (size_t i = 0; i < dims.size()-1; ++i) {
      numPoints *= dims[i];
    }
    VsLog::debugLog() <<methodSig <<"Total number of points is " <<numPoints <<"." <<std::endl;

    // Check points data type
    hid_t type = pointsDataset->getType();
    if (!isDoubleType(type) && !isFloatType(type)) {
      VsLog::debugLog() <<methodSig <<"Error: hdf5 data type not handled: " <<type << endl;
      VsLog::debugLog() <<methodSig <<"Returning NULL." << endl;
      return NULL;
    }

    // Read points and add in zero for any lacking dimension
    VsLog::debugLog() <<methodSig <<"Reading in point data." <<std::endl;
    size_t dsize = 0;
    void* dataPtr = 0;
    double* dblDataPtr = 0;
    float* fltDataPtr = 0;
    if (isDoubleType(type)) {
      dsize = sizeof(double);
      VsLog::debugLog() <<methodSig <<"Declaring array of doubles of length " <<(numPoints * 3) <<"." <<std::endl;
      VsLog::debugLog() <<methodSig <<"Total allocation: " <<(numPoints * 3 * dsize) <<" bytes." <<std::endl;
      dblDataPtr = new double[numPoints*3];
      dataPtr = dblDataPtr;
    }
    else if (isFloatType(type)) {
      dsize = sizeof(float);
      VsLog::debugLog() <<methodSig <<"Declaring array of floats of length " <<(numPoints * 3) <<"." <<std::endl;
      VsLog::debugLog() <<methodSig <<"Total allocation: " <<(numPoints * 3 * dsize) <<" bytes." <<std::endl;
      fltDataPtr = new float[numPoints*3];
      dataPtr = fltDataPtr;
    }

    if (!dataPtr) {
      VsLog::debugLog() <<methodSig <<"Allocation failed, pointer is NULL." <<std::endl;
      VsLog::debugLog() <<methodSig <<"Returning NULL." <<std::endl;
      return NULL;
    }

    VsLog::debugLog() <<methodSig <<"Allocation succeeded.  Now reading in data." <<std::endl;

    // Read in the data
    herr_t err = reader->getDatasetMeshComponent(pointsDataset, *structuredMesh, dataPtr);
    if (err < 0) {
      VsLog::debugLog() <<methodSig <<"Error reading mesh data.  Deleting temporary storage." <<std::endl;
      if (isDoubleType(type)) {
        delete [] dblDataPtr;
      } else if (isFloatType(type)) {
        delete [] fltDataPtr;
      } else {
        VsLog::debugLog() <<methodSig <<"Unknown data type: " <<type <<std::endl;
      }
      VsLog::debugLog() <<methodSig <<"Returning NULL." <<std::endl;
      return NULL;
    } else {
      VsLog::debugLog() <<methodSig <<"Succeeded reading mesh data." <<std::endl;
    }

    // If source data is less than 3D, move data to correct position
    // and set dest data extra dim values to 0
    if (nDims < 3) {
      VsLog::debugLog() <<methodSig <<"Dimensionality is less than 3.  Moving data into correct location." <<std::endl;
      for (size_t i = numPoints; i > 0; --i) {
        unsigned char* destPtr = (unsigned char*)dataPtr + (i-1)*3*dsize;
        unsigned char* srcPtr = (unsigned char*)dataPtr + (i-1)*nDims*dsize;
        memmove(destPtr, srcPtr, nDims*dsize);
        destPtr += nDims*dsize;
        memset(destPtr, 0, (3-nDims)*dsize);
      }
      VsLog::debugLog() <<methodSig <<"Data move succeeded." <<std::endl;
    }
    /*
     VsLog::debugLog() <<methodSig <<"OUtputting points" <<std::endl;
     // debug: output points
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
    // Create the mesh and set its dimensions, including unused to zero
    VsLog::debugLog() <<methodSig <<"Creating the mesh." <<std::endl;
    vtkStructuredGrid* sgrid = vtkStructuredGrid::New();
    int idims[3];
    size_t len = 1;
    for (size_t i = 0; i < 3; ++i) {
      if (i < nDims) idims[i] = dims[i];
      else idims[i] = 1;
      len *= idims[i];
    }
    sgrid->SetDimensions(idims);

    // add the points, changing to C ordering
    VsLog::debugLog() <<methodSig <<"Adding points to mesh." <<std::endl;
    vtkPoints* vpoints = vtkPoints::New();
    if (isDoubleType(type)) {
      vpoints->SetDataTypeToDouble();
    }
    else if (isFloatType(type)) {
      vpoints->SetDataTypeToFloat();
    } else {
      VsLog::debugLog() <<methodSig <<"Unknown data type: " <<type <<std::endl;
    }

    vpoints->SetNumberOfPoints(numPoints);
    //void* ptsPtr = vpoints->GetVoidPointer(0);

    // Step through by global C index to reverse
    VsLog::debugLog() <<methodSig <<"Adding " << len
    << " points with index order '" << structuredMesh->getIndexOrder() << "'." << endl;

    if (structuredMesh->isFortranOrder()) {
      VsLog::debugLog() <<methodSig <<"Using FORTRAN data ordering." <<std::endl;
      if (isDoubleType(type)) {
        for (size_t k = 0; k<len; ++k) {
          vpoints->SetPoint(k, &dblDataPtr[k*3]);
        }
      } else if (isFloatType(type)) {
        for (size_t k = 0; k<len; ++k) {
          vpoints->SetPoint(k, &fltDataPtr[k*3]);
        }
      } else {
          VsLog::debugLog() <<methodSig <<"Unknown data type: " <<type <<std::endl;
      }
    }
    else {
      VsLog::debugLog() <<methodSig <<"Using C data ordering." <<std::endl;
      size_t indices[3] = {0, 0, 0};
      //we cache the data type because it's faster 
      bool isDouble = isDoubleType(type);
      bool isFloat = isFloatType(type);
      for (size_t k = 0; k<len; ++k) {
        //VsLog::debugLog() <<methodSig <<"k = " <<k <<std::endl;
        
        // Accumulate the Fortran index
        size_t indx = indices[2];
        for (size_t j = 2; j<=3; ++j) {
          indx = indx*idims[3-j] + indices[3-j];
        }
        //        VsLog::debugLog() <<methodSig <<"part1 finished " <<k <<std::endl;
        // Set the value in the VTK array at the Fortran index to the
        // value in the C array at the C index
        if (isDouble) {
          //VsLog::debugLog() <<methodSig <<"setting index " <<indx <<" to dblDataPtr[" <<k * 3 <<"]" <<std::endl;
          vpoints->SetPoint(indx, &dblDataPtr[k*3]);
        }
        else if (isFloat) {
          // VsLog::debugLog() <<methodSig <<"setting index " <<indx <<" to fltDataPtr[" <<k * 3 <<"]" <<std::endl;
          vpoints->SetPoint(indx, &fltDataPtr[k*3]);
        } else {
          VsLog::debugLog() <<methodSig <<"Unknown data type: " <<type <<std::endl;
        }
        size_t j = 3;
        // VsLog::debugLog() <<methodSig <<"part2 finished " <<k <<std::endl;
        do {
          --j;
          ++indices[j];
          if (indices[j] == idims[j])
          indices[j] = 0;
          else break;
        }while (j != 0);
        //VsLog::debugLog() <<methodSig <<"part3 finished " <<k <<std::endl;
      }
    }
    VsLog::debugLog() <<methodSig <<"Points added successfully.  Deleting temporary storage." <<endl;
    if (isDoubleType(type)) {
      delete [] dblDataPtr;
    } else if (isFloatType(type)) {
      delete [] fltDataPtr;
    } else {
      VsLog::debugLog() <<methodSig <<"Unknown data type: " <<type <<std::endl;
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
    sgrid->SetPoints(vpoints);
    vpoints->Delete();

    VsLog::debugLog() <<methodSig <<"Returning data." <<endl;
    return sgrid;
  }

  vtkDataSet* avtVsFileFormat::getUnstructuredMesh(VsUnstructuredMesh* unstructuredMesh) {
    std::stringstream sstr;
    sstr <<"avtVsFileFormat::getUnstructuredMesh() - ";
    std::string methodSig = sstr.str();
    VsLog::debugLog() <<methodSig <<"Entering function." <<std::endl;
    LoadData();

    hid_t type0 = unstructuredMesh->getDataType();
    // Check for points type
    if (!isDoubleType(type0) && !isFloatType(type0)) {
      VsLog::debugLog() <<methodSig <<"Points data type not recognized:" <<type0 <<std::endl;
      VsLog::debugLog() <<methodSig <<"Returning NULL" <<std::endl;
      return NULL;
    }

    VsLog::debugLog() <<methodSig <<"Checking dimensionality of mesh." <<std::endl;
    size_t nnodes = unstructuredMesh->getNumPoints();
    size_t ndims = unstructuredMesh->getNumSpatialDims();

    // Get ready to read in points
    VsLog::debugLog() <<methodSig <<"Setting up data structures." <<std::endl;
    vtkUnstructuredGrid* ugridPtr = vtkUnstructuredGrid::New();

    // Create and set points while small so minimal memory usage
    vtkPoints* vpts = vtkPoints::New();
    ugridPtr->SetPoints(vpts);
    vpts->Delete();
    vtkPoints* vpoints = ugridPtr->GetPoints();

    // Allocate
    size_t dsize = 0;
    if (isDoubleType(type0)) {
      vpoints->SetDataTypeToDouble();
      dsize = sizeof(double);
    }
    else if (isFloatType(type0)) {
      vpoints->SetDataTypeToFloat();
      dsize = sizeof(float);
    } else {
      VsLog::debugLog() <<methodSig <<"Unknown data type: " <<type0 <<std::endl;
      return NULL;
    }

    VsLog::debugLog() <<methodSig <<"VPoints array will have " <<nnodes <<" points." <<std::endl;
    vpoints->SetNumberOfPoints(nnodes);
    void* dataPtr = vpoints->GetVoidPointer(0);
    if (!dataPtr) {
      VsLog::debugLog() <<methodSig <<"Unable to allocate the points.  Cleaning up." << endl;
      ugridPtr->Delete();
      VsLog::debugLog() <<methodSig <<"Returning NULL." << endl;
      return NULL;
    }

    // Read in the data
    VsLog::debugLog() <<methodSig <<"Reading in data." <<std::endl;
    if (unstructuredMesh->usesSplitPoints()) {
      VsLog::debugLog() <<methodSig <<"Using split-points method" <<std::endl;

      reader->getSplitMeshData(*unstructuredMesh, dataPtr);
      VsLog::debugLog() <<methodSig <<"Returned from getSplitMeshData" <<std::endl;
    }
    else {
      VsLog::debugLog() <<methodSig <<"Using all-in-one method" <<std::endl;
      //all in one dataset
      VsH5Dataset* pointsDataset = unstructuredMesh->getPointsDataset();
      reader->getDatasetMeshComponent(pointsDataset, *unstructuredMesh, dataPtr);

      // Move the data back to where it should be in a 3D array
      if (ndims < 3) {
        for (int i=nnodes-1; i>=0; --i) {
          char* destPtr = (char*) dataPtr + i*3*dsize;
          char* srcPtr = (char*) dataPtr + i*ndims*dsize;
          memmove(destPtr, srcPtr, ndims*dsize);
          destPtr += ndims*dsize;
          memset(destPtr, 0, (3-ndims)*dsize);
        }
      }

    }
    VsLog::debugLog() <<methodSig <<"Read finished." <<std::endl;

    // If there is no polygon data, then this mesh was registered as a point mesh
    // and we don't need to go any further
    if (unstructuredMesh->isPointMesh()) {
      VsLog::debugLog() <<methodSig <<"Mesh was registered as a point mesh." <<std::endl;
      VsLog::debugLog() <<methodSig <<"Because no connectivity data was found." <<std::endl;
      VsLog::debugLog() <<methodSig <<"Adding vertices as single points and returning." <<std::endl;
      vtkIdType vertex;
      for (size_t i = 0; i < nnodes; ++i) {
        vertex = i;
        ugridPtr->InsertNextCell(VTK_VERTEX, 1, &vertex);
      }
      return ugridPtr;
    }

    // Next, look for vertex data
    VsH5Dataset* vertexMeta = 0;

    unsigned int rank;
    unsigned int haveVertexCount = 0;
    std::string connectivityDatasetName;

    // For now users can have only one vertex dataset.
    if( (vertexMeta = unstructuredMesh->getLinesDataset())) {
      haveVertexCount = 2; rank = 2; connectivityDatasetName = unstructuredMesh->getLinesDatasetName();
    } else if( (vertexMeta = unstructuredMesh->getPolygonsDataset()) ) {
      haveVertexCount = 0; rank = 2; connectivityDatasetName = unstructuredMesh->getPolygonsDatasetName();
    } else if( (vertexMeta = unstructuredMesh->getTrianglesDataset()) ) {
      haveVertexCount = 3; rank = 2; connectivityDatasetName = unstructuredMesh->getTrianglesDatasetName();
    } else if( (vertexMeta = unstructuredMesh->getQuadrilateralsDataset()) ) {
      haveVertexCount = 4; rank = 2; connectivityDatasetName = unstructuredMesh->getQuadrilateralsDatasetName();
    } else if( (vertexMeta = unstructuredMesh->getPolyhedraDataset()) ) {
      haveVertexCount = 0; rank = 3; connectivityDatasetName = unstructuredMesh->getPolyhedraDatasetName();
    } else if( (vertexMeta = unstructuredMesh->getTetrahedralsDataset()) ) {
      haveVertexCount = 4; rank = 3; connectivityDatasetName = unstructuredMesh->getTetrahedralsDatasetName();
    } else if( (vertexMeta = unstructuredMesh->getPyramidsDataset()) ) {
      haveVertexCount = 5; rank = 3; connectivityDatasetName = unstructuredMesh->getPyramidsDatasetName();
    } else if( (vertexMeta = unstructuredMesh->getPrismsDataset()) ) {
      haveVertexCount = 6; rank = 3; connectivityDatasetName = unstructuredMesh->getPrismsDatasetName();
    } else if( (vertexMeta = unstructuredMesh->getHexahedralsDataset()) ) {
      haveVertexCount = 8; rank = 3; connectivityDatasetName = unstructuredMesh->getHexahedralsDatasetName();
    }

    else {
      VsLog::debugLog() <<methodSig <<"ERROR - unable to find vertex dataset." <<std::endl;
      VsLog::debugLog() <<methodSig <<"ERROR - Returning points data without connectivity." <<std::endl;
      //we used to try to convert this into a pointMesh
      // but it never worked
      // So basically we're giving up here
      return ugridPtr;
    }

    VsLog::debugLog() <<methodSig <<"Found vertex data." << endl;
    VsLog::debugLog() <<methodSig <<"haveVertexCount = " << haveVertexCount << endl;
    VsLog::debugLog() <<methodSig <<"Connectivity dataset name is " <<connectivityDatasetName <<std::endl;
    
    VsH5Dataset* connectivityDataset = registry->getDataset(connectivityDatasetName);
    
    std::vector<int> dim1 = vertexMeta->getDims();
    size_t ncells = dim1[0];
    size_t nverts = dim1[1];
    VsLog::debugLog() <<methodSig <<"ncells = " <<ncells << ", nverts = " << nverts << "." << endl;
    size_t datasetLength = 1;
    for (size_t i =0; i< dim1.size(); ++i)
    datasetLength *= dim1[i];
    hid_t type1 = vertexMeta->getType();
    // Check for vertex list type
    if (!isIntegerType(type1)) {
      VsLog::debugLog() <<methodSig <<"Indices are not integers. Cleaning up" <<std::endl;
      ugridPtr->Delete();
      VsLog::debugLog() <<methodSig <<"Returning NULL" <<std::endl;
      return NULL;
    }

    // Vertices
    VsLog::debugLog() <<methodSig <<"Allocating space for " <<datasetLength <<" integers of connectivity data." << endl;
    int* vertices = new int[datasetLength];
    if (!vertices) {
      VsLog::debugLog() <<methodSig <<"Unable to allocate vertices.  Cleaning up." <<std::endl;
      ugridPtr->Delete();
      VsLog::debugLog() <<methodSig <<"Returning NULL." <<std::endl;
      return NULL;
    }

    VsLog::debugLog() <<methodSig <<"Reading vertex list data." << endl;
    reader->getDatasetMeshComponent(connectivityDataset, *unstructuredMesh, vertices);
    
    // VsLog::debugLog() << "avtVsFileFormat::getUnstructuredMesh(...): vertices =";
    // for (size_t j = 0; j < 100; ++j) VsLog::debugLog() << " " << vertices[j];
    // VsLog::debugLog() << endl;

    try {
      VsLog::debugLog() <<methodSig <<"Allocating " <<ncells << " cells.  "
      <<"If old VTK and this fails, it will just abort." <<std::endl;
      ugridPtr->Allocate(ncells);
      VsLog::debugLog() <<methodSig <<"Allocation succeeded." <<std::endl;
    }
    // JRC: what goes here to detect failure to allocate?
    catch (vtkstd::bad_alloc) {
      VsLog::debugLog() << methodSig <<"Caught vtkstd::bad_alloc exception." <<std::endl;
      VsLog::debugLog() << methodSig <<"Unable to allocate space for cells.  Cleaning up."
      <<std::endl;
      delete [] vertices;
      ugridPtr->Delete();
      VsLog::debugLog() << methodSig <<"Returning NULL.";
      return NULL;
    } catch (...) {
      VsLog::debugLog() << methodSig <<"Unknown exception allocating cells.  Cleaning up."
      <<std::endl;
      delete [] vertices;
      ugridPtr->Delete();
      VsLog::debugLog() << methodSig <<"Returning NULL.";
      return NULL;
    }

    VsLog::debugLog() <<methodSig <<"Inserting cells into grid." <<std::endl;
    size_t k = 0;
    int warningCount = 0;
    int cellCount = 0;
    unsigned int cellVerts = 0; // cell's connected node indices
    int cellType;

    // Dealing with fixed length vertex lists.
    if( haveVertexCount ) {
      cellVerts = haveVertexCount;
      VsLog::debugLog() <<methodSig <<"Inserting " << cellVerts << " vertices into each cell." <<std::endl;
    }

    for (size_t i = 0; i < ncells; ++i) {
      if (k >= datasetLength) {
        VsLog::debugLog() <<methodSig <<"While iterating over the vertices, the index variable 'k' went beyond the end of the array." <<std::endl;
        VsLog::debugLog() <<methodSig <<"Existing cells will be returned but the loop is terminating now." <<std::endl;
        break;
      }

      ++cellCount;

      // Dealing with cells with variable number of vertices.
      if( haveVertexCount == 0 )
      cellVerts = vertices[k++];

      if (cellVerts > nverts) {
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
        if( rank == 2 ) cellType = VTK_QUAD;
        else if( rank == 3 ) cellType = VTK_TETRA;
        break;
        case 5:
        if( rank == 2 ) cellType = VTK_POLYGON;
        else if( rank == 3 ) cellType = VTK_PYRAMID;
        break;
        case 6:
        if( rank == 2 ) cellType = VTK_POLYGON;
        else if( rank == 3 ) cellType = VTK_WEDGE;
        break;
        case 8:
        if( rank == 2 ) cellType = VTK_POLYGON;
        else if( rank == 3 ) cellType = VTK_HEXAHEDRON;
        break;
        default:
        if (warningCount < 30) {
          VsLog::debugLog() <<methodSig << "Error: invalid number of vertices for cell #" <<cellCount <<": " << cellVerts << endl;
        } else if (warningCount == 30) {
          VsLog::debugLog() <<methodSig << "Exceeded maximum number of errors.  Error messages disabled for remaining cells." <<std::endl;
        }
        ++warningCount;
        cellType = VTK_EMPTY_CELL;
        break;
      }

      //create cell and insert into mesh
      if (cellType != VTK_EMPTY_CELL) {
        std::vector<vtkIdType> verts(cellVerts);
        for (size_t j = 0; j < cellVerts; ++j) {
          verts[j] = (vtkIdType) vertices[k++];
          if ((verts[j] < 0) || (verts[j] >= nnodes)) {
            VsLog::errorLog() <<methodSig <<"ERROR in connectivity dataset - requested vertex number " <<verts[j] <<" exceeds number of vertices" <<std::endl;
            verts[j] = 0;
          }
        }
        // insert cell into mesh
        ugridPtr->InsertNextCell(cellType, cellVerts, &verts[0]);
        if( haveVertexCount == 0 )
        k += nverts - 1 - cellVerts;
      } else {
        //there was some error
        // so we add each vertex as a single point
        // NO!  Unless we've registered as a pointmesh, adding single points won't work
        //so we treat the entire row of the dataset as a single cell
        //Maybe something will work!
        std::vector<vtkIdType> verts(nverts);
        k--;
        for (size_t j = 0; j < nverts; ++j) {
          if (warningCount < 30) {
            VsLog::debugLog() <<"WARNING: ADDING cell #" <<cellCount <<" as cell: " <<vertices[k] <<std::endl;
          }
          verts[j] = (vtkIdType) vertices[k++];
        }
        ugridPtr->InsertNextCell (VTK_POLYGON, nverts, &verts[0]);
      }
    }

    VsLog::debugLog() <<methodSig <<"Finished.  Cleaning up." <<std::endl;
    // Done, so clean up memory and return
    delete [] vertices;

    VsLog::debugLog() <<methodSig <<"Returning data." <<std::endl;
    return ugridPtr;
  }
  

  vtkDataSet* avtVsFileFormat::getRectilinearMesh(VsRectilinearMesh* rectilinearMesh) {
    //TODO - make "cleanupAndReturnNull" label, and do a "go to" instead of just returning NULL all the time
    
    std::stringstream sstr;
    sstr <<"avtVsFileFormat::getRectilinearMesh() - ";
    std::string methodSig = sstr.str();
    VsLog::debugLog() <<methodSig <<"Entering function." <<std::endl;
    LoadData();

    // Get dimensions
    std::vector<int> dims;
    rectilinearMesh->getMeshDims(&dims, reader->useStride, this->stride);
    size_t rank = rectilinearMesh->getNumSpatialDims();
    
    if (rank > 3) {
      VsLog::debugLog() <<methodSig <<"Error: rank of data is larger than 3." << endl;
      VsLog::debugLog() <<methodSig <<"Returning NULL." << endl;
      return NULL;
    }
    
    // Size of mesh in VisIt (it requires 3 dimensions)
    size_t vsdim = 3;
    
    VsLog::debugLog() <<methodSig <<"Determining size of coordinate arrays." << endl;
    std::vector<int> idims(vsdim);
    for (size_t i = 0; i < vsdim; ++i) {
      // Number of nodes is given by the size of the axis definition arrays
      if (i < rank)
        idims[i] = dims[i];
      else idims[i] = 1; // set unused dims to 1
    }

    //Are coordinate arrays float or double?
    //We assume they are the same
    //TODO - if not the same, convert!
    hid_t ftype = rectilinearMesh->getDataType();
    
    VsLog::debugLog() <<methodSig <<"Building coordinate arrays." << endl;
    vtkPoints* vpoints1 = vtkPoints::New();
    vtkPoints* vpoints2 = vtkPoints::New();
    
    size_t dsize = 0;
    if (isDoubleType(ftype)) {
      VsLog::debugLog() <<methodSig <<"Coordinate arrays are 64-bit real." << endl;
      vpoints1->SetDataTypeToDouble();
      vpoints2->SetDataTypeToDouble();
      dsize = sizeof(double);
    }
    else if (isFloatType(ftype)) {
      VsLog::debugLog() <<methodSig <<"Coordinate arrays are 32-bit real." << endl;
      vpoints1->SetDataTypeToFloat();
      vpoints2->SetDataTypeToFloat();
      dsize = sizeof(float);
    } else {
      VsLog::debugLog() <<methodSig <<"Unknown data type: " <<ftype <<std::endl;
      return NULL;
    }

    vpoints1->SetNumberOfPoints(rank);
    vpoints2->SetNumberOfPoints(rank);

    VsLog::debugLog() <<methodSig <<"Loading data for axis 0." << endl;
    VsH5Dataset* axis0Data = rectilinearMesh->getAxisDataset(0);
    if (axis0Data == NULL) {
      VsLog::debugLog() <<methodSig <<"Axis 0 data not found. Returning NULL." << endl;
      return NULL;
    }

    // Check data type
    if (!H5Tequal(ftype, axis0Data->getType())) {
      VsLog::debugLog() <<methodSig <<"Axis 0 data type is different from declared mesh data type." <<std::endl;
      VsLog::debugLog() <<methodSig <<"Returning NULL" <<std::endl;
      return NULL;
    }

    // Read points and add in zero for any lacking dimension
    VsLog::debugLog() <<methodSig <<"Reading in axis0 data." <<std::endl;
    void* dataPtr0 = 0;
    double* dblDataPtr0 = 0;
    float* fltDataPtr0 = 0;
    if (isDoubleType(ftype)) {
      VsLog::debugLog() <<methodSig <<"Declaring array of doubles of length " <<dims[0] <<"." <<std::endl;
      VsLog::debugLog() <<methodSig <<"Total allocation: " <<(dims[0] * dsize) <<" bytes." <<std::endl;
      dblDataPtr0 = new double[dims[0]];
      dataPtr0 = dblDataPtr0;
    }
    else if (isFloatType(ftype)) {
      VsLog::debugLog() <<methodSig <<"Declaring array of floats of length " <<dims[0] <<"." <<std::endl;
      VsLog::debugLog() <<methodSig <<"Total allocation: " <<(dims[0] * dsize) <<" bytes." <<std::endl;
      fltDataPtr0 = new float[dims[0]];
      dataPtr0 = fltDataPtr0;
    }

    if (!dataPtr0) {
      VsLog::debugLog() <<methodSig <<"Allocation failed, pointer is NULL." <<std::endl;
      VsLog::debugLog() <<methodSig <<"Returning NULL." <<std::endl;
      return NULL;
    }

    VsLog::debugLog() <<methodSig <<"Allocation succeeded.  Now reading in data." <<std::endl;

    // Read in the data
    herr_t err = reader->getDatasetMeshComponent(rectilinearMesh->getAxisDataset(0), *rectilinearMesh, dataPtr0);
    if (err != 0) {
      VsLog::debugLog() <<methodSig <<"Got error " <<err <<" while reading data.  Returning NULL." <<std::endl;
      return NULL;
    }
    

    VsLog::debugLog() <<methodSig <<"Loading data for axis 1." << endl;
    VsH5Dataset* axis1Data = rectilinearMesh->getAxisDataset(1);
    void* dataPtr1 = 0;
    double* dblDataPtr1 = 0;
    float* fltDataPtr1 = 0;
    if (axis1Data != NULL) {
      // Check data type
      if (!H5Tequal(ftype, axis1Data->getType())) {
        VsLog::debugLog() <<methodSig <<"Axis 1 data type is different from declared mesh data type." <<std::endl;
        VsLog::debugLog() <<methodSig <<"Returning NULL" <<std::endl;
        return NULL;
      }

      // Read points and add in zero for any lacking dimension
      VsLog::debugLog() <<methodSig <<"Reading in axis1 data." <<std::endl;
      if (isDoubleType(ftype)) {
        VsLog::debugLog() <<methodSig <<"Declaring array of doubles of length " <<dims[1] <<"." <<std::endl;
        VsLog::debugLog() <<methodSig <<"Total allocation: " <<(dims[1] * dsize) <<" bytes." <<std::endl;
        dblDataPtr1 = new double[dims[1]];
        dataPtr1 = dblDataPtr1;
      }
      else if (isFloatType(ftype)) {
        VsLog::debugLog() <<methodSig <<"Declaring array of floats of length " <<dims[1] <<"." <<std::endl;
        VsLog::debugLog() <<methodSig <<"Total allocation: " <<(dims[1] * dsize) <<" bytes." <<std::endl;
        fltDataPtr1 = new float[dims[1]];
        dataPtr1 = fltDataPtr1;
      } else {
        VsLog::debugLog() <<methodSig <<"Unknown data type: " <<ftype <<std::endl;
        return NULL;
      }
  
      if (!dataPtr1) {
        VsLog::debugLog() <<methodSig <<"Allocation failed, pointer is NULL." <<std::endl;
        VsLog::debugLog() <<methodSig <<"Returning NULL." <<std::endl;
        return NULL;
      }
  
      VsLog::debugLog() <<methodSig <<"Allocation succeeded.  Now reading in data." <<std::endl;
  
      // Read in the data
      herr_t err = reader->getDatasetMeshComponent(axis1Data, *rectilinearMesh, dataPtr1);
      if (err != 0) {
        VsLog::debugLog() <<methodSig <<"Got error " <<err <<" while reading data.  Returning NULL." <<std::endl;
        return NULL;
      }
    }
    
   
    VsLog::debugLog() <<methodSig <<"Loading data for axis 2." << endl;
    VsH5Dataset* axis2Data = rectilinearMesh->getAxisDataset(2);
    double* dblDataPtr2 = 0;
    float* fltDataPtr2 = 0;
    void* dataPtr2 = 0;
    if (axis2Data != NULL) {
      // Check data type
      if (!H5Tequal(ftype, axis2Data->getType())) {
        VsLog::debugLog() <<methodSig <<"Axis 2 data type is different from declared mesh data type." <<std::endl;
        VsLog::debugLog() <<methodSig <<"Returning NULL" <<std::endl;
        return NULL;
      }
      
      // Read points and add in zero for any lacking dimension
      VsLog::debugLog() <<methodSig <<"Reading in axis2 data." <<std::endl;
      if (isDoubleType(ftype)) {
        VsLog::debugLog() <<methodSig <<"Declaring array of doubles of length " <<dims[2] <<"." <<std::endl;
        VsLog::debugLog() <<methodSig <<"Total allocation: " <<(dims[2] * dsize) <<" bytes." <<std::endl;
        dblDataPtr2 = new double[dims[2]];
        dataPtr2 = dblDataPtr2;
      }
      else if (isFloatType(ftype)) {
        VsLog::debugLog() <<methodSig <<"Declaring array of floats of length " <<dims[2] <<"." <<std::endl;
        VsLog::debugLog() <<methodSig <<"Total allocation: " <<(dims[2] * dsize) <<" bytes." <<std::endl;
        fltDataPtr2 = new float[dims[2]];
        dataPtr2 = fltDataPtr2;
      } else {
        VsLog::debugLog() <<methodSig <<"Unknown data type: " <<ftype <<std::endl;
        return NULL;
      }
  
      if (!dataPtr2) {
        VsLog::debugLog() <<methodSig <<"Allocation failed, pointer is NULL." <<std::endl;
        VsLog::debugLog() <<methodSig <<"Returning NULL." <<std::endl;
        return NULL;
      }
  
      VsLog::debugLog() <<methodSig <<"Allocation succeeded.  Now reading in data." <<std::endl;
  
      // Read in the data
      herr_t err = reader->getDatasetMeshComponent(axis2Data, *rectilinearMesh, dataPtr2);
      if (err != 0) {
        VsLog::debugLog() <<methodSig <<"Got error " <<err <<" while reading data.  Returning NULL." <<std::endl;
        return NULL;
      }
    }
    
    // Create vtkRectilinearGrid
    VsLog::debugLog() <<methodSig <<"Creating rectilinear grid." << endl;
    vtkRectilinearGrid* rgrid = vtkRectilinearGrid::New();
    rgrid->SetDimensions(&idims[0]);
  
    // Create coords arrays
    VsLog::debugLog() << methodSig << "Creating coordinate arrays." << endl;
    std::vector<vtkDataArray*> coords(vsdim);
    if (isDoubleType(ftype)) {
      //axis 0
      coords[0] = vtkDoubleArray::New();
      for (int j = 0; j < idims[0]; ++j) {
        double temp = dblDataPtr0[j];
        coords[0]->InsertTuple(j, &temp);
      }
      
      //axis 1
      coords[1] = vtkDoubleArray::New();
      if (rank > 1) {
        for (int j = 0; j < idims[1]; ++j) {
          double temp = dblDataPtr1[j];
          coords[1]->InsertTuple(j, &temp);
        }
      } else {
        coords[1]->SetNumberOfTuples(1);
        coords[1]->SetComponent(0, 0, 0);    
      }
      
      //axis 2
      coords[2] = vtkDoubleArray::New();
      if (rank > 2) {
        for (int j = 0; j < idims[2]; ++j) {
          double temp = dblDataPtr2[j];
          coords[2]->InsertTuple(j, &temp);
        }
      } else {
        coords[2]->SetNumberOfTuples(1);
        coords[2]->SetComponent(0, 0, 0);
      }
    } else if (isFloatType(ftype)) {
      //axis 0
      coords[0] = vtkFloatArray::New();
      for (int j = 0; j < idims[0]; ++j) {
        float temp = fltDataPtr0[j];
        coords[0]->InsertTuple(j, &temp);
      }
      
      //axis 1
      coords[1] = vtkFloatArray::New();
      if (rank > 1) {
        for (int j = 0; j < idims[1]; ++j) {
          float temp = fltDataPtr1[j];
          coords[1]->InsertTuple(j, &temp);
        }
      } else {
        coords[1]->SetNumberOfTuples(1);
        coords[1]->SetComponent(0, 0, 0);    
      }
      
      //axis 2
      coords[2] = vtkFloatArray::New();
      if (rank > 2) {
        for (int j = 0; j < idims[2]; ++j) {
          float temp = fltDataPtr2[j];
          coords[2]->InsertTuple(j, &temp);
        }
      } else {
        coords[2]->SetNumberOfTuples(1);
        coords[2]->SetComponent(0, 0, 0);
      }
    } else {
      VsLog::debugLog() <<methodSig <<"Unknown data type: " <<ftype <<std::endl;
      return NULL;
    }

    // Set grid data
    VsLog::debugLog() <<methodSig <<"Adding coordinates to grid." << endl;
    rgrid->SetXCoordinates(coords[0]);
    rgrid->SetYCoordinates(coords[1]);
    rgrid->SetZCoordinates(coords[2]);

    // Clean local data
    VsLog::debugLog() <<methodSig <<"Cleaning up." << endl;
    for (size_t i = 0; i < vsdim; ++i) {
      coords[i]->Delete();
    }

    if (fltDataPtr0) delete fltDataPtr0;
    if (fltDataPtr1) delete fltDataPtr1;
    if (fltDataPtr2) delete fltDataPtr2;
    if (dblDataPtr0) delete dblDataPtr0;
    if (dblDataPtr1) delete dblDataPtr1;
    if (dblDataPtr2) delete dblDataPtr2;
    VsLog::debugLog() <<methodSig <<"Returning data." << endl;
    return rgrid;
  }
  
  //end getReclinearMesh

  vtkDataSet* avtVsFileFormat::getUniformMesh(VsUniformMesh* uniformMesh) {
    std::stringstream sstr;
    sstr <<"avtVsFileFormat::getUniformMesh() - ";
    std::string methodSig = sstr.str();
    VsLog::debugLog() <<methodSig <<"Entering function." <<std::endl;
    LoadData();

    // Read int data
    std::vector<int> numCells;
    uniformMesh->getMeshDims(&numCells, reader->useStride, this->stride);

    size_t rank = uniformMesh->getNumSpatialDims();
    //Since VisIt downcasts to float anyways, we go float by default now.
    //hid_t ftype = uniformMesh->getDataType();

    // startCell
    VsLog::debugLog() <<methodSig <<"Loading optional startCells attribute." << endl;
    std::vector<int> startCell;
    herr_t err = uniformMesh->getStartCell(&startCell);
    if (err < 0) {
      VsLog::warningLog() <<methodSig <<"Uniform mesh does not have information about starting cell position." <<std::endl;
    } else {
      // Adjust the box by startCell
      VsLog::debugLog() <<methodSig <<"Adjusting numCells by startCells." << endl;
      for (size_t i = 0; i < rank; ++i)
      numCells[i] -= startCell[i];
    }

    // Storage for mesh in VisIt, and it expects 3D
    size_t vsdim = 3;
    if (rank > 3) {
      VsLog::debugLog() <<methodSig <<"Error: rank of data is larger than 3." << endl;
      VsLog::debugLog() <<methodSig <<"Returning NULL." << endl;
      return NULL;
    }

    VsLog::debugLog() <<methodSig <<"Determining size of point arrays." << endl;
    std::vector<int> idims(vsdim);
    for (size_t i = 0; i < vsdim; ++i) {
      // Number of nodes is equal to number of cells plus one
      if (i<rank)
      idims[i] = numCells[i]+1;
      else idims[i] = 1; // set unused dims to 1
    }

    VsLog::debugLog() <<methodSig <<"Getting lower bounds for mesh." << endl;
    std::vector<float> lowerBounds;
    uniformMesh->getLowerBounds(&lowerBounds);

    VsLog::debugLog() <<methodSig <<"Getting upper bounds for mesh." << endl;
    std::vector<float> upperBounds;
    uniformMesh->getUpperBounds(&upperBounds);

#if (defined PARALLEL && defined VIZSCHEMA_DECOMPOSE_DOMAINS)
    VsLog::debugLog() <<methodSig <<"Parallel & Decompose_Domains are defined, entering parallel code." << endl;
    size_t splitAxis = 0;
    size_t largestCount = numCells[splitAxis];

    for (size_t currAxis = 1; currAxis < rank; ++currAxis) {
      if (numCells[currAxis] > largestCount) {
        splitAxis = currAxis;
        largestCount = numCells[currAxis];
      }
    }

    VsLog::debugLog() <<methodSig <<"Splitting along axis " << splitAxis << endl;

    // FIXME: Figure out exact upper bounds/number of cells semantics

    // Split along axis
    size_t numCellsAlongSplitAxis = numCells[splitAxis]; // FIXME: May depend on centering
    if (numCellsAlongSplitAxis) {
      size_t numCellsPerPart = numCellsAlongSplitAxis / PAR_Size();
      size_t numPartsWithAdditionalCell = numCellsAlongSplitAxis % PAR_Size();

      size_t startCell, nCells;
      if (PAR_Rank() < numPartsWithAdditionalCell)
      {
        startCell = PAR_Rank() * (numCellsPerPart + 1);
        nCells = numCellsPerPart + 1;
      }
      else
      {
        startCell = numPartsWithAdditionalCell * (numCellsPerPart + 1) +
        (PAR_Rank() - numPartsWithAdditionalCell) * numCellsPerPart;
        nCells = numCellsPerPart;
      }
      // Adjust bounds
      float delta = (upperBounds[splitAxis] - lowerBounds[splitAxis])/numCellsAlongSplitAxis;
      lowerBounds[splitAxis] += startCell * delta;
      upperBounds[splitAxis] = lowerBounds[splitAxis] + nCells * delta;
      //std::cout << "After adjust: Proc=" << PAR_Rank() << " start=" << startCell << " nCells=" << nCells << " delta=" << delta << " lowerBound=" << lowerBounds[splitAxis] << " upperBound=" << upperBounds[splitAxis] << std::endl;
      numCells[splitAxis] = nCells;
      idims[splitAxis] = nCells + 1; // FIXME: May depend on centering
    }
    VsLog::debugLog() <<methodSig <<"Parallel & Decompose_Domains are defined, exiting parallel code." << endl;
#endif

    // Create vtkRectilinearGrid
    VsLog::debugLog() <<methodSig <<"Creating rectilinear grid." << endl;
    vtkRectilinearGrid* rgrid = vtkRectilinearGrid::New();
    rgrid->SetDimensions(&idims[0]);

    // Create coords arrays
    VsLog::debugLog() <<methodSig <<"Creating coordinate arrays." << endl;
    std::vector<vtkDataArray*> coords(vsdim);
    for (size_t i = 0; i<rank;++i) {
      float delta = 0;
      coords[i] = vtkFloatArray::New();
      if (numCells[i] != 0)
        delta = (upperBounds[i] - lowerBounds[i]) / numCells[i];
      VsLog::debugLog() <<methodSig <<"delta is : " <<delta <<std::endl;
      for (size_t j = 0; j < idims[i]; ++j) {
        float temp = lowerBounds[i] + j*delta;
        coords[i]->InsertTuple(j, &temp);
      }
    }

    VsLog::debugLog() <<methodSig <<"Clearing data in unused coordinate arrays." << endl;
    for (size_t i = rank; i < vsdim; ++i) {
      coords[i] = vtkFloatArray::New();
      coords[i]->SetNumberOfTuples(1);
      coords[i]->SetComponent(0, 0, 0);
    }
          
    // Set grid data
    VsLog::debugLog() <<methodSig <<"Adding coordinates to grid." << endl;
    rgrid->SetXCoordinates(coords[0]);
    rgrid->SetYCoordinates(coords[1]);
    rgrid->SetZCoordinates(coords[2]);

    // Clean local data
    VsLog::debugLog() <<methodSig <<"Cleaning up." << endl;
    for (size_t i = 0; i<vsdim; ++i)
    coords[i]->Delete();

    VsLog::debugLog() <<methodSig <<"Returning data." << endl;
    return rgrid;
  }

  vtkDataSet* avtVsFileFormat::getPointMesh(VsVariableWithMesh* meta) {
    std::stringstream sstr;
    sstr <<"avtVsFileFormat::getPointMesh() - ";
    std::string methodSig = sstr.str();
    VsLog::debugLog() <<methodSig <<"Entering function." <<std::endl;
    LoadData();

    hid_t type = meta->getType();
    if (!isDoubleType(type) && !isFloatType(type)) {
      VsLog::debugLog() <<methodSig <<"Unsupported data type: " <<type <<std::endl;
      VsLog::debugLog() <<methodSig <<"Returning NULL." <<std::endl;
      return NULL;
    }

    // Get the number of values
    VsLog::debugLog() <<methodSig <<"Getting the number of points." <<std::endl;
    std::vector<int> dims = meta->getDims();
    int numValues = 0;
    if (meta->isCompMajor())
    numValues = dims[dims.size() - 1];
    else
    numValues = dims[0];

    //Stride
    if (stride[0] != 1) {
      VsLog::debugLog() <<methodSig <<"Filtering points based on stride.  Before = " <<numValues <<std::endl;
      numValues = numValues / stride[0];
      VsLog::debugLog() <<methodSig <<"Filtering points based on stride.  After = " <<numValues <<std::endl;
    }
#if (defined PARALLEL && defined VIZSCHEMA_DECOMPOSE_DOMAINS)
    VsLog::debugLog() <<methodSig <<"Parallel & Decompose_Domains are defined, entering parallel code." << endl;
    size_t numPartsPerProc = numValues / PAR_Size();
    size_t numProcsWithExtraPart = numValues % PAR_Size();
    size_t start, count;
    if (PAR_Rank() < numProcsWithExtraPart)
    {
      start = PAR_Rank() * (numPartsPerProc+1);
      count = numPartsPerProc + 1;
    }
    else
    {
      start = numProcsWithExtraPart * (numPartsPerProc + 1) + (PAR_Rank() - numProcsWithExtraPart) * numPartsPerProc;
      count = numPartsPerProc;
    }
    numValues = count;
    VsLog::debugLog() <<methodSig <<"Parallel & Decompose_Domains are defined, exiting parallel code." << endl;
#endif

    VsLog::debugLog() <<methodSig <<"There are " << numValues <<
    " points." << endl;

    // Read in points
    //
    // Create the unstructured meshPtr
    VsLog::debugLog() <<methodSig <<"Creating the vtkUnstructuredGrid." <<std::endl;
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
      VsLog::debugLog() <<"Double data" <<std::endl;
    }
    else if (isFloatType(type)) {
      vpoints->SetDataTypeToFloat();
      dsize = sizeof(float);
      VsLog::debugLog() <<"Float data" <<std::endl;
    } else {
      VsLog::debugLog() <<methodSig <<"Unknown data type: " <<type <<std::endl;
      return NULL;
    }
    
    VsLog::debugLog() <<methodSig <<"Allocating for " <<numValues <<" values." <<std::endl;
    vpoints->SetNumberOfPoints(numValues);
    void* dataPtr = vpoints->GetVoidPointer(0);
    if (!dataPtr) {
      VsLog::debugLog() << methodSig <<"Unable to allocate the points.  Cleaning up."
      << std::endl;
      meshPtr->Delete();
      VsLog::debugLog() << methodSig <<"Returning NULL." <<std::endl;
      return NULL;
    }

    // Read in the data
    VsLog::debugLog() << methodSig <<"Reading data." <<std::endl;
#if (defined PARALLEL && defined VIZSCHEMA_DECOMPOSE_DOMAINS)
    herr_t err = reader->getVarWithMeshMesh(*meta, dataPtr, start, count);
#else
    herr_t err = reader->getVarWithMeshMesh(*meta, dataPtr);
#endif

    if (err < 0) {
      VsLog::debugLog() << methodSig <<"Call to getVarWithMeshMeta returned error: " <<err <<endl;
      VsLog::debugLog() << methodSig <<"Returning NULL." <<std::endl;
      return NULL;
    } else {
      VsLog::debugLog() << methodSig <<"Mesh points read." << endl;
    }

    // Move the data back to where it should be in a 3D array
    // WORK HERE - does this work properly for compMajor?
    if (meta->getNumSpatialDims() < 3) {
      VsLog::debugLog() << methodSig <<"Moving data into correct positions." << endl;
      for (int i=numValues-1; i>=0; --i) {
        char* destPtr = (char*) dataPtr + i*3*dsize;
        char* srcPtr = (char*) dataPtr + i*meta->getNumSpatialDims()*dsize;
        memmove(destPtr, srcPtr, meta->getNumSpatialDims()*dsize);
        destPtr += meta->getNumSpatialDims()*dsize;
        memset(destPtr, 0, (3-meta->getNumSpatialDims())*dsize);
      }
    }

    // create point mesh
    try {
      VsLog::debugLog() <<methodSig <<"Allocating " <<numValues
      << " vertices.  If old VTK and this fails, it will just abort." << endl;
      meshPtr->Allocate(numValues);
    } catch (vtkstd::bad_alloc) {
      VsLog::debugLog() <<methodSig <<"Caught vtkstd::bad_alloc. Unable to allocate cells."
      <<"Cleaning up." <<std::endl;
      meshPtr->Delete();
      VsLog::debugLog() <<methodSig <<"Returning NULL." <<std::endl;
      return NULL;
    } catch (...) {
      VsLog::debugLog() <<methodSig <<"Unknown exception. Unable to allocate cells."
      <<"Cleaning up." <<std::endl;
      meshPtr->Delete();
      VsLog::debugLog() <<methodSig <<"Returning NULL." <<std::endl;
      return NULL;
    }

    VsLog::debugLog() <<methodSig <<"Allocation succeeded.  Setting mesh to connectivity 'VERTEX'." <<std::endl;
    vtkIdType vertex;
    for (int i = 0; i < numValues; ++i) {
      vertex = i;
      meshPtr->InsertNextCell(VTK_VERTEX, 1, &vertex);
    }

    // Clean up memory
    ///TODO: IS there anything to clean up???
    VsLog::debugLog() <<methodSig <<"Returning data." <<std::endl;
    return meshPtr;
  }

  vtkDataSet* avtVsFileFormat::getSplitPointMesh(VsUnstructuredMesh* unstructuredMesh) {
    std::stringstream sstr;
    sstr <<"avtVsFileFormat::getSplitPointMesh() - ";
    std::string methodSig = sstr.str();
    VsLog::debugLog() <<methodSig <<"Entering function." <<std::endl;
    LoadData();

    hid_t type0 = unstructuredMesh->getDataType();
    if (!isDoubleType(type0) && !isFloatType(type0)) {
      VsLog::debugLog() <<methodSig <<"Points are neither float nor double." <<std::endl;
      VsLog::debugLog() <<methodSig <<"Returning NULL" <<std::endl;
      return NULL;
    }

    // Get the number of values
    VsLog::debugLog() <<methodSig <<"Getting the number of points." <<std::endl;
    int numValues = unstructuredMesh->getNumPoints();
    VsLog::debugLog() <<methodSig <<"There are " << numValues <<" points." << endl;

    // Read in points
    //
    // Create the unstructured meshPtr
    VsLog::debugLog() <<methodSig <<"Creating the vtkUnstructuredGrid." <<std::endl;
    vtkUnstructuredGrid* meshPtr = vtkUnstructuredGrid::New();

    // Create and set points while small so minimal memory usage
    vtkPoints* vpts = vtkPoints::New();
    meshPtr->SetPoints(vpts);
    vpts->Delete();
    vtkPoints* vpoints = meshPtr->GetPoints();

    // Allocate
    size_t dsize = 0;
    if (isDoubleType(type0)) {
      vpoints->SetDataTypeToDouble();
      dsize = sizeof(double);
      VsLog::debugLog() <<"Double data" <<std::endl;
    }
    else if (isFloatType(type0)) {
      vpoints->SetDataTypeToFloat();
      dsize = sizeof(float);
      VsLog::debugLog() <<"Float data" <<std::endl;
    } else {
      VsLog::debugLog() <<methodSig <<"Unknown data type: " <<type0 <<std::endl;
      return NULL;
    }

    VsLog::debugLog() <<methodSig <<"Allocating for " <<numValues <<" values." <<std::endl;
    vpoints->SetNumberOfPoints(numValues);
    void* dataPtr = vpoints->GetVoidPointer(0);
    if (!dataPtr) {
      VsLog::debugLog() << methodSig <<"Unable to allocate the points.  Cleaning up."
      << std::endl;
      meshPtr->Delete();
      VsLog::debugLog() << methodSig <<"Returning NULL." <<std::endl;
      return NULL;
    }

    // Read in the data
    VsLog::debugLog() << methodSig <<"Reading data." <<std::endl;

    herr_t err = reader->getSplitMeshData(*unstructuredMesh, dataPtr);

    if (err < 0) {
      VsLog::debugLog() << methodSig <<"Call to getVarWithMeshMeta returned error: " <<err <<endl;
      VsLog::debugLog() << methodSig <<"Returning NULL." <<std::endl;
      return NULL;
    } else {
      VsLog::debugLog() << methodSig <<"Mesh points read." << endl;
    }

    // create point mesh
    try {
      VsLog::debugLog() <<methodSig <<"Allocating " <<numValues
      << " vertices.  If old VTK and this fails, it will just abort." << endl;
      meshPtr->Allocate(numValues);
    } catch (vtkstd::bad_alloc) {
      VsLog::debugLog() <<methodSig <<"Caught vtkstd::bad_alloc. Unable to allocate cells."
      <<"Cleaning up." <<std::endl;
      meshPtr->Delete();
      VsLog::debugLog() <<methodSig <<"Returning NULL." <<std::endl;
      return NULL;
    } catch (...) {
      VsLog::debugLog() <<methodSig <<"Unknown exception. Unable to allocate cells."
      <<"Cleaning up." <<std::endl;
      meshPtr->Delete();
      VsLog::debugLog() <<methodSig <<"Returning NULL." <<std::endl;
      return NULL;
    }

    VsLog::debugLog() <<methodSig <<"Allocation succeeded.  Setting mesh to connectivity 'VERTEX'." <<std::endl;
    vtkIdType vertex;
    for (int i = 0; i < numValues; ++i) {
      vertex = i;
      meshPtr->InsertNextCell(VTK_VERTEX, 1, &vertex);
    }

    // Clean up memory
    ///TODO: IS there anything to clean up???
    VsLog::debugLog() <<methodSig <<"Returning data." <<std::endl;
    return meshPtr;
  }

  vtkDataArray* avtVsFileFormat::GetVar(int domain, const char* requestedName) {

    int mins[3], maxs[3], strides[3];
    if( 0 && ProcessDataSelections(mins, maxs, strides) )
    {
      cerr << "have a data selection for a variable "<< endl
           << "(" << mins[0] << "," << maxs[0] << " stride " << strides[0] << ") "
           << "(" << mins[1] << "," << maxs[1] << " stride " << strides[1] << ") "
           << "(" << mins[2] << "," << maxs[2] << " stride " << strides[2] << ") "
           << endl;
    }

    std::string name = requestedName;

    std::stringstream sstr;
    sstr <<"avtVsFileFormat::getVar(" <<domain <<", " <<name <<") - ";
    std::string methodSig = sstr.str();
    VsLog::debugLog() <<methodSig <<"Entering function." <<std::endl;
    LoadData();

    //Is this a component?
    //If so, we swap the component name with the "real" variable name
    //And remember that we're a component
    bool isAComponent = false;
    int componentIndex = 0;
    NamePair foundName;
    registry->getComponentInfo(name, &foundName);
    if (!foundName.first.empty()) {
      name = foundName.first.c_str();
      componentIndex = foundName.second;
      VsLog::debugLog() <<methodSig <<"This is a component, and actually refers to variable " <<name <<" and index " <<componentIndex <<std::endl;
      isAComponent = true;
    }

    //the goal in all of this metadata loading is to fill one of these two variables:
    VsVariable* meta = NULL;
    VsVariableWithMesh* vmMeta = NULL;

    //could be an MD variable
    //if so, we retrieve the md metadata, look up the "real" variable name using the domain number,
    //and replace "name" with the name of that variable
    VsLog::debugLog() <<methodSig <<"Checking for possible MD var." <<std::endl;
    VsMDVariable* mdMeta = registry->getMDVariable(name);
    if (mdMeta == NULL) {
      VsLog::debugLog() <<methodSig <<"No MD Var or component found under the name: " <<name <<std::endl;
    } else {
      VsLog::debugLog() <<methodSig <<"Found MD metadata for this name: " <<name <<std::endl;
      if ((domain < 0) || (domain > mdMeta->blocks.size())) {
        VsLog::warningLog() <<methodSig <<"Requested domain number is out of bounds for this variable." <<std::endl;
      } else {
        meta = mdMeta->blocks[domain];
        name = meta->getFullName();
      }
    }

    //Have we managed to retrieve the metadata for the variable yet?
    // if not, look for a "regular" variable with this name
    if (meta == NULL) {
      VsLog::debugLog() <<methodSig <<"Looking for regular (non-md) variable." <<std::endl;
      meta = registry->getVariable(name);
    }

    //How about now?
    // If no, look for a VarWithMesh with this name
    if (meta == NULL) {
      VsLog::debugLog() <<methodSig <<"Looking for VarWithMesh variable." <<std::endl;
      vmMeta = registry->getVariableWithMesh(name);
    }

    //If we haven't found metadata yet, we give up
    if ((meta == NULL) && (vmMeta == NULL)) {
      VsLog::debugLog() << methodSig <<"ERROR: Could not find metadata for name: "<<name <<std::endl;
      //DEBUG CODE
      std::vector<std::string> varNames;
      registry->getAllVariableNames(varNames);
      VsLog::debugLog() << methodSig <<"All available names = ";
      for (unsigned int i = 0; i < varNames.size(); i++) {
        VsLog::debugLog() << varNames[i] <<", ";
      }
      VsLog::debugLog() <<std::endl;

      VsLog::debugLog() << "Returning NULL" << endl;
      //END DEBUG
      return NULL;
    }

    //ok, we have metadata from the variable
    //load some info and look for the mesh
    hid_t type = 0;
    std::vector<int> dims;
    bool isCompMajor = false;
    if (vmMeta) {
      dims = vmMeta->getDims();
      type = vmMeta->getType();
      if (vmMeta->isCompMajor()) {
        isCompMajor = true;
        if (reader->useStride) {
          dims[1] = dims[1] / stride[0];
        }
      } else {
        if (reader->useStride) {
          dims[0] = dims[0] / stride[0];
        }
      }
      //note that there's no mesh metadata for VarWithMesh
    } else {
      dims = meta->getDims();
      std::string meshName = meta->getMeshName();
      type = meta->getType();

      VsLog::debugLog() <<methodSig <<"Mesh for variable is '" <<meshName << "'." << endl;
      VsLog::debugLog() <<methodSig <<"Getting metadata for mesh." << endl;
      VsMesh* meshMetaPtr = registry->getMesh(meshName);

      if (meshMetaPtr) {
        VsLog::debugLog() <<methodSig <<"Found metadata for "
        << "mesh '" << meshName << "'." << endl;
      }
      else {
        VsLog::debugLog() <<methodSig <<"Metadata not found "
        "for mesh '" << meshName << "'." << endl;
        VsLog::debugLog() <<methodSig <<"Returning NULL" <<std::endl;
        return NULL;
      }
    }

    VsLog::debugLog() <<methodSig <<"Determining dimensionality." <<std::endl;
    size_t rank = dims.size();
    if (isAComponent) --rank;

    //this gets complicated because it depends on the kind of variable (zonal vs nodal)
    // and the kind of the mesh (uniform vs structured)
    // We must also be careful to stay away if the mesh is NOT uniform or structured
    //specifically we must stay away from VarWithMesh, since it has already adjusted the sizes
    if (reader->useStride && (vmMeta == NULL)) {
      VsMesh* meshMeta = meta->getMesh();
      if (!meshMeta) {
        VsLog::debugLog() <<methodSig <<"ERROR - Unable to load mesh metadata with name : " <<meta->getMeshName() <<std::endl;
        VsLog::debugLog() <<methodSig <<"ERROR - returning NULL" <<std::endl;
        return NULL;
      }
      
      int addBefore = 0;
      int addAfter = 0;
      if (meshMeta->isUniformMesh()) {
        if (meta->isZonal()) {
          VsLog::debugLog() <<methodSig <<"Zonal on uniform = no change" <<std::endl;
          adjustSize_vector(&dims, rank, stride, addBefore, addAfter);
        } else {
          //nodal
          VsLog::debugLog() <<methodSig <<"Nodal on uniform = -1/+1" <<std::endl;
          addBefore = -1;
          addAfter = 1;
          adjustSize_vector(&dims, rank, stride, addBefore, addAfter);
        }
      } else if (meshMeta->isStructuredMesh()) {
        VsStructuredMesh* structuredMesh = static_cast<VsStructuredMesh*>(meshMeta);
        if (meta->isZonal()) {
          VsLog::debugLog() <<methodSig <<"Zonal on structured = +1/-1" <<std::endl;
          std::vector<int> meshDims;
          structuredMesh->getMeshDims(&meshDims, reader->useStride, this->stride);
          for (unsigned int i = 0; i < rank; i++)
          {
            VsLog::debugLog() <<methodSig <<"About to override size " <<dims[i] <<" with size from mesh - 1: " <<(meshDims[i] - 1) <<std::endl;
            dims[i] = meshDims[i] - 1;
          }

        } else {
          //nodal
          VsLog::debugLog() <<methodSig <<"Nodal on structured" <<std::endl;
          std::vector<int> meshDims;
          structuredMesh->getMeshDims(&meshDims, reader->useStride, this->stride);
          for (unsigned int i = 0; i < rank; i++)
          {
            VsLog::debugLog() <<methodSig <<"About to override size " <<dims[i] <<" with size from mesh: " <<(meshDims[i]) <<std::endl;
            dims[i] = meshDims[i];
          }
        }
      }
    }

    VsLog::debugLog() <<methodSig << "Variable " <<name << " has dimensions =";
    size_t len = 1;
    if (isCompMajor) {
      for (size_t i=1; i < rank + 1; ++i) {
        VsLog::debugLog() << " " << dims[i];
        len *= dims[i];
      }
    } else {
      for (size_t i=0; i < rank; ++i) {
        VsLog::debugLog() << " " << dims[i];
        len *= dims[i];
      }
    }
    VsLog::debugLog() << ", rank = " << rank << ", isComponent = " <<
    isAComponent << "." << std::endl;
    VsLog::debugLog() <<"Length is " <<len <<std::endl;

    VsLog::debugLog() <<methodSig <<"Declaring vtkArray of proper type." <<std::endl;
    vtkDataArray* rv = 0;
    if (isDoubleType(type)) {
      VsLog::debugLog() <<methodSig <<"Declaring vtkDoubleArray." <<std::endl;
      rv = vtkDoubleArray::New();
    }
    else if (isFloatType(type)) {
      VsLog::debugLog() <<methodSig <<"Declaring vtkFloatArray." <<std::endl;
      rv = vtkFloatArray::New();
    }
    else if (isIntegerType(type)) {
      VsLog::debugLog() <<methodSig <<"Declaring vtkIntArray." <<std::endl;
      rv = vtkIntArray::New();
    }
    else {
      VsLog::debugLog() <<methodSig <<"Unknown data type:" <<type <<std::endl;
      VsLog::debugLog() <<methodSig <<"Returning NULL." << std::endl;
      return NULL;
    }

    // Read in the data
    VsLog::debugLog() << methodSig <<"Reading in the data." << endl;
    void* data = 0;
    //  size_t len = 0;
    size_t sz = H5Tget_size(type);
    if (vmMeta) {
      VsLog::debugLog() << methodSig <<"Entering VarWithMesh section." << endl;
      //    int lastDim = dims[dims.size()-1];
      //    len = vmmeta->getLength()/lastDim;
#if (defined PARALLEL && defined VIZSCHEMA_DECOMPOSE_DOMAINS)
      VsLog::debugLog() <<methodSig <<"Parallel & Decompose_Domains are defined, entering parallel code." << endl;
      size_t numPartsPerProc = len / PAR_Size();
      size_t numProcsWithExtraPart = len % PAR_Size();
      size_t start, count;
      if (PAR_Rank() < numProcsWithExtraPart) {
        start = PAR_Rank() * (numPartsPerProc+1);
        count = numPartsPerProc + 1;
      }
      else {
        start = numProcsWithExtraPart * (numPartsPerProc + 1) + (PAR_Rank() - numProcsWithExtraPart) * numPartsPerProc;
        count = numPartsPerProc;
      }
      len = count;
      VsLog::debugLog() <<methodSig <<"Parallel & Decompose_Domains are defined, exiting parallel code." << endl;
#endif
      //changed from "new unsigned" to "new char" because unsigned is 4 bytes, char is 1
      VsLog::debugLog() <<methodSig <<"About to allocate space for " <<len <<" values of size " <<sz <<"." << endl;
      VsLog::debugLog() <<methodSig <<"Total allocation: " <<(len * sz) <<" bytes." << endl;
      data = new char[len*sz];
      if (!data) {
        VsLog::debugLog() <<methodSig <<"Unable to allocate memory." << endl;
        VsLog::debugLog() <<methodSig <<"Returning NULL." << endl;
        return NULL;
      }

      VsLog::debugLog() <<methodSig <<"Reading var with mesh data." << endl;
#if (defined PARALLEL && defined VIZSCHEMA_DECOMPOSE_DOMAINS)
      herr_t err = reader->getVarWithMeshComponent(name, componentIndex, data, start, count);
#else
      herr_t err = reader->getVarWithMeshComponent(name, componentIndex, data);
#endif
      if (err < 0) {
        VsLog::debugLog() <<methodSig <<"GetVarWithMeshComponent returned error: " <<err <<endl;
        VsLog::debugLog() <<methodSig <<"Returning NULL." << endl;
        return NULL;
      }
    }
    else if (isAComponent) {
      VsLog::debugLog() << methodSig <<"Entering Component section." << endl;
      // Read a var comp
#if (defined PARALLEL && defined VIZSCHEMA_DECOMPOSE_DOMAINS)
      VsLog::debugLog() <<methodSig <<"Parallel & Decompose_Domains are defined, entering parallel code." << endl;
      size_t splitDims[rank];
      data = reader->getVariableComponent(name, componentIndex, PAR_Rank(), PAR_Size(), splitDims);
      if (!data) return 0;
      VsLog::debugLog() << methodSig <<"Original dimensions are";
      for (size_t i=0; i<rank; ++i) {
        VsLog::debugLog() << " " << dims[i];
      }
      len = 1;
      for (size_t i=0; i<rank; ++i) {
        dims[i] = splitDims[i];
        len *= dims[i];
      }
      VsLog::debugLog() << std::endl;
      VsLog::debugLog() <<methodSig <<"Dimensions after split are";
      for (size_t i=0; i<rank; ++i) {
        VsLog::debugLog() << " " << dims[i];
      }
      VsLog::debugLog() << " len=" << len << std::endl;
      VsLog::debugLog() <<methodSig <<"Parallel & Decompose_Domains are defined, exiting parallel code." << endl;
#else
      VsLog::debugLog() <<methodSig <<"Determining number of components for variable." <<std::endl;
      /*    size_t nc = reader->getNumComps(nm);
       if (!nc) {
       VsLog::debugLog() <<methodSig <<"Number of components is zero for variable." <<endl;
       VsLog::debugLog() <<methodSig <<"Returning NULL." <<endl;
       return NULL;
       } else {
       VsLog::debugLog() <<methodSig <<"Variable has " <<nc <<" components." <<std::endl;
       }
       */
      //    len = meta->getLength()/nc;
      //changed from "new unsigned" to "new char" because unsigned is 4 bytes, char is 1
      VsLog::debugLog() <<methodSig <<"About to allocate space for " <<len <<" values of size " <<sz <<"." << endl;
      VsLog::debugLog() <<methodSig <<"Total allocation: " <<(len * sz) <<" bytes." << endl;
      data = new char[len*sz];
      if (!data) {
        VsLog::debugLog() <<methodSig <<"Unable to allocate memory." << endl;
        VsLog::debugLog() <<methodSig <<"Returning NULL." << endl;
        return NULL;
      }

      VsLog::debugLog() <<methodSig <<"Loading variable data." << endl;
      herr_t err = reader->getVariableComponent(name, componentIndex, data);
      if (err < 0) {
        VsLog::debugLog() <<methodSig <<"GetVariableComponent returned error: " <<err <<endl;
        VsLog::debugLog() <<methodSig <<"Returning NULL." << endl;
        return NULL;
      } else {
        VsLog::debugLog() <<methodSig <<"Found component " << componentIndex <<" for variable " <<name <<std::endl;
      }
#endif
    }
    else {
      VsLog::debugLog() << methodSig <<"Entering regular Variable section." << endl;
#if (defined PARALLEL && defined VIZSCHEMA_DECOMPOSE_DOMAINS)
      // Don't know how to decompose this type of mesh -> load it on proc 0 only
      if (PAR_Rank() > 0)
      {
        return NULL;
      }
#endif
      //   len = meta->getLength();
      //changed from "new unsigned" to "new char" because unsigned is 4 bytes, char is 1
      VsLog::debugLog() <<methodSig <<"About to allocate space for " <<len <<" values of size " <<sz <<"." << endl;
      VsLog::debugLog() <<methodSig <<"Total allocation: " <<(len * sz) <<" bytes." << endl;
      data = new char[len*sz];
      if (!data) {
        VsLog::debugLog() <<methodSig <<"Unable to allocate memory." << endl;
        VsLog::debugLog() <<methodSig <<"Returning NULL." << endl;
        return NULL;
      }

      VsLog::debugLog() <<methodSig <<"Loading variable data." << endl;
      herr_t err = reader->getVariable(name, data);
      if (err < 0) {
        VsLog::debugLog() <<methodSig <<"GetVariable returned error: " <<err <<endl;
        VsLog::debugLog() <<methodSig <<"Returning NULL." << endl;
        return NULL;
      } else {
        VsLog::debugLog() <<methodSig <<"Successfully loaded data." <<std::endl;
      }
    }

    VsLog::debugLog() <<methodSig <<"Finished reading the data, building VTK structures." <<endl;

    //DEBUG
    /*
     VsLog::debugLog() <<methodSig <<"Dumping data: " <<std::endl;
     for (int i = 0; i < len; i++) {
     if (H5Tequal(type, H5T_NATIVE_DOUBLE)) {
     VsLog::debugLog() <<"data[" <<i <<"] = " <<((double*)data)[i] <<std::endl;}
     else if (H5Tequal(type, H5T_NATIVE_FLOAT)) {
     VsLog::debugLog() <<"data[" <<i <<"] = " <<((float*)data)[i] <<std::endl;}
     else if (H5Tequal(type, H5T_NATIVE_INT)) {
     VsLog::debugLog() <<"data[" <<i <<"] = " <<((int*)data)[i] <<std::endl;}
     }
     VsLog::debugLog() <<methodSig <<"Finished dumping data. " <<std::endl;
     */
    //END DEBUG

    rv->SetNumberOfTuples(len);

    // Perform if needed permutation of index as VTK expect Fortran order

    // The index tuple is initially all zeros
    size_t* indices = new size_t[rank];
    for (size_t k=0; k<rank; ++k)
    indices[k] = 0;

    // Store data
    VsLog::debugLog() <<methodSig <<"Storing " << len <<" data elements" <<std::endl;

    // Attempt to reverse data in place
    //#define IN_PLACE
#ifdef IN_PLACE
    VsLog::debugLog() <<methodSig <<"Attempting to swap data in place." <<std::endl;
    double* dblDataPtr = (double*) data;
    float* fltDataPtr = (float*) data;
    int* intDataPtr = (int*) data;

    // Step through by global C index and reverse
    if (rank > 1) {
      bool isDouble = isDoubleType(type);
      bool isFloat = isFloatType(type);
      bool isInteger = isIntegerType(type);
      for (size_t k = 0; k<len; ++k) {
        // Accumulate the Fortran index
        size_t indx = indices[rank-1];
        for (size_t j = 2; j<=rank; ++j)
        indx = indx*dims[rank-j] + indices[rank-j];
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
          VsLog::debugLog() <<methodSig <<"Unknown data type: " <<type <<std::endl;
          return NULL;
        }
        // Update the index tuple
        size_t j = rank;
        do {
          --j;
          ++indices[j];
          if (indices[j] == dims[j]) indices[j] = 0;
          else break;
        }while (j != 0);
      }
    }

    // Reversed in place so now can copy
    if (isDoubleType(type))  {
      for (size_t k = 0; k<len; ++k) {
        rv->SetTuple(k, &dblDataPtr[k]);
      }
    } else if (isFloatType(type)) {
      for (size_t k = 0; k<len; ++k) {
        rv->SetTuple(k, &fltDataPtr[k]);
      }
    } else if (isIntegerType(type)) {
      for (size_t k = 0; k<len; ++k) {
        rv->SetTuple(k, &intDataPtr[k]);
      }
    } else {
        VsLog::debugLog() <<methodSig <<"Unknown data type: " <<type <<std::endl;
        return NULL;
    }

    VsLog::debugLog() <<methodSig <<"Done swapping data in place." <<std::endl;
#else
    VsLog::debugLog() <<methodSig <<"Swapping data into correct places, NOT using 'in place' code." <<std::endl;

    if (isCompMajor) {
      double* dblDataPtr = (double*) data;
      float* fltDataPtr = (float*) data;
      int* intDataPtr = (int*) data;

      // If we're compMajor, we don't need to swap data around
      if (isDoubleType(type)) {
        for (size_t k = 0; k<len; ++k) {
          rv->SetTuple(k, &dblDataPtr[k]);
        }
      } else if (isFloatType(type)) {
        for (size_t k = 0; k<len; ++k) {
          rv->SetTuple(k, &fltDataPtr[k]);
        }
      } else if (isIntegerType(type)) {
        for (size_t k = 0; k<len; ++k) {
          //we convert to float because SetTuple doesn't take ints
          float* temp = (float*)&intDataPtr[k];
          rv->SetTuple(k, temp);
        }
      } else {
        VsLog::debugLog() <<methodSig <<"Unknown data type." <<std::endl;
      }
    } else {
      // Step through by global C index
      bool isDouble = isDoubleType(type);
      bool isFloat = isFloatType(type);
      bool isInteger = isIntegerType(type);
      for (size_t k = 0; k<len; ++k) {
        // Accumulate the Fortran index
        size_t indx = indices[rank-1];
        for (size_t j = 2; j<=rank; ++j) {
          indx = indx*dims[rank-j] + indices[rank-j];
        }

        // Set the value in the VTK array at the Fortran index to the
        // value in the C array at the C index
        if (isDouble) {
          rv->SetTuple(indx, &((double*) data)[k]);
        }
        else if (isFloat) {
          rv->SetTuple(indx, &((float*) data)[k]);
        }
        else if (isInteger) {
          //we cast to float because VisIt doesn't take integer data
          float temp = ((int*)data)[k];
          rv->SetTuple(indx, &temp);
        } else {
          VsLog::debugLog() <<methodSig <<"Unknown data type:" <<type <<std::endl;
          VsLog::debugLog() <<methodSig <<"Returning NULL." << std::endl;
          return NULL;
        }

        // Update the index tuple
        size_t j = rank;
        do {
          --j;
          ++indices[j];
          if (indices[j] == dims[j]) indices[j] = 0;
          else break;
        }while (j != 0);
      }
    }

    VsLog::debugLog() <<methodSig <<"Done swapping data into correct places, NOT using 'in place' code." <<std::endl;
#endif

    // Done with data
    VsLog::debugLog() <<methodSig <<"Cleaning up." <<std::endl;
    delete [] (char*) data;
    delete [] indices;

    VsLog::debugLog() <<methodSig <<"Returning data." <<std::endl;
    return rv;
  }

  void avtVsFileFormat::FreeUpResources(void) {
    VsLog::debugLog() <<"avtVsFileFormat::FreeUpResources() enter/exit." <<std::endl;
  }

  void avtVsFileFormat::RegisterExpressions(avtDatabaseMetaData* md) {
    std::stringstream sstr;
    sstr <<"avtVsFileFormat::RegisterExpressions() - ";
    std::string methodSig = sstr.str();
    VsLog::debugLog() <<methodSig <<"Entering function." <<std::endl;
    LoadData();

    //get list of expressions from reader
    std::map<std::string, std::string>* expressions = registry->getAllExpressions();
    
    if (expressions->empty()) {
      VsLog::debugLog() <<methodSig <<"WARNING: No expressions found in file. Returning." <<std::endl;
      return;
    } else {
      VsLog::debugLog() <<methodSig <<"Found " <<expressions->size() <<" expressions in file." <<std::endl;
    }

    //iterate over list of expressions, insert each one into database
    std::map<std::string, std::string>::const_iterator iv;
    for (iv = expressions->begin(); iv != expressions->end(); ++iv) {
      VsLog::debugLog() <<methodSig <<"Adding expression " << iv->first << " = "
      << iv->second << endl;
      Expression e;
      e.SetName (iv->first);

      //TODO: if the user has supplied a label for a component
      // but the vsVar expression still refers to the old component name
      // we need to either 1. register the old component name as an extra component
      // or 2. edit the vsVar expression to replace old component names with the user-specified labels.
      e.SetDefinition(iv->second);

      // See if the expression is a vector
      if ((iv->second.size() > 0) && (iv->second[0] == '{')) {
        VsLog::debugLog() <<methodSig <<"It is a vector expression." << endl;
        e.SetType(Expression::VectorMeshVar);
      }
      else {
        VsLog::debugLog() <<methodSig <<"It is a scalar expression." << endl;
        e.SetType(Expression::ScalarMeshVar);
      }
      md->AddExpression(&e);
    }

    VsLog::debugLog() <<methodSig <<"Exiting normally." << endl;
  }

  void avtVsFileFormat::RegisterVars(avtDatabaseMetaData* md) {
    std::stringstream sstr;
    sstr <<"avtVsFileFormat::RegisterVars() - ";
    std::string methodSig = sstr.str();
    VsLog::debugLog() <<methodSig <<"Entering function." <<std::endl;
    LoadData();

    // Get var names
    std::vector<std::string> names;
    registry->getAllVariableNames(names);
    
    if (names.empty()) {
      VsLog::debugLog() <<methodSig <<"WARNING: No variables were found in this file.  Returning." <<endl;
      return;
    } else {
      VsLog::debugLog() <<methodSig <<"Found " <<names.size() <<" variables in this file." <<endl;
    }

    std::vector<std::string>::const_iterator it;
    for (it = names.begin(); it != names.end(); ++it) {
      VsLog::debugLog() <<methodSig <<"Processing var: "<< *it <<std::endl;

      //get metadata for var
      VsVariable* vMeta = registry->getVariable(*it);
      
      //If this var is part of an MD var, we don't register it separately
      VsVariable* testForMD = registry->findSubordinateMDVar(*it);
      if (testForMD) {
        VsLog::debugLog() <<methodSig <<"Var is part of an MD variable, will not be registered separately." <<std::endl;
        continue;
      }

      // Name of the mesh of the var
      std::string mesh = vMeta->getMeshName();
      VsMesh* meshMeta = vMeta->getMesh();
      VsLog::debugLog() <<methodSig <<"Var lives on mesh " << mesh << "." << endl;

      // Centering of the variable
      // Default is node centered data
      avtCentering centering = AVT_NODECENT;
      if (vMeta->isZonal()) {
        VsLog::debugLog() <<methodSig <<"Var is zonal." << endl;
        centering = AVT_ZONECENT;
      } else {
        VsLog::debugLog() <<methodSig <<"Var is nodal." << endl;
      }

      // 1-D variable?
      VsLog::debugLog() <<methodSig <<"Determining if var is 1-D." << endl;
      int numSpatialDims = 3;
      if (meshMeta) {
        numSpatialDims = meshMeta->getNumSpatialDims();
      } else {
        numSpatialDims = 3;
        VsLog::errorLog() <<methodSig <<"Unable to load mesh for variable?" <<std::endl;
        VsLog::errorLog() <<methodSig <<"Using numSpatialDims = 3, hope it's right!" <<std::endl;     
      }

      //if this mesh is 1-D, we leave it for later (curves)
      bool isOneDVar = false;
      if (numSpatialDims == 1) {
        VsLog::debugLog() <<methodSig <<"Var is 1-D." << endl;
        isOneDVar = true;
      } else {
        VsLog::debugLog() <<methodSig <<"Var is not 1-D." << endl;
        isOneDVar = false;
      }

      // Number of component of the var
      size_t numComps = vMeta->getNumComps(reader->useStride, this->stride);
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
              VsLog::debugLog() <<methodSig <<"Adding curve component " <<componentName <<"." <<std::endl;
              avtCurveMetaData* cmd = new avtCurveMetaData(componentName.c_str());
              cmd->hasDataExtents = false;
              md->Add(cmd);
            } else {
              VsLog::debugLog() <<methodSig <<"Unable to find match for curve variable in component registry." <<std::endl;
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
          std::string componentName = registry->getComponentName(*it, i);

          if (!componentName.empty()) {
            VsLog::debugLog() <<methodSig <<"Adding variable component " <<componentName <<"." <<std::endl;
            avtScalarMetaData* smd = new avtScalarMetaData(componentName, mesh.c_str(), centering);
            smd->hasUnits = false;
            md->Add(smd);
          } else {
            VsLog::debugLog() <<methodSig <<"Unable to find match for variable in component registry." <<std::endl;
          }
        }
      }
      else if (numComps == 1) {
        VsLog::debugLog() <<methodSig <<"Adding single-component variable." <<std::endl;
        avtScalarMetaData* smd = new avtScalarMetaData(*it, mesh.c_str(), centering);
        smd->hasUnits = false;
        md->Add(smd);
      }
      else {
        VsLog::debugLog() <<methodSig <<"Variable '" << *it << "' has no components. Not being added." << endl;
      }
    }

    VsLog::debugLog() <<methodSig <<"Exiting normally." << endl;
  }

  void avtVsFileFormat::RegisterMeshes(avtDatabaseMetaData* md) {
    std::stringstream sstr;
    sstr <<"avtVsFileFormat::RegisterMeshes() - ";
    std::string methodSig = sstr.str();
    VsLog::debugLog() <<methodSig <<"Entering function." <<std::endl;
    LoadData();

    // Number of mesh dims
    int mdims;

    // All meshes names
    std::vector<std::string> names;
    registry->getAllMeshNames(names);
    if (names.empty()) {
      VsLog::debugLog() <<methodSig <<"WARNING: no meshes were found in this file. Returning." <<endl;
      return;
    } else {
      VsLog::debugLog() <<methodSig <<"Found " <<names.size() << " meshes." << endl;
    }

    std::vector<std::string>::const_iterator it;
    for (it = names.begin(); it != names.end(); ++it) {
      VsMesh* meta = registry->getMesh(*it);
      mdims = meta->getNumSpatialDims();

      //if this mesh is 1-D, we leave it for later (curves)
      if (mdims == 1) {
        VsLog::debugLog() <<methodSig <<"Found 1-d mesh.  Skipping for now, will be added as a curve." <<endl;
        continue;
      } else if ((mdims <= 0) || (mdims > 3)) {
        VsLog::errorLog() <<methodSig <<"NumSpatialDims is out of range: " <<mdims <<std::endl;
        VsLog::errorLog() <<methodSig <<"Skipping mesh." <<endl;
        continue;
      }
      
      //If this mesh is part of an MD mesh, we don't register it separately
      VsMesh* testForMD = registry->findSubordinateMDMesh(*it);
      if (testForMD) {
        VsLog::debugLog() <<methodSig <<"Mesh is part of an MD mesh, will not be registered separately." <<std::endl;
        continue;
      }
      
      VsLog::debugLog() <<methodSig <<"Found mesh '"
      << *it << "' of kind '" << meta->getKind() << "'." << endl;

      if (meta->isUniformMesh()) {
        // 09.06.01 Marc Durant We used to report uniform cartesian
        // meshes as being 3d no matter what I have changed this to
        // use the correct mesh dimensions The changed plugin passes
        // vstests, and is motivated because the VisIt Lineout
        // operator requires 2-d data.  EXCEPT! then we can't plot 3-d
        // vectors on the 2-d data, so for now we continue to report 3
        // mdims = dims.size();
        VsLog::debugLog() <<methodSig <<"Mesh's dimension = " << mdims << endl;
        if (mdims != 3) {
          VsLog::debugLog() <<methodSig <<"But reporting as dimension 3 to side-step VisIt bug." <<std::endl;
          mdims = 3;
        }
        
        VsLog::debugLog() <<methodSig <<"Adding uniform mesh " <<*it <<"." <<endl;
        // Add in the logical bounds of the data.
        std::vector<int> dims;
        meta->getMeshDims(&dims, reader->useStride, this->stride);
        int bounds[3] = {1,1,1};
        for( int i=0; i<dims.size(); ++i )
          bounds[i] = dims[i];

        avtMeshMetaData* vmd = new avtMeshMetaData(bounds, 0, it->c_str(),
            1, 1, 1, 0, mdims, mdims, AVT_RECTILINEAR_MESH);
        setAxisLabels(vmd);
        md->Add(vmd);
        VsLog::debugLog() <<methodSig <<"Succeeded in adding mesh " <<*it <<"." <<endl;
      }
      else if (meta->isUnstructuredMesh()) {
        //Unstructured meshes without connectivity data are registered as point meshes
        VsUnstructuredMesh* unstructuredMesh = (VsUnstructuredMesh*)meta;
        if (unstructuredMesh->isPointMesh()) {
          VsLog::debugLog() <<methodSig <<"Registering mesh " <<it->c_str() <<" as AVT_POINT_MESH" <<std::endl;
          avtMeshMetaData* vmd = new avtMeshMetaData(it->c_str(),
              1, 1, 1, 0, mdims, 0, AVT_POINT_MESH);
          setAxisLabels(vmd);
          md->Add(vmd);
        }
        else {
          VsLog::debugLog() <<methodSig <<"Registering mesh " <<it->c_str() <<" as AVT_UNSTRUCTURED_MESH" <<std::endl;
          mdims = meta->getNumSpatialDims();
          VsLog::debugLog() <<methodSig <<"Adding unstructured mesh " <<*it <<" with " <<mdims <<" spatial dimensions." <<endl;
          avtMeshMetaData* vmd = new avtMeshMetaData(it->c_str(),
              1, 1, 1, 0, mdims, mdims, AVT_UNSTRUCTURED_MESH);
          setAxisLabels(vmd);
          md->Add(vmd);
        }
      }
      else if (meta->isStructuredMesh()) {
        VsLog::debugLog() <<methodSig <<"Adding structured mesh " <<*it <<"." <<endl;

        // Add in the logical bounds of the data.
        std::vector<int> dims;
        meta->getMeshDims(&dims, reader->useStride, this->stride);
        int bounds[3] = {1,1,1};
        for( int i=0; i<dims.size(); ++i )
          bounds[i] = dims[i];

        avtMeshMetaData* vmd = new avtMeshMetaData(bounds, 0, it->c_str(),
            1, 1, 1, 0, mdims, mdims, AVT_CURVILINEAR_MESH);
        setAxisLabels(vmd);
        md->Add(vmd);
      }
      else if (meta->isRectilinearMesh()) {
        VsLog::debugLog() <<methodSig <<"Adding rectilinear mesh" <<*it <<"." <<std::endl;
        VsLog::debugLog() <<methodSig <<"MDims = " <<mdims <<"." <<std::endl;

        // Add in the logical bounds of the data.
        std::vector<int> dims;
        meta->getMeshDims(&dims, reader->useStride, this->stride);
        int bounds[3] = {1,1,1};
        for( int i=0; i<dims.size(); ++i )
          bounds[i] = dims[i];

        avtMeshMetaData* vmd = new avtMeshMetaData(bounds, 0, it->c_str(),
            1, 1, 1, 0, mdims, mdims, AVT_RECTILINEAR_MESH);
        setAxisLabels(vmd);
        md->Add(vmd);
      }
      else {
        VsLog::debugLog() <<methodSig <<"Unrecognized mesh kind: " <<meta->getKind() <<"." <<std::endl;
      }
    }
    VsLog::debugLog() <<methodSig <<"Exiting normally." << endl;
  }

  void avtVsFileFormat::RegisterMdVars(avtDatabaseMetaData* md) {
    std::stringstream sstr;
    sstr <<"avtVsFileFormat::RegisterMdVars() - ";
    std::string methodSig = sstr.str();
    VsLog::debugLog() <<methodSig <<"Entering function." <<std::endl;
    LoadData();

    // Get vars names
    std::vector<std::string> names;
    registry->getAllMDVariableNames(names);
    
    if (names.empty()) {
      VsLog::debugLog() <<methodSig <<"WARNING: No MD variables were found in this file.  Returning." <<std::endl;
      return;
    } else {
      VsLog::debugLog() <<methodSig <<"Found " <<names.size() <<" MD variables in this file." <<std::endl;
    }

    std::vector<std::string>::const_iterator it;
    for (it = names.begin(); it != names.end(); ++it) {
      VsLog::debugLog() <<methodSig <<"Processing md var '"
      << *it << "'." << std::endl;
      VsMDVariable* vMeta = registry->getMDVariable(*it);
      
      // Name of the mesh of the var
      std::string mesh = vMeta->getMesh();
      std::string vscentering = vMeta->getCentering();
      VsLog::debugLog() <<methodSig <<"MD var lives on mesh " << mesh << "." << std::endl;

      //TODO: Mesh should either exist in an mdMesh, or separately in the list of meshes

      // Centering of the variable
      // Default is node centered data
      avtCentering centering = AVT_NODECENT;
      if (vMeta->isZonal()) {
        VsLog::debugLog() <<methodSig <<"Var is zonal" << std::endl;
        centering = AVT_ZONECENT;
      } else {
        VsLog::debugLog() <<methodSig <<"Var is nodal" << std::endl;
      }

      // Number of component of the var
      size_t numComps = vMeta->getNumComps(reader->useStride, this->stride);
      VsLog::debugLog() <<methodSig <<"Variable has " <<numComps <<" components." <<std::endl;
      if (numComps > 1) {
        for (size_t i = 0; i<numComps; ++i) {
          //first, get a unique name for this component
          std::string compName = registry->getComponentName(*it, i);
          
          if (!compName.empty()) {
            VsLog::debugLog() <<methodSig <<"Adding variable component " <<compName <<"." <<std::endl;
            avtScalarMetaData* smd = new avtScalarMetaData(compName.c_str(),
              mesh.c_str(), centering);
            smd->hasUnits = false;
            md->Add(smd);
          } else {
            VsLog::debugLog() <<methodSig <<"Unable to find component name for var " <<*it <<" and index " <<i <<std::endl;
          }
        }
      }
      else if (numComps == 1) {
        VsLog::debugLog() <<methodSig <<"Adding single variable component " <<*it <<"." <<std::endl;
        avtScalarMetaData* smd = new avtScalarMetaData(*it, mesh.c_str(), centering);
        smd->hasUnits = false;
        md->Add(smd);
      } else {
        VsLog::debugLog() <<methodSig <<"Variable '" << *it << "' has no components. Not being added." << std::endl;
      }
    }
    VsLog::debugLog() <<methodSig <<"Exiting normally." << std::endl;
  }

  void avtVsFileFormat::RegisterMdMeshes(avtDatabaseMetaData* md) {
    std::stringstream sstr;
    sstr <<"avtVsFileFormat::RegisterMdMeshes() - ";
    std::string methodSig = sstr.str();
    VsLog::debugLog() <<methodSig <<"Entering function." <<std::endl;
    LoadData();

    std::vector<std::string> names;
    registry->getAllMDMeshNames(names);
    if (names.empty()) {
      VsLog::debugLog() <<methodSig <<"WARNING: no md meshes were found in this file. Returning" <<std::endl;
      return;
    } else {
      VsLog::debugLog() <<methodSig <<"Found " <<names.size() <<" MD meshes in this file." <<std::endl;
    }

    std::vector<std::string>::const_iterator it;
    for (it = names.begin(); it != names.end(); ++it) {
      VsLog::debugLog() << methodSig <<" Adding md mesh '"
      << *it << "'." << endl;

      VsMDMesh* meta = registry->getMDMesh(*it);
      if (!meta) {
        VsLog::debugLog() <<methodSig <<"Unable to find mesh " <<*it <<std::endl;
        continue;
      }

      avtMeshType meshType;
      std::string kind = meta->getMeshKind();
      if (meta->isUniformMesh()) {
        VsLog::debugLog() << methodSig <<"Mesh is rectilinear" <<std::endl;
        meshType = AVT_RECTILINEAR_MESH;
      } else if (meta->isUnstructuredMesh()) {
        VsLog::debugLog() << methodSig <<"Mesh is unstructured" <<std::endl;
        meshType = AVT_UNSTRUCTURED_MESH;
      } else if (meta->isStructuredMesh()) {
        VsLog::debugLog() << methodSig <<"Mesh is structured" <<std::endl;
        meshType = AVT_CURVILINEAR_MESH;
      }

      VsLog::debugLog() << methodSig <<"Mesh has dimension " <<meta->getNumSpatialDims() <<"." <<std::endl;

      avtMeshMetaData* vmd =new avtMeshMetaData(it->c_str(),
          meta->getNumBlocks(), 1, 1, 0, meta->getNumSpatialDims(), meta->getNumSpatialDims(), meshType);
      setAxisLabels(vmd);
      md->Add(vmd);
    }

    VsLog::debugLog() <<methodSig <<"Exiting normally." << endl;
  }

  //This method is different from the others because it adds
  // BOTH the mesh and the associated variables
  void avtVsFileFormat::RegisterVarsWithMesh(avtDatabaseMetaData* md) {
    std::stringstream sstr;
    sstr <<"avtVsFileFormat::RegisterVarsWithMesh() - ";
    std::string methodSig = sstr.str();
    VsLog::debugLog() <<methodSig <<"Entering function." <<std::endl;
    LoadData();

    std::vector<std::string> names;
    registry->getAllVariableWithMeshNames(names);
    
    if (names.empty()) {
      VsLog::debugLog() <<methodSig <<"WARNING: no variables with mesh were found in this file. Returning." <<endl;
      return;
    } else {
      VsLog::debugLog() <<methodSig <<"Found " <<names.size() <<" variables with mesh in this file." <<endl;
    }

    std::vector<std::string>::const_iterator it;
    for (it = names.begin(); it != names.end(); ++it) {
      VsLog::debugLog() <<methodSig <<"Processing varWithMesh '"
      << *it << "'." << endl;
      VsVariableWithMesh* vMeta = registry->getVariableWithMesh(*it);

      // add var components
      std::vector<int> dims = vMeta->getDims();
      if (dims.size() <= 0) {
        std::string msg = "avtVsFileFormat::RegisterVarsWithMesh() - could not get dimensions of variable with mesh.";
        VsLog::debugLog() << msg << std::endl;
        throw std::out_of_range(msg.c_str());
      }

      size_t lastDim = 0;
      if (vMeta->isCompMinor())
      lastDim = dims[dims.size()-1];
      else lastDim = dims[0];

      if (lastDim < vMeta->getNumSpatialDims()) {
        VsLog::debugLog() <<methodSig <<"Error: "
        "for variable with mesh '" << *it << "', numSpatialDims = " <<
        vMeta->getNumSpatialDims() << " must be larger then the last dimension, " <<
        "lastDim = " << lastDim << "." << endl;
        //JRC: Remove the associated mesh from the system!
        //Actually, the associated mesh doesn't get added until the end of this method
        //So there is no associated mesh to remove
        //We just drop through the loop and start on the next varWithMesh
        continue;
      }

      // SS: we need all components so going to lastDim here.
      //    for (size_t i = 0; i < lastDim-vm->numSpatialDims; ++i) {
      for (size_t i = 0; i < lastDim; ++i) {
        //first, get a unique name for this component
        std::string compName = registry->getComponentName(*it, i);
        
        if (!compName.empty()) {
          //register with VisIt
          VsLog::debugLog() <<methodSig <<"Adding variable component " <<compName <<"." <<std::endl;
          avtScalarMetaData* smd = new avtScalarMetaData(compName.c_str(),
            it->c_str(), AVT_NODECENT);
          smd->hasUnits = false;
          md->Add(smd);
        } else {
          VsLog::debugLog() <<methodSig <<"Unable to get component name for variable " <<*it <<" and index " <<i <<std::endl;
        }
      }

      // add var mesh
      VsLog::debugLog() <<methodSig <<"Adding point mesh for this variable." <<std::endl;
      avtMeshMetaData* vmd = new avtMeshMetaData(it->c_str(),
          1, 1, 1, 0, vMeta->getNumSpatialDims(), 0, AVT_POINT_MESH);
      setAxisLabels(vmd);
      md->Add(vmd);
    }

    VsLog::debugLog() <<methodSig <<"Exiting normally." << endl;
  }

  /**
   * Called to alert the database reader that VisIt is about to ask for data
   * at this timestep.  Since this reader is single-time, this is 
   * only provided for reference and does nothing.
   */
  void avtVsFileFormat::ActivateTimestep() {
    LoadData();
  }
  
  void avtVsFileFormat::UpdateCyclesAndTimes() {
    LoadData();
    
    //If we have time data, tell VisIt
    if (registry->hasTime()) {
      VsLog::debugLog() <<"avtVsFileFormat::UpdateCyclesAndTimes() - This file supplies time: " <<registry->getTime() <<std::endl;
      doubleVector times;
      times.push_back(registry->getTime());
      metadata->SetTimes(times);
      metadata->SetTimeIsAccurate(true, registry->getTime());
    }
    
    //If we have step data, tell VisIt
    if (registry->hasStep()) {
      VsLog::debugLog() <<"avtVsFileFormat::UpdateCyclesAndTimes() - This file supplies step: " <<registry->getStep() <<std::endl;
      intVector cycles;
      cycles.push_back(registry->getStep());
      metadata->SetCycles(cycles);
      metadata->SetCycleIsAccurate(true, registry->getStep());
    }
  }
  
  void avtVsFileFormat::LoadData() {
    if (reader)
      return;

    VsLog::debugLog() <<"avtVsFileFormat::LoadData() - loading data for file " <<dataFileName <<std::endl;
    //Actually open the file & read metadata for the first time
    VsLog::debugLog() <<"avtVsFileFormat::LoadData() - Initializing VsH5Reader()" <<std::endl;
    try {
      reader = new VsH5Reader(dataFileName, stride, registry);
    }
    catch (std::invalid_argument& ex) {
      std::string msg("avtVsFileFormat::LoadData() - ");
      msg += " Error initializing VsH5Reader: ";
      msg += ex.what();
      VsLog::debugLog() << msg << endl;
      EXCEPTION1(InvalidDBTypeException, msg.c_str());
    }

    VsLog::debugLog() <<"avtVsFileFormat::LoadData() - returning." <<std::endl;
  }

  void avtVsFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData* md) {
    std::stringstream sstr;
    sstr <<"avtVsFileFormat::PopulateDatabaseMetaData() - ";
    std::string methodSig = sstr.str();
    VsLog::debugLog() <<methodSig <<"Entering function." <<std::endl;
    LoadData();

    // Tell visit that we can split meshes into subparts when running in parallel
    // NOTE that we can't decompose domains if we have MD meshes
    // So it's one or the other
    std::vector<std::string> names;
#ifdef VIZSCHEMA_DECOMPOSE_DOMAINS
    VsLog::debugLog() <<methodSig <<"Decompose_domains is defined.  Entering code block." <<std::endl;
    if (registry->numMDMeshes() > 0) {
      VsLog::debugLog() <<methodSig <<"MD meshes are present in the data file.  Domain Decomposition is turned off." <<std::endl;
      md->SetFormatCanDoDomainDecomposition(false);
    } else {
      VsLog::debugLog() <<methodSig <<"NO MD meshes are present in the data file.  Domain Decomposition is turned on." <<std::endl;
      md->SetFormatCanDoDomainDecomposition(true);
    }
    VsLog::debugLog() <<methodSig <<"Decompose_domains is defined.  Exiting code block." <<std::endl;
#endif

    RegisterMeshes(md);
    RegisterMdMeshes(md);

    RegisterVarsWithMesh(md);

    RegisterVars(md);
    RegisterMdVars(md);

    RegisterExpressions(md);

    //add desperation last-ditch mesh if none exist in metadata
    if ((md->GetNumCurves() == 0) && (md->GetNumMeshes() == 0)) {
      VsLog::debugLog() <<methodSig <<"Warning: " << dataFileName << " contains no mesh "
      "information. Creating default mesh." << endl;
      avtMeshMetaData* mmd = new avtMeshMetaData("ERROR_READING_FILE", 1, 1, 1, 0, 3, 3,
          AVT_RECTILINEAR_MESH);
      setAxisLabels(mmd);
      md->Add(mmd);
    }

    UpdateCyclesAndTimes();
    
    VsLog::debugLog() <<methodSig <<"Exiting normally." << endl;
  }

  void avtVsFileFormat::setAxisLabels(avtMeshMetaData* mmd) {
    VsLog::debugLog() <<"avtVsFileFormat::setAxisLabels() - entering." <<std::endl;
    if (mmd == NULL) {
      VsLog::debugLog() <<"avtVsFileFormat::setAxisLabels() - Input pointer was NULL?" <<std::endl;
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
    VsLog::debugLog() <<"avtVsFileFormat::setAxisLabels() - exiting." <<std::endl;
  }
#endif
