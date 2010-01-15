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

// definition of VISIT_VERSION
#include <visit-config.h>

#include <avtVsFileFormat.h>

#if (defined PARALLEL && defined VIZSCHEMA_DECOMPOSE_DOMAINS)
#include <avtParallel.h>
#endif

// VizSchema includes
//#include <VsSchema.h>
#include <VsUtils.h>

avtVsFileFormat::avtVsFileFormat(const char* dfnm, std::vector<int> settings) :
#ifdef VISIT_VERSION //Visit 2.0.0 and later use VISIT_VERSION instead of VERSION, and require a different debugstream accessor as well
  avtSTMDFileFormat(&dfnm, 1), dataFileName(dfnm),debugStrmRef(DebugStream::Stream3()) {
#else
  avtSTMDFileFormat(&dfnm, 1), dataFileName(dfnm),debugStrmRef(debug3_real) {
#endif

    debugStrmRef <<"avtVsFileFormat::constructor() - entering" <<std::endl;

    //initialize settings
    stride.resize(3);
    if (settings.size() != 3) {
      debugStrmRef <<"avtVsFileFormat::constructor() - Expected settings array of length 3." <<std::endl;
      debugStrmRef <<"avtVsFileFormat::constructor() - Got length " <<settings.size() <<"." <<std::endl;
      stride[0] = VsCommonPluginInfo::defaultStride;
      stride[1] = VsCommonPluginInfo::defaultStride;
      stride[2] = VsCommonPluginInfo::defaultStride;
    } else {
      stride[0] = settings[0];
      stride[1] = settings[1];
      stride[2] = settings[2];
    }

    debugStrmRef <<"avtVsFileFormat::constructor() - strides are: " <<stride[0] <<", " <<stride[1] <<", " <<stride[2] <<"." <<std::endl;

    herr_t err = H5check();

    if (err < 0) {
      std::string msg("avtVsFileFormat::constructor(): HDF5 version mismatch."
          "  Vs reader built with ");
      msg += H5_VERS_INFO;
      msg += ".";
      debugStrmRef << msg << endl;
      EXCEPTION1(InvalidDBTypeException, msg.c_str());
    }

    debugStrmRef <<"avtVsFileFormat::constructor() - initializing VsH5Reader()" <<std::endl;
    try {
      reader = new VsH5Reader(dataFileName, debugStrmRef, stride);
    }
    catch (std::invalid_argument& ex) {
      std::string msg("avtVsFileFormat::constructor(): error initializing VsH5Reader: ");
      msg += ex.what();
      debugStrmRef << msg << endl;
      EXCEPTION1(InvalidDBTypeException, msg.c_str());
    }

    debugStrmRef <<"avtVsFileFormat::constructor() - exiting" <<std::endl;
  }

  avtVsFileFormat::~avtVsFileFormat() {
    debugStrmRef <<"avtVsFileFormat::destructor() - entering" <<std::endl;
    if (reader != NULL) {
      delete reader;
      reader = NULL;
    }
    debugStrmRef <<"avtVsFileFormat::destructor() - exiting" <<std::endl;
  }

  vtkDataSet* avtVsFileFormat::GetMesh(int domain, const char* name) {
    std::stringstream sstr;
    sstr <<"avtVsFileFormat::GetMesh(" <<domain <<", " <<name <<") - ";
    std::string methodSig = sstr.str();
    debugStrmRef <<methodSig <<"Entering function." <<std::endl;
    std::string meshName = name;

    //The MD system works by filtering the requests directed to it
    // into the name of the appropriate subordinate mesh.
    // For example, in facets_core-edge-explicit we have three meshes joined into one:
    // MdMesh = {coreMesh, solMesh, privMesh}
    // So if we get a request for (MdMesh, 0), we change the name to coreMesh and proceed normally

    //Check for MD mesh
    debugStrmRef <<methodSig <<"Looking for MD mesh with name " <<meshName <<std::endl;
    const VsMDMeshMeta* mdMeshMeta = reader->getMDMeshMeta(meshName);

    //If we found an MD mesh with this name, try to load the mesh data from it
    const VsMeshMeta* meta = NULL;
    if (mdMeshMeta != NULL) {
      debugStrmRef <<methodSig <<"Found MD mesh with that name." <<std::endl;
      meshName = mdMeshMeta->getNameForBlock(domain);
      debugStrmRef <<methodSig <<"Request for md mesh was filtered to regular mesh: " <<meshName <<std::endl;
      meta = mdMeshMeta->getBlock(domain);
    } else {
      debugStrmRef <<methodSig <<"No MD mesh with that name." <<std::endl;
    }

    //Did we succeed in loading mesh data from MD mesh?
    if (meta == NULL) {
      debugStrmRef <<methodSig <<"Trying to find regular mesh named: " <<meshName <<std::endl;
      meta = reader->getMeshMeta(meshName);
    }

    if (meta != NULL) {
      debugStrmRef <<methodSig <<"Found mesh named: " <<meshName <<std::endl;
      if (meta->isUniformMesh()) {
        debugStrmRef <<methodSig <<"Trying to load & return uniform mesh" <<std::endl;
        return getUniformMesh(meshName, *meta);
      }
#if (defined PARALLEL && defined VIZSCHEMA_DECOMPOSE_DOMAINS)
      // Don't know how to decompose any other type of mesh -> load it on proc 0 only
      if (PAR_Rank() > 0) {
        debugStrmRef <<methodSig <<"In parallel mode on procesor " <<PAR_Rank <<" and mesh is not uniform" <<std::endl;
        debugStrmRef <<methodSig <<"Returning NULL, mesh will be loaded on processor 0 only." <<std::endl;
        return NULL;
      }
#endif
      if (meta->isUnstructuredMesh()) {
        debugStrmRef <<methodSig <<"Trying to load & return unstructured mesh" <<std::endl;
        VsUnstructuredMesh* unstructuredMesh = (VsUnstructuredMesh*)meta;
        // getUnstructuredMesh handles split points so getSplitPointMesh is not needed.
        //  if (unstructuredMesh->usesSplitPoints())
        //    return getSplitPointMesh(meshName, *unstructuredMesh);
        //  else
        return getUnstructuredMesh(meshName, *meta);
      }

      if (meta->isStructuredMesh()) {
        debugStrmRef <<methodSig <<"Trying to load & return structured mesh" <<std::endl;
        return getStructuredMesh(meshName, *meta);
      }

      //if we get here, we don't know what kind of mesh it is.
      debugStrmRef <<methodSig <<"Mesh has unknown type: " <<meta->kind <<"Returning." << endl;
      return NULL;
    }

    //Variable with mesh
    debugStrmRef <<methodSig <<"Looking for Variable With Mesh with this name." << endl;
    const VsVariableWithMeshMeta* vmeta = reader->getVariableWithMeshMeta(name);
    if (vmeta != NULL) {
      debugStrmRef <<methodSig <<"Found Variable With Mesh. Loading data and returning." << endl;
      return getPointMesh(name, *vmeta);
    } else {
      debugStrmRef <<methodSig <<"Did not find Variable With Mesh." << endl;
    }

    //Curve
    debugStrmRef <<methodSig <<"Looking for Curve with this name." << endl;
    vtkDataArray* foundCurve = this->GetVar(domain, name);
    if (foundCurve != NULL) {
      debugStrmRef <<methodSig <<"Found curve.  Loading data and returning. <<std::endl";
      return getCurve(domain, name);
    }

    debugStrmRef <<methodSig <<"Failed to load data for given name and domain number.  Returning NULL." <<std::endl;
    return NULL;
  }

  vtkDataSet* avtVsFileFormat::getCurve(int domain, const std::string& name) {
    std::stringstream sstr;
    sstr <<"avtVsFileFormat::getCurve(" <<domain <<", " <<name <<") - ";
    std::string methodSig = sstr.str();
    debugStrmRef <<methodSig <<"Entering function." <<std::endl;

    ///TODO: This method piggybacks on getVar and getMesh -
    ///       Could be made much more efficient
    //1. get var
    //2. get matching mesh
    //3. create 2 coord arrays - x from mesh and y from var
    //4. Combine coord arrays to form 1-d Rectilinear mesh
    // (attempts to follow visit plugin Curve2d in /visit/src/databases/Curve2d/avtCurve2DFileFormat.C)

    //retrieve var metadata and extract the mesh name
    debugStrmRef <<methodSig <<"Looking for variable metadata." <<endl;
    const VsVariableMeta* varMeta = reader->getVariableMeta(name);
    if (varMeta == NULL) {
      debugStrmRef <<methodSig <<"NO variable metadata found. Returning NULL." <<endl;
      return NULL;
    }
    debugStrmRef <<methodSig <<"Found variable metadata." <<name <<endl;

    std::string meshName = varMeta->mesh;
    debugStrmRef <<methodSig <<"Looking for mesh metadata for name: " <<meshName <<endl;
    const VsMeshMeta* meshMeta = reader->getMeshMeta(meshName.c_str());
    if (meshMeta == NULL) {
      debugStrmRef <<methodSig <<"No mesh metadata found. Returning NULL." <<endl;
      return NULL;
    }

    debugStrmRef <<methodSig <<"Found all metadata, loading variable data." <<endl;
    vtkDataArray* varData = NULL;
    try {
      varData = GetVar(domain, name.c_str());
    } catch (...) {
      debugStrmRef <<methodSig <<"Caught exception from GetVar() - returning NULL." <<endl;
      return NULL;
    }

    if (varData == NULL) {
      debugStrmRef <<methodSig <<"Failed to load var data - returning NULL." <<endl;
      return NULL;
    }

    debugStrmRef <<methodSig <<"Loaded variable data, trying to load mesh data." <<endl;
    vtkRectilinearGrid* meshData = NULL;
    try {
      meshData = (vtkRectilinearGrid*)GetMesh(domain, meshName.c_str());
    } catch (...) {
      debugStrmRef <<methodSig <<"Caught exception from GetMesh() - returning NULL." <<endl;
      ///TODO: delete varData?
      varData->Delete();
      return NULL;
    }

    if (meshData == NULL) {
      debugStrmRef <<methodSig <<"Failed to load mesh data - returning NULL." <<endl;
      ///TODO: delete varData?
      varData->Delete();
      return NULL;
    }

    int numDims = meshMeta->numSpatialDims;
    debugStrmRef <<methodSig <<"Mesh has " <<numDims <<" dimensions" <<endl;

    int nPts = varMeta->getLength();
    debugStrmRef <<methodSig <<"Variable has " <<nPts <<" points." <<endl;

    // Create 1-D RectilinearGrid
    hid_t varDataType = varMeta->getType();
    if (H5Tequal (varDataType, H5T_NATIVE_DOUBLE)) {
      debugStrmRef <<methodSig <<"Var is double" <<endl;
    } else if (H5Tequal(varDataType, H5T_NATIVE_FLOAT)) {
      debugStrmRef <<methodSig <<"Var is float" <<endl;
    } else if (H5Tequal(varDataType, H5T_NATIVE_INT)) {
      debugStrmRef <<methodSig <<"Var is int" <<endl;
    } else {
      debugStrmRef <<methodSig <<"Var is unknown type (known are double, float, int)." <<endl;
    }

    debugStrmRef <<methodSig <<"Building Rectilinear grid." <<std::endl;
    vtkFloatArray* vals = vtkFloatArray::New();
    vals->SetNumberOfComponents(1);
    vals->SetNumberOfTuples(nPts);
    vals->SetName(name.c_str());

    vtkRectilinearGrid* rg = vtkVisItUtility::Create1DRGrid(nPts, VTK_FLOAT);
    rg->GetPointData()->SetScalars(vals);
    vtkFloatArray* xc = vtkFloatArray::SafeDownCast(rg->GetXCoordinates());

    debugStrmRef <<methodSig <<"Retrieving X coordinates from mesh." <<std::endl;
    vtkDataArray* meshXCoord = meshData->GetXCoordinates();

    debugStrmRef <<methodSig <<"Adding all points to curve" <<std::endl;
    for (int i = 0; i < nPts; i++) {
      double* var_i = varData->GetTuple(i);
      double* mesh_i = meshXCoord->GetTuple(i);
      //debugStrmRef <<"Adding tuple[" <<i <<"] = (" <<mesh_i[0] <<", " <<var_i[0] <<")" <<endl;

      xc->SetValue(i, mesh_i[0]);
      vals->SetValue(i, var_i[0]);
    }

    debugStrmRef <<methodSig <<"Deleting temporary variables." <<std::endl;
    vals->Delete();

    debugStrmRef <<methodSig <<"Returning data." <<endl;
    return rg;
  }

  vtkDataSet* avtVsFileFormat::getStructuredMesh(const std::string& name,
      const VsMeshMeta& meta) {
    std::stringstream sstr;
    sstr <<"avtVsFileFormat::getStructuredMesh(" <<name <<") - ";
    std::string methodSig = sstr.str();
    debugStrmRef <<methodSig <<"Entering function." <<std::endl;

    //Find points
    //structured meshes used to be groups, and the points were a dataset in that group
    //NOW structured meshes are datasets, and the data is the points
    //So, we look for the dataset with the same name as the mesh

    VsDMeta* pointsDataset = meta.getDataset(name);
    if (pointsDataset == NULL) {
      debugStrmRef <<methodSig <<"Component '" <<
      name << "' not found. Returning NULL." << endl;
      return NULL;
    }

    // Get dims of points array
    debugStrmRef <<methodSig <<"Determining dimension of points array." <<std::endl;
    std::vector<int> dims;
    reader->getMeshDims(name, &dims);
    if (dims.size() < 0) {
      std::string msg = "avtVsFileFormat::getStructuredMesh: could not get dimensions of structured mesh.";
      debugStrmRef << msg << std::endl;
      throw std::out_of_range(msg.c_str());
    }
    size_t nDims = dims[dims.size()-1];
    size_t numPoints = 1;
    for (size_t i = 0; i < dims.size()-1; ++i) {
      numPoints *= dims[i];
    }
    debugStrmRef <<methodSig <<"Total number of points is " <<numPoints <<"." <<std::endl;

    // Check points data type
    hid_t type = pointsDataset->type;
    if (!H5Tequal(type, H5T_NATIVE_DOUBLE) &&
        !H5Tequal(type, H5T_NATIVE_FLOAT)) {
      debugStrmRef <<methodSig <<"Error: data type not handled (can handle H5T_NATIVE_FLOAT or H5T_NATIVE_DOUBLE." << endl;
      debugStrmRef <<methodSig <<"Returning NULL." << endl;
      return NULL;
    }

    // Read points and add in zero for any lacking dimension
    debugStrmRef <<methodSig <<"Reading in point data." <<std::endl;
    size_t dsize = 0;
    void* dataPtr = 0;
    double* dblDataPtr = 0;
    float* fltDataPtr = 0;
    if (H5Tequal(type, H5T_NATIVE_DOUBLE)) {
      debugStrmRef <<methodSig <<"Declaring array of doubles of length " <<(numPoints * 3) <<"." <<std::endl;
      debugStrmRef <<methodSig <<"Total allocation: " <<(numPoints * 3 * dsize) <<" bytes." <<std::endl;
      dsize = sizeof(double);
      dblDataPtr = new double[numPoints*3];
      dataPtr = dblDataPtr;
    }
    else if (H5Tequal(type, H5T_NATIVE_FLOAT)) {
      debugStrmRef <<methodSig <<"Declaring array of floats of length " <<(numPoints * 3) <<"." <<std::endl;
      debugStrmRef <<methodSig <<"Total allocation: " <<(numPoints * 3 * dsize) <<" bytes." <<std::endl;
      dsize = sizeof(float);
      fltDataPtr = new float[numPoints*3];
      dataPtr = fltDataPtr;
    }

    if (!dataPtr) {
      debugStrmRef <<methodSig <<"Allocation failed, pointer is NULL." <<std::endl;
      debugStrmRef <<methodSig <<"Returning NULL." <<std::endl;
      return NULL;
    }

    debugStrmRef <<methodSig <<"Allocation succeeded.  Now reading in data." <<std::endl;

    // Read in the data
    herr_t err = reader->getDatasetMeshComponent(name, meta, dataPtr);
    if (err < 0) {
      debugStrmRef <<methodSig <<"Error reading mesh data.  Deleting temporary storage." <<std::endl;
      if (H5Tequal(type, H5T_NATIVE_DOUBLE))
      delete [] dblDataPtr;
      else delete [] fltDataPtr;
      debugStrmRef <<methodSig <<"Returning NULL." <<std::endl;
      return NULL;
    } else {
      debugStrmRef <<methodSig <<"Succeeded reading mesh data." <<std::endl;
    }

    // If source data is less than 3D, move data to correct position
    // and set dest data extra dim values to 0
    if (nDims < 3) {
      debugStrmRef <<methodSig <<"Dimensionality is less than 3.  Moving data into correct location." <<std::endl;
      for (size_t i = numPoints; i > 0; --i) {
        unsigned char* destPtr = (unsigned char*)dataPtr + (i-1)*3*dsize;
        unsigned char* srcPtr = (unsigned char*)dataPtr + (i-1)*nDims*dsize;
        memmove(destPtr, srcPtr, nDims*dsize);
        destPtr += nDims*dsize;
        memset(destPtr, 0, (3-nDims)*dsize);
      }
      debugStrmRef <<methodSig <<"Data move succeeded." <<std::endl;
    }
    /*
     debugStrmRef <<methodSig <<"OUtputting points" <<std::endl;
     // debug: output points
     for (size_t i = 0; i < numPoints; ++i) {
     debugStrmRef << i << ":";
     for (size_t j = 0; j < 3; ++j) {
     if (H5Tequal(type, H5T_NATIVE_DOUBLE)) {
     debugStrmRef << " " << dblDataPtr[(3*i)+j];
     }
     else debugStrmRef << " " << fltDataPtr[(3*i)+j];
     }
     debugStrmRef << endl;
     }
     // end debug
     */
    // Create the mesh and set its dimensions, including unused to zero
    debugStrmRef <<methodSig <<"Creating the mesh." <<std::endl;
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
    debugStrmRef <<methodSig <<"Adding points to mesh." <<std::endl;
    vtkPoints* vpoints = vtkPoints::New();
    if (H5Tequal(type, H5T_NATIVE_DOUBLE)) {
      vpoints->SetDataTypeToDouble();
    }
    else if (H5Tequal(type, H5T_NATIVE_FLOAT)) {
      vpoints->SetDataTypeToFloat();
    } else {
      //this should never happen because we check up above, but for safety...
      debugStrmRef <<methodSig <<"Unknown data type: " <<type <<"Can only handle H5T_NATIVE_FLOAT or H5T_NATIVE_DOUBLE." <<endl;
    }

    vpoints->SetNumberOfPoints(numPoints);
    //void* ptsPtr = vpoints->GetVoidPointer(0);

    // Step through by global C index to reverse
    debugStrmRef <<methodSig <<"Adding " << len
    << " points with index order '" << meta.indexOrder << "'." << endl;

    if (meta.isFortranOrder()) {
      debugStrmRef <<methodSig <<"Using FORTRAN data ordering." <<std::endl;
      for (size_t k = 0; k<len; ++k) {
        if (H5Tequal(type, H5T_NATIVE_DOUBLE)) {
          vpoints->SetPoint(k, &dblDataPtr[k*3]);
        }
        else {
          vpoints->SetPoint(k, &fltDataPtr[k*3]);
        }
      }
    }
    else {
      debugStrmRef <<methodSig <<"Using C data ordering." <<std::endl;
      size_t indices[3] = {0, 0, 0};
      for (size_t k = 0; k<len; ++k) {
        // Accumulate the Fortran index
        size_t indx = indices[2];
        for (size_t j = 2; j<=3; ++j) {
          indx = indx*idims[3-j] + indices[3-j];
        }

        // Set the value in the VTK array at the Fortran index to the
        // value in the C array at the C index
        if (H5Tequal(type, H5T_NATIVE_DOUBLE)) {
          vpoints->SetPoint(indx, &dblDataPtr[k*3]);
        }
        else {
          vpoints->SetPoint(indx, &fltDataPtr[k*3]);
        }
        size_t j = 3;
        do {
          --j;
          ++indices[j];
          if (indices[j] == idims[j])
          indices[j] = 0;
          else break;
        }while (j != 0);
      }
    }
    debugStrmRef <<methodSig <<"Points added successfully.  Deleting temporary storage." <<endl;
    if (H5Tequal(type, H5T_NATIVE_DOUBLE))
    delete [] dblDataPtr;
    else delete [] fltDataPtr;
    /*
     // debug: output points
     fltDataPtr = (float*)ptsPtr;
     dblDataPtr = (double*)ptsPtr;
     for (size_t i = 0; i < numPoints; ++i) {
     debugStrmRef << i << ":";
     for (size_t j = 0; j < 3; ++j) {
     if (H5Tequal(type, H5T_NATIVE_DOUBLE)) {
     debugStrmRef << " " << dblDataPtr[(3*i)+j];
     }
     else debugStrmRef << " " << fltDataPtr[(3*i)+j];
     }
     debugStrmRef << endl;
     }
     // end debug
     */
    sgrid->SetPoints(vpoints);
    vpoints->Delete();

    debugStrmRef <<methodSig <<"Returning data." <<endl;
    return sgrid;
  }

  vtkDataSet* avtVsFileFormat::getUnstructuredMesh(const std::string& name,
      const VsMeshMeta& meta) {
    std::stringstream sstr;
    sstr <<"avtVsFileFormat::getUnstructuredMesh(" <<name <<") - ";
    std::string methodSig = sstr.str();
    debugStrmRef <<methodSig <<"Entering function." <<std::endl;

    VsUnstructuredMesh* unstructuredMesh = (VsUnstructuredMesh*)&meta;

    hid_t type0 = unstructuredMesh->getDataType();
    // Check for points type
    if ( !H5Tequal(type0, H5T_NATIVE_DOUBLE) &&
        !H5Tequal(type0, H5T_NATIVE_FLOAT) ) {
      debugStrmRef <<methodSig <<"Points are neither float nor double." <<std::endl;
      debugStrmRef <<methodSig <<"Returning NULL" <<std::endl;
      return NULL;
    }

    debugStrmRef <<methodSig <<"Checking dimensionality of mesh." <<std::endl;
    size_t nnodes = unstructuredMesh->numnodes;
    size_t ndims = unstructuredMesh->numSpatialDims;

    // Get ready to read in points
    debugStrmRef <<methodSig <<"Setting up data structures." <<std::endl;
    vtkUnstructuredGrid* ugridPtr = vtkUnstructuredGrid::New();

    // Create and set points while small so minimal memory usage
    vtkPoints* vpts = vtkPoints::New();
    ugridPtr->SetPoints(vpts);
    vpts->Delete();
    vtkPoints* vpoints = ugridPtr->GetPoints();

    // Allocate
    size_t dsize = 0;
    if (H5Tequal(type0, H5T_NATIVE_DOUBLE)) {
      vpoints->SetDataTypeToDouble();
      dsize = sizeof(double);
    }
    else if (H5Tequal(type0, H5T_NATIVE_FLOAT)) {
      vpoints->SetDataTypeToFloat();
      dsize = sizeof(float);
    }

    debugStrmRef <<methodSig <<"VPoints array will have " <<nnodes <<" points." <<std::endl;
    vpoints->SetNumberOfPoints(nnodes);
    void* dataPtr = vpoints->GetVoidPointer(0);
    if (!dataPtr) {
      debugStrmRef <<methodSig <<"Unable to allocate the points.  Cleaning up." << endl;
      ugridPtr->Delete();
      debugStrmRef <<methodSig <<"Returning NULL." << endl;
      return NULL;
    }

    // Read in the data
    debugStrmRef <<methodSig <<"Reading in data." <<std::endl;
    if (unstructuredMesh->usesSplitPoints()) {
      debugStrmRef <<methodSig <<"Using split-points method" <<std::endl;

      std::string points0 = unstructuredMesh->getPointsDatasetName(0);
      std::string points1 = unstructuredMesh->getPointsDatasetName(1);
      std::string points2 = unstructuredMesh->getPointsDatasetName(2);

      reader->getSplitMeshData(points0, points1, points2, *unstructuredMesh, dataPtr);
    }
    else {
      debugStrmRef <<methodSig <<"Using all-in-one method" <<std::endl;
      //all in one dataset
      reader->getDatasetMeshComponent(unstructuredMesh->getPointsDatasetName(), meta, dataPtr);

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
    debugStrmRef <<methodSig <<"Read finished." <<std::endl;

    // If there is no polygon data, then this mesh was registered as a point mesh
    // and we don't need to go any further
    if (unstructuredMesh->isPointMesh()) {
      debugStrmRef <<methodSig <<"Mesh was registered as a point mesh." <<std::endl;
      debugStrmRef <<methodSig <<"Because no connectivity data was found." <<std::endl;
      debugStrmRef <<methodSig <<"Adding vertices as single points and returning." <<std::endl;
      vtkIdType vertex;
      for (size_t i = 0; i < nnodes; ++i) {
        vertex = i;
        ugridPtr->InsertNextCell(VTK_VERTEX, 1, &vertex);
      }
      return ugridPtr;
    }

    // Next, look for vertex data
    VsDMeta* vertexMeta = 0;

    unsigned int rank;
    unsigned int haveVertexCount = 0;

    // For now users can have only one vertex dataset.
    if( (vertexMeta = unstructuredMesh->getPolygonsDataset()) ) {
      haveVertexCount = 0; rank = 2;
    } else if( (vertexMeta = unstructuredMesh->getTrianglesDataset()) ) {
      haveVertexCount = 3; rank = 2;
    } else if( (vertexMeta = unstructuredMesh->getQuadrilateralsDataset()) ) {
      haveVertexCount = 4; rank = 2;

    } else if( (vertexMeta = unstructuredMesh->getPolyhedraDataset()) ) {
      haveVertexCount = 0; rank = 3;
    } else if( (vertexMeta = unstructuredMesh->getTetrahedralsDataset()) ) {
      haveVertexCount = 4; rank = 3;
    } else if( (vertexMeta = unstructuredMesh->getPyramidsDataset()) ) {
      haveVertexCount = 5; rank = 3;
    } else if( (vertexMeta = unstructuredMesh->getPrismsDataset()) ) {
      haveVertexCount = 6; rank = 3;
    } else if( (vertexMeta = unstructuredMesh->getHexahedralsDataset()) ) {
      haveVertexCount = 8; rank = 3;
    }

    else {
      debugStrmRef <<methodSig <<"ERROR - unable to find vertex dataset." <<std::endl;
      debugStrmRef <<methodSig <<"ERROR - Returning points data without connectivity." <<std::endl;
      //we used to try to convert this into a pointMesh
      // but it never worked
      // So basically we're giving up here
      return ugridPtr;
    }

    debugStrmRef <<methodSig <<"Found vertex data." << endl;
    debugStrmRef <<methodSig <<"haveVertexCount = " << haveVertexCount << endl;

    std::vector<int> dim1;
    getDims(vertexMeta->iid, true, dim1);
    size_t ncells = dim1[0];
    size_t nverts = dim1[1];
    debugStrmRef <<methodSig <<"ncells = " <<ncells << ", nverts = " << nverts << "." << endl;
    size_t datasetLength = 1;
    for (size_t i =0; i< dim1.size(); ++i)
    datasetLength *= dim1[i];
    hid_t type1 = vertexMeta->type;
    // Check for vertex list type
    if (!H5Tequal(type1, H5T_NATIVE_INT)) {
      debugStrmRef <<methodSig <<"Indices are not integers. Cleaning up" <<std::endl;
      ugridPtr->Delete();
      debugStrmRef <<methodSig <<"Returning NULL" <<std::endl;
      return NULL;
    }

    // Vertices
    debugStrmRef <<methodSig <<"Allocating space for " <<datasetLength <<" integers of connectivity data." << endl;
    int* vertices = new int[datasetLength];
    if (!vertices) {
      debugStrmRef <<methodSig <<"Unable to allocate vertices.  Cleaning up." <<std::endl;
      ugridPtr->Delete();
      debugStrmRef <<methodSig <<"Returning NULL." <<std::endl;
      return NULL;
    }

    debugStrmRef <<methodSig <<"Reading vertex list data." << endl;

    if( rank == 2 && haveVertexCount == 0 )
    reader->getDatasetMeshComponent(unstructuredMesh->getPolygonsDatasetName(), meta, vertices);
    else if( rank == 2 && haveVertexCount == 2 )
    reader->getDatasetMeshComponent(unstructuredMesh->getLinesDatasetName(), meta, vertices);
    else if( rank == 2 && haveVertexCount == 3 )
    reader->getDatasetMeshComponent(unstructuredMesh->getTrianglesDatasetName(), meta, vertices);
    else if( rank == 2 && haveVertexCount == 4 )
    reader->getDatasetMeshComponent(unstructuredMesh->getQuadrilateralsDatasetName(), meta, vertices);

    else if( rank == 3 && haveVertexCount == 0 )
    reader->getDatasetMeshComponent(unstructuredMesh->getPolyhedraDatasetName(), meta, vertices);
    else if( rank == 3 && haveVertexCount == 4 )
    reader->getDatasetMeshComponent(unstructuredMesh->getTetrahedralsDatasetName(), meta, vertices);
    else if( rank == 3 && haveVertexCount == 5 )
    reader->getDatasetMeshComponent(unstructuredMesh->getPyramidsDatasetName(), meta, vertices);
    else if( rank == 3 && haveVertexCount == 6 )
    reader->getDatasetMeshComponent(unstructuredMesh->getPrismsDatasetName(), meta, vertices);
    else if( rank == 3 && haveVertexCount == 8 )
    reader->getDatasetMeshComponent(unstructuredMesh->getHexahedralsDatasetName(), meta, vertices);

    // debugStrmRef << "avtVsFileFormat::getUnstructuredMesh(...): vertices =";
    // for (size_t j = 0; j < 100; ++j) debugStrmRef << " " << vertices[j];
    // debugStrmRef << endl;

    try {
      debugStrmRef <<methodSig <<"Allocating " <<ncells << " cells.  "
      <<"If old VTK and this fails, it will just abort." <<std::endl;
      ugridPtr->Allocate(ncells);
      debugStrmRef <<methodSig <<"Allocation succeeded." <<std::endl;
    }
    // JRC: what goes here to detect failure to allocate?
    catch (vtkstd::bad_alloc& ex) {
      debugStrmRef << methodSig <<"Caught vtkstd::bad_alloc exception." <<std::endl;
      debugStrmRef << methodSig <<"Unable to allocate space for cells.  Cleaning up."
      <<std::endl;
      delete [] vertices;
      ugridPtr->Delete();
      debugStrmRef << methodSig <<"Returning NULL.";
      return NULL;
    } catch (...) {
      debugStrmRef << methodSig <<"Unknown exception allocating cells.  Cleaning up."
      <<std::endl;
      delete [] vertices;
      ugridPtr->Delete();
      debugStrmRef << methodSig <<"Returning NULL.";
      return NULL;
    }

    debugStrmRef <<methodSig <<"Inserting cells into grid." <<std::endl;
    size_t k = 0;
    int warningCount = 0;
    int cellCount = 0;
    unsigned int cellVerts; // cell's connected node indices
    int cellType;

    // Dealing with fixed length vertice lists.
    if( haveVertexCount ) {
      cellVerts = haveVertexCount;
      debugStrmRef <<methodSig <<"Inserting " << cellVerts << " into each cell." <<std::endl;
    }

    for (size_t i = 0; i < ncells; ++i) {
      if (k >= datasetLength) {
        debugStrmRef <<methodSig <<"While iterating over the vertices, the index variable 'k' went beyond the end of the array." <<std::endl;
        debugStrmRef <<methodSig <<"Existing cells will be returned but the loop is terminating now." <<std::endl;
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
          debugStrmRef <<methodSig << "Error: invalid number of vertices for cell #" <<cellCount <<": " << cellVerts << endl;
        } else if (warningCount == 30) {
          debugStrmRef <<methodSig << "Exceeded maximum number of errors.  Error messages disabled for remaining cells." <<std::endl;
        }
        ++warningCount;
        cellType = VTK_EMPTY_CELL;
        break;
      }

      //create cell and insert into mesh
      if (cellType != VTK_EMPTY_CELL) {
        vtkIdType verts[cellVerts];
        for (size_t j = 0; j < cellVerts; ++j) {
          verts[j] = (vtkIdType) vertices[k++];
        }
        // insert cell into mesh
        ugridPtr->InsertNextCell(cellType, cellVerts, verts);
        if( haveVertexCount == 0 )
        k += nverts - 1 - cellVerts;
      } else {
        //there was some error
        // so we add each vertex as a single point
        // NO!  Unless we've registered as a pointmesh, adding single points won't work
        //so we treat the entire row of the dataset as a single cell
        //Maybe something will work!
        vtkIdType verts[nverts];
        k--;
        for (size_t j = 0; j < nverts; ++j) {
          if (warningCount < 30) {
            debugStrmRef <<"WARNING: ADDING cell #" <<cellCount <<" as cell: " <<vertices[k] <<std::endl;
          }
          verts[j] = (vtkIdType) vertices[k++];
        }
        ugridPtr->InsertNextCell (VTK_POLYGON, nverts, verts);
      }
    }

    debugStrmRef <<methodSig <<"Finished.  Cleaning up." <<std::endl;
    // Done, so clean up memory and return
    delete [] vertices;

    debugStrmRef <<methodSig <<"Returning data." <<std::endl;
    return ugridPtr;
  }

  vtkDataSet* avtVsFileFormat::getUniformMesh(const std::string& name,
      const VsMeshMeta& meta) {
    std::stringstream sstr;
    sstr <<"avtVsFileFormat::getUniformMesh(" <<name <<") - ";
    std::string methodSig = sstr.str();
    debugStrmRef <<methodSig <<"Entering function." <<std::endl;

    // Cast to Uniform Mesh object
    VsUniformMesh* uniformMesh = (VsUniformMesh*)&meta;

    // Read int data
    int* startCell = 0;
    std::vector<int> numCells;
    reader->getMeshDims(name, &numCells);
    size_t rank = numCells.size();

    hid_t ftype = uniformMesh->getDataType();

    // startCell
    debugStrmRef <<methodSig <<"Loading startCells attribute." << endl;
    startCell = new int[rank];
    reader->getAttMeshComponent(VsSchema::Uniform::comp1,
        meta, startCell);

    // Adjust the box by startCell
    debugStrmRef <<methodSig <<"Adjusting numCells by startCells." << endl;
    for (size_t i = 0; i < rank; ++i)
    numCells[i] -= startCell[i];

    // Storage for mesh in VisIt, and it expects 3D
    size_t vsdim = 3;
    if (rank > 3) {
      debugStrmRef <<methodSig <<"Error: rank of data is larger than 3." << endl;
      debugStrmRef <<methodSig <<"Returning NULL." << endl;
      return NULL;
    }

    debugStrmRef <<methodSig <<"Determining size of point arrays." << endl;
    int idims[vsdim];
    for (size_t i = 0; i < vsdim; ++i) {
      // Number of nodes is equal to number of cells plus one
      if (i<rank)
      idims[i] = numCells[i]+1;
      else idims[i] = 1; // set unused dims to 1
    }

    debugStrmRef <<methodSig <<"Building point arrays." << endl;
    vtkPoints* vpoints1 = vtkPoints::New();
    vtkPoints* vpoints2 = vtkPoints::New();
    size_t dsize = 0;
    if (H5Tequal(ftype, H5T_NATIVE_DOUBLE)) {
      debugStrmRef <<methodSig <<"Point arrays are H5T_NATIVE_DOUBLE." << endl;
      vpoints1->SetDataTypeToDouble();
      vpoints2->SetDataTypeToDouble();
      dsize = sizeof(double);
    }
    else if (H5Tequal(ftype, H5T_NATIVE_FLOAT)) {
      debugStrmRef <<methodSig <<"Point arrays are H5T_NATIVE_FLOAT." << endl;
      vpoints1->SetDataTypeToFloat();
      vpoints2->SetDataTypeToFloat();
      dsize = sizeof(float);
    }

    vpoints1->SetNumberOfPoints(rank);
    vpoints2->SetNumberOfPoints(rank);

    debugStrmRef <<methodSig <<"Loading data for point array vspoints1." << endl;
    void* lowerBounds = vpoints1->GetVoidPointer(0);
    if (lowerBounds) {
      reader->getAttMeshComponent(VsSchema::Uniform::comp0,
          meta, lowerBounds);
    }
    else {
      debugStrmRef <<methodSig <<"Unable to allocate the points. Cleaning up." << endl;
      vpoints1->Delete();
      debugStrmRef <<methodSig <<"Returning NULL." << endl;
      return NULL;
    }

    debugStrmRef <<methodSig <<"Loading data for point array vspoints2." << endl;
    void* upperBounds = vpoints2->GetVoidPointer(0);
    if (upperBounds) {
      reader->getAttMeshComponent(VsSchema::Uniform::comp3,
          meta, upperBounds);
    }
    else {
      debugStrmRef <<methodSig <<"Unable to allocate the points. Cleaning up." << endl;
      vpoints1->Delete();
      vpoints2->Delete();
      debugStrmRef <<methodSig <<"Returning NULL." << endl;
      return NULL;
    }

#if (defined PARALLEL && defined VIZSCHEMA_DECOMPOSE_DOMAINS)
    debugStrmRef <<methodSig <<"Parallel & Decompose_Domains are defined, entering parallel code." << endl;
    size_t splitAxis = 0;
    size_t largestCount = numCells[splitAxis];

    for (size_t currAxis = 1; currAxis < rank; ++currAxis) {
      if (numCells[currAxis] > largestCount) {
        splitAxis = currAxis;
        largestCount = numCells[currAxis];
      }
    }

    debugStrmRef <<methodSig <<"Splitting along axis " << splitAxis << endl;

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
      if (H5Tequal (ftype, H5T_NATIVE_DOUBLE)) {
        double delta = (((double*)upperBounds)[splitAxis] -
            ((double*)lowerBounds)[splitAxis])/numCellsAlongSplitAxis;
        static_cast<double*>(lowerBounds)[splitAxis] += startCell * delta;
        static_cast<double*>(upperBounds)[splitAxis] =
        static_cast<double*>(lowerBounds)[splitAxis] + nCells * delta;
        //std::cout << "After adjust: Proc=" << PAR_Rank() << " start=" << startCell << " nCells=" << nCells << " delta=" << delta << " lowerBound=" << static_cast<double*>(lowerBounds)[splitAxis] << " upperBound=" << static_cast<double*>(upperBounds)[splitAxis] << std::endl;
      }
      else if (H5Tequal(ftype, H5T_NATIVE_FLOAT)) {
        float delta = (((float*)upperBounds)[splitAxis] -
            ((float*)lowerBounds)[splitAxis])/numCellsAlongSplitAxis;
        static_cast<float*>(lowerBounds)[splitAxis] += startCell * delta;
        static_cast<float*>(upperBounds)[splitAxis] =
        static_cast<float*>(lowerBounds)[splitAxis] + nCells * delta;
        //std::cout << "After adjust: Proc=" << PAR_Rank() << " start=" << startCell << " nCells=" << nCells << " delta=" << delta << " lowerBound=" << static_cast<float*>(lowerBounds)[splitAxis] << " upperBound=" << static_cast<float*>(upperBounds)[splitAxis] << std::endl;
      }
      numCells[splitAxis] = nCells;
      idims[splitAxis] = nCells + 1; // FIXME: May depend on centering
    }
    debugStrmRef <<methodSig <<"Parallel & Decompose_Domains are defined, exiting parallel code." << endl;
#endif

    // Create vtkRectilinearGrid
    debugStrmRef <<methodSig <<"Creating rectilinear grid." << endl;
    vtkRectilinearGrid* rgrid = vtkRectilinearGrid::New();
    rgrid->SetDimensions(idims);

    // Create coords arrays
    debugStrmRef <<methodSig <<"Creating coordinate arrays." << endl;
    vtkDataArray* coords[vsdim];
    for (size_t i = 0; i<rank;++i) {
      if (H5Tequal (ftype, H5T_NATIVE_DOUBLE)) {
        double delta = 0;
        coords[i] = vtkDoubleArray::New();
        if (numCells[i] != 0)
        delta = (((double*)upperBounds)[i] -
            ((double*)lowerBounds)[i])/numCells[i];
        for (size_t j = 0; j < idims[i]; ++j) {
          double temp = ((double*)lowerBounds)[i] + j*delta;
          coords[i]->InsertTuple(j, &temp);
        }
      }
      else if (H5Tequal(ftype, H5T_NATIVE_FLOAT)) {
        float delta = 0;
        coords[i] = vtkFloatArray::New();
        if (numCells[i] != 0)
        delta = (((float*)upperBounds)[i] -
            ((float*)lowerBounds)[i])/numCells[i];
        for (size_t j = 0; j < idims[i]; ++j) {
          float temp = ((float*)lowerBounds)[i] + j*delta;
          coords[i]->InsertTuple(j, &temp);
        }
      }
    }

    debugStrmRef <<methodSig <<"Setting misc data in coordinate arrays." << endl;
    for (size_t i = rank; i < vsdim; ++i) {
      if (H5Tequal (ftype, H5T_NATIVE_DOUBLE))
      coords[i] = vtkDoubleArray::New();
      else if (H5Tequal(ftype, H5T_NATIVE_FLOAT))
      coords[i] = vtkFloatArray::New();
      coords[i]->SetNumberOfTuples(1);
      coords[i]->SetComponent(0, 0, 0);
    }

    // Set grid data
    debugStrmRef <<methodSig <<"Adding coordinates to grid." << endl;
    rgrid->SetXCoordinates(coords[0]);
    rgrid->SetYCoordinates(coords[1]);
    rgrid->SetZCoordinates(coords[2]);

    // Clean local data
    debugStrmRef <<methodSig <<"Cleaning up." << endl;
    for (size_t i = 0; i<vsdim; ++i)
    coords[i]->Delete();
    delete [] startCell;

    debugStrmRef <<methodSig <<"Returning data." << endl;
    return rgrid;
  }

  vtkDataSet* avtVsFileFormat::getPointMesh(const std::string& name,
      const VsVariableWithMeshMeta& meta) {
    std::stringstream sstr;
    sstr <<"avtVsFileFormat::getPointMesh(" <<name <<") - ";
    std::string methodSig = sstr.str();
    debugStrmRef <<methodSig <<"Entering function." <<std::endl;

    hid_t type = meta.getType();
    if (!H5Tequal(type, H5T_NATIVE_DOUBLE) && !H5Tequal(type, H5T_NATIVE_FLOAT)) {
      debugStrmRef <<methodSig <<"Unsupported data type (only accept H5T_NATIVE_DOUBLE and H5T_NATIVE_FLOAT)"
      <<endl;
      debugStrmRef <<methodSig <<"Returning NULL." <<std::endl;
      return NULL;
    }

    // Get the number of values
    debugStrmRef <<methodSig <<"Getting the number of points." <<std::endl;
    std::vector<int> dims = meta.getDims();
    int numValues = 0;
    if (meta.isCompMajor())
    numValues = dims[dims.size() - 1];
    else
    numValues = dims[0];

    //Stride
    if (stride[0] != 1) {
      debugStrmRef <<methodSig <<"Filtering points based on stride.  Before = " <<numValues <<std::endl;
      numValues = numValues / stride[0];
      debugStrmRef <<methodSig <<"Filtering points based on stride.  After = " <<numValues <<std::endl;
    }
#if (defined PARALLEL && defined VIZSCHEMA_DECOMPOSE_DOMAINS)
    debugStrmRef <<methodSig <<"Parallel & Decompose_Domains are defined, entering parallel code." << endl;
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
    debugStrmRef <<methodSig <<"Parallel & Decompose_Domains are defined, exiting parallel code." << endl;
#endif

    debugStrmRef <<methodSig <<"There are " << numValues <<
    " points." << endl;

    // Read in points
    //
    // Create the unstructured meshPtr
    debugStrmRef <<methodSig <<"Creating the vtkUnstructuredGrid." <<std::endl;
    vtkUnstructuredGrid* meshPtr = vtkUnstructuredGrid::New();

    // Create and set points while small so minimal memory usage
    vtkPoints* vpts = vtkPoints::New();
    meshPtr->SetPoints(vpts);
    vpts->Delete();
    vtkPoints* vpoints = meshPtr->GetPoints();

    // Allocate
    size_t dsize = 0;
    if (H5Tequal(type, H5T_NATIVE_DOUBLE)) {
      vpoints->SetDataTypeToDouble();
      dsize = sizeof(double);
      debugStrmRef <<"Double data" <<std::endl;
    }
    else if (H5Tequal(type, H5T_NATIVE_FLOAT)) {
      vpoints->SetDataTypeToFloat();
      dsize = sizeof(float);
      debugStrmRef <<"Float data" <<std::endl;
    }
    debugStrmRef <<methodSig <<"Allocating for " <<numValues <<" values." <<std::endl;
    vpoints->SetNumberOfPoints(numValues);
    void* dataPtr = vpoints->GetVoidPointer(0);
    if (!dataPtr) {
      debugStrmRef << methodSig <<"Unable to allocate the points.  Cleaning up."
      << std::endl;
      meshPtr->Delete();
      debugStrmRef << methodSig <<"Returning NULL." <<std::endl;
      return NULL;
    }

    // Read in the data
    debugStrmRef << methodSig <<"Reading data." <<std::endl;
#if (defined PARALLEL && defined VIZSCHEMA_DECOMPOSE_DOMAINS)
    herr_t err = reader->getVarWithMeshMesh(meta, dataPtr, start, count);
#else
    herr_t err = reader->getVarWithMeshMesh(meta, dataPtr);
#endif

    if (err < 0) {
      debugStrmRef << methodSig <<"Call to getVarWithMeshMeta returned error: " <<err <<endl;
      debugStrmRef << methodSig <<"Returning NULL." <<std::endl;
      return NULL;
    } else {
      debugStrmRef << methodSig <<"Mesh points read." << endl;
    }

    // Move the data back to where it should be in a 3D array
    // WORK HERE - does this work properly for compMajor?
    if (meta.getNumSpatialDims() < 3) {
      debugStrmRef << methodSig <<"Moving data into correct positions." << endl;
      for (int i=numValues-1; i>=0; --i) {
        char* destPtr = (char*) dataPtr + i*3*dsize;
        char* srcPtr = (char*) dataPtr + i*meta.getNumSpatialDims()*dsize;
        memmove(destPtr, srcPtr, meta.getNumSpatialDims()*dsize);
        destPtr += meta.getNumSpatialDims()*dsize;
        memset(destPtr, 0, (3-meta.getNumSpatialDims())*dsize);
      }
    }

    // create point mesh
    try {
      debugStrmRef <<methodSig <<"Allocating " <<numValues
      << " vertices.  If old VTK and this fails, it will just abort." << endl;
      meshPtr->Allocate(numValues);
    } catch (vtkstd::bad_alloc& ex) {
      debugStrmRef <<methodSig <<"Caught vtkstd::bad_alloc. Unable to allocate cells."
      <<"Cleaning up." <<std::endl;
      meshPtr->Delete();
      debugStrmRef <<methodSig <<"Returning NULL." <<std::endl;
      return NULL;
    } catch (...) {
      debugStrmRef <<methodSig <<"Unknown exception. Unable to allocate cells."
      <<"Cleaning up." <<std::endl;
      meshPtr->Delete();
      debugStrmRef <<methodSig <<"Returning NULL." <<std::endl;
      return NULL;
    }

    debugStrmRef <<methodSig <<"Allocation succeeded.  Setting mesh to connectivity 'VERTEX'." <<std::endl;
    vtkIdType vertex;
    for (size_t i = 0; i < numValues; ++i) {
      vertex = i;
      meshPtr->InsertNextCell(VTK_VERTEX, 1, &vertex);
    }

    // Clean up memory
    ///TODO: IS there anything to clean up???
    debugStrmRef <<methodSig <<"Returning data." <<std::endl;
    return meshPtr;
  }

  vtkDataSet* avtVsFileFormat::getSplitPointMesh(const std::string& name,
      const VsUnstructuredMesh& meta) {
    std::stringstream sstr;
    sstr <<"avtVsFileFormat::getSplitPointMesh(" <<name <<") - ";
    std::string methodSig = sstr.str();
    debugStrmRef <<methodSig <<"Entering function." <<std::endl;

    VsUnstructuredMesh* unstructuredMesh = (VsUnstructuredMesh*)&meta;

    hid_t type0 = unstructuredMesh->getDataType();
    if (!H5Tequal(type0, H5T_NATIVE_DOUBLE) &&
        !H5Tequal(type0, H5T_NATIVE_FLOAT)) {
      debugStrmRef <<methodSig <<"Points are neither float nor double." <<std::endl;
      debugStrmRef <<methodSig <<"Returning NULL" <<std::endl;
      return NULL;
    }

    // Get the number of values
    debugStrmRef <<methodSig <<"Getting the number of points." <<std::endl;
    int numValues = unstructuredMesh->numnodes;
    debugStrmRef <<methodSig <<"There are " << numValues <<" points." << endl;

    // Read in points
    //
    // Create the unstructured meshPtr
    debugStrmRef <<methodSig <<"Creating the vtkUnstructuredGrid." <<std::endl;
    vtkUnstructuredGrid* meshPtr = vtkUnstructuredGrid::New();

    // Create and set points while small so minimal memory usage
    vtkPoints* vpts = vtkPoints::New();
    meshPtr->SetPoints(vpts);
    vpts->Delete();
    vtkPoints* vpoints = meshPtr->GetPoints();

    // Allocate
    size_t dsize = 0;
    if (H5Tequal(type0, H5T_NATIVE_DOUBLE)) {
      vpoints->SetDataTypeToDouble();
      dsize = sizeof(double);
      debugStrmRef <<"Double data" <<std::endl;
    }
    else if (H5Tequal(type0, H5T_NATIVE_FLOAT)) {
      vpoints->SetDataTypeToFloat();
      dsize = sizeof(float);
      debugStrmRef <<"Float data" <<std::endl;
    }

    debugStrmRef <<methodSig <<"Allocating for " <<numValues <<" values." <<std::endl;
    vpoints->SetNumberOfPoints(numValues);
    void* dataPtr = vpoints->GetVoidPointer(0);
    if (!dataPtr) {
      debugStrmRef << methodSig <<"Unable to allocate the points.  Cleaning up."
      << std::endl;
      meshPtr->Delete();
      debugStrmRef << methodSig <<"Returning NULL." <<std::endl;
      return NULL;
    }

    // Read in the data
    debugStrmRef << methodSig <<"Reading data." <<std::endl;

    std::string points0 = unstructuredMesh->getPointsDatasetName(0);
    std::string points1 = unstructuredMesh->getPointsDatasetName(1);
    std::string points2 = unstructuredMesh->getPointsDatasetName(2);

    herr_t err = reader->getSplitMeshData(points0, points1, points2, *unstructuredMesh, dataPtr);

    if (err < 0) {
      debugStrmRef << methodSig <<"Call to getVarWithMeshMeta returned error: " <<err <<endl;
      debugStrmRef << methodSig <<"Returning NULL." <<std::endl;
      return NULL;
    } else {
      debugStrmRef << methodSig <<"Mesh points read." << endl;
    }

    // create point mesh
    try {
      debugStrmRef <<methodSig <<"Allocating " <<numValues
      << " vertices.  If old VTK and this fails, it will just abort." << endl;
      meshPtr->Allocate(numValues);
    } catch (vtkstd::bad_alloc& ex) {
      debugStrmRef <<methodSig <<"Caught vtkstd::bad_alloc. Unable to allocate cells."
      <<"Cleaning up." <<std::endl;
      meshPtr->Delete();
      debugStrmRef <<methodSig <<"Returning NULL." <<std::endl;
      return NULL;
    } catch (...) {
      debugStrmRef <<methodSig <<"Unknown exception. Unable to allocate cells."
      <<"Cleaning up." <<std::endl;
      meshPtr->Delete();
      debugStrmRef <<methodSig <<"Returning NULL." <<std::endl;
      return NULL;
    }

    debugStrmRef <<methodSig <<"Allocation succeeded.  Setting mesh to connectivity 'VERTEX'." <<std::endl;
    vtkIdType vertex;
    for (size_t i = 0; i < numValues; ++i) {
      vertex = i;
      meshPtr->InsertNextCell(VTK_VERTEX, 1, &vertex);
    }

    // Clean up memory
    ///TODO: IS there anything to clean up???
    debugStrmRef <<methodSig <<"Returning data." <<std::endl;
    return meshPtr;
  }

  vtkDataArray* avtVsFileFormat::GetVar(int domain, const char* requestedName) {
    std::string name = requestedName;

    std::stringstream sstr;
    sstr <<"avtVsFileFormat::getVar(" <<domain <<", " <<name <<") - ";
    std::string methodSig = sstr.str();
    debugStrmRef <<methodSig <<"Entering function." <<std::endl;

    //Is this a component?
    //If so, we swap the component name with the "real" variable name
    //And remember that we're a component
    bool isAComponent = false;
    int componentIndex = 0;
    NamePair foundName;
    reader->getComponentInfo(name, &foundName);
    if (!foundName.first.empty()) {
      name = foundName.first.c_str();
      componentIndex = foundName.second;
      debugStrmRef <<methodSig <<"This is a component, and actually refers to variable " <<name <<" and index " <<componentIndex <<std::endl;
      isAComponent = true;
    }

    //the goal in all of this metadata loading is to fill these two variables:
    const VsVariableMeta* meta = NULL;
    const VsVariableWithMeshMeta* vmMeta = NULL;

    //could be an MD variable
    //if so, we retrieve the md metadata, look up the "real" variable name using the domain number,
    //and replace "name" with the name of that variable
    debugStrmRef <<methodSig <<"Checking for possible MD var." <<std::endl;
    const VsMDVariableMeta* mdMeta = reader->getMDVariableMeta(name);
    if (mdMeta == NULL) {
      debugStrmRef <<methodSig <<"No MD Var or component found under the name: " <<name <<std::endl;
    } else {
      debugStrmRef <<methodSig <<"Found MD metadata for this name: " <<name <<std::endl;
      //Go through the list of subordinate variables in this MD variable
      //For each variable
      //  get the name of the mesh that variable lives on
      //  find the matching MD Mesh
      //  figure out what domain number the MD Mesh has assigned to this mesh
      //  if this domain number matches the requested domain number, we found our variable
      for (unsigned int i = 0; i < mdMeta->blocks.size(); i++) {
        VsVariableMeta* foundVar = mdMeta->blocks[i];
        debugStrmRef <<methodSig <<"Checking subordinate var: " <<foundVar->getFullName() <<std::endl;
        std::string meshName = foundVar->getMesh();
        debugStrmRef <<methodSig <<"Subordinate var lives on mesh: " <<meshName <<std::endl;

        //find the matching MD mesh
        int meshDomain = reader->getDomainNumberForMesh(meshName);
        debugStrmRef <<methodSig <<"Domain number for mesh is " <<meshDomain <<std::endl;
        if (meshDomain == domain) {
          debugStrmRef <<methodSig <<"MD variable - domain numbers match: " <<meshDomain <<std::endl;
          debugStrmRef <<methodSig <<"Switching variableName from " <<name;
          name = foundVar->getFullName().c_str();
          debugStrmRef <<" to " <<name <<std::endl;
          meta = foundVar;
        }
      }
    }

    //Have we managed to retrieve the metadata for the variable yet?
    // if not, look for a "regular" variable with this name
    if (meta == NULL) {
      debugStrmRef <<methodSig <<"Looking for regular (non-md) variable." <<std::endl;
      meta = reader->getVariableMeta(name);
    }

    //How about now?
    // If no, look for a VarWithMesh with this name
    if (meta == NULL) {
      debugStrmRef <<methodSig <<"Looking for VarWithMesh variable." <<std::endl;
      vmMeta = reader->getVariableWithMeshMeta(name);
    }

    //If we haven't found metadata yet, we give up
    if ((meta == NULL) && (vmMeta == NULL)) {
      debugStrmRef << methodSig <<"ERROR: Could not find metadata for name: "<<name <<std::endl;
      debugStrmRef << "Returning NULL" << endl;
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
      std::string meshName = meta->getMesh();
      type = meta->getType();

      debugStrmRef <<methodSig <<"Mesh for variable is '" <<meshName << "'." << endl;
      debugStrmRef <<methodSig <<"Getting metadata for mesh." << endl;
      const VsMeshMeta* meshMetaPtr = reader->getMeshMeta(meshName);

      //could be an MD mesh...
      if (!meshMetaPtr) {
        debugStrmRef <<methodSig <<"Did not find mesh " << meshName <<" in regular meshes, looking in MD." <<std::endl;
        meshMetaPtr = reader->findSubordinateMDMesh(meshName);
      }

      if (meshMetaPtr) {
        debugStrmRef <<methodSig <<"Found metadata for "
        << "mesh '" << meshName << "'." << endl;
      }
      else {
        debugStrmRef <<methodSig <<"Metadata not found "
        "for mesh '" << meshName << "'." << endl;
        debugStrmRef <<methodSig <<"Returning NULL" <<std::endl;
        return NULL;
      }
    }

    debugStrmRef <<methodSig <<"Determining dimensionality." <<std::endl;
    size_t rank = dims.size();
    if (isAComponent) --rank;

    //this gets complicated because it depends on the kind of variable (zonal vs nodal)
    // and the kind of the mesh (uniform vs structured)
    // We must also be careful to stay away if the mesh is NOT uniform or structured
    //specifically we must stay away from VarWithMesh, since it has already adjusted the sizes
    if (reader->useStride && (vmMeta == NULL)) {
      const VsMeshMeta* meshMeta = reader->getMeshMeta(meta->mesh);

      int addBefore = 0;
      int addAfter = 0;
      if (!meshMeta) {
        debugStrmRef <<methodSig <<"ERROR - Unable to load mesh metadata with name : " <<meta->mesh <<std::endl;
        debugStrmRef <<methodSig <<"ERROR - returning NULL" <<std::endl;
        return NULL;
      }

      if (meshMeta->isUniformMesh()) {
        if (meta->isZonal()) {
          debugStrmRef <<methodSig <<"Zonal on uniform = no change" <<std::endl;
          adjustSize_vector(&dims, rank, stride, addBefore, addAfter);
        } else {
          //nodal
          debugStrmRef <<methodSig <<"Nodal on uniform = -1/+1" <<std::endl;
          addBefore = -1;
          addAfter = 1;
          adjustSize_vector(&dims, rank, stride, addBefore, addAfter);
        }
      } else if (meshMeta->isStructuredMesh()) {
        if (meta->isZonal()) {
          debugStrmRef <<methodSig <<"Zonal on structured = +1/-1" <<std::endl;
          std::vector<int> meshDims;
          reader->getMeshDims(meta->mesh, &meshDims);
          for (unsigned int i = 0; i < rank; i++)
          {
            debugStrmRef <<methodSig <<"About to override size " <<dims[i] <<" with size from mesh - 1: " <<(meshDims[i] - 1) <<std::endl;
            dims[i] = meshDims[i] - 1;
          }

        } else {
          //nodal
          debugStrmRef <<methodSig <<"Nodal on structured" <<std::endl;
          std::vector<int> meshDims;
          reader->getMeshDims(meta->mesh, &meshDims);
          for (unsigned int i = 0; i < rank; i++)
          {
            debugStrmRef <<methodSig <<"About to override size " <<dims[i] <<" with size from mesh: " <<(meshDims[i]) <<std::endl;
            dims[i] = meshDims[i];
          }
        }
      }
    }

    debugStrmRef <<methodSig << "Variable " <<name << " has dimensions =";
    size_t len = 1;
    if (isCompMajor) {
      for (size_t i=1; i < rank + 1; ++i) {
        debugStrmRef << " " << dims[i];
        len *= dims[i];
      }
    } else {
      for (size_t i=0; i < rank; ++i) {
        debugStrmRef << " " << dims[i];
        len *= dims[i];
      }
    }
    debugStrmRef << ", rank = " << rank << ", isComponent = " <<
    isAComponent << "." << std::endl;
    debugStrmRef <<"Length is " <<len <<std::endl;

    debugStrmRef <<methodSig <<"Declaring vtkArray of proper type." <<std::endl;
    vtkDataArray* rv = 0;
    if (H5Tequal(type, H5T_NATIVE_DOUBLE)) {
      debugStrmRef <<methodSig <<"Declaring vtkDoubleArray." <<std::endl;
      rv = vtkDoubleArray::New();
    }
    else if (H5Tequal(type, H5T_NATIVE_FLOAT)) {
      debugStrmRef <<methodSig <<"Declaring vtkFloatArray." <<std::endl;
      rv = vtkFloatArray::New();
    }
    else if (H5Tequal(type, H5T_NATIVE_INT)) {
      debugStrmRef <<methodSig <<"Declaring vtkIntArray." <<std::endl;
      rv = vtkIntArray::New();
    }
    else {
      debugStrmRef <<methodSig <<"Unknown data type in " << name << "." << std::endl;
      debugStrmRef <<methodSig <<"Returning NULL." << std::endl;
      return NULL;
    }

    // Read in the data
    debugStrmRef << methodSig <<"Reading in the data." << endl;
    void* data = 0;
    //  size_t len = 0;
    size_t sz = H5Tget_size(type);
    if (vmMeta) {
      debugStrmRef << methodSig <<"Entering VarWithMesh section." << endl;
      //    int lastDim = dims[dims.size()-1];
      //    len = vmmeta->getLength()/lastDim;
#if (defined PARALLEL && defined VIZSCHEMA_DECOMPOSE_DOMAINS)
      debugStrmRef <<methodSig <<"Parallel & Decompose_Domains are defined, entering parallel code." << endl;
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
      debugStrmRef <<methodSig <<"Parallel & Decompose_Domains are defined, exiting parallel code." << endl;
#endif
      //changed from "new unsigned" to "new char" because unsigned is 4 bytes, char is 1
      debugStrmRef <<methodSig <<"About to allocate space for " <<len <<" values of size " <<sz <<"." << endl;
      debugStrmRef <<methodSig <<"Total allocation: " <<(len * sz) <<" bytes." << endl;
      data = new char[len*sz];
      if (!data) {
        debugStrmRef <<methodSig <<"Unable to allocate memory." << endl;
        debugStrmRef <<methodSig <<"Returning NULL." << endl;
        return NULL;
      }

      debugStrmRef <<methodSig <<"Reading var with mesh data." << endl;
#if (defined PARALLEL && defined VIZSCHEMA_DECOMPOSE_DOMAINS)
      herr_t err = reader->getVarWithMeshComponent(name, componentIndex, data, start, count);
#else
      herr_t err = reader->getVarWithMeshComponent(name, componentIndex, data);
#endif
      if (err < 0) {
        debugStrmRef <<methodSig <<"GetVarWithMeshComponent returned error: " <<err <<endl;
        debugStrmRef <<methodSig <<"Returning NULL." << endl;
        return NULL;
      }
    }
    else if (isAComponent) {
      debugStrmRef << methodSig <<"Entering Component section." << endl;
      // Read a var comp
#if (defined PARALLEL && defined VIZSCHEMA_DECOMPOSE_DOMAINS)
      debugStrmRef <<methodSig <<"Parallel & Decompose_Domains are defined, entering parallel code." << endl;
      size_t splitDims[rank];
      data = reader->getVariableComponent(name, componentIndex, PAR_Rank(), PAR_Size(), splitDims);
      if (!data) return 0;
      debugStrmRef << methodSig <<"Original dimensions are";
      for (size_t i=0; i<rank; ++i) {
        debugStrmRef << " " << dims[i];
      }
      len = 1;
      for (size_t i=0; i<rank; ++i) {
        dims[i] = splitDims[i];
        len *= dims[i];
      }
      debugStrmRef << std::endl;
      debugStrmRef <<methodSig <<"Dimensions after split are";
      for (size_t i=0; i<rank; ++i) {
        debugStrmRef << " " << dims[i];
      }
      debugStrmRef << " len=" << len << std::endl;
      debugStrmRef <<methodSig <<"Parallel & Decompose_Domains are defined, exiting parallel code." << endl;
#else
      debugStrmRef <<methodSig <<"Determining number of components for variable." <<std::endl;
      /*    size_t nc = reader->getNumComps(nm);
       if (!nc) {
       debugStrmRef <<methodSig <<"Number of components is zero for variable." <<endl;
       debugStrmRef <<methodSig <<"Returning NULL." <<endl;
       return NULL;
       } else {
       debugStrmRef <<methodSig <<"Variable has " <<nc <<" components." <<std::endl;
       }
       */
      //    len = meta->getLength()/nc;
      //changed from "new unsigned" to "new char" because unsigned is 4 bytes, char is 1
      debugStrmRef <<methodSig <<"About to allocate space for " <<len <<" values of size " <<sz <<"." << endl;
      debugStrmRef <<methodSig <<"Total allocation: " <<(len * sz) <<" bytes." << endl;
      data = new char[len*sz];
      if (!data) {
        debugStrmRef <<methodSig <<"Unable to allocate memory." << endl;
        debugStrmRef <<methodSig <<"Returning NULL." << endl;
        return NULL;
      }

      debugStrmRef <<methodSig <<"Loading variable data." << endl;
      herr_t err = reader->getVariableComponent(name, componentIndex, data);
      if (err < 0) {
        debugStrmRef <<methodSig <<"GetVariableComponent returned error: " <<err <<endl;
        debugStrmRef <<methodSig <<"Returning NULL." << endl;
        return NULL;
      } else {
        debugStrmRef <<methodSig <<"Found component " << componentIndex <<" for variable " <<name <<std::endl;
      }
#endif
    }
    else {
      debugStrmRef << methodSig <<"Entering regular Variable section." << endl;
#if (defined PARALLEL && defined VIZSCHEMA_DECOMPOSE_DOMAINS)
      // Don't know how to decompose this type of mesh -> load it on proc 0 only
      if (PAR_Rank() > 0)
      {
        return NULL;
      }
#endif
      //   len = meta->getLength();
      //changed from "new unsigned" to "new char" because unsigned is 4 bytes, char is 1
      debugStrmRef <<methodSig <<"About to allocate space for " <<len <<" values of size " <<sz <<"." << endl;
      debugStrmRef <<methodSig <<"Total allocation: " <<(len * sz) <<" bytes." << endl;
      data = new char[len*sz];
      if (!data) {
        debugStrmRef <<methodSig <<"Unable to allocate memory." << endl;
        debugStrmRef <<methodSig <<"Returning NULL." << endl;
        return NULL;
      }

      debugStrmRef <<methodSig <<"Loading variable data." << endl;
      herr_t err = reader->getVariable(name, data);
      if (err < 0) {
        debugStrmRef <<methodSig <<"GetVariable returned error: " <<err <<endl;
        debugStrmRef <<methodSig <<"Returning NULL." << endl;
        return NULL;
      } else {
        debugStrmRef <<methodSig <<"Successfully loaded data." <<std::endl;
      }
    }

    debugStrmRef <<methodSig <<"Finished reading the data, building VTK structures." <<endl;

    //DEBUG
    /*
     debugStrmRef <<methodSig <<"Dumping data: " <<std::endl;
     for (int i = 0; i < len; i++) {
     if (H5Tequal(type, H5T_NATIVE_DOUBLE)) {
     debugStrmRef <<"data[" <<i <<"] = " <<((double*)data)[i] <<std::endl;}
     else if (H5Tequal(type, H5T_NATIVE_FLOAT)) {
     debugStrmRef <<"data[" <<i <<"] = " <<((float*)data)[i] <<std::endl;}
     else if (H5Tequal(type, H5T_NATIVE_INT)) {
     debugStrmRef <<"data[" <<i <<"] = " <<((int*)data)[i] <<std::endl;}
     }
     debugStrmRef <<methodSig <<"Finished dumping data. " <<std::endl;
     */
    //END DEBUG

    rv->SetNumberOfTuples(len);

    // Perform if needed permutation of index as VTK expect Fortran order

    // The index tuple is initially all zeros
    size_t* indices = new size_t[rank];
    for (size_t k=0; k<rank; ++k)
    indices[k] = 0;

    // Store data
    debugStrmRef <<methodSig <<"Storing " << len <<" data elements" <<std::endl;

    // Attempt to reverse data in place
    //#define IN_PLACE
#ifdef IN_PLACE
    debugStrmRef <<methodSig <<"Attempting to swap data in place." <<std::endl;
    double* dblDataPtr = (double*) data;
    float* fltDataPtr = (float*) data;
    int* intDataPtr = (int*) data;

    // Step through by global C index and reverse
    if (rank > 1) {
      for (size_t k = 0; k<len; ++k) {
        // Accumulate the Fortran index
        size_t indx = indices[rank-1];
        for (size_t j = 2; j<=rank; ++j)
        indx = indx*dims[rank-j] + indices[rank-j];
        // Set the value in the VTK array at the Fortran index to the
        // value in the C array at the C index
        if (H5Tequal(type, H5T_NATIVE_DOUBLE)) {
          double tmp = dblDataPtr[indx];
          dblDataPtr[indx] = dblDataPtr[k];
          dblDataPtr[k] = tmp;
        }
        else if (H5Tequal(type, H5T_NATIVE_FLOAT)) {
          float tmp = fltDataPtr[indx];
          fltDataPtr[indx] = fltDataPtr[k];
          fltDataPtr[k] = tmp;
        }
        else if (H5Tequal(type, H5T_NATIVE_INT)) {
          int tmp = intDataPtr[indx];
          intDataPtr[indx = intDataPtr[k];
          intDataPtr[k] = tmp;
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
    for (size_t k = 0; k<len; ++k) {
      if (H5Tequal(type, H5T_NATIVE_DOUBLE)) {
        rv->SetTuple(k, &dblDataPtr[k]);
      }
      else if (H5Tequal(type, H5T_NATIVE_FLOAT)) {
        rv->SetTuple(k, &fltDataPtr[k]);
      } else if (H5Tequal(type, H5T_NATIVE_INT)) {
        rv->SetTuple(k, &intDataPtr[k]);
      }
    }

    debugStrmRef <<methodSig <<"Done swapping data in place." <<std::endl;
#else
    debugStrmRef <<methodSig <<"Swapping data into correct places, NOT using 'in place' code." <<std::endl;

    if (isCompMajor) {
      double* dblDataPtr = (double*) data;
      float* fltDataPtr = (float*) data;
      int* intDataPtr = (int*) data;

      // If we're compMajor, we don't need to swap data around
      for (size_t k = 0; k<len; ++k) {
        //debugStrmRef <<"value at index " <<k <<" is " <<dblDataPtr[k] <<std::endl;
        if (H5Tequal(type, H5T_NATIVE_DOUBLE)) {
          rv->SetTuple(k, &dblDataPtr[k]);
        } else if (H5Tequal(type, H5T_NATIVE_FLOAT)) {
          rv->SetTuple(k, &fltDataPtr[k]);
        } else if (H5Tequal(type, H5T_NATIVE_INT)) {
          //we convert to float because SetTuple doesn't take ints
          float* temp = (float*)&intDataPtr[k];
          rv->SetTuple(k, temp);
        }
      }
    } else {
      // Step through by global C index
      for (size_t k = 0; k<len; ++k) {
        // Accumulate the Fortran index
        size_t indx = indices[rank-1];
        for (size_t j = 2; j<=rank; ++j) {
          indx = indx*dims[rank-j] + indices[rank-j];
        }

        // Set the value in the VTK array at the Fortran index to the
        // value in the C array at the C index
        if (H5Tequal(type, H5T_NATIVE_DOUBLE)) {
          rv->SetTuple(indx, &((double*) data)[k]);
        }
        else if (H5Tequal(type, H5T_NATIVE_FLOAT)) {
          rv->SetTuple(indx, &((float*) data)[k]);
        }
        else if (H5Tequal(type, H5T_NATIVE_INT)) {
          float temp = ((int*)data)[k];
          rv->SetTuple(indx, &temp);
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

    debugStrmRef <<methodSig <<"Done swapping data into correct places, NOT using 'in place' code." <<std::endl;
#endif

    // Done with data
    debugStrmRef <<methodSig <<"Cleaning up." <<std::endl;
    delete [] (char*) data;
    delete [] indices;

    debugStrmRef <<methodSig <<"Returning data." <<std::endl;
    return rv;
  }

  void avtVsFileFormat::FreeUpResources(void) {
    debugStrmRef <<"avtVsFileFormat::FreeUpResources() enter/exit." <<std::endl;
  }

  void avtVsFileFormat::RegisterExpressions(avtDatabaseMetaData* md) {
    std::stringstream sstr;
    sstr <<"avtVsFileFormat::RegisterExpressions() - ";
    std::string methodSig = sstr.str();
    debugStrmRef <<methodSig <<"Entering function." <<std::endl;

    //get list of expressions from reader
    std::map<std::string, std::string> vsVars = reader->getVsVars();

    if (vsVars.size() == 0) {
      debugStrmRef <<methodSig <<"WARNING: No VsVars found in file. Returning." <<std::endl;
      return;
    } else {
      debugStrmRef <<methodSig <<"Found " <<vsVars.size() <<" vsVars in file." <<std::endl;
    }

    //iterate over list of vars, insert each one into database
    std::map<std::string, std::string>::const_iterator iv;
    for (iv = vsVars.begin(); iv != vsVars.end(); ++iv) {
      debugStrmRef <<methodSig <<"Adding expression " << iv->first << " = "
      << iv->second << endl;
      Expression e;
      e.SetName (iv->first);

      //TODO: if the user has supplied a label for a component
      // but the vsVar expression still refers to the old component name
      // we need to either 1. register the old component name as an extra component
      // or 2. edit the vsVar expression to replace old component names with the user-specified labels.
      e.SetDefinition(iv->second);

      // See if the expression is a vector
      if (iv->second[0] == '{') {
        debugStrmRef <<methodSig <<"It is a vector expression." << endl;
        e.SetType(Expression::VectorMeshVar);
      }
      else {
        debugStrmRef <<methodSig <<"It is a scalar expression." << endl;
        e.SetType(Expression::ScalarMeshVar);
      }
      md->AddExpression(&e);
    }

    debugStrmRef <<methodSig <<"Exiting normally." << endl;
  }

  void avtVsFileFormat::RegisterVars(avtDatabaseMetaData* md) {
    std::stringstream sstr;
    sstr <<"avtVsFileFormat::RegisterVars() - ";
    std::string methodSig = sstr.str();
    debugStrmRef <<methodSig <<"Entering function." <<std::endl;

    // Get var names
    std::vector<std::string> names;
    reader->getVarsNames(names);

    if (names.size() == 0) {
      debugStrmRef <<methodSig <<"WARNING: No variables were found in this file.  Returning." <<endl;
      return;
    } else {
      debugStrmRef <<methodSig <<"Found " <<names.size() <<" variables in this file." <<endl;
    }

    std::vector<std::string>::const_iterator it;
    for (it = names.begin(); it != names.end(); ++it) {
      debugStrmRef <<methodSig <<"Processing var: "<< *it <<std::endl;

      //get metadata for var
      const VsVariableMeta* vMeta = reader->getVariableMeta(*it);

      // Name of the mesh of the var
      std::string mesh = vMeta->mesh;
      debugStrmRef <<methodSig <<"Var lives on mesh " << mesh << "." << endl;

      // Centering of the variable
      // Default is node centered data
      avtCentering centering = AVT_NODECENT;
      if (vMeta->isZonal()) {
        debugStrmRef <<methodSig <<"Var is zonal." << endl;
        centering = AVT_ZONECENT;
      } else {
        debugStrmRef <<methodSig <<"Var is nodal." << endl;
      }

      // 1-D variable?
      debugStrmRef <<methodSig <<"Determining if var is 1-D." << endl;
      std::vector<int> dims;
      reader->getMeshDims(mesh, &dims);

      //if this mesh is 1-D, we leave it for later (curves)
      bool isOneDVar = false;
      if (dims.size() == 1) {
        debugStrmRef <<methodSig <<"Var is 1-D." << endl;
        isOneDVar = true;
      } else {
        debugStrmRef <<methodSig <<"Var is not 1-D." << endl;
        isOneDVar = false;
      }

      // Number of component of the var
      size_t numComps = reader->getNumComps(*it);
      if (isOneDVar) {
        debugStrmRef <<methodSig <<"Adding curve metadata for " <<*it <<endl;
        avtCurveMetaData* cmd = new avtCurveMetaData((*it).c_str());
        cmd->hasDataExtents = false;
        md->Add(cmd);
      }
      else if (numComps > 1) {
        //go through list of components
        //generate a name for each one
        //then add to VisIt registry
        for (size_t i = 0; i<numComps; ++i) {
          //first, get a unique name for this component
          std::string compName = reader->getUniqueComponentName(vMeta->getLabel(i), *it, i);

          //next, register the component info
          reader->registerComponentInfo(compName, *it, i);

          //for backwards compatibility, register the "old-style" name too
          std::string oldCompName = reader->getOldComponentName(*it, i);
          reader->registerComponentInfo(oldCompName, *it, i);

          debugStrmRef <<methodSig <<"Adding variable component " <<compName <<"." <<std::endl;
          avtScalarMetaData* smd = new avtScalarMetaData(compName.c_str(), mesh.c_str(), centering);
          smd->hasUnits = false;
          md->Add(smd);
        }
      }
      else if (numComps == 1) {
        debugStrmRef <<methodSig <<"Adding single-component variable." <<std::endl;
        avtScalarMetaData* smd = new avtScalarMetaData(*it, mesh.c_str(), centering);
        smd->hasUnits = false;
        md->Add(smd);
      }
      else {
        debugStrmRef <<methodSig <<"Variable '" << *it << "' has no components. Not being added." << endl;
      }
    }

    debugStrmRef <<methodSig <<"Exiting normally." << endl;
  }

  void avtVsFileFormat::RegisterMeshes(avtDatabaseMetaData* md) {
    std::stringstream sstr;
    sstr <<"avtVsFileFormat::RegisterMeshes() - ";
    std::string methodSig = sstr.str();
    debugStrmRef <<methodSig <<"Entering function." <<std::endl;

    // Number of mesh dims
    int mdims;

    // Dims read from HDF5
    std::vector<int> dims;

    // All meshes names
    std::vector<std::string> names;
    reader->getMeshesNames(names);
    if (names.size() == 0) {
      debugStrmRef <<methodSig <<"WARNING: no meshes were found in this file. Returning." <<endl;
      return;
    } else {
      debugStrmRef <<methodSig <<"Found " <<names.size() << " meshes." << endl;
    }

    std::vector<std::string>::const_iterator it;
    for (it = names.begin(); it != names.end(); ++it) {
      const VsMeshMeta* meta = reader->getMeshMeta(*it);
      debugStrmRef <<methodSig <<"Found mesh '"
      << *it << "' of kind '" << meta->kind << "'." << endl;
      reader->getMeshDims(*it, &dims);

      //if this mesh is 1-D, we leave it for later (curves)
      if (dims.size() == 1) {
        debugStrmRef <<methodSig <<"Found 1-d mesh.  Skipping for now, will be added as a curve." <<endl;
        continue;
      } else if (dims.size() == 0) {
        debugStrmRef <<methodSig <<"WARNING: getMeshDims returned empty dims array." <<endl;
        debugStrmRef <<methodSig <<"WARNING: skipping mesh." <<endl;
        continue;
      }

      if (meta->isUniformMesh()) {
        // 09.06.01 Marc Durant
        // We used to report uniform cartesian meshes as being 3d no matter what
        // I have changed this to use the correct mesh dimensions
        // The changed plugin passes vstests, and is motivated because
        // the VisIt Lineout operator requires 2-d data.
        // EXCEPT! then we can't plot 3-d vectors on the 2-d data, so for now we continue to report 3
        mdims = 3;
        //mdims = dims.size();
        debugStrmRef <<methodSig <<"Mesh's dimension = " << dims.size() << endl;
        if (dims.size() != 3)
        debugStrmRef <<methodSig <<"But reporting as dimension = " <<mdims <<" to side-step VisIt bug in 1.11.2." <<std::endl;

        debugStrmRef <<methodSig <<"Adding uniform mesh " <<*it <<"." <<endl;
        avtMeshMetaData* vmd = new avtMeshMetaData(it->c_str(),
            1, 1, 1, 0, mdims, mdims, AVT_RECTILINEAR_MESH);
        setAxisLabels(vmd);
        md->Add(vmd);
        debugStrmRef <<methodSig <<"Succeeded in adding mesh " <<*it <<"." <<endl;
      }
      else if (meta->isUnstructuredMesh()) {
        //Unstructured meshes without connectivity data are registered as point meshes
        VsUnstructuredMesh* unstructuredMesh = (VsUnstructuredMesh*)meta;
        if (unstructuredMesh->isPointMesh()) {
          debugStrmRef <<methodSig <<"Registering mesh " <<it->c_str() <<" as AVT_POINT_MESH" <<std::endl;
          avtMeshMetaData* vmd = new avtMeshMetaData(it->c_str(),
              1, 1, 1, 0, unstructuredMesh->numSpatialDims, 0, AVT_POINT_MESH);
          setAxisLabels(vmd);
          md->Add(vmd);
        }
        else {
          debugStrmRef <<methodSig <<"Registering mesh " <<it->c_str() <<" as AVT_UNSTRUCTURED_MESH" <<std::endl;
          mdims = dims[1];
          debugStrmRef <<methodSig <<"Adding unstructured mesh " <<*it <<"." <<endl;
          avtMeshMetaData* vmd = new avtMeshMetaData(it->c_str(),
              1, 1, 1, 0, mdims, mdims, AVT_UNSTRUCTURED_MESH);
          setAxisLabels(vmd);
          md->Add(vmd);
        }
      }
      else if (meta->isStructuredMesh()) {
        mdims = dims.size()-1;
        debugStrmRef <<methodSig <<"Adding structured mesh " <<*it <<"." <<endl;
        avtMeshMetaData* vmd = new avtMeshMetaData(it->c_str(),
            1, 1, 1, 0, mdims, mdims, AVT_CURVILINEAR_MESH);
        setAxisLabels(vmd);
        md->Add(vmd);
      }
      else {
        debugStrmRef <<methodSig <<"Unrecognized mesh kind: " <<meta->kind <<"." <<std::endl;
      }
    }
    debugStrmRef <<methodSig <<"Exiting normally." << endl;
  }

  void avtVsFileFormat::RegisterMdVars(avtDatabaseMetaData* md) {
    std::stringstream sstr;
    sstr <<"avtVsFileFormat::RegisterMdVars() - ";
    std::string methodSig = sstr.str();
    debugStrmRef <<methodSig <<"Entering function." <<std::endl;

    // Get vars names
    std::vector<std::string> names;
    reader->getMDVarsNames(names);

    if (names.size() == 0) {
      debugStrmRef <<methodSig <<"WARNING: No MD variables were found in this file.  Returning." <<std::endl;
      return;
    } else {
      debugStrmRef <<methodSig <<"Found " <<names.size() <<" MD variables in this file." <<std::endl;
    }

    std::vector<std::string>::const_iterator it;
    for (it = names.begin(); it != names.end(); ++it) {
      debugStrmRef <<methodSig <<"Processing md var '"
      << *it << "'." << std::endl;
      const VsMDVariableMeta* vMeta = reader->getMDVariableMeta(*it);
      // Name of the mesh of the var
      std::string mesh = vMeta->mesh;
      std::string vscentering = vMeta->centering;
      debugStrmRef <<methodSig <<"MD var lives on mesh " << mesh << "." << std::endl;

      //TODO: Mesh should either exist in an mdMesh, or separately in the list of meshes

      // Centering of the variable
      // Default is node centered data
      avtCentering centering = AVT_NODECENT;
      if (vMeta->isZonal()) {
        debugStrmRef <<methodSig <<"Var is zonal" << std::endl;
        centering = AVT_ZONECENT;
      } else {
        debugStrmRef <<methodSig <<"Var is nodal" << std::endl;
      }

      // Number of component of the var
      size_t numComps = reader->getMDNumComps(*it);
      debugStrmRef <<methodSig <<"Variable has " <<numComps <<" components." <<std::endl;
      if (numComps > 1) {
        for (size_t i = 0; i<numComps; ++i) {
          //first, get a unique name for this component
          std::string compName = reader->getUniqueComponentName(vMeta->getLabel(i), *it, i);

          //next, register the component info
          reader->registerComponentInfo(compName, *it, i);

          //for backwards compatibility, register the "old-style" name too
          std::string oldCompName = reader->getOldComponentName(*it, i);
          reader->registerComponentInfo(oldCompName, *it, i);

          debugStrmRef <<methodSig <<"Adding variable component " <<compName <<"." <<std::endl;
          avtScalarMetaData* smd = new avtScalarMetaData(compName.c_str(),
              mesh.c_str(), centering);
          smd->hasUnits = false;
          md->Add(smd);
        }
      }
      else if (numComps == 1) {
        debugStrmRef <<methodSig <<"Adding single variable component " <<*it <<"." <<std::endl;
        avtScalarMetaData* smd = new avtScalarMetaData(*it, mesh.c_str(), centering);
        smd->hasUnits = false;
        md->Add(smd);
      } else {
        debugStrmRef <<methodSig <<"Variable '" << *it << "' has no components. Not being added." << std::endl;
      }
    }
    debugStrmRef <<methodSig <<"Exiting normally." << std::endl;
  }

  void avtVsFileFormat::RegisterMdMeshes(avtDatabaseMetaData* md) {
    std::stringstream sstr;
    sstr <<"avtVsFileFormat::RegisterMdMeshes() - ";
    std::string methodSig = sstr.str();
    debugStrmRef <<methodSig <<"Entering function." <<std::endl;

    std::vector<std::string> names;
    reader->getMDMeshNames(names);
    if (names.size() == 0) {
      debugStrmRef <<methodSig <<"WARNING: no md meshes were found in this file. Returning" <<std::endl;
      return;
    } else {
      debugStrmRef <<methodSig <<"Found " <<names.size() <<" MD meshes in this file." <<std::endl;
    }

    std::vector<std::string>::const_iterator it;
    for (it = names.begin(); it != names.end(); ++it) {
      debugStrmRef << methodSig <<" Adding md mesh '"
      << *it << "'." << endl;

      const VsMDMeshMeta* meta = reader->getMDMeshMeta(*it);

      avtMeshType meshType;
      std::string kind = meta->getMeshKind();
      if (meta->isUniformMesh()) {
        debugStrmRef << methodSig <<"Mesh is rectilinear" <<std::endl;
        meshType = AVT_RECTILINEAR_MESH;
      } else if (meta->isUnstructuredMesh()) {
        debugStrmRef << methodSig <<"Mesh is unstructured" <<std::endl;
        meshType = AVT_UNSTRUCTURED_MESH;
      } else if (meta->isStructuredMesh()) {
        debugStrmRef << methodSig <<"Mesh is structured" <<std::endl;
        meshType = AVT_CURVILINEAR_MESH;
      }

      debugStrmRef << methodSig <<"Mesh has dimension " <<meta->getDims() <<"." <<std::endl;

      avtMeshMetaData* vmd = new avtMeshMetaData(it->c_str(),
          meta->getNumBlocks(), 1, 1, 0, meta->getDims(), meta->getDims(), meshType);
      setAxisLabels(vmd);
      md->Add(vmd);
    }

    debugStrmRef <<methodSig <<"Exiting normally." << endl;
  }

  //This method is different from the others because it adds
  // BOTH the mesh and the associated variables
  void avtVsFileFormat::RegisterVarsWithMesh(avtDatabaseMetaData* md) {
    std::stringstream sstr;
    sstr <<"avtVsFileFormat::RegisterVarsWithMesh() - ";
    std::string methodSig = sstr.str();
    debugStrmRef <<methodSig <<"Entering function." <<std::endl;

    std::vector<std::string> names;
    reader->getVarsWithMeshNames(names);

    if (names.size() == 0) {
      debugStrmRef <<methodSig <<"WARNING: no variables with mesh were found in this file. Returning." <<endl;
      return;
    } else {
      debugStrmRef <<methodSig <<"Found " <<names.size() <<" variables with mesh in this file." <<endl;
    }

    std::vector<std::string>::const_iterator it;
    for (it = names.begin(); it != names.end(); ++it) {
      debugStrmRef <<methodSig <<"Processing varWithMesh '"
      << *it << "'." << endl;
      const VsVariableWithMeshMeta* vMeta = reader->getVariableWithMeshMeta(*it);

      // add var components
      std::vector<int> dims = vMeta->getDims();
      if (dims.size() <= 0) {
        std::string msg = "avtVsFileFormat::PopulateDatabaseMetaData: could not get dimensions of variable with mesh.";
        debugStrmRef << msg << std::endl;
        throw std::out_of_range(msg.c_str());
      }

      size_t lastDim = 0;
      if (vMeta->isCompMinor())
      lastDim = dims[dims.size()-1];
      else lastDim = dims[0];

      if (lastDim < vMeta->getNumSpatialDims()) {
        debugStrmRef <<methodSig <<"Error: "
        "for variable with mesh '" << *it << "', numSpatialDims = " <<
        vMeta->getNumSpatialDims() << " must be larger then the last dimension, " <<
        "lastDim = " << lastDim << "." << endl;
        debugStrmRef <<methodSig <<"Error: Attempting to remove associated mesh from system." <<std::endl;
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
        std::string compName = reader->getUniqueComponentName(vMeta->getLabel(i), *it, i);

        //next, register the component info
        reader->registerComponentInfo(compName, *it, i);

        //for backwards compatibility, register the "old-style" name too
        std::string oldCompName = reader->getOldComponentName(*it, i);
        reader->registerComponentInfo(oldCompName, *it, i);

        //register with VisIt
        debugStrmRef <<methodSig <<"Adding variable component " <<compName <<"." <<std::endl;
        avtScalarMetaData* smd = new avtScalarMetaData(compName.c_str(),
            it->c_str(), AVT_NODECENT);
        smd->hasUnits = false;
        md->Add(smd);
      }

      // add var mesh
      debugStrmRef <<methodSig <<"Adding point mesh for this variable." <<std::endl;
      avtMeshMetaData* vmd = new avtMeshMetaData(it->c_str(),
          1, 1, 1, 0, vMeta->getNumSpatialDims(), 0, AVT_POINT_MESH);
      setAxisLabels(vmd);
      md->Add(vmd);
    }

    debugStrmRef <<methodSig <<"Exiting normally." << endl;
  }

  void avtVsFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData* md) {
    std::stringstream sstr;
    sstr <<"avtVsFileFormat::PopulateDatabaseMetaData() - ";
    std::string methodSig = sstr.str();
    debugStrmRef <<methodSig <<"Entering function." <<std::endl;

    // Tell visit that we can split meshes into subparts when running in parallel
    // NOTE that we can't decompose domains if we have MD meshes
    // So it's one or the other
    std::vector<std::string> names;
#ifdef VIZSCHEMA_DECOMPOSE_DOMAINS
    debugStrmRef <<methodSig <<"Decompose_domains is defined.  Entering code block." <<std::endl;
    reader->getMDMeshNames(names);
    if (names.size() > 0) {
      debugStrmRef <<methodSig <<"MD meshes are present in the data file.  Domain Decomposition is turned off." <<std::endl;
      md->SetFormatCanDoDomainDecomposition(false);
    } else {
      debugStrmRef <<methodSig <<"NO MD meshes are present in the data file.  Domain Decomposition is turned on." <<std::endl;
      md->SetFormatCanDoDomainDecomposition(true);
    }
    names.clear();
    debugStrmRef <<methodSig <<"Decompose_domains is defined.  Exiting code block." <<std::endl;
#endif

    RegisterMeshes(md);
    RegisterMdMeshes(md);

    RegisterVarsWithMesh(md);

    RegisterVars(md);
    RegisterMdVars(md);

    RegisterExpressions(md);

    //add desperation last-ditch mesh if none exist in metadata
    if (md->GetNumMeshes() == 0) {
      debugStrmRef <<methodSig <<"Warning: " << dataFileName << " contains no mesh "
      "information. Creating default mesh." << endl;
      avtMeshMetaData* mmd = new avtMeshMetaData("mesh", 1, 1, 1, 0, 3, 3,
          AVT_RECTILINEAR_MESH);
      setAxisLabels(mmd);
      md->Add(mmd);
    }

    debugStrmRef <<methodSig <<"Exiting normally." << endl;
  }

  void avtVsFileFormat::setAxisLabels(avtMeshMetaData* mmd) {
    debugStrmRef <<"avtVsFileFormat::setAxisLabels() - entering." <<std::endl;
    if (mmd == NULL) {
      debugStrmRef <<"avtVsFileFormat::setAxisLabels() - Input pointer was NULL?" <<std::endl;
    } else {
      mmd->xLabel = "x";
      mmd->yLabel = "y";
      mmd->zLabel = "z";
    }
    debugStrmRef <<"avtVsFileFormat::setAxisLabels() - exiting." <<std::endl;
  }
#endif
