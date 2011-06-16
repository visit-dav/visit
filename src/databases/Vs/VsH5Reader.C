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

using namespace std;

int VsH5Reader::numInstances = 0;

VsH5Reader::VsH5Reader(const std::string& nm, VsRegistry* r) {

  string methodSig("VsH5Reader::VsH5Reader() - ");

  numInstances++;
  VsLog::debugLog() << methodSig << "This VsH5Reader is #"
                    << numInstances << endl;

  VsLog::debugLog() << methodSig
                    << "VsH5Reader::VsH5Reader(" << nm << ") entering." << endl;

  if (numInstances > 1) {
    VsLog::warningLog() << methodSig
      << "Warning!  More than one concurrent copy of VsH5Reader." << endl;
    VsLog::warningLog()
      << methodSig << "Warning!  Debug messages may be interleaved." << endl;
  }

  registry = r;
  
  // Read metadata
  fileData = VsFilter::readFile(registry, nm);
  if (!fileData) {
    VsLog::errorLog() << methodSig
                      << "Unable to load metadata from file." << endl;
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
  registry->createComponents();

  // Was this even a vizschema file?
  if ((registry->numMeshes() == 0) &&
      (registry->numVariables() == 0) &&
      (registry->numExpressions() == 0) &&
      (registry->numVariablesWithMesh() == 0) &&
      (registry->numMDMeshes() == 0) &&
      (registry->numMDVariables() == 0)) {
    VsLog::errorLog() << methodSig
                      << "file format not recognized." << endl;
    EXCEPTION1(InvalidFilesException, nm.c_str());
  }
  
  VsLog::debugLog() << methodSig << "exiting." << std::endl;
}


VsH5Reader::~VsH5Reader() {
  string methodSig("VsH5Reader::~VsH5Reader() - ");

  VsLog::debugLog() << methodSig << "Entering." << std::endl;
  
  if (fileData) {
    delete (fileData);
    fileData = NULL;
  }
  
  VsLog::debugLog() << methodSig << "Exiting." << endl;
}


herr_t VsH5Reader::getDataSet( VsH5Dataset* dataSet,
                               void* data,
                               std::string indexOrder,
                               int components,
                               int* srcMins,
                               int* srcCounts,
                               int* srcStrides,
                               int  mdims,
                               int* destSizes,
                               int* destMins,
                               int* destCounts,
                               int* destStrides ) const
{
  // components = -2 No component array (i.e. scalar variable)
  // components = -1 Component array present read all values.
  //                 (i.e. vector variable or point coordinates)
  // component >= 0  Component array present read single value at that index.

  string methodSig("VsH5Reader::getVariable() - ");

  if (dataSet == NULL) {
    VsLog::debugLog() << methodSig << "Requested dataset is null?" << endl;
    return -1;
  }
  
  VsLog::debugLog() << methodSig << dataSet->getFullName() << " - Entering."
                    << std::endl;
  
  herr_t err = 0;

  // No subset info so read all data.
  if (indexOrder.length() == 0)
  {
    err = H5Dread(dataSet->getId(), dataSet->getType(),
                  H5S_ALL, H5S_ALL, H5P_DEFAULT, data);

    if (err < 0) {
      VsLog::debugLog() << methodSig << "Error " << err
                        << " in reading variable " << dataSet->getFullName()
                        << "." << std::endl;
      return err;
    }
  }

  // mins, counts, and/or strides so hyperslab.
  else
  {
    hid_t dataspace = H5Dget_space(dataSet->getId());

    int ndims = H5Sget_simple_extent_ndims(dataspace);

    if (ndims == 0) {
      VsLog::debugLog() << methodSig
                        << "Unable to load dimensions for variable."
                        << "Returning -1." << endl;
      return -1;
    }
    
    std::vector<hsize_t> dims(ndims);

    int ndim = H5Sget_simple_extent_dims(dataspace, &(dims[0]), NULL);

    if( ndim != ndims ) {
      VsLog::debugLog() << methodSig
                        << "Data dimensions not match. " << endl;
      return -1;
    }

    VsLog::debugLog() << methodSig << "about to set up arguments." << endl;

    std::vector<hsize_t> count(ndims);
    std::vector<hsize_t> start(ndims);
    std::vector<hsize_t> stride(ndims);

    // FORTRAN ordering.
    if((indexOrder == VsSchema::compMinorFKey) ||
       (indexOrder == VsSchema::compMajorFKey))
    {
      // No components - [iz][iy][ix]
      if( components == -2 )
      {
        for (unsigned int i = 0; i<ndims; i++)
        {
          // Flip the ordering of the input.
          int base = ndims - 1;

          start[1] = (hsize_t) srcMins[base-i];
          count[1] = (hsize_t) srcCounts[base-i];
          stride[1] = (hsize_t) srcStrides[base-i];
          
          VsLog::debugLog() << methodSig << "For i = " << i
                            << ", start = " << start[i]
                            << ", count = " << count[i]
                            << ", stride = " << stride[i]
                            << std::endl;
        }
      }
      else if(indexOrder == VsSchema::compMajorFKey)
      {
        // Multiple components - [ic][iz][iy][ix] compMajorF
        for (unsigned int i = 0; i<ndims-1; i++)
        {
          // Flip the ordering of the input.
          int base = ndims - 1;

          start[i+1] = (hsize_t) srcMins[base-i];
          count[i+1] = (hsize_t) srcCounts[base-i];
          stride[i+1] = (hsize_t) srcStrides[base-i];
          
          VsLog::debugLog() << methodSig << "For i = " << i
                            << ", start = " << start[i+1]
                            << ", count = " << count[i+1]
                            << ", stride = " << stride[i+1]
                            << std::endl;
        }

        // Components present but read all
        if( components < 0 )
        {
          start[0] = 0;
          count[0] = dims[ndims-1];
          stride[0] = 1;
        }
        else // Components present but read a single value
        {
          start[0] = components;
          count[0] = 1;
          stride[0] = 1;
        }
      }
      else //if( indexOrder == VsSchema::compMinorFKey )
      {
        // Multiple components - [iz][iy][ix][ic] compMinorF
        for (unsigned int i = 0; i<ndims-1; i++)
        {
          // Flip the ordering of the input.
          int base = ndims - 1;

          start[i] = (hsize_t) srcMins[base-i];
          count[i] = (hsize_t) srcCounts[base-i];
          stride[i] = (hsize_t) srcStrides[base-i];
          
          VsLog::debugLog() << methodSig << "For i = " << i
                            << ", start = " << start[i]
                            << ", count = " << count[i]
                            << ", stride = " << stride[i]
                            << std::endl;
        }

        // Components present but read all
        if( components < 0 )
        {
          start[ndims-1] = 0;
          count[ndims-1] = dims[ndims-1];
          stride[ndims-1] = 1;
        }
        else // Components present but read a single value
        {
          start[ndims-1] = components;
          count[ndims-1] = 1;
          stride[ndims-1] = 1;
        }
      }
    }
    // C ordering.
    else //if((indexOrder == VsSchema::compMinorCKey) ||
         //   (indexOrder == VsSchema::compMajorCKey))
    {
      if( components == -2 )
      {
        // No components - [ix][iy][iz]
        for (unsigned int i = 0; i<ndims; i++)
        {
          start[i] = (hsize_t) srcMins[i];
          count[i] = (hsize_t) srcCounts[i];
          stride[i] = (hsize_t) srcStrides[i];
          
          VsLog::debugLog() << methodSig << "For i = " << i
                            << ", start = " << start[i]
                            << ", count = " << count[i]
                            << ", stride = " << stride[i]
                            << std::endl;
        }
      }

      else if(indexOrder == VsSchema::compMajorCKey)
      {
        // Multiple components - [ic][ix][iy][iz] compMajorC
        for (unsigned int i = 0; i<ndims-1; i++)
        {
          start[i+1] = (hsize_t) srcMins[i];
          count[i+1] = (hsize_t) srcCounts[i];
          stride[i+1] = (hsize_t) srcStrides[i];
          
          VsLog::debugLog() << methodSig << "For i = " << i
                            << ", start = " << start[i+1]
                            << ", count = " << count[i+1]
                            << ", stride = " << stride[i+1]
                            << std::endl;
        }

        // Components present but read all
        if( components < 0 )
        {
          start[0] = 0;
          count[0] = dims[ndims-1];
          stride[0] = 1;
        }
        else // Components present but read a single value
        {
          start[0] = components;
          count[0] = 1;
          stride[0] = 1;
        }
      }

      else if(indexOrder == VsSchema::compMinorCKey)
      {
        // Multiple components - [ix][iy][iz][ic] compMinorC
        for (unsigned int i = 0; i<ndims-1; i++)
        {
          start[i] = (hsize_t) srcMins[i];
          count[i] = (hsize_t) srcCounts[i];
          stride[i] = (hsize_t) srcStrides[i];
          
          VsLog::debugLog() << methodSig << "For i = " << i
                            << ", start = " << start[i]
                            << ", count = " << count[i]
                            << ", stride = " << stride[i]
                            << std::endl;
        }

        // Components present but read all
        if( components < 0 )
        {
          start[ndims-1] = 0;
          count[ndims-1] = dims[ndims-1];
          stride[ndims-1] = 1;
        }
        else // Components present but read a single value
        {
          start[ndims-1] = components;
          count[ndims-1] = 1;
          stride[ndims-1] = 1;
        }
      }
    }

    // Select subset of the data
    if( srcMins )
      err = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET,
                                &(start[0]), &(stride[0]), &(count[0]), NULL);

    VsLog::debugLog() << methodSig
                      << "After selecting the hyperslab, err is " <<err << endl;

    // Create memory space for the data
    hid_t memspace;

    std::vector<hsize_t> destSize(ndims);

    std::vector<hsize_t> destCount(ndims);
    std::vector<hsize_t> destStart(ndims);
    std::vector<hsize_t> destStride(ndims);

    if( mdims == 0 )
      memspace = H5Screate_simple(ndims, &(count[0]), NULL);
    else
    {
      for( int i=0; i<mdims; ++i )
      {
        destSize[i] = (hsize_t) destSizes[i];

        destStart[i] = (hsize_t) destMins[i];
        destCount[i] = (hsize_t) destCounts[i];
        destStride[i] = (hsize_t) destStrides[i];
      }

      memspace = H5Screate_simple(mdims, &(destSize[0]), NULL);

      H5Sselect_hyperslab(memspace, H5S_SELECT_SET,
                          &(destStart[0]), &(destStride[0]), &(destCount[0]), NULL);
    }

    // Read data
    err = H5Dread(dataSet->getId(), dataSet->getType(), memspace, dataspace,
                  H5P_DEFAULT, data);

    if (err < 0) {
      VsLog::debugLog() << methodSig << ": error " << err
                        << " in reading dataset." << std::endl;
    }

    err = H5Sclose(memspace);
    err = H5Sclose(dataspace);
  }
  
  VsLog::debugLog() << methodSig << "Returning " << err << "." << std::endl;
  return err;
}



void* VsH5Reader::getVariableComponent(const std::string& name, size_t indx,
                                       size_t partnumber,
                                       size_t numparts, size_t* splitDims)
{
  VsLog::debugLog() << "VsH5Reader::getVariableComponent(" <<name <<", " <<indx
  <<", " <<partnumber <<", " <<numparts <<", splitDims): Entering." << std::endl;
  
  if (partnumber >= numparts) {
    VsLog::debugLog() <<"VsH5Reader::getVariableComponent() - Variable has " <<numparts
    <<"parts but we were asked for part number #" <<partnumber << endl;
    VsLog::debugLog() <<"VsH5Reader::getVariableComponent() - returning NULL." << endl;
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


herr_t VsH5Reader::getVarWithMeshMesh(VsVariableWithMesh& meta, void* data,
                                      size_t partStart, size_t partCount) const
{
  std::string methodSig("VsH5Reader::getVarWithMeshMesh() - ");

  VsLog::debugLog() << methodSig << "Entering." << std::endl;
  
  std::vector<int> dims = meta.getDims();
  size_t rank = dims.size();
  if (rank != 2)
  {
    VsLog::debugLog() << methodSig << "don't know what to do if rank != 2" << std::endl;
    VsLog::debugLog() << methodSig << "Returning 1." << std::endl;
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
  VsLog::debugLog() << methodSig << "start =";
  for (size_t i=0; i<rank; ++i) VsLog::debugLog() << " " << start[i];
  VsLog::debugLog() << std::endl;
  VsLog::debugLog() << methodSig << "count =";
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
    VsLog::debugLog() << methodSig << "read error " << std::endl;
  }
  err = 10 * err + H5Sclose(memspace);
  err = 10 * err + H5Sclose(dataspace);
  VsLog::debugLog() << methodSig << "Returning cumulative error code: " <<err <<"." << std::endl;
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
    VsLog::debugLog() <<"VsH5Reader::getVarWithMeshComponent() - WARNING: failed assertion idx < dims[dims.size()-1]" << endl;
    VsLog::debugLog() <<"dims.size() is " <<dims.size() << endl;
    VsLog::debugLog() <<"dims[dims.size()-1] is " <<dims[dims.size() - 1] << endl;
    VsLog::debugLog() <<"meta->numSpatialDims is " <<meta->getNumSpatialDims() << endl;
    VsLog::debugLog() <<"idx is " <<idx << endl;
    VsLog::debugLog() <<"VsH5Reader::getVarWithMeshComponent() - returning error 1." << endl;
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

#endif
