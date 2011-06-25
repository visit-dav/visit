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

                               // Use these variables for adjusting
                               // the read memory space.
                               int components,   // Index for a component
                               int* srcMins,     // start locations
                               int* srcCounts,   // number of entries
                               int* srcStrides,  // stride in memory

                               // Use these variables for adjusting
                               // the write memory space.
                               int  mdims,               // spatial dims
                               int* destSizes,           // over memory size
                               int* destMins,            // start locations
                               int* destCounts,          // number of entries
                               int* destStrides ) const  // stride in memory
{
  // components = -2 No component array (i.e. scalar variable)
  // components = -1 Component array present but read all values.
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

#endif
