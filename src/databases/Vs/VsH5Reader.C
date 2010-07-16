#include <hdf5.h>
#include <visit-hdf5.h>
#include <InvalidFilesException.h>

#if HDF5_VERSION_GE(1,8,1)
/**
 *
 * @file        VsH5Reader.cpp
 *
 * @brief       Implementation reader of data for VSH5 schema
 *
 * @version $Id: VsH5Reader.C 496 2009-08-05 22:57:22Z mdurant $
 *
 * Copyright &copy; 2007-2008, Tech-X Corporation
 * See LICENSE file for conditions of use.
 *
 */

#include <sstream>
#include <stdexcept>

#include <VsH5Reader.h>
#include <VsSchema.h>
#include <VsUtils.h>
#include "VsH5Dataset.h"
#include "VsH5Attribute.h"
#include "VsMDVariable.h"
#include "VsMDMesh.h"
#include "VsMesh.h"
#include "VsVariable.h"
#include "VsVariableWithMesh.h"
#include "VsRectilinearMesh.h"
#include "VsUnstructuredMesh.h"
#include "VsStructuredMesh.h"
#include "VsUniformMesh.h"
#include "VsLog.h"
#include "VsRegistry.h"

int VsH5Reader::numInstances = 0;

VsH5Reader::VsH5Reader(const std::string& nm, std::vector<int> strideSettings, VsRegistry* r) {
  numInstances++;
  VsLog::debugLog() <<"VsH5Reader::VsH5Reader() - this VsH5Reader is #" <<numInstances <<std::endl;
  VsLog::debugLog() <<"VsH5Reader::VsH5Reader(" <<nm <<") entering." <<std::endl;
  if (numInstances > 1) {
    VsLog::warningLog() <<"VsH5Reader::VsH5Reader() - Warning!  More than one concurrent copy of VsH5Reader." <<std::endl;
    VsLog::warningLog() <<"VsH5Reader::VsH5Reader() - Warning!  Debug messages may be interleaved." <<std::endl;
  }

  registry = r;
  
  VsLog::debugLog() <<"VsH5Reader::VsH5Reader(" <<nm <<") handling stride settings." <<std::endl;
  stride = strideSettings;
  useStride = false;
  for (unsigned int i = 0; i < stride.size(); i++) {
    if (stride[i] != 1)
      useStride = true;
  }

  // Read metadata
  fileData = VsFilter::readFile(registry, nm);
  if (!fileData) {
    VsLog::errorLog() <<"VsH5Reader::VsH5Reader(" <<nm <<") - Unable to load metadata from file." <<std::endl;
    EXCEPTION1(InvalidFilesException, nm.c_str());
  }
  
  // Was this even a vizschema file?
  if ((registry->numGroups() == 0) && (registry->numDatasets() == 0)) {
    VsLog::errorLog() <<"VsH5Reader::VsH5Reader(" <<nm <<") - file format not recognized." <<std::endl;
    EXCEPTION1(InvalidFilesException, nm.c_str());
  }

  //debugging output
  registry->writeAllGroups();
  registry->writeAllDatasets();
  
  //Build "Core" (Mesh and Var) objects
  registry->buildGroupObjects();
  registry->buildDatasetObjects();

  //Build MD objects
  registry->buildMDMeshes();
  registry->buildMDVars();
  
  //debugging output
  registry->writeAllMeshes();
  registry->writeAllExpressions();
  registry->writeAllVariables();
  registry->writeAllVariablesWithMesh();
  registry->writeAllMDMeshes();
  registry->writeAllMDVariables();

  //Do a third pass to create components
  registry->createComponents(useStride, stride);
  
  VsLog::debugLog() <<"VsH5Reader::VsH5Reader(" <<nm <<") exiting." << std::endl;
}

VsH5Reader::~VsH5Reader() {
  VsLog::debugLog() <<"VsH5Reader::~VsH5Reader() entering." << std::endl;
  
  if (fileData) {
    delete (fileData);
    fileData = NULL;
  }
  
  VsLog::debugLog() <<"VsH5Reader::~VsH5Reader() exiting." <<std::endl;
}

herr_t VsH5Reader::getVariable(const std::string& name, void* data) const {
  //std::map<std::string, VsVariableMeta*>::const_iterator it = meta->vars.find(name);
  VsLog::debugLog() <<"VsH5Reader::getVariable(" <<name <<", data) - Entering" <<std::endl;
  VsVariable* var = registry->getVariable(name);
  if (!var) {
    VsLog::debugLog() << "VsH5Reader::getVariable(" <<name <<"): Unable to find variable " <<name << "'." << std::endl;
    VsLog::debugLog() << "VsH5Reader::getVariable(" <<name <<"): Returning -1 (error)." << std::endl;
    return -1;
  }
  
  hid_t id = var->getId();
  herr_t err = 0;
  if (!useStride) {
    err = H5Dread(id, var->getType(), H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
  } else {
    ///TODO:: Improve error handling
    VsLog::debugLog() << "VsH5Reader::getVariable(" <<name <<"): Loading partial data set." << std::endl;
    //do some fancy H5S hyperslab selection stuff here, then load partial data set
    //id is the dataset
    hid_t dataspace = H5Dget_space(id);
    int rank = H5Sget_simple_extent_ndims (dataspace);
    std::vector<hsize_t> dims_out(rank);
    H5Sget_simple_extent_dims (dataspace, &dims_out[0], NULL);
    if (rank == 2) {
      VsLog::debugLog() <<"Dimensions: " <<(unsigned long)(dims_out[0]) <<" x " <<(unsigned long)(dims_out[1]) <<std::endl;
    } else if (rank == 3) {
      VsLog::debugLog() <<"Dimensions: " <<(unsigned long)(dims_out[0]) <<" x " <<(unsigned long)(dims_out[1])
      <<" x " <<(unsigned long)(dims_out[2]) <<std::endl;
    }
    
    //count is how many elements to choose in each direction
    std::vector<hsize_t> count(rank);
    VsMesh* meshMeta = var->getMesh();
    int addBefore = 0;
    int addAfter = 0;
    bool uniformMesh = false;
    bool structuredMesh = false;
    if (meshMeta) {
      if (meshMeta->isUniformMesh())
        uniformMesh = true;
      else if (meshMeta->isStructuredMesh())
        structuredMesh = true;
    } else {
      VsLog::debugLog() <<"VsH5Reader::getVariable(...): " <<"Unable to load mesh metadata for name: " <<var->getMeshName() <<std::endl;
      VsLog::debugLog() <<"VsH5Reader::getVariable(...): " <<"Assuming uniform mesh, but who knows?" <<std::endl;
    }
    
    if (uniformMesh) {
      if (var->isZonal()) {
        VsLog::debugLog() <<"VsH5Reader::getVariable(...): " <<"Zonal on uniform = no change" <<std::endl;
        adjustSize_hsize_t(&count[0], rank, stride, addBefore, addAfter);
      } else {
        //nodal
        VsLog::debugLog() <<"VsH5Reader::getVariable(...): " <<"Nodal on uniform = -1/+1" <<std::endl;
        addBefore = -1;
        addAfter = 1;
        adjustSize_hsize_t(&count[0], rank, stride, addBefore, addAfter);
      }
    } else if (structuredMesh) {
      VsStructuredMesh* structuredMesh = static_cast<VsStructuredMesh*>(meshMeta);
      if (var->isZonal()) {
        VsLog::debugLog() <<"VsH5Reader::getVariable(...): " <<"Zonal on structured = +1/-1" <<std::endl;
        std::vector<int> meshDims;
        structuredMesh->getMeshDims(&meshDims, useStride, stride);
        if (meshDims.empty()) {
          VsLog::debugLog() <<"VsH5Reader::getVariable(...): Unable to load dimensions for mesh." <<std::endl;
          VsLog::debugLog() <<"VsH5Reader::getVariable(...): Returning -1." <<std::endl;
          return -1;
        }
        
        for (int i = 0; i < rank; i++)
        {
          VsLog::debugLog() <<"VsH5Reader::getVariable(...): " <<"About to override size " <<count[i] <<" with size from mesh - 1: " <<(meshDims[i] - 1) <<std::endl;
          count[i] = meshDims[i] - 1;
        }
        
      } else {
        //nodal
        VsLog::debugLog() <<"VsH5Reader::getVariable(...): " <<"Nodal on structured" <<std::endl;
        std::vector<int> meshDims;
        structuredMesh->getMeshDims(&meshDims, useStride, stride);
        if (meshDims.empty()) {
          VsLog::debugLog() <<"VsH5Reader::getVariable(...): Unable to load dimensions for mesh." <<std::endl;
          VsLog::debugLog() <<"VsH5Reader::getVariable(...): Returning -1." <<std::endl;
          return -1;
        }
        
        for (int i = 0; i < rank; i++)
        {
          VsLog::debugLog() <<"VsH5Reader::getVariable(...): " <<"About to override size " <<count[i] <<" with size from mesh: " <<(meshDims[i]) <<std::endl;
          count[i] = meshDims[i];
        }
      }
    }
    
    /*
     * Define hyperslab in the dataset.
     */
    //offset is the starting location
    std::vector<hsize_t> offset(rank);
    //stride is the stride in each direction
    std::vector<hsize_t> strideArray(rank);
    
    VsLog::debugLog() <<"About to select variable hyperslab with size: ";
    for (int i = 0; i < rank; i++) {
      offset[i] = 0;
      strideArray[i] = stride[i];
      VsLog::debugLog() <<count[i] <<" x ";
    }
    VsLog::debugLog() <<std::endl;
    
    herr_t status = H5Sselect_hyperslab (dataspace, H5S_SELECT_SET, &offset[0], &strideArray[0],
                                         &count[0], NULL);
    
    /*
     * Define the memory dataspace.
     */
    std::vector<hsize_t> dimsm(rank);
    for (int i = 0; i < rank; i++) {
      dimsm[i] = count[i];
    }
    hid_t memspace = H5Screate_simple (rank, &dimsm[0], NULL);
    
    /*
     * Define memory hyperslab.
     */
    std::vector<hsize_t> offset_out(rank);
    std::vector<hsize_t> count_out(rank);
    for (int i = 0; i < rank; i++) {
      offset_out[i] = 0;
      count_out[i] = count[i];
    }
    
    status = H5Sselect_hyperslab (memspace, H5S_SELECT_SET, &offset_out[0], NULL,
                                  &count_out[0], NULL);
    
    /*
     * Read data from hyperslab in the file into the hyperslab in
     * memory and display.
     */
    status = H5Dread (id, var->getType(), memspace, dataspace,
                      H5P_DEFAULT, data);
    
    //    H5Dclose (dataset);
    H5Sclose (dataspace);
    H5Sclose (memspace);
  }
  
  if (err != 0) {
    VsLog::debugLog() << "VsH5Reader::getVariable(...): error " << err <<
    " in reading variable '" << name << "'." << std::endl;
  }
  VsLog::debugLog() << "VsH5Reader::getVariable(" <<name <<"): Returning " <<err <<"." << std::endl;
  return err;
}

herr_t VsH5Reader::getVariableComponent(const std::string& name, size_t indx, void* data) {
  VsLog::debugLog() << "VsH5Reader::getVariableComponent(" <<name <<", " <<indx <<"): Entering." << std::endl;
  
  herr_t err = 0;
  VsVariable* meta = registry->getVariable(name);
  if (!meta) {
    VsLog::debugLog() << "VsH5Reader::getVariableComponent(): error: var " << name <<
    " has no metadata." << std::endl;
    VsLog::debugLog() << "VsH5Reader::getVariableComponent(): Returning 1 (error)." << std::endl;
    return 1;
  }
  std::vector<int> dims = meta->getDims();
  ///do we need -1 because we're in a component variable situation here?
  // answer - No, because we need to actually reference the component index
  size_t rank = dims.size();
  VsLog::debugLog() << "VsH5Reader::getVariableComponent(...): " << name <<
  " has rank, " << rank << "." << std::endl;
  
  std::vector<hsize_t> count(rank);
  std::vector<hsize_t> start(rank);
  hid_t dataspace = H5Dget_space(meta->getId());
  
  if (meta->isCompMajor()) {
    for (size_t i = 1; i<rank; ++i) {
      count[i] = dims[i];
      start[i] = 0;
    }
    count[0] = 1;
    start[0] = indx;
    
  }
  else { //compMinor
    for (size_t i = 0; i<(rank-1); ++i) {
      count[i] = dims[i];
      start[i] = 0;
    }
    count[rank-1] = 1;
    start[rank-1] = indx;
  }
  
  //this gets complicated because it depends on the kind of variable (zonal vs nodal)
  // and the kind of the mesh (uniform vs structured)
  // We must also be careful to stay away if the mesh is NOT uniform or structured
  if (useStride) {
    VsMesh* meshMeta = meta->getMesh();
    if (!meshMeta) {
      VsLog::debugLog() <<"ERROR VsH5Reader::getVariableComponent(...): Unable to get mesh metadata for name: " <<meta->getMeshName() <<std::endl;
      VsLog::debugLog() <<"ERROR VsH5Reader::getVariableComponent(...): returning -1" <<std::endl;
      return -1;
    }
    int addBefore = 0;
    int addAfter = 0;
    
    if (meshMeta->isUniformMesh()) {
      if (meta->isZonal()) {
        VsLog::debugLog() <<"VsH5Reader::getVariableComponent(...): " <<"Zonal on uniform = no change" <<std::endl;
        adjustSize_hsize_t(&count[0], rank - 1, stride, addBefore, addAfter);
      } else {
        //nodal
        VsLog::debugLog() <<"VsH5Reader::getVariableComponent(...): " <<"Nodal on uniform = -1/+1" <<std::endl;
        addBefore = -1;
        addAfter = 1;
        adjustSize_hsize_t(&count[0], rank - 1, stride, addBefore, addAfter);
      }
    } else if (meshMeta->isStructuredMesh()) {
      VsStructuredMesh* structuredMesh = static_cast<VsStructuredMesh*>(meshMeta);
      if (meta->isZonal()) {
        VsLog::debugLog() <<"VsH5Reader::getVariableComponent(...): " <<"Zonal on structured = +1/-1" <<std::endl;
        std::vector<int> meshDims;
        structuredMesh->getMeshDims(&meshDims, useStride, stride);
        if (meshDims.empty()) {
          VsLog::debugLog() <<"VsH5Reader::getVariable(...): Unable to load dimensions for mesh." <<std::endl;
          VsLog::debugLog() <<"VsH5Reader::getVariable(...): Returning -1." <<std::endl;
          return -1;
        }
        
        for (unsigned int i = 0; i < rank - 1; i++)
        {
          VsLog::debugLog() <<"VsH5Reader::getVariableComponent(...): " <<"About to override size " <<count[i] <<" with size from mesh - 1: " <<(meshDims[i] - 1) <<std::endl;
          count[i] = meshDims[i] - 1;
        }
        
      } else {
        //nodal
        VsLog::debugLog() <<"VsH5Reader::getVariableComponent(...): " <<"Nodal on structured" <<std::endl;
        std::vector<int> meshDims;
        structuredMesh->getMeshDims(&meshDims, useStride, stride);
        if (meshDims.empty()) {
          VsLog::debugLog() <<"VsH5Reader::getVariable(...): Unable to load dimensions for mesh." <<std::endl;
          VsLog::debugLog() <<"VsH5Reader::getVariable(...): Returning -1." <<std::endl;
          return -1;
        }
        
        for (unsigned int i = 0; i < rank - 1; i++)
        {
          VsLog::debugLog() <<"VsH5Reader::getVariableComponent(...): " <<"About to override size " <<count[i] <<" with size from mesh: " <<(meshDims[i]) <<std::endl;
          count[i] = meshDims[i];
        }
      }
    }
  }
  
  VsLog::debugLog() << "VsH5Reader::getVariableComponent(...): start =";
  for (size_t i=0; i<rank; ++i) VsLog::debugLog() << " " << start[i];
  VsLog::debugLog() << std::endl;
  VsLog::debugLog() << "VsH5Reader::getVariableComponent(...): count =";
  for (size_t i=0; i<rank; ++i) VsLog::debugLog() << " " << count[i];
  VsLog::debugLog() << std::endl;
  
  std::vector<hsize_t> strideArray(rank);
  for (unsigned int i = 0; i < rank; i++) {
    if (i < stride.size())
      strideArray[i] = (hsize_t)stride[i];
    else strideArray[i] = 1;
  }
  
  // Select data
  err = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, &start[0], &strideArray[0],
                            &count[0], NULL);
  // Create memory space for the data
  hid_t memspace = H5Screate_simple(rank, &count[0], NULL);
  // Read data
  err = H5Dread(meta->getId(), meta->getType(), memspace, dataspace,
                H5P_DEFAULT, data);
  if (err < 0) {
    VsLog::debugLog() << "VsH5Reader::getVariableComponent(...): error " << err <<
    " in reading variable '" << name << "'." << std::endl;
  }
  err = H5Sclose(memspace);
  err = H5Sclose(dataspace);
  VsLog::debugLog() << "VsH5Reader::getVariableComponent(): Returning " <<err <<"." << std::endl;
  return err;
}

void* VsH5Reader::getVariableComponent(const std::string& name, size_t indx,
                                       size_t partnumber, size_t numparts, size_t* splitDims)
{
  VsLog::debugLog() << "VsH5Reader::getVariableComponent(" <<name <<", " <<indx
  <<", " <<partnumber <<", " <<numparts <<", splitDims): Entering." << std::endl;
  
  if (partnumber >= numparts) {
    VsLog::debugLog() <<"VsH5Reader::getVariableComponent() - Variable has " <<numparts
    <<"parts but we were asked for part number #" <<partnumber <<std::endl;
    VsLog::debugLog() <<"VsH5Reader::getVariableComponent() - returning NULL." <<std::endl;
    return NULL;
  }
  
  VsVariable* meta = registry->getVariable(name);
  if (!meta)
  {
    VsLog::debugLog() << "VsH5Reader::getVariableComponent(): error: var " <<
    name << " has no metadata." << std::endl;
    VsLog::debugLog() << "VsH5Reader::getVariableComponent(): Returning 0." << std::endl;
    return 0;
  }
  
  std::vector<int> dims = meta->getDims();
  size_t rank = dims.size();
  VsLog::debugLog() << "VsH5Reader::getVariableComponent(...): " << name <<
  " has rank " << rank << "." << std::endl;
  
  std::vector<hsize_t> count(rank);
  std::vector<hsize_t> start(rank);
  hid_t dataspace = H5Dget_space(meta->getId());
  
  if (meta->isCompMajor())
  {
    for (size_t i = 1; i<rank; ++i)
    {
      count[i] = dims[i];
      start[i] = 0;
    }
    count[0] = 1;
    start[0] = indx;
    
  }
  else
  { //compMinor
    for (size_t i = 0; i<(rank-1); ++i)
    {
      count[i] = dims[i];
      start[i] = 0;
    }
    count[rank-1] = 1;
    start[rank-1] = indx;
  }
  
  // Diagnostic output
  VsLog::debugLog() << "VsH5Reader::getVariableComponent() start =";
  for (size_t i=0; i<rank; ++i) VsLog::debugLog() << " " << start[i];
  VsLog::debugLog() << std::endl;
  VsLog::debugLog() << "VsH5Reader::getVariableComponent() count =";
  for (size_t i=0; i<rank; ++i) VsLog::debugLog() << " " << count[i];
  VsLog::debugLog() << std::endl;
  
  if (numparts > 1)
  {
    // Determine along which axis to split
    // NOTE: We assume that all spatial extents are larger than 1 so that we will
    // never split along the "component direction"
    size_t splitAxis = 0;
    size_t largestCount = count[splitAxis];
    
    for (size_t currAxis = 1; currAxis < rank; ++currAxis)
    {
      if (count[currAxis] > largestCount)
      {
        splitAxis = currAxis;
        largestCount = count[currAxis];
      }
    }
    
    // Split along axis
    size_t numCellsAlongSplitAxis = count[splitAxis] - 1;
    if (numCellsAlongSplitAxis)
    {
      size_t numCellsPerPart = numCellsAlongSplitAxis / numparts;
      size_t numPartsWithAdditionalCell = numCellsAlongSplitAxis % numparts;
      
      if (partnumber < numPartsWithAdditionalCell)
      {
        start[splitAxis] = partnumber * (numCellsPerPart + 1);
        count[splitAxis] = (numCellsPerPart + 1) + 1;
      }
      else
      {
        start[splitAxis] = numPartsWithAdditionalCell * (numCellsPerPart + 1) +
        (partnumber - numPartsWithAdditionalCell) * numCellsPerPart;
        count[splitAxis] = numCellsPerPart + 1;
      }
    }
  }
  
  if (splitDims)
  {
    if (meta->isCompMajor())
    {
      for (size_t i = 0; i<(rank-1); ++i)
      {
        splitDims[i] = count[i+1];
      }
    }
    else
    { //compMinor
      for (size_t i = 0; i<(rank-1); ++i)
      {
        splitDims[i] = count[i];
      }
    }
  }
  
  // Select data
  herr_t err = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, &start[0], NULL,
                                   &count[0], NULL);
  if (err < 0)
  {
    VsLog::debugLog() << "VsH5Reader::getVariableComponent(): error " << err <<
    " selecting hyperslab for variable '" << name << "'." << std::endl;
    VsLog::debugLog() << "VsH5Reader::getVariableComponent(): Returning 0." << std::endl;
    return 0;
  }
  
  // Create memory space for the data
  hid_t memspace = H5Screate_simple(rank, &count[0], NULL);
  
  // Allocate memory
  hid_t type = meta->getType();
  int numElems = 1;
  for (size_t i = 0; i < rank; ++i) numElems *= count[i];
  void* data = new unsigned char[numElems*H5Tget_size(type)];
  
  // Read data
  err = H5Dread(meta->getId(), type, memspace, dataspace,
                H5P_DEFAULT, data);
  if (err < 0)
  {
    VsLog::debugLog() << "VsH5Reader::getVariableComponent(...): error " << err <<
    " in reading variable '" << name << "'." << std::endl;
    delete [] static_cast<unsigned char*>(data);
    VsLog::debugLog() << "VsH5Reader::getVariableComponent(): Returning 0." << std::endl;
    return 0;
  }
  
  // Cleanup
  H5Sclose(memspace);
  H5Sclose(dataspace);
  
  // Return
  VsLog::debugLog() << "VsH5Reader::getVariableComponent(): Returning data." << std::endl;
  return data;
}

herr_t VsH5Reader::getDatasetMeshComponent(VsH5Dataset* dataSet,
                                           VsMesh& mmeta, void* data) const {
  if (dataSet == NULL) {
    VsLog::debugLog() << "VsH5Reader::getDatasetMeshComponent() - Requested dataset is null?" <<std::endl;
    return -1;
  }
  
  VsLog::debugLog() << "VsH5Reader::getDatasetMeshComponent(" <<dataSet->getFullName() <<", " <<mmeta.getFullName() <<", data) - Entering." << std::endl;
  
  herr_t err;
  if (!useStride) {
    err = H5Dread(dataSet->getId(), dataSet->getType(), H5S_ALL, H5S_ALL,
                  H5P_DEFAULT, data);
    if (err < 0) {
      VsLog::debugLog() << "VsH5Reader::getDatasetMeshComponent(...): error " << err <<
      " in reading mesh component." << std::endl;
      return err;
    }
    
    //debug
    /*
     VsLog::debugLog() <<"Dumping data: " <<std::endl;
     for (int i = 0; i < 10; i++) {
     if (H5Tequal(dataSet->type, H5T_NATIVE_DOUBLE)) {
     VsLog::debugLog() <<"data[" <<i <<"] = " <<((double*)data)[i] <<std::endl;}
     else if (H5Tequal(dataSet->type, H5T_NATIVE_FLOAT)) {
     VsLog::debugLog() <<"data[" <<i <<"] = " <<((float*)data)[i] <<std::endl;}
     else if (H5Tequal(dataSet->type, H5T_NATIVE_INT)) {
     VsLog::debugLog() <<"data[" <<i <<"] = " <<((int*)data)[i] <<std::endl;}
     }
     VsLog::debugLog() <<"Finished dumping data. " <<std::endl;
     */
    //end debug
    
  } else {
    std::vector<int> dims;
    mmeta.getMeshDims(&dims, useStride, stride);
    hsize_t rank = dims.size();
    if (rank == 0) {
      VsLog::debugLog() <<"VsH5Reader::getDatasetMeshComponent(...): Unable to load dimensions for mesh." <<std::endl;
      VsLog::debugLog() <<"VsH5Reader::getDatasetMeshComponent(...): Returning -1." <<std::endl;
      return -1;
    }
    
    VsLog::debugLog() <<"VsH5Reader::getDatasetMeshComponent() - about to set up arguments." <<std::endl;
    std::vector<hsize_t> count(rank);
    std::vector<hsize_t> start(rank);
    std::vector<hsize_t> strideArray(rank);
    for (unsigned int i = 0; i < rank; i++) {
      if (i < rank - 1)
        strideArray[i] = (hsize_t)stride[i];
      else strideArray[i] = 1;
      
      start[i] = 0;
      count[i] = dims[i];
      
      VsLog::debugLog() <<"For i = " <<i <<", start = " <<start[i] <<", count = " <<count[i] <<", and strideArray = " <<strideArray[i] <<std::endl;
    }
    hid_t dataspace = H5Dget_space(dataSet->getId());
    
    // Select data
    err = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, &start[0], &strideArray[0],
                              &count[0], NULL);
    VsLog::debugLog() <<"After selecting the hyperslab, err is " <<err <<std::endl;
    // Create memory space for the data
    hid_t memspace = H5Screate_simple(rank, &count[0], NULL);
    // Read data
    err = H5Dread(dataSet->getId(), dataSet->getType(), memspace, dataspace,
                  H5P_DEFAULT, data);
    if (err < 0) {
      VsLog::debugLog() << "VsH5Reader::getDatasetMeshComponent(...): error " << err <<
      " in reading dataset." << std::endl;
    }
    err = H5Sclose(memspace);
    err = H5Sclose(dataspace);
  }
  
  VsLog::debugLog() << "VsH5Reader::getDatasetMeshComponent() - Returning " <<err <<"." << std::endl;
  return err;
}

herr_t VsH5Reader::getSplitMeshData(VsUnstructuredMesh& mmeta, void* data) const {
  VsLog::debugLog() << "VsH5Reader::getSplitMeshData() - Entering." << std::endl;
  
  VsH5Dataset* dataset0 = mmeta.getPointsDataset(0);
  if (!dataset0) {
    VsLog::debugLog() << "VsH5Reader::getSplitMeshData(): Unable to get dataset for points 0." <<std::endl;
    VsLog::debugLog() << "VsH5Reader::getSplitMeshData(): - Returning -1." << std::endl;
    return -1;
  }
  
  herr_t err;
  if (!useStride) {
    std::vector<int> dims = dataset0->getDims();
    int numPoints;
    //all data gets loaded as though it has 3 dimensions
    //if only 2-d, the third dimension is left blank
    int dimensions = 3;
    if (mmeta.isCompMajor()) {
      numPoints = dims[1];
    } else {
      numPoints = dims[0];
    }
    
    VsLog::debugLog() << "VsH5Reader::getSplitMeshData() - getting points0 dataset" << std::endl;
  
    // Create memory space for the data
    hsize_t memSpaceSize[1];
    memSpaceSize[0] = dimensions * numPoints;
    hid_t memspace = H5Screate_simple(1, memSpaceSize, NULL);
      
    //select first hyperslab in memory space
    hsize_t stride[1];
    stride[0] = dimensions;
    hsize_t start[1];
    start[0] = 0;
    hsize_t count[1];
    count[0] = numPoints;
    H5Sselect_hyperslab(memspace, H5S_SELECT_SET, start, stride, count, NULL);
      
    //read entire points0 into hyperslab
    err = H5Dread(dataset0->getId(), dataset0->getType(), memspace, H5S_ALL, H5P_DEFAULT, data);
    if (err < 0) {
      VsLog::debugLog() << "VsH5Reader::getSplitMeshData(...): error " << err <<
      " in reading mesh component vsPoints0." << std::endl;
      return err;
    }

    VsLog::debugLog() << "VsH5Reader::getSplitMeshData() - getting points1 dataset" << std::endl;
    //get second dataset
    VsH5Dataset* dataset1 = mmeta.getPointsDataset(1);
    if (!dataset1) {
      VsLog::debugLog() << "VsH5Reader::getSplitMeshData(...): "
      << "error getting mesh component 'points1'." << std::endl;
      VsLog::debugLog() << "VsH5Reader::getSplitMeshData() - Returning -1." << std::endl;
      return -1;
    }
    
    //select second hyperslab in memory
    start[0] = 1;
    H5Sselect_hyperslab(memspace, H5S_SELECT_SET, start, stride, count, NULL);
    
    //read entire points1 into hyperslab
    err = H5Dread(dataset1->getId(), dataset1->getType(), memspace, H5S_ALL, H5P_DEFAULT, data);
    if (err < 0) {
      VsLog::debugLog() << "VsH5Reader::getSplitMeshData(...): error " << err <<
      " in reading mesh component vsPoints1." << std::endl;
      return err;
    }
    
    VsLog::debugLog() << "VsH5Reader::getSplitMeshData() - getting points2 dataset" << std::endl;
    //get third dataset (if it exists)
    VsH5Dataset* dataset2 = mmeta.getPointsDataset(2);
    if (!dataset2) {
      VsLog::debugLog() << "VsH5Reader::getSplitMeshData() - didn't find dataset2, zeroing points and returning" << std::endl;

      VsLog::debugLog() << "VsH5Reader::getSplitMeshData() - closing dataspace" <<std::endl;
      err = H5Sclose(memspace);
      VsLog::debugLog() << "VsH5Reader::getSplitMeshData() - returning data." <<std::endl;
      return 1;
      /* This code has apparently not been tested
       * since most split-point datasets have been 3d until now
       * But needs to be fixed up for use with Scott Kruger's "Ray" files
 
      // Check for vertex list type
      hid_t type1 = dataset2->getType();
      
      //zero the blank values
      if (H5Tequal(type1, H5T_NATIVE_FLOAT)) {
        for (int i = 0; i < numPoints; i++) {
         ((float*)&data)[i * 3 + 2] = 0;
        }       
      } else if (H5Tequal(type1, H5T_NATIVE_DOUBLE)) {
        for (int i = 0; i < numPoints; i++) {
         ((double*)&data)[i * 3 + 2] = 0;
        }
      }
      VsLog::debugLog() << "VsH5Reader::getSplitMeshData() - closing dataspace" <<std::endl;
      err = H5Sclose(memspace);
      VsLog::debugLog() << "VsH5Reader::getSplitMeshData() - returning data." <<std::endl;
      return err;
      */
    }
    
    VsLog::debugLog() << "VsH5Reader::getSplitMeshData() - found points2 dataset" << std::endl;
        
    //select third hyperslab in memory
    start[0] = 2;
    H5Sselect_hyperslab(memspace, H5S_SELECT_SET, start, stride, count, NULL);
    
    //read entire points2 into hyperslab
    err = H5Dread(dataset2->getId(), dataset2->getType(), memspace, H5S_ALL, H5P_DEFAULT, data);
    if (err < 0) {
      VsLog::debugLog() << "VsH5Reader::getSplitMeshData(...): error " << err <<
      " in reading mesh component vsPoints1." << std::endl;
      return err;
    }
    
    err = H5Sclose(memspace);
    
    VsLog::debugLog() <<"VsH5Reader::getSplitMeshData() - returning data." <<std::endl;
    return err;
  } else {
    VsLog::debugLog() <<"VsH5Reader::getSplitMeshData() - ERROR - not implemented for useStride" <<std::endl;
    return -1;
  }
  
  VsLog::debugLog() << "VsH5Reader::getSplitMeshData() - Returning " <<err <<"." << std::endl;
  return err;
}
/*
herr_t VsH5Reader::getAttMeshComponent(const std::string& name,
                                       VsMesh& mmeta, void* data) const {
  VsLog::debugLog() << "VsH5Reader::getAttMeshComponent(" <<name <<", mmeta, data) - Entering." << std::endl;
  VsH5Attribute* attribute = mmeta.getAttribute(name);
  if (attribute == NULL) {
    VsLog::debugLog() << "VsH5Reader::getAttMeshComponent(" <<name <<", mmeta, data) - Unable to find attribute, returning -1." << std::endl;
    return -1;
  }
  
  herr_t err = H5Aread(attribute->getId(), attribute->getType(), data);
  if (err < 0) {
    VsLog::debugLog() << "VsH5Reader::getAttMeshComponent(...): error " << err <<
    " in reading attribute '" << name << "'." << std::endl;
  }
  VsLog::debugLog() << "VsH5Reader::getDatasetMeshComponent(" <<name <<", mmeta, data) - Returning " <<err <<"." << std::endl;
  return err;
}*/

herr_t VsH5Reader::getVarWithMeshMesh(VsVariableWithMesh& meta, void* data) const {
  VsLog::debugLog() << "VsH5Reader::getMeshVarWithMesh(meta, data)" << std::endl;
  
  //Since we allow the spatial indices to be specified individually,
  // we load the position data in slices
  // old style: start at 0 and read "numSpatialDims" columns
  // new style: read a total of "numSpatialDims" columns, one at a time
  herr_t err = 0;
  std::vector<int> dims = meta.getDims();
  size_t rank = dims.size();
  //rank should always be 2 for VarWithMesh data
  if (rank != 2) {
    VsLog::debugLog() <<"VsH5Reader::getMeshVarWithMesh(meta, data) - Rank must be 2 but is " <<rank <<"." <<std::endl;
    VsLog::debugLog() <<"VsH5Reader::getMeshVarWithMesh(meta, data) - unable to load data." <<std::endl;
    return -1;
  }
  
  std::vector<hsize_t> memCount(rank); //tracks the size of the memory block we need to load the data
  std::vector<hsize_t> start(rank); //start position of each slice
  std::vector<hsize_t> sliceCount(rank); //tracks the size of each slice
  std::vector<hsize_t> strideCount(rank);
  hid_t dataspace = H5Dget_space(meta.getId());
  int numPoints = 0;
  if (meta.isCompMajor())
    numPoints = dims[rank-1];
  else numPoints = dims[0];
  int dimensions = meta.getNumSpatialDims();
  
  //adjust points for stride
  if (stride[0] != 1) {
    VsLog::debugLog() <<"VsH5Reader::getMeshVarWithMesh(meta, data) -" <<"Filtering points based on stride.  Before = " <<numPoints <<std::endl;
    numPoints = numPoints / stride[0];
    VsLog::debugLog() <<"VsH5Reader::getMeshVarWithMesh(meta, data) -" <<"Filtering points based on stride.  After = " <<numPoints <<std::endl;
  }
  
  //The index to change depends on whether we are compMinor or compMajor
  //we use compMajor as default (index = 0)
  size_t indexToChange = 0;
  
  if (meta.isCompMajor()) {
    for (size_t i = 1; i<rank; ++i) {
      memCount[i] = numPoints;
      sliceCount[i] = numPoints;
      start[i] = 0;
    }
    indexToChange = 0;
    memCount[0] = meta.getNumSpatialDims();
    sliceCount[0] = 1; //the slice takes a single spatial dimension at a time
    start[0] = 0;
    strideCount[0] = 1;
    strideCount[1] = stride[0];
  }
  else { //compMinor
    for (size_t i = 0; i<rank-1; ++i) {
      memCount[i] = numPoints;
      sliceCount[i] = numPoints;
      start[i] = 0;
    }
    indexToChange = rank-1;
    memCount[rank-1] = meta.getNumSpatialDims();
    sliceCount[rank-1] = 1; //the slice takes a single spatial dimension at a time
    start[rank-1] = 0;
    strideCount[0] = stride[0];
    strideCount[1] = 1;
  }
  VsLog::debugLog() << "VsH5Reader::getMeshVarWithMesh() - start =";
  for (size_t i=0; i<rank; ++i) VsLog::debugLog() << " " << start[i];
  VsLog::debugLog() << std::endl;
  VsLog::debugLog() << "VsH5Reader::getMeshVarWithMesh() - memCount =";
  for (size_t i=0; i<rank; ++i) VsLog::debugLog() << " " << memCount[i];
  VsLog::debugLog() << std::endl;
  VsLog::debugLog() << "VsH5Reader::getMeshVarWithMesh() - sliceCount =";
  for (size_t i=0; i<rank; ++i) VsLog::debugLog() << " " << sliceCount[i];
  VsLog::debugLog() << std::endl;
  VsLog::debugLog() << "VsH5Reader::getMeshVarWithMesh() - stride =";
  for (size_t i=0; i<rank; ++i) VsLog::debugLog() << " " << stride[i];
  VsLog::debugLog() << std::endl;
  
  herr_t lastError = 0;
  
  // Select data
  // Note that we select each spatial index separately
  H5Sselect_none(dataspace);
  hsize_t numElements = H5Sget_select_npoints(dataspace);
  VsLog::debugLog() <<" VsH5Reader::getMeshVarWithMesh(): - we have selected this many elements: " <<numElements <<std::endl;
  
  // Create memory space for the data
  VsLog::debugLog() << "VsH5Reader::getMeshVarWithMesh(): making call to H5Screate_simple: " <<std::endl;
  hsize_t memSpaceSize[1];
  memSpaceSize[0] = dimensions * numPoints;
  hid_t memspace = H5Screate_simple(1, memSpaceSize, NULL);
  
  for (unsigned int d = 0; d < meta.getNumSpatialDims(); d++) {
    start[indexToChange] = meta.getSpatialDim(d);
    VsLog::debugLog() <<std::endl <<"Spatial Dim #" <<d <<std::endl;
    VsLog::debugLog() << "VsH5Reader::getMeshVarWithMesh() - start =";
    for (size_t i=0; i<rank; ++i) VsLog::debugLog() << " " << start[i];
    VsLog::debugLog() << std::endl;
    VsLog::debugLog() << "VsH5Reader::getMeshVarWithMesh() - sliceCount =";
    for (size_t i=0; i<rank; ++i) VsLog::debugLog() << " " << sliceCount[i];
    VsLog::debugLog() << std::endl;
    
    err = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, &start[0], &strideCount[0], &sliceCount[0], NULL);
    hsize_t numElements = H5Sget_select_npoints(dataspace);
    VsLog::debugLog() <<" VsH5Reader::getMeshVerWithMesh(): - we have selected this many elements in dataset: " <<numElements <<std::endl;
    if (err < 0) {
      VsLog::debugLog() << "VsH5Reader::getMeshVarWithMesh(): error in H5select_hyperslab: " <<err << std::endl;
      lastError = err;
    }
    
    //NOTE: This routine will work perfectly for C-order data
    // but is not tested and probably will not work for Fortran-order data
    //select the destination hyperslab in memory
    hsize_t memStart[1];
    hsize_t memStride[1];
    hsize_t memSliceCount[1];
    // We use stride to spread the data out into the correct places
    // starting location is all zeros, but we bump the location by 1 for each dimension
    // Thus first pass starts at 0 and we get x 0 0 x 0 0 x 0 0 (for dimensionality == 3)
    // Second pass starts at 1 and we get x y 0 x y 0 x y 0 (for dimensionality == 3)
    // Third pass starts at 2 and we get x y z x y z x y z (for dimensionality == 3)
    // Note that we're treating the memory block as a single long array
    // instead of as a multi-dimensional matrix
    memStride[0] = dimensions;
    memStart[0] = d;
    memSliceCount[0] = numPoints;
    
    VsLog::debugLog() <<"memStride[0] = " <<memStride[0] <<std::endl;
    VsLog::debugLog() <<"memSliceCount[0] = " <<memSliceCount[0] <<std::endl;
    VsLog::debugLog() <<"memStart[0] = " <<memStart[0] <<std::endl;
    
    H5Sselect_hyperslab(memspace, H5S_SELECT_SET, memStart, memStride, memSliceCount, NULL);
    numElements = H5Sget_select_npoints(memspace);
    VsLog::debugLog() <<" VsH5Reader::getMeshVerWithMesh(): - we have selected this many elements in memory: " <<numElements <<std::endl;
    
    // Read data
    VsLog::debugLog() << "VsH5Reader::getMeshVarWithMesh(): making call to H5Dread: " <<std::endl;
    err = H5Dread(meta.getId(), meta.getType(), memspace, dataspace,
                  H5P_DEFAULT, data);
    if (err < 0) {
      VsLog::debugLog() << "VsH5Reader::getMeshVarWithMesh(): error in H5Dread: " <<err << std::endl;
      lastError = err;
    }
    
  }
  
  // Read data
  VsLog::debugLog() << "VsH5Reader::getMeshVarWithMesh(): making call to H5Dread: " <<std::endl;
  err = H5Dread(meta.getId(), meta.getType(), memspace, dataspace,
                H5P_DEFAULT, data);
  if (err < 0) {
    VsLog::debugLog() << "VsH5Reader::getMeshVarWithMesh(): error in H5Dread: " <<err << std::endl;
    lastError = err;
  }
  
  VsLog::debugLog() << "VsH5Reader::getMeshVarWithMesh(): making call to H5Sclose: " <<std::endl;
  err = H5Sclose(memspace);
  if (err < 0) {
    VsLog::debugLog() << "VsH5Reader::getMeshVarWithMesh(): error in H5Sclose(memspace): " <<err << std::endl;
    lastError = err;
  }
  
  err = H5Sclose(dataspace);
  if (err < 0) {
    VsLog::debugLog() << "VsH5Reader::getMeshVarWithMesh(): error in H5Sclose(dataspace): " <<err << std::endl;
    lastError = err;
  }
  
  VsLog::debugLog() << "VsH5Reader::getMeshVarWithMesh() - Returning " <<lastError <<"." << std::endl;
  return lastError;
}

herr_t VsH5Reader::getVarWithMeshMesh(VsVariableWithMesh& meta,
                                      void* data, size_t partStart, size_t partCount) const {
  VsLog::debugLog() << "VsH5Reader::getMeshVarWithMesh()" << std::endl;
  
  std::vector<int> dims = meta.getDims();
  size_t rank = dims.size();
  if (rank != 2)
  {
    VsLog::debugLog() << "VsH5Reader::getMeshVarWithMesh(): don't know what to do if rank != 2" << std::endl;
    VsLog::debugLog() << "VsH5Reader::getMeshVarWithMesh(): Returning 1." << std::endl;
    return 1;
  }
  hsize_t count[2];
  hsize_t start[2];
  hid_t dataspace = H5Dget_space(meta.getId());
  
  if (meta.isCompMajor()) {
    count[1] = partCount;
    start[1] = partStart;
    count[0] = meta.getNumSpatialDims();
    start[0] = 0;
  }
  else {
    count[0] = partCount;
    start[0] = partStart;
    count[1] = meta.getNumSpatialDims();
    start[1] = 0;
  }
  
  //WORK HERE - MARC - to select hyperslabs in correct order for spatialDims
  VsLog::debugLog() << "VsH5Reader::getMeshVarWithMesh() - start =";
  for (size_t i=0; i<rank; ++i) VsLog::debugLog() << " " << start[i];
  VsLog::debugLog() << std::endl;
  VsLog::debugLog() << "VsH5Reader::getMeshVarWithMesh() - count =";
  for (size_t i=0; i<rank; ++i) VsLog::debugLog() << " " << count[i];
  VsLog::debugLog() << std::endl;
  
  // Select data
  herr_t err = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, start, NULL,
                                   count, NULL);
  // Create memory space for the data
  hid_t memspace = H5Screate_simple(rank, count, NULL);
  // Read data
  err = 10 * err + H5Dread(meta.getId(), meta.getType(), memspace, dataspace,
                           H5P_DEFAULT, data);
  if (err < 0) {
    VsLog::debugLog() << "VsH5Reader::getMeshVarWithMesh(): read error " << std::endl;
  }
  err = 10 * err + H5Sclose(memspace);
  err = 10 * err + H5Sclose(dataspace);
  VsLog::debugLog() << "VsH5Reader::getMeshVarWithMesh(): Returning cumulative error code: " <<err <<"." << std::endl;
  return err;
}

herr_t VsH5Reader::getVarWithMeshComponent(const std::string& name, size_t idx,
                                           void* data) const {
  VsLog::debugLog() << "VsH5Reader::getVarWithMeshComponent(" <<name <<", " <<idx
  <<", data) " << std::endl;
  
  VsVariableWithMesh* meta = registry->getVariableWithMesh(name);
  if (!meta) {
    VsLog::debugLog() << "VsH5Reader::getVarWithMeshComponent(): error: " << name <<
    " has no metadata." << std::endl;
    VsLog::debugLog() << "VsH5Reader::getVarWithMeshComponent(): Returning 1." << std::endl;
    return 1;
  }
  std::vector<int> dims = meta->getDims();
  
  size_t rank = dims.size();
  std::vector<hsize_t> count(rank);
  std::vector<hsize_t> start(rank);
  std::vector<hsize_t> strideCount(rank);
  hid_t dataspace = H5Dget_space(meta->getId());
  
  if (useStride) {
    if (meta->isCompMajor()) {
      dims[1] = dims[1] / stride[0];
      strideCount[0] = 1;
      strideCount[1] = stride[0];
    } else {
      dims[0] = dims[0] / stride[0];
      strideCount[0] = stride[0];
      strideCount[1] = 1;
    }
  } else {
    strideCount[0] = 1;
    strideCount[1] = 1;
  }
  
  //assert(dims[dims.size()-1] >= (int)(idx));
  if ((int)idx > dims[dims.size()-1]) {
    VsLog::debugLog() <<"VsH5Reader::getVarWithMeshComponent() - WARNING: failed assertion idx < dims[dims.size()-1]" <<std::endl;
    VsLog::debugLog() <<"dims.size() is " <<dims.size() <<std::endl;
    VsLog::debugLog() <<"dims[dims.size()-1] is " <<dims[dims.size() - 1] <<std::endl;
    VsLog::debugLog() <<"meta->numSpatialDims is " <<meta->getNumSpatialDims() <<std::endl;
    VsLog::debugLog() <<"idx is " <<idx <<std::endl;
    std::string msg = "VsH5Reader::VsH5Reader: index out of range.";
    VsLog::debugLog() << msg << std::endl;
    VsLog::debugLog() << "VsH5Reader::getVarWithMeshComponent(): Throwing exception." << std::endl;
    throw std::out_of_range(msg.c_str());
    //     VsLog::debugLog() <<"VsH5Reader::getVarWithMeshComponent() - returning NULL" <<std::endl;
    //    return NULL;
  }
  if (meta->isCompMajor()) {
    for (size_t i = 1; i<rank; ++i) {
      count[i] = dims[i];
      start[i] = 0;
    }
    count[0] = 1;
    start[0] = idx;
  }
  else { //compMinor
    for (size_t i = 0; i<(rank-1); ++i) {
      count[i] = dims[i];
      start[i] = 0;
    }
    count[rank-1] = 1;
    start[rank-1] = idx;
  }
  
  VsLog::debugLog() << "start =";
  for (size_t i=0; i<rank; ++i) VsLog::debugLog() << " " << start[i];
  VsLog::debugLog() << std::endl;
  VsLog::debugLog() << "count =";
  for (size_t i=0; i<rank; ++i) VsLog::debugLog() << " " << count[i];
  VsLog::debugLog() << std::endl;
  VsLog::debugLog() << "stride =";
  for (size_t i=0; i<rank; ++i) VsLog::debugLog() << " " << strideCount[i];
  VsLog::debugLog() << std::endl;
  
  // Select data
  herr_t err = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, &start[0], &strideCount[0],
                                   &count[0], NULL);
  // Create memory space for the data
  hid_t memspace = H5Screate_simple(rank, &count[0], NULL);
  // Read data
  err = 10 * err + H5Dread(meta->getId(), meta->getType(), memspace, dataspace,
                           H5P_DEFAULT, data);
  if (err < 0) {
    VsLog::debugLog() << "VsH5Reader::getVarWithMeshComponent(...): error " << err <<
    " reading variable '" << name << "'." << std::endl;
  }
  err = 10 * err + H5Sclose(memspace);
  err = 10 * err + H5Sclose(dataspace);
  VsLog::debugLog() << "VsH5Reader::getVarWithMeshComponent(): Returning cumulative error: " <<err <<"." << std::endl;
  return err;
}

herr_t VsH5Reader::getVarWithMeshComponent(const std::string& name, size_t idx,
                                           void* data, size_t partStart, size_t partCount) const {
  VsLog::debugLog() << "VsH5Reader::getVarWithMeshComponent(" <<name <<", " <<idx <<", data, "
  <<partStart <<", " <<partCount <<") - Entering." << std::endl;
  
  VsVariableWithMesh* meta = registry->getVariableWithMesh(name);
  if (!meta) {
    VsLog::debugLog() << "VsH5Reader::getVarWithMeshComponent(): error: " << name <<
    " has no metadata." << std::endl;
    VsLog::debugLog() << "VsH5Reader::getVarWithMeshComponent(): Returning 1." << std::endl;
    return 1;
  }
  std::vector<int> dims = meta->getDims();
  size_t rank = dims.size();
  if (rank != 2)
  {
    VsLog::debugLog() << "VsH5Reader::getVarWithMeshComponent(): don't know what to do if rank != 2" << std::endl;
    VsLog::debugLog() << "VsH5Reader::getVarWithMeshComponent(): Returning 1." << std::endl;
    return 1;
  }
  hsize_t count[2];
  hsize_t start[2];
  hid_t dataspace = H5Dget_space(meta->getId());
  
  // Assert used to be this:
  // assert(dims[dims.size()-1] >= (int)(meta->numSpatialDims+idx));
  // But we decided that idx can be between 0 and dims[dims.size() - 1]
  // And that we dont' need to take into account the spatial dims
  // Because those are allowable components.
  if ((int)idx > dims[dims.size()-1]) {
    VsLog::debugLog() <<"VsH5Reader::getVarWithMeshComponent() - WARNING: failed assertion idx < dims[dims.size()-1]" <<std::endl;
    VsLog::debugLog() <<"dims.size() is " <<dims.size() <<std::endl;
    VsLog::debugLog() <<"dims[dims.size()-1] is " <<dims[dims.size() - 1] <<std::endl;
    VsLog::debugLog() <<"meta->numSpatialDims is " <<meta->getNumSpatialDims() <<std::endl;
    VsLog::debugLog() <<"idx is " <<idx <<std::endl;
    VsLog::debugLog() <<"VsH5Reader::getVarWithMeshComponent() - returning error 1." <<std::endl;
    return 1;
  }
  
  if (meta->isCompMajor()) {
    count[1] = partCount;
    start[1] = partStart;
    count[0] = 1;
    start[0] = meta->getNumSpatialDims()+idx;
  }
  else { //compMinor
    count[0] = partCount;
    start[0] = partStart;
    count[1] = 1;
    start[1] = meta->getNumSpatialDims()+idx;
  }
  
  VsLog::debugLog() << "VsH5Reader::getVarWithMeshComponent() - start =";
  for (size_t i=0; i<rank; ++i) VsLog::debugLog() << " " << start[i];
  VsLog::debugLog() << std::endl;
  VsLog::debugLog() << "VsH5Reader::getVarWithMeshComponent() - count =";
  for (size_t i=0; i<rank; ++i) VsLog::debugLog() << " " << count[i];
  VsLog::debugLog() << std::endl;
  
  // Select data
  herr_t err = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, start, NULL,
                                   count, NULL);
  // Create memory space for the data
  hid_t memspace = H5Screate_simple(rank, count, NULL);
  // Read data
  err = 10 * err + H5Dread(meta->getId(), meta->getType(), memspace, dataspace,
                           H5P_DEFAULT, data);
  if (err < 0) {
    VsLog::debugLog() << "VsH5Reader::getVarWithMeshComponent(...): error " << err <<
    " reading variable '" << name << "'." << std::endl;
  }
  err = 10 * err + H5Sclose(memspace);
  err = 10 * err + H5Sclose(dataspace);
  VsLog::debugLog() << "VsH5Reader::getVarWithMeshComponent(): Returning cumulative error: " <<err <<"." << std::endl;
  return err;
}

herr_t VsH5Reader::getVariableWithMesh(const std::string& name, void* data) const {
  VsVariableWithMesh* var = registry->getVariableWithMesh(name);
  if (var == 0) {
    VsLog::debugLog() << "VsH5Reader::getVariableWithMesh(...): unable to find "
    << "variable with mesh by name " << name << "'." << std::endl;
    return -1;
  }
  
  herr_t err = H5Dread(var->getId(), var->getType(),
                       H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
  if (err < 0) {
    VsLog::debugLog() << "VsH5Reader::getVariableWithMesh(...): error " << err <<
    " in reading dataset '" << name << "'." << std::endl;
  }
  VsLog::debugLog() << "VsH5Reader::getVariableWithMesh(): Returning " <<err <<"." << std::endl;
  return err;
}

/*
size_t VsH5Reader::getMeshDims(VsMesh* meshMeta, std::vector<int>* dims) const {
  if (!meshMeta) {
    dims->resize(0);
    return 0;
  }
  
  if (meshMeta->isStructuredMesh()) {
    return getMeshDims((VsStructuredMesh*)meshMeta, dims);
  } else if (meshMeta->isUnstructuredMesh()) {
    return getMeshDims((VsUnstructuredMesh*)meshMeta, dims);
  } else if (meshMeta->isUniformMesh()) {
    return getMeshDims((VsUniformMesh*)meshMeta, dims);
  } else if (meshMeta->isRectilinearMesh()) {
    return ((VsRectilinearMesh*)meshMeta)->getMeshDims(dims);
  }
  
  dims->resize(0);
  return 0; 
}
*/


#endif
